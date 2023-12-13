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

CHIP_ERROR CheckinMessage::GenerateCheckinMessagePayload(const Crypto::Aes128KeyHandle & aes128KeyHandle,
                                                         const Crypto::Hmac128KeyHandle & hmacKeyHandle,
                                                         const CounterType & counter, const ByteSpan & appData,
                                                         MutableByteSpan & output)
{
    VerifyOrReturnError(output.size() >= (appData.size() + sMinPayloadSize), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Generate Nonce from Key and counter value
    {
        MutableByteSpan nonceByteSpan = output.SubSpan(0, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
        Encoding::LittleEndian::BufferWriter writer(nonceByteSpan);

        ReturnErrorOnFailure(GenerateCheckInMessageNonce(hmacKeyHandle, counter, writer));
    }

    // Encrypt Counter and Application Data
    {
        size_t applicationSize          = sizeof(CounterType) + appData.size();
        MutableByteSpan payloadByteSpan = output.SubSpan(CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, applicationSize);
        Encoding::LittleEndian::BufferWriter payloadWriter(payloadByteSpan);

        payloadWriter.EndianPut(counter, sizeof(counter));
        payloadWriter.Put(appData.data(), appData.size());

        MutableByteSpan micByteSpan =
            output.SubSpan(CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + applicationSize, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

        ReturnErrorOnFailure(Crypto::AES_CCM_encrypt(payloadWriter.Buffer(), applicationSize, nullptr, 0, aes128KeyHandle,
                                                     output.data(), CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, payloadWriter.Buffer(),
                                                     micByteSpan.data(), CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));
    }

    output.reduce_size(appData.size() + sMinPayloadSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckinMessage::ParseCheckinMessagePayload(const Crypto::Aes128KeyHandle & aes128KeyHandle,
                                                      const Crypto::Hmac128KeyHandle & hmacKeyHandle, ByteSpan & payload,
                                                      CounterType & counter, MutableByteSpan & appData)
{
    size_t appDataSize = GetAppDataSize(payload);

    VerifyOrReturnError(payload.size() >= sMinPayloadSize, CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    // To prevent workbuffer usage, appData size needs to be large enough to hold both the appData and the counter
    VerifyOrReturnError(appData.size() >= sizeof(CounterType) + appDataSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Decrypt received data
    {
        ByteSpan nonce         = payload.SubSpan(0, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
        ByteSpan encryptedData = payload.SubSpan(CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, sizeof(CounterType) + appDataSize);
        ByteSpan mic           = payload.SubSpan(CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + sizeof(CounterType) + appDataSize,
                                                 CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

        ReturnErrorOnFailure(Crypto::AES_CCM_decrypt(encryptedData.data(), encryptedData.size(), nullptr, 0, mic.data(), mic.size(),
                                                     aes128KeyHandle, nonce.data(), nonce.size(), appData.data()));
    }

    // Read decrypted counter and application data
    counter = Encoding::LittleEndian::Get32(appData.data());
    // Shift to remove the counter from the appData
    memmove(appData.data(), sizeof(CounterType) + appData.data(), appDataSize);
    appData.reduce_size(appDataSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckinMessage::GenerateCheckInMessageNonce(const Crypto::Hmac128KeyHandle & hmacKeyHandle, CounterType counter,
                                                       Encoding::LittleEndian::BufferWriter & writer)
{
    VerifyOrReturnError(writer.Available() >= CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t hashWorkBuffer[CHIP_CRYPTO_HASH_LEN_BYTES] = { 0 };

    chip::Crypto::HMAC_sha shaHandler;
    ReturnErrorOnFailure(shaHandler.HMAC_SHA256(hmacKeyHandle, reinterpret_cast<uint8_t *>(&counter), sizeof(CounterType),
                                                hashWorkBuffer, CHIP_CRYPTO_HASH_LEN_BYTES));

    writer.Put(hashWorkBuffer, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
    VerifyOrReturnError(writer.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    return CHIP_NO_ERROR;
}

size_t CheckinMessage::GetAppDataSize(ByteSpan & payload)
{
    return (payload.size() <= sMinPayloadSize) ? 0 : payload.size() - sMinPayloadSize;
}

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
