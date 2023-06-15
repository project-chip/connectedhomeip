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
 *      This file implements the utility class for the Matter Checkin protocol.
 */

#include "CheckinMessage.h"
#include <lib/core/CHIPCore.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {

static const uint16_t mMaxPlainTextSize = CHIP_CHECK_IN_APP_DATA_MAX_SIZE + CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES;
static const uint16_t mMaxWorBufferSize = mMaxPlainTextSize + sizeof(uint32_t);
static const uint16_t minPayloadSize = CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + sizeof(uint32_t) + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;

CHIP_ERROR CheckinMessage::GenerateCheckingMessagePayload(Crypto::Aes128KeyHandle & key, uint32_t counter, ByteSpan appData,
                                                          uint8_t * output, uint16_t * payloadSize)
{
    VerifyOrReturnError(nullptr != output, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(appData.size() <= CHIP_CHECK_IN_APP_DATA_MAX_SIZE, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint8_t * outputPtr = output;

    {
        chip::Crypto::HMAC_sha shaHandler;
        uint8_t nonceWorkBuffer[CHIP_CRYPTO_HASH_LEN_BYTES] = { 0 };
        ReturnErrorOnFailure(shaHandler.HMAC_SHA256(key.As<Aes128KeyByteArray>(), sizeof(Aes128KeyByteArray) * 8,
                                                    (uint8_t *) &counter, sizeof(counter), nonceWorkBuffer,
                                                    CHIP_CRYPTO_HASH_LEN_BYTES));
        memcpy(outputPtr, nonceWorkBuffer, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
        outputPtr += CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES;
    }

    // Concatenate the App data (if any) with the counter value
    uint8_t workbuffer[mMaxWorBufferSize];
    uint8_t mic[CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES];
    uint8_t * p = workbuffer;
    Encoding::LittleEndian::Write32(p, counter);
    memcpy(workbuffer + sizeof(counter), appData.data(), appData.size());

    err = Crypto::AES_CCM_encrypt(workbuffer, appData.size() + sizeof(counter), nullptr, 0, key, output,
                                  CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, outputPtr, mic, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    memcpy(outputPtr + appData.size() + sizeof(counter), mic, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    if (payloadSize != nullptr)
    {
        *payloadSize = static_cast<uint16_t>(appData.size() + minPayloadSize);
    }

    return err;
}

CHIP_ERROR CheckinMessage::ParseCheckingMessagePayload(Crypto::Aes128KeyHandle & key, ByteSpan & payload, uint32_t & counter,
                                                       uint8_t * appData)
{
    VerifyOrReturnError(nullptr != appData, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(payload.size() >= minPayloadSize, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(payload.size() <= (minPayloadSize + CHIP_CHECK_IN_APP_DATA_MAX_SIZE), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t plaintext[mMaxPlainTextSize];
    uint16_t appDataSize = GetAppDataSize(payload);

    ByteSpan nonce         = payload.SubSpan(0, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
    ByteSpan encryptedData = payload.SubSpan(CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, appDataSize + sizeof(counter));
    ByteSpan mic =
        payload.SubSpan(CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + appDataSize + sizeof(counter), CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    err = Crypto::AES_CCM_decrypt(encryptedData.data(), encryptedData.size(), nullptr, 0, mic.data(),
                                  CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, key, nonce.data(), CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES,
                                  plaintext);

    ReturnErrorOnFailure(err);

    uint8_t * ptr = static_cast<uint8_t *>(plaintext);
    counter       = Encoding::LittleEndian::Read32(ptr);
    memcpy(appData, plaintext + sizeof(counter), appDataSize);
    return err;
}

uint16_t CheckinMessage::GetAppDataSize(ByteSpan & payload)
{
    return (payload.size() < minPayloadSize) ? 0 : static_cast<uint16_t>(payload.size() - minPayloadSize);
}

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
