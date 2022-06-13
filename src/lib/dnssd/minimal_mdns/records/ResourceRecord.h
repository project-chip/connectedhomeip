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

#include <lib/dnssd/minimal_mdns/core/Constants.h>
#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/dnssd/minimal_mdns/core/RecordWriter.h>

#include <lib/support/BufferWriter.h>

namespace mdns {
namespace Minimal {

/// A generic Reply record that supports data serialization
class ResourceRecord
{
public:
    static constexpr uint32_t kDefaultTtl = 120; // 2 minutes

    virtual ~ResourceRecord() {}

    ResourceRecord & operator=(const ResourceRecord & other) = default;

    const FullQName & GetName() const { return mQName; }
    QClass GetClass() const { return mCacheFlush ? QClass::IN_FLUSH : QClass::IN; }
    QType GetType() const { return mType; }

    uint32_t GetTtl() const { return mTtl; }
    ResourceRecord & SetTtl(uint32_t ttl)
    {
        mTtl = ttl;
        return *this;
    }

    ResourceRecord & SetCacheFlush(bool set)
    {
        mCacheFlush = set;
        return *this;
    }
    bool GetCacheFlush() const { return mCacheFlush; }

    /// Append the given record to the underlying output.
    /// Updates header item count on success, does NOT update header on failure.
    bool Append(HeaderRef & hdr, ResourceType asType, RecordWriter & out) const;

protected:
    /// Output the data portion of the resource record.
    virtual bool WriteData(RecordWriter & out) const = 0;

    ResourceRecord(QType type, FullQName name) : mType(type), mQName(name) {}

private:
    QType mType;
    uint32_t mTtl = kDefaultTtl;
    FullQName mQName;
    bool mCacheFlush = false;
};

} // namespace Minimal
} // namespace mdns
