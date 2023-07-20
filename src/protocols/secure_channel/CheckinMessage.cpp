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

#include "CheckinMessage.h"
#include <lib/core/CHIPCore.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {

static constexpr uint16_t sMinPayloadSize =
    CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + sizeof(uint32_t) + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;

CHIP_ERROR CheckinMessage::GenerateCheckinMessagePayload(Crypto::Aes128KeyHandle & key, uint32_t counter, const ByteSpan & appData,
                                                         MutableByteSpan & output, uint16_t * payloadSize)
{
    VerifyOrReturnError(appData.size() <= CHIP_CHECK_IN_APP_DATA_MAX_SIZE, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(output.size() >= (appData.size() + sMinPayloadSize), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err            = CHIP_NO_ERROR;
    uint8_t * appDataStartPtr = output.data() + CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES;

    // Write32 increase the pointer by sizeof(uint32_t)
    Encoding::LittleEndian::Write32(appDataStartPtr, counter);
    appDataStartPtr -= sizeof(uint32_t);

    {
        chip::Crypto::HMAC_sha shaHandler;
        uint8_t nonceWorkBuffer[CHIP_CRYPTO_HASH_LEN_BYTES] = { 0 };
        ReturnErrorOnFailure(shaHandler.HMAC_SHA256(key.As<Aes128KeyByteArray>(), sizeof(Aes128KeyByteArray), appDataStartPtr,
                                                    sizeof(counter), nonceWorkBuffer, CHIP_CRYPTO_HASH_LEN_BYTES));
        memcpy(output.data(), nonceWorkBuffer, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
    }

    // In place encryption to save some RAM
    memcpy(appDataStartPtr + sizeof(counter), appData.data(), appData.size());

    // API constraint. Mic buffer needs to be separated.
    uint8_t mic[CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES];
    uint8_t * micPtr = appDataStartPtr + appData.size() + sizeof(counter);

    ReturnErrorOnFailure(Crypto::AES_CCM_encrypt(appDataStartPtr, appData.size() + sizeof(counter), nullptr, 0, key, output.data(),
                                                 CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, appDataStartPtr, mic,
                                                 CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));

    memcpy(micPtr, mic, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    if (payloadSize != nullptr)
    {
        *payloadSize = static_cast<uint16_t>(appData.size() + sMinPayloadSize);
    }

    return err;
}

CHIP_ERROR CheckinMessage::ParseCheckinMessagePayload(Crypto::Aes128KeyHandle & key, ByteSpan & payload, uint32_t & counter,
                                                      MutableByteSpan & appData)
{
    VerifyOrReturnError(payload.size() >= sMinPayloadSize, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(payload.size() <= (sMinPayloadSize + CHIP_CHECK_IN_APP_DATA_MAX_SIZE), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err       = CHIP_NO_ERROR;
    uint16_t appDataSize = GetAppDataSize(payload);

    // To prevent workbuffer usage, appData size need to be large enough to hold both the appData and the counter
    VerifyOrReturnError(appData.size() >= appDataSize + sizeof(uint32_t), CHIP_ERROR_INVALID_ARGUMENT);

    ByteSpan nonce         = payload.SubSpan(0, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
    ByteSpan encryptedData = payload.SubSpan(CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, appDataSize + sizeof(counter));
    ByteSpan mic =
        payload.SubSpan(CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + appDataSize + sizeof(counter), CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    err = Crypto::AES_CCM_decrypt(encryptedData.data(), encryptedData.size(), nullptr, 0, mic.data(), mic.size(), key, nonce.data(),
                                  nonce.size(), appData.data());

    ReturnErrorOnFailure(err);

    counter = Encoding::LittleEndian::Get32(appData.data());
    // Shift to remove the counter from the appData
    memcpy(appData.data(), appData.data() + sizeof(uint32_t), appDataSize);
    return err;
}

uint16_t CheckinMessage::GetAppDataSize(ByteSpan & payload)
{
    return (payload.size() < sMinPayloadSize) ? 0 : static_cast<uint16_t>(payload.size() - sMinPayloadSize);
}

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
