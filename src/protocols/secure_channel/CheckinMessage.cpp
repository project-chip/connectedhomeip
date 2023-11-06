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

#include <lib/core/CHIPEncoding.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {

CHIP_ERROR CheckinMessage::GenerateCheckinMessagePayload(Crypto::Aes128KeyHandle & key, CounterType counter,
                                                         const ByteSpan & appData, MutableByteSpan & output)
{
    VerifyOrReturnError(appData.size() <= sMaxAppDataSize, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(output.size() >= (appData.size() + sMinPayloadSize), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err            = CHIP_NO_ERROR;
    uint8_t * appDataStartPtr = output.data() + CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES;
    Encoding::LittleEndian::Put32(appDataStartPtr, counter);

    chip::Crypto::HMAC_sha shaHandler;
    uint8_t nonceWorkBuffer[CHIP_CRYPTO_HASH_LEN_BYTES] = { 0 };

    ReturnErrorOnFailure(shaHandler.HMAC_SHA256(key.As<Aes128KeyByteArray>(), sizeof(Aes128KeyByteArray), appDataStartPtr,
                                                sizeof(CounterType), nonceWorkBuffer, CHIP_CRYPTO_HASH_LEN_BYTES));

    static_assert(sizeof(nonceWorkBuffer) >= CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, "We're reading off the end of our buffer.");
    memcpy(output.data(), nonceWorkBuffer, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);

    // In place encryption to save some RAM
    memcpy(appDataStartPtr + sizeof(CounterType), appData.data(), appData.size());

    uint8_t * micPtr = appDataStartPtr + sizeof(CounterType) + appData.size();
    ReturnErrorOnFailure(Crypto::AES_CCM_encrypt(appDataStartPtr, sizeof(CounterType) + appData.size(), nullptr, 0, key,
                                                 output.data(), CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, appDataStartPtr, micPtr,
                                                 CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));

    output.reduce_size(appData.size() + sMinPayloadSize);

    return err;
}

CHIP_ERROR CheckinMessage::ParseCheckinMessagePayload(Crypto::Aes128KeyHandle & key, ByteSpan & payload, CounterType & counter,
                                                      MutableByteSpan & appData)
{
    VerifyOrReturnError(payload.size() >= sMinPayloadSize, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(payload.size() <= (sMinPayloadSize + sMaxAppDataSize), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err     = CHIP_NO_ERROR;
    size_t appDataSize = GetAppDataSize(payload);

    // To prevent workbuffer usage, appData size needs to be large enough to hold both the appData and the counter
    VerifyOrReturnError(appData.size() >= sizeof(CounterType) + appDataSize, CHIP_ERROR_INVALID_ARGUMENT);

    ByteSpan nonce         = payload.SubSpan(0, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
    ByteSpan encryptedData = payload.SubSpan(CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, sizeof(CounterType) + appDataSize);
    ByteSpan mic =
        payload.SubSpan(CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + sizeof(CounterType) + appDataSize, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    err = Crypto::AES_CCM_decrypt(encryptedData.data(), encryptedData.size(), nullptr, 0, mic.data(), mic.size(), key, nonce.data(),
                                  nonce.size(), appData.data());

    ReturnErrorOnFailure(err);

    counter = Encoding::LittleEndian::Get32(appData.data());
    // Shift to remove the counter from the appData
    memmove(appData.data(), sizeof(CounterType) + appData.data(), appDataSize);

    appData.reduce_size(appDataSize);
    return err;
}

size_t CheckinMessage::GetAppDataSize(ByteSpan & payload)
{
    return (payload.size() <= sMinPayloadSize) ? 0 : payload.size() - sMinPayloadSize;
}

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
