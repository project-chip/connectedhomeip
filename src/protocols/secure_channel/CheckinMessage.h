/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *      This file implements the Matter Checkin protocol.
 */

#pragma once

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Span.h>
#include <stdint.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {
using namespace Crypto;

using CounterType = uint32_t;

/**
 * @brief   Implement section 4.18.2 of the spec regarding
 *          Check-in message payload
 *
 */
class DLL_EXPORT CheckinMessage
{
public:
    ~CheckinMessage();
    /**
     * @brief Generate Check-in Message payload
     *
     * @param key       Key with which to encrypt the check-in payload
     * @param counter   Check-in counter
     * @param appData   Application Data to incorporate within the Check-in message. Allowed to be empty.
     * @param output    Buffer in Which to store the generated payload. SUFFICIENT SPACE MUST BE ALLOCATED by the caller
     *                  Required Buffer Size is : GetCheckinPayloadSize(appData.size())
     * @return CHIP_ERROR
     */
    static CHIP_ERROR GenerateCheckinMessagePayload(Crypto::Aes128KeyHandle & key, CounterType counter, const ByteSpan & appData,
                                                    MutableByteSpan & output);

    /**
     * @brief Parse Check-in Message payload
     *
     * @param key       Key with which to decrypt the check-in payload
     * @param payload   The received payload to decrypt and parse
     * @param counter   The counter value retrieved from the payload
     * @param appData   The optional application data decrypted. The size of appData must be at least the size of
     *                  GetAppDataSize(payload) + sizeof(CounterType)
     * @return CHIP_ERROR
     */
    static CHIP_ERROR ParseCheckinMessagePayload(Crypto::Aes128KeyHandle & key, ByteSpan & payload, CounterType & counter,
                                                 MutableByteSpan & appData);

    static inline size_t GetCheckinPayloadSize(size_t appDataSize) { return appDataSize + sMinPayloadSize; }

    /**
     * @brief Get the App Data Size
     *
     * @param payload   The undecrypted payload
     * @return size_t size in byte of the application data from the payload
     */
    static size_t GetAppDataSize(ByteSpan & payload);

    static constexpr uint16_t sMinPayloadSize =
        CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + sizeof(CounterType) + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;

    // Issue #28603
    static constexpr uint16_t sMaxAppDataSize = 1024;
};

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
