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

#pragma once

#include "QName.h"

namespace mdns {
namespace Minimal {

// Assigned by IANA: https://www.iana.org/assignments/dns-parameters/dns-parameters.xhtml#dns-parameters-4
enum class QType : uint16_t
{
    A         = 1,
    NS        = 2,
    CNAME     = 5,
    SOA       = 6,
    NULLVALUE = 10,
    WKS       = 11,
    PTR       = 12,
    HINFO     = 13,
    MINFO     = 14,
    MX        = 15,
    TXT       = 16,
    ISDN      = 20,
    AAAA      = 28,
    SRV       = 33,
    DNAM      = 39,
    ANY       = 255,
};
enum class QClass : uint16_t
{
    IN  = 1,
    ANY = 255,
};

/// Represents a MDNS Query: QName and flags
class Query
{
public:
    /// Flag encoded in QCLASS requesting unicast answers
    /// public for usage in parsers.
    static constexpr uint16_t kUnicastAnswerFlag = 0x8000;

    Query(const QNamePart * names, uint16_t namesCount) : mQNameCount(namesCount), mQName(names) {}

    bool IsAnswerViaUnicast() const { return mAnswerViaUnicast; }
    Query & SetAnswerViaUnicast(bool value)
    {
        mAnswerViaUnicast = value;
        return *this;
    }

    QType GetType() const { return mType; }
    Query & SetType(QType value)
    {
        mType = value;
        return *this;
    }

    QClass GetClass() const { return mClass; }
    Query & SetClass(QClass value)
    {
        mClass = value;
        return *this;
    }

    size_t WriteSizeBytes() const
    {
        size_t size = 2 * sizeof(uint16_t); // QTYPE and QClass
        for (uint16_t i = 0; i < mQNameCount; i++)
        {
            size += strlen(mQName[i]) + 1;
        }
        size++; // final 0 for qnames end
        return size;
    }

    /// Append the query to the specified buffer
    ///
    /// @param hdr will be updated with a query count
    /// @param buffer where to place the query data
    /// @param maxSize maximum buffer space in buffer
    /// @return nullptr on failure, end of writing on success
    uint8_t * Append(HeaderRef & hdr, uint8_t * buffer, size_t maxSize) const
    {
        // Questions can only be appended before any other data is added
        if ((hdr.GetAdditionalCount() != 0) || (hdr.GetAnswerCount() != 0) || (hdr.GetAuthorityCount() != 0))
        {
            return nullptr;
        }
        // Write all QName parts
        for (uint16_t i = 0; i < mQNameCount; i++)
        {
            size_t namelen = strlen(mQName[i]);
            if (maxSize < namelen + 1)
            {
                return nullptr;
            }
            if ((namelen > 0x3F) || (namelen == 0))
            {
                return nullptr;
            }
            *buffer = static_cast<uint8_t>(namelen);
            memcpy(buffer + 1, mQName[i], namelen);
            buffer += namelen + 1;
            maxSize -= namelen + 1;
        }

        if (maxSize < 1 + 2 * sizeof(uint16_t))
        {
            return nullptr;
        }

        *buffer++ = 0; // end of qnames
        chip::Encoding::BigEndian::Write16(buffer, static_cast<uint16_t>(mType));
        chip::Encoding::BigEndian::Write16(
            buffer, static_cast<uint16_t>(static_cast<uint16_t>(mClass) | (mAnswerViaUnicast ? kUnicastAnswerFlag : 0)));

        hdr.SetQueryCount(static_cast<uint16_t>(hdr.GetQueryCount() + 1));

        return buffer;
    }

private:
    const uint16_t mQNameCount;
    const QNamePart * mQName;
    QType mType            = QType::ANY;
    QClass mClass          = QClass::IN;
    bool mAnswerViaUnicast = true;
};

} // namespace Minimal

} // namespace mdns
