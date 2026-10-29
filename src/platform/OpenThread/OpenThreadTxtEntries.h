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

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Parses DNS TXT record data into key/value entries, using the OpenThread TXT entry iterator.
 *
 * Keys and values are copied into `buffer`, so the entries stay valid after `txtData` is released,
 * for as long as `buffer` is. Strings without a value, and strings whose key is longer than
 * OT_DNS_TXT_KEY_ITER_MAX_LENGTH, are skipped. Parsing stops once `entries` is full, and at the
 * first string whose length runs past the end of `txtData`.
 *
 * @param[in]     txtData  The TXT record data: a sequence of length-prefixed strings.
 * @param[in,out] entries  Storage for the entries; on success, reduced to the number of entries parsed.
 * @param[in]     buffer   Storage for the keys and values the entries point to.
 *
 * @retval CHIP_NO_ERROR                On success.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL  `buffer` cannot hold the keys and values.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  `txtData` is longer than the OpenThread iterator accepts.
 */
inline CHIP_ERROR ParseOpenThreadTxtEntries(ByteSpan txtData, Span<Dnssd::TextEntry> & entries, MutableByteSpan buffer)
{
    VerifyOrReturnError(CanCastTo<uint16_t>(txtData.size()), CHIP_ERROR_INVALID_ARGUMENT);

    size_t entryCount = 0;

    if (!txtData.empty())
    {
        otDnsTxtEntryIterator iterator;
        otDnsInitTxtEntryIterator(&iterator, txtData.data(), static_cast<uint16_t>(txtData.size()));

        FixedBufferAllocator alloc(buffer.data(), buffer.size());
        otDnsTxtEntry txtEntry;

        while (entryCount < entries.size() && otDnsGetNextTxtEntry(&iterator, &txtEntry) == OT_ERROR_NONE)
        {
            if (txtEntry.mKey == nullptr || txtEntry.mValue == nullptr)
            {
                continue;
            }

            Dnssd::TextEntry & entry = entries[entryCount++];
            entry.mKey               = alloc.Clone(txtEntry.mKey);
            entry.mData              = alloc.Clone(txtEntry.mValue, txtEntry.mValueLength);
            entry.mDataSize          = txtEntry.mValueLength;
            VerifyOrReturnError(!alloc.AnyAllocFailed(), CHIP_ERROR_BUFFER_TOO_SMALL);
        }
    }

    entries.reduce_size(entryCount);
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
