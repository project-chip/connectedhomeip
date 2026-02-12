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
#include <lib/dnssd/Advertiser.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

#include <stddef.h>

namespace chip {
namespace app {

/**
 * @brief Helper class to build TXT record entries for Thread MeshCoP advertisement.
 */
class TxtStringsBuilder
{
public:
    TxtStringsBuilder() = default;

    /**
     * @brief Fills the TXT record entries from the given advertising parameters.
     *
     * @param params The advertising parameters.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or CHIP_ERROR_BUFFER_TOO_SMALL if the buffer is too small.
     */
    CHIP_ERROR Fill(const Dnssd::CommissionAdvertisingParameters & params);

    size_t GetCount() const { return mCount; }
    const char ** GetEntries() const { return const_cast<const char **>(mTxtStrings); }

private:
    CHIP_ERROR FormatAndAdd(const char * format, ...) ENFORCE_FORMAT(2, 3);

    static constexpr size_t kMaxTxtStringsBuffer = 256;

    char mTxtBuffer[kMaxTxtStringsBuffer];
    char * mNextStart = &mTxtBuffer[0];
    size_t mCount     = 0;
    const char * mTxtStrings[Dnssd::CommissionAdvertisingParameters::kTxtMaxNumber];
};

/**
 * @brief Builds the Thread Rendezvous Announcement packet buffer.
 *
 * @param params The advertising parameters.
 * @param outBuffer The resulting packet buffer handle.
 * @return CHIP_ERROR CHIP_NO_ERROR on success, or other error on failure.
 */
CHIP_ERROR BuildThreadRendezvousAnnouncement(const Dnssd::CommissionAdvertisingParameters & params,
                                             System::PacketBufferHandle & outBuffer);

} // namespace app
} // namespace chip
