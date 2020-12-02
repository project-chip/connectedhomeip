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

#include <support/BufBound.h>

#include "Constants.h"
#include "QName.h"

namespace mdns {
namespace Minimal {

/// Represents a MDNS Query: QName and flags
class Query
{
public:
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

    /// Append the query to the specified buffer
    ///
    /// @param hdr will be updated with a query count
    /// @param out where to write the query data
    bool Append(HeaderRef & hdr, chip::BufBound & out) const
    {
        // Questions can only be appended before any other data is added
        if ((hdr.GetAdditionalCount() != 0) || (hdr.GetAnswerCount() != 0) || (hdr.GetAuthorityCount() != 0))
        {
            return false;
        }
        for (uint16_t i = 0; i < mQNameCount; i++)
        {

            out.Put8(static_cast<uint8_t>(strlen(mQName[i])));
            out.Put(mQName[i]);
        }
        out.Put8(0); // end of qnames

        out.PutBE16(static_cast<uint16_t>(mType));
        out.PutBE16(static_cast<uint16_t>(static_cast<uint16_t>(mClass) | (mAnswerViaUnicast ? kQClassUnicastAnswerFlag : 0)));

        if (out.Fit())
        {
            hdr.SetQueryCount(static_cast<uint16_t>(hdr.GetQueryCount() + 1));
        }

        return out.Fit();
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
