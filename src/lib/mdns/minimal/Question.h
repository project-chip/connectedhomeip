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

/// Represents a MDNS question: QName and flags
class Question
{
public:
    enum class QType : uint16_t
    {
        A     = 1,
        NS    = 2,
        CNAME = 5,
        SOA   = 6,
        WKS   = 11,
        PTR   = 12,
        MX    = 15,
        SRV   = 33,
        AAAA  = 28,
        ANY   = 255,
    };
    enum class QClass : uint16_t
    {
        IN  = 1,
        ANY = 255,
    };

    Question(const QNamePart * names, uint16_t namesCount) : mQNameCount(namesCount), mQName(names) {}

    bool IsAnswerViaUnicast() const { return mAnswerViaUnicast; }
    Question & SetAnswerViaUnicast(bool value)
    {
        mAnswerViaUnicast = value;
        return *this;
    }

    QType GetType() const { return mType; }
    Question & SetType(QType value)
    {
        mType = value;
        return *this;
    }

    QClass GetClass() const { return mClass; }
    Question & SetClass(QClass value)
    {
        mClass = value;
        return *this;
    }

    size_t WriteSizeBytes() const
    {
        size_t s = 2 * sizeof(uint16_t); // QTYPE and QClass
        for (uint16_t i = 0; i < mQNameCount; i++)
        {
            s += strlen(mQName[i]) + 1;
        }
        s++; // final 0 for qnames end
        return s;
    }

    /// Append the question to the specified buffer
    /// @param hdr will be updated with a question count
    /// @return nullptr on failure, end of writing on success
    uint8_t * Append(HeaderRef & hdr, uint8_t * buffer, size_t max_size) const
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
            if (max_size < namelen + 1)
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
            max_size -= namelen + 1;
        }

        if (max_size < 1 + 2 * sizeof(uint16_t))
        {
            return nullptr;
        }

        *buffer++ = 0; // end of qnames
        chip::Encoding::BigEndian::Write16(buffer, static_cast<uint16_t>(mType));
        chip::Encoding::BigEndian::Write16(
            buffer, static_cast<uint16_t>(static_cast<uint16_t>(mClass) | (mAnswerViaUnicast ? kUnicastAnswerFlag : 0)));

        hdr.SetQuestionCount(static_cast<uint16_t>(hdr.GetQuestionCount() + 1));

        return buffer;
    }

private:
    /// Flag encoded in QCLASS requesting unicast answers
    static constexpr uint16_t kUnicastAnswerFlag = 0x8000;

    const uint16_t mQNameCount;
    const QNamePart * mQName;
    QType mType            = QType::ANY;
    QClass mClass          = QClass::IN;
    bool mAnswerViaUnicast = true;
};

} // namespace Minimal

} // namespace mdns