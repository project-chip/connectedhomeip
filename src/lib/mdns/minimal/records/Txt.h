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

#include <string.h>

#include <mdns/minimal/records/ResourceRecord.h>

namespace mdns {
namespace Minimal {

class TxtResourceRecord : public ResourceRecord
{
public:
    static constexpr uint64_t kDefaultTtl = 10;

    TxtResourceRecord(const FullQName & qName, const char ** entries, size_t entryCount) :
        ResourceRecord(QType::TXT, qName), mEntries(entries), mEntryCount(entryCount)
    {
        SetTtl(kDefaultTtl);
    }

    template <size_t N>
    TxtResourceRecord(const FullQName & qName, const char * (&entries)[N]) :
        ResourceRecord(QType::TXT, qName), mEntries(entries), mEntryCount(N)
    {
        SetTtl(kDefaultTtl);
    }

protected:
    bool WriteData(chip::Encoding::BigEndian::BufferWriter & out) const override
    {
        for (size_t i = 0; i < mEntryCount; i++)
        {
            size_t len = strlen(mEntries[i]);
            if (len > kMaxQNamePartLength)
            {
                return false;
            }

            out.Put8(static_cast<uint8_t>(len));
            out.Put(mEntries[i]);
        }
        return out.Fit();
    }

private:
    const char ** mEntries;
    const size_t mEntryCount;
};

} // namespace Minimal
} // namespace mdns
