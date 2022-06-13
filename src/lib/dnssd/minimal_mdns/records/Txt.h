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

#include <lib/dnssd/minimal_mdns/records/ResourceRecord.h>

namespace mdns {
namespace Minimal {

class TxtResourceRecord : public ResourceRecord
{
public:
    static constexpr uint64_t kDefaultTtl = 4500; // 75 minutes

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

    // A FullQName is a holder of a string array.
    TxtResourceRecord(const FullQName & qName, FullQName entries) :
        ResourceRecord(QType::TXT, qName), mEntries(entries.names), mEntryCount(entries.nameCount)
    {
        SetTtl(kDefaultTtl);
    }
    size_t GetNumEntries() const { return mEntryCount; }
    const char * const * GetEntries() const { return mEntries; }

protected:
    bool WriteData(RecordWriter & out) const override
    {
        for (size_t i = 0; i < mEntryCount; i++)
        {
            size_t len = strlen(mEntries[i]);
            if (len > kMaxTxtRecordLength)
            {
                return false;
            }

            out.Put8(static_cast<uint8_t>(len)).PutString(mEntries[i]);
        }
        return out.Fit();
    }

private:
    const char * const * mEntries;
    const size_t mEntryCount;

    static constexpr size_t kMaxTxtRecordLength = 63;
};

} // namespace Minimal
} // namespace mdns
