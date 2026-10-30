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

#include <platform/OpenThread/TxtEntries.h>

#include <cstddef>
#include <cstdint>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Reads TXT entries with the OpenThread TXT entry iterator, as `TxtIterator` of ParseTxtEntries.
 */
struct OpenThreadTxtIterator
{
    using Iterator = otDnsTxtEntryIterator;
    using Entry    = otDnsTxtEntry;

    static void Init(Iterator & iterator, const uint8_t * txtData, uint16_t txtDataLength)
    {
        otDnsInitTxtEntryIterator(&iterator, txtData, txtDataLength);
    }

    static bool Next(Iterator & iterator, Entry & entry) { return otDnsGetNextTxtEntry(&iterator, &entry) == OT_ERROR_NONE; }
};

template <size_t kMaxEntries, size_t kBufferSize>
using OpenThreadTxtEntries = TxtEntries<OpenThreadTxtIterator, kMaxEntries, kBufferSize>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
