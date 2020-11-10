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

namespace mdns {
namespace Minimal {

bool QueryData::Parse(const uint8_t * dataStart, const uint8_t * dataEnd, const uint8_t ** start)
{
    // Structure is:
    //    QNAME
    //    TYPE
    //    CLASS (plus a flag for unicast)
    const uint8_t * nameEnd = nullptr;

    {
        SerializedQNameIterator it(dataStart, dataEnd, *start);
        nameEnd = it.FindDataEnd();
    }
    if (nameEnd == nullptr)
    {
        return false;
    }

    if (dataEnd - nameEnd < 4)
    {
        return false;
    }

    // TODO: should there be checks for valid mType/class?

    mType = static_cast<QType>(chip::Encoding::BigEndian::Read16(nameEnd));

    uint16_t klass = chip::Encoding::BigEndian::Read16(nameEnd);

    mAnswerViaUnicast = (klass & Query::kUnicastAnswerFlag) != 0;
    mClass            = static_cast<QClass>(klass & ~Query::kUnicastAnswerFlag);
    mNameIterator     = SerializedQNameIterator(dataStart, dataEnd, *start);

    *start = nameEnd;

    return true;
}

bool ResourceData::Parse(const uint8_t * dataStart, const uint8_t * dataEnd, const uint8_t ** start)
{
    // Structure is:
    //    QNAME
    //    TYPE      (16 bit)
    //    CLASS     (16 bit)
    //    TTL       (32 bit)
    //    RDLENGTH  (16 bit)
    //    <DATA>    (RDLENGTH bytpes)
    const uint8_t * nameEnd = nullptr;

    {
        SerializedQNameIterator it(dataStart, dataEnd, *start);
        nameEnd = it.FindDataEnd();
    }
    if (nameEnd == nullptr)
    {
        return false;
    }

    // need 3*u16 + u32
    if (dataEnd - nameEnd < 10)
    {
        return false;
    }

    chip::Encoding::BigEndian::Read16(nameEnd);                    // Type
    chip::Encoding::BigEndian::Read16(nameEnd);                    // Class
    chip::Encoding::BigEndian::Read32(nameEnd);                    // TTL
    uint16_t dataLen = chip::Encoding::BigEndian::Read16(nameEnd); // resource data

    if (dataEnd - nameEnd < dataLen)
    {
        return false; // no space for RDATA
    }

    mNameIterator = SerializedQNameIterator(dataStart, dataEnd, *start);

    *start = nameEnd + dataLen;

    return true;
}

bool ParsePacket(const uint8_t * packet, size_t length, ParserDelegate * delegate)
{
    if (length < HeaderRef::kSizeBytes)
    {
        return false;
    }

    // header is used as const, so cast is safe
    HeaderRef header(const_cast<uint8_t *>(packet));

    if (!header.GetFlags().IsValidMdns())
    {
        return false;
    }

    delegate->Header(header);

    const uint8_t * dataStart = packet;
    const uint8_t * dataEnd   = packet + length;

    const uint8_t * data = packet + HeaderRef::kSizeBytes;

    {
        QueryData queryData;
        for (unsigned i = 0; i < header.GetQueryCount(); i++)
        {
            if (!queryData.Parse(dataStart, dataEnd, &data))
            {
                return false;
            }

            delegate->Query(queryData);
        }
    }

    {
        ResourceData resourceData;
        for (unsigned i = 0; i < header.GetAnswerCount(); i++)
        {
            if (!resourceData.Parse(dataStart, dataEnd, &data))
            {
                return false;
            }

            delegate->Resource(ParserDelegate::ResourceType::kAnswer, resourceData);
        }

        for (unsigned i = 0; i < header.GetAuthorityCount(); i++)
        {
            if (!resourceData.Parse(dataStart, dataEnd, &data))
            {
                return false;
            }

            delegate->Resource(ParserDelegate::ResourceType::kAuthority, resourceData);
        }

        for (unsigned i = 0; i < header.GetAdditionalCount(); i++)
        {
            if (!resourceData.Parse(dataStart, dataEnd, &data))
            {
                return false;
            }

            delegate->Resource(ParserDelegate::ResourceType::kAdditional, resourceData);
        }
    }

    return true;
}

} // namespace Minimal
} // namespace mdns
