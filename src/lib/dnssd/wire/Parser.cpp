/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "Parser.h"
#include "Query.h"

#include <lib/support/CodeUtils.h>
#include <stdio.h>

namespace chip {
namespace Dnssd {

bool QueryData::Parse(const BytesRange & validData, const uint8_t ** start)
{
    // Structure is:
    //    QNAME
    //    TYPE
    //    CLASS (plus a flag for unicast)

    if (!validData.Contains(*start))
    {
        return false;
    }

    const uint8_t * nameEnd = nullptr;
    {
        SerializedQNameIterator it(validData, *start);
        nameEnd = it.FindDataEnd();
    }
    if (nameEnd == nullptr)
    {
        return false;
    }

    if (!validData.Contains(nameEnd + 3))
    {
        return false;
    }

    // TODO: should there be checks for valid mType/class?

    mType = static_cast<QType>(chip::Encoding::BigEndian::Read16(nameEnd));

    uint16_t klass = chip::Encoding::BigEndian::Read16(nameEnd);

    mAnswerViaUnicast = (klass & kQClassUnicastAnswerFlag) != 0;
    mClass            = static_cast<QClass>(klass & ~kQClassUnicastAnswerFlag);
    mNameIterator     = SerializedQNameIterator(validData, *start);

    *start = nameEnd;

    return true;
}

bool QueryData::Append(HeaderRef & hdr, RecordWriter & out) const
{
    if ((hdr.GetAdditionalCount() != 0) || (hdr.GetAnswerCount() != 0) || (hdr.GetAuthorityCount() != 0))
    {
        return false;
    }

    out.WriteQName(GetName())
        .Put16(static_cast<uint16_t>(mType))
        .Put16(static_cast<uint16_t>(static_cast<uint16_t>(mClass) | (mAnswerViaUnicast ? kQClassUnicastAnswerFlag : 0)));

    if (!out.Fit())
    {
        return false;
    }

    hdr.SetQueryCount(static_cast<uint16_t>(hdr.GetQueryCount() + 1));
    return true;
}

bool ResourceData::Parse(const BytesRange & validData, const uint8_t ** start)
{
    // Structure is:
    //    QNAME
    //    TYPE      (16 bit)
    //    CLASS     (16 bit)
    //    TTL       (32 bit)
    //    RDLENGTH  (16 bit)
    //    <DATA>    (RDLENGTH bytes)
    if (!validData.Contains(*start))
    {
        return false;
    }

    const uint8_t * nameEnd = nullptr;

    {
        SerializedQNameIterator it(validData, *start);
        nameEnd = it.FindDataEnd();
    }
    if (nameEnd == nullptr)
    {
        return false;
    }

    // need 3*u16 + u32
    if (!validData.Contains(nameEnd + 9))
    {
        return false;
    }

    mType  = static_cast<QType>(chip::Encoding::BigEndian::Read16(nameEnd));
    mClass = static_cast<QClass>(chip::Encoding::BigEndian::Read16(nameEnd));
    mTtl   = chip::Encoding::BigEndian::Read32(nameEnd);

    uint16_t dataLen = chip::Encoding::BigEndian::Read16(nameEnd); // resource data

    if (!validData.Contains(nameEnd + dataLen - 1))
    {
        return false; // no space for RDATA
    }
    mData = BytesRange(nameEnd, nameEnd + dataLen);

    mNameIterator = SerializedQNameIterator(validData, *start);

    *start = nameEnd + dataLen;

    return true;
}

bool ParseDnsPacket(const BytesRange & packetData, ParserDelegate * delegate)
{
    VerifyOrReturnValue(packetData.Size() >= static_cast<ptrdiff_t>(HeaderRef::kSizeBytes), false);

    // header is used as const, so cast is safe
    ConstHeaderRef header(packetData.Start());

    // Reject packets with unreasonable record counts to prevent CPU exhaustion.
    // for DNS/mDNS packets, 256 records is a generous upper bound for any single section.
    static constexpr uint16_t kMaxRecordCount = 256;

    if (header.GetQueryCount() > kMaxRecordCount || header.GetAnswerCount() > kMaxRecordCount ||
        header.GetAuthorityCount() > kMaxRecordCount || header.GetAdditionalCount() > kMaxRecordCount)
    {
        return false;
    }

    delegate->OnHeader(header);

    const uint8_t * data = packetData.Start() + HeaderRef::kSizeBytes;

    {
        QueryData queryData;
        for (uint16_t i = 0; i < header.GetQueryCount(); i++)
        {
            if (!queryData.Parse(packetData, &data))
            {
                return false;
            }

            delegate->OnQuery(queryData);
        }
    }

    {
        ResourceData resourceData;
        for (uint16_t i = 0; i < header.GetAnswerCount(); i++)
        {
            if (!resourceData.Parse(packetData, &data))
            {
                return false;
            }

            delegate->OnResource(ResourceType::kAnswer, resourceData);
        }

        for (uint16_t i = 0; i < header.GetAuthorityCount(); i++)
        {
            if (!resourceData.Parse(packetData, &data))
            {
                return false;
            }

            delegate->OnResource(ResourceType::kAuthority, resourceData);
        }

        for (uint16_t i = 0; i < header.GetAdditionalCount(); i++)
        {
            if (!resourceData.Parse(packetData, &data))
            {
                return false;
            }

            delegate->OnResource(ResourceType::kAdditional, resourceData);
        }
    }

    return true;
}

bool ParseMdnsPacket(const BytesRange & packetData, ParserDelegate * delegate)
{
    VerifyOrReturnValue(packetData.Size() >= static_cast<ptrdiff_t>(HeaderRef::kSizeBytes), false);

    ConstHeaderRef header(packetData.Start());
    VerifyOrReturnValue(header.GetFlags().IsValidMdns(), false);

    return ParseDnsPacket(packetData, delegate);
}

} // namespace Dnssd
} // namespace chip
