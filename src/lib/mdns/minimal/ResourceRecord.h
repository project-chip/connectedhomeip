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

#include <cstddef>

#include "Constants.h"
#include "QName.h"

#include <support/BufBound.h>

namespace mdns {
namespace Minimal {

/// A generic Reply record that supports data serialization
class ResourceRecord
{
public:
    virtual ~ResourceRecord() {}

    QClass GetClass() const { return QClass::IN; }
    QType GetType() const { return mType; }

    uint64_t GetTtl() const { return mTtl; }
    ResourceRecord & SetTtl(uint64_t & ttl)
    {
        mTtl = ttl;
        return *this;
    }

    bool Append(HeaderRef & hdr, ResourceType asType, chip::BufBound & out) const;

protected:
    /// Output the data portion of the resource record.
    virtual bool WriteData(chip::BufBound & out) const = 0;

    ResourceRecord(QType type, const QNamePart * names, uint16_t namesCount) : mType(type), mQNameCount(namesCount), mQName(names)
    {}

private:
    const QType mType;
    uint64_t mTtl = 0;

    const uint16_t mQNameCount;
    const QNamePart * mQName;
};

} // namespace Minimal
} // namespace mdns
