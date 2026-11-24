/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/dnssd/platform/Dnssd.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/FixedBufferAllocator.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>

#include <cstddef>
#include <cstdint>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Parses DNS TXT record data into key/value entries.
 *
 * `TxtIterator` reads the entries of the TXT data, with the shape of the OpenThread TXT entry iterator API:
 *   - `TxtIterator::Iterator`: the iterator state.
 *   - `TxtIterator::Entry`: one entry, with `mKey` (null-terminated), `mValue` and `mValueLength`.
 *     Entries with a null `mKey` or `mValue` are skipped.
 *   - `static void Init(Iterator & iterator, const uint8_t * txtData, uint16_t txtDataLength)`
 *   - `static bool Next(Iterator & iterator, Entry & entry)`: returns false when no entry is left.
 *
 * Keys and values are copied into `buffer`. Parsing stops when `entries` is full.
 *
 * @param[in]     txtData  The TXT record data.
 * @param[in,out] entries  Storage for the entries. Reduced to the parsed entries on success, and to size 0 on failure.
 * @param[in]     buffer   Storage for the keys and values.
 *
 * @retval CHIP_NO_ERROR                On success.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL  `buffer` cannot hold the keys and values.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  `txtData` is longer than UINT16_MAX bytes.
 */
template <class TxtIterator>
CHIP_ERROR ParseTxtEntries(ByteSpan txtData, Span<Dnssd::TextEntry> & entries, MutableByteSpan buffer)
{
    Span<Dnssd::TextEntry> storage = entries;
    entries.reduce_size(0);

    VerifyOrReturnError(CanCastTo<uint16_t>(txtData.size()), CHIP_ERROR_INVALID_ARGUMENT);

    typename TxtIterator::Iterator iterator;
    TxtIterator::Init(iterator, txtData.data(), static_cast<uint16_t>(txtData.size()));

    FixedBufferAllocator alloc(buffer.data(), buffer.size());
    typename TxtIterator::Entry txtEntry;
    size_t entryCount = 0;

    while (entryCount < storage.size() && TxtIterator::Next(iterator, txtEntry))
    {
        if (txtEntry.mKey == nullptr || txtEntry.mValue == nullptr)
        {
            continue;
        }

        const char * key     = alloc.Clone(txtEntry.mKey);
        const uint8_t * data = alloc.Clone(txtEntry.mValue, txtEntry.mValueLength);
        VerifyOrReturnError(!alloc.AnyAllocFailed(), CHIP_ERROR_BUFFER_TOO_SMALL);

        storage[entryCount++] = Dnssd::TextEntry{ key, data, txtEntry.mValueLength };
    }

    entries = storage.SubSpan(0, entryCount);
    return CHIP_NO_ERROR;
}

/**
 * Storage for the TXT entries of one resolved DNS-SD service.
 *
 * @tparam TxtIterator  Reads the entries of the TXT data, as for ParseTxtEntries.
 * @tparam kMaxEntries  The number of entries kept. Further entries are dropped.
 * @tparam kBufferSize  The size of the buffer holding the keys and values.
 */
template <class TxtIterator, size_t kMaxEntries, size_t kBufferSize>
struct TxtEntries
{
    uint8_t mBuffer[kBufferSize];
    Dnssd::TextEntry mTxtEntries[kMaxEntries];

    /**
     * Parses `txtData` into this storage and sets the text entries of `service` to point into it.
     *
     * The text entries of `service` stay valid until this storage is destroyed or parsed into again.
     * On failure, `service` has no text entries.
     */
    CHIP_ERROR Parse(ByteSpan txtData, Dnssd::DnssdService & service)
    {
        Span<Dnssd::TextEntry> entries(mTxtEntries);
        CHIP_ERROR err = ParseTxtEntries<TxtIterator>(txtData, entries, MutableByteSpan(mBuffer));

        service.mTextEntries   = entries.data();
        service.mTextEntrySize = entries.size();
        return err;
    }
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
