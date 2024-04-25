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
#include <lib/support/BufferWriter.h>
#include <lib/support/Span.h>
#include <stdint.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {

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
     * @note Function requires two key handles to generate the Check-In message.
     *       Due to the way some key stores work, the same key handle cannot be used for AES-CCM and HMAC-SHA-256 operations.
     *
     * @param[in]  aes128KeyHandle   Key handle with which to encrypt the check-in payload (using AEAD).
     * @param[in]  hmac128KeyHandle  Key handle with which to generate the nonce for the check-in payload (using HMAC).
     * @param[in]  counter           Check-in counter
     * @param[in]  appData           Application Data to incorporate within the Check-in message. Allowed to be empty.
     * @param[out] output            Buffer in Which to store the generated payload. SUFFICIENT SPACE MUST BE ALLOCATED by the
     *                               caller Required Buffer Size is : GetCheckinPayloadSize(appData.size())
     *
     * @return CHIP_ERROR_BUFFER_TOO_SMALL if output buffer is too small
     *         CHIP_ERROR_INVALID_ARGUMENT if the provided arguments cannot be used to generate the Check-In message
     *         CHIP_ERROR_INTERNAL if an error occurs during the generation of the Check-In message
     */
    static CHIP_ERROR GenerateCheckinMessagePayload(const Crypto::Aes128KeyHandle & aes128KeyHandle,
                                                    const Crypto::Hmac128KeyHandle & hmacKeyHandle, const CounterType & counter,
                                                    const ByteSpan & appData, MutableByteSpan & output);

    /**
     * @brief Parse Check-in Message payload
     *
     * @note Function requires two key handles to parse the Check-In message.
     *       Due to the way some key stores work, the same key handle cannot be used for AES-CCM and HMAC-SHA-256 operations.
     *
     * @param[in]       aes128KeyHandle   Key handle with which to decrypt the received check-in payload (using AEAD).
     * @param[in]       hmac128KeyHandle  Key handle with which to verify the received nonce in the check-in payload (using HMAC).
     * @param[in]       payload           The received payload to decrypt and parse
     * @param[out]      counter           The counter value retrieved from the payload
     *                                    If an error occurs, no value will be set.
     * @param[in,out]   appData           The optional application data decrypted. The input size of appData must be at least the
     *                                    size of GetAppDataSize(payload) + sizeof(CounterType), because appData is used as a work
     *                                    buffer for the decryption process. The output size on success will be
     *                                    GetAppDataSize(payload). If an error occurs, appData might countain data,
     *                                    but the data CANNOT be used since we were not able to validate it.
     *
     * @return CHIP_ERROR_INVALID_MESSAGE_LENGTH if the payload is shorter than the minimum payload size
     *         CHIP_ERROR_BUFFER_TOO_SMALL if appData buffer is too small
     *         CHIP_ERROR_INTERNAL if we were not able to decrypt or validate the Check-In message
     */

    static CHIP_ERROR ParseCheckinMessagePayload(const Crypto::Aes128KeyHandle & aes128KeyHandle,
                                                 const Crypto::Hmac128KeyHandle & hmacKeyHandle, const ByteSpan & payload,
                                                 CounterType & counter, MutableByteSpan & appData);

    static inline size_t GetCheckinPayloadSize(size_t appDataSize) { return appDataSize + kMinPayloadSize; }

    /**
     * @brief Get the App Data Size
     *
     * @param payload   The undecrypted payload
     * @return size_t size in byte of the application data from the payload
     */
    static size_t GetAppDataSize(const ByteSpan & payload);

    static constexpr uint16_t kMinPayloadSize =
        Crypto::CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + sizeof(CounterType) + Crypto::CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;

private:
    /**
     * @brief Generate the Nonce for the Check-In message
     *
     * @param[in]   hmacKeyHandle Key handle to use with the HMAC algorithm
     * @param[in]   counter       Check-In Counter value to use as message of the HMAC algorithm
     * @param[out]  output        output buffer for the generated Nonce.
     *                            SUFFICIENT SPACE MUST BE ALLOCATED by the caller
     *                            Size must be at least CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES
     *
     * @return CHIP_ERROR_BUFFER_TOO_SMALL if output buffer is too small
     *         CHIP_ERROR_INVALID_ARGUMENT if the provided arguments cannot be used to generate the Check-In message Nonce
     */
    static CHIP_ERROR GenerateCheckInMessageNonce(const Crypto::Hmac128KeyHandle & hmacKeyHandle, CounterType counter,
                                                  Encoding::LittleEndian::BufferWriter & writer);
};

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
