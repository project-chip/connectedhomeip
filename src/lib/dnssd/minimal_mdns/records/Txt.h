/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
