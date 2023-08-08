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
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {
using namespace Crypto;

#define CHIP_CHECK_IN_APP_DATA_MAX_SIZE 1024

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
     * @param appData   Optionnal : Application Data to incorporate within the Check-in message
     * @param output    Buffer in Which to store the generated payload. SUFFICIENT SPACE MUST BE ALLOCATED by the caller
     *                  Required Buffer Size is : User Data + CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + SizeOf(uint32_t) +
     * CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES
     * @return CHIP_ERROR
     */
    static CHIP_ERROR GenerateCheckinMessagePayload(Crypto::Aes128KeyHandle & key, uint32_t counter, const ByteSpan & appData,
                                                    MutableByteSpan & output);

    /**
     * @brief Parse Check-in Message payload
     *
     * @param key       Key with which to decrypt the check-in payload
     * @param payload   The received payload to decrypt and parse
     * @param counter   The counter value retrieved from the payload
     * @param appData   The optional application data decrypted
     * @return CHIP_ERROR
     */
    static CHIP_ERROR ParseCheckinMessagePayload(Crypto::Aes128KeyHandle & key, ByteSpan & payload, uint32_t & counter,
                                                 MutableByteSpan & appData);

    static inline uint64_t GetRequiredBufferSize(uint32_t & payloadSize) { return payloadSize + sMinPayloadSize; }

    static constexpr uint16_t sMinPayloadSize =
        CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + sizeof(CounterType) + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;

private:
    static uint16_t GetAppDataSize(ByteSpan & payload);
};

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
