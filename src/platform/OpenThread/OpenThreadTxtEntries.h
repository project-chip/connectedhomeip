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

#include <openthread/dns.h>

#include <lib/core/CHIPError.h>
#include <lib/dnssd/platform/Dnssd.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/FixedBufferAllocator.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>

#include <cstddef>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Parses DNS TXT record data into key/value entries.
 *
 * Keys and values are copied into `buffer`. Parsing stops when `entries` is full.
 *
 * @param[in]     txtData  The TXT record data.
 * @param[in,out] entries  Storage for the entries. Reduced to the parsed entries on success, and to size 0 on failure.
 * @param[in]     buffer   Storage for the keys and values.
 *
 * @retval CHIP_NO_ERROR                On success.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL  `buffer` cannot hold the keys and values.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  `txtData` is longer than OpenThread accepts.
 */
inline CHIP_ERROR ParseOpenThreadTxtEntries(ByteSpan txtData, Span<Dnssd::TextEntry> & entries, MutableByteSpan buffer)
{
    Span<Dnssd::TextEntry> storage = entries;
    entries.reduce_size(0);

    VerifyOrReturnError(CanCastTo<uint16_t>(txtData.size()), CHIP_ERROR_INVALID_ARGUMENT);

    otDnsTxtEntryIterator iterator;
    otDnsInitTxtEntryIterator(&iterator, txtData.data(), static_cast<uint16_t>(txtData.size()));

    FixedBufferAllocator alloc(buffer.data(), buffer.size());
    otDnsTxtEntry txtEntry;
    size_t entryCount = 0;

    while (entryCount < storage.size() && otDnsGetNextTxtEntry(&iterator, &txtEntry) == OT_ERROR_NONE)
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
 * @tparam kMaxEntries  The number of entries kept. Further entries are dropped.
 * @tparam kBufferSize  The size of the buffer holding the keys and values.
 */
template <size_t kMaxEntries, size_t kBufferSize>
struct OpenThreadTxtEntries
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
        CHIP_ERROR err = ParseOpenThreadTxtEntries(txtData, entries, MutableByteSpan(mBuffer));

        service.mTextEntries   = entries.data();
        service.mTextEntrySize = entries.size();
        return err;
    }
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
