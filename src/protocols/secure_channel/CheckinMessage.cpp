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

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <protocols/secure_channel/CheckinMessage.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {

CHIP_ERROR CheckinMessage::GenerateCheckinMessagePayload(const Crypto::Aes128KeyHandle & aes128KeyHandle,
                                                         const Crypto::Hmac128KeyHandle & hmacKeyHandle,
                                                         const CounterType & counter, const ByteSpan & appData,
                                                         MutableByteSpan & output)
{
    VerifyOrReturnError(output.size() >= (appData.size() + kMinPayloadSize), CHIP_ERROR_BUFFER_TOO_SMALL);
    size_t cursorIndex = 0;

    // Generate Nonce from Key and counter value
    {
        MutableByteSpan nonce = output.SubSpan(0, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
        cursorIndex += nonce.size();

        Encoding::LittleEndian::BufferWriter writer(nonce);
        ReturnErrorOnFailure(GenerateCheckInMessageNonce(hmacKeyHandle, counter, writer));
    }

    // Encrypt Counter and Application Data
    {
        MutableByteSpan payloadByteSpan = output.SubSpan(cursorIndex, sizeof(CounterType) + appData.size());
        cursorIndex += payloadByteSpan.size();

        Encoding::LittleEndian::BufferWriter payloadWriter(payloadByteSpan);

        payloadWriter.EndianPut(counter, sizeof(counter));
        payloadWriter.Put(appData.data(), appData.size());
        VerifyOrReturnError(payloadWriter.Fit(), CHIP_ERROR_INTERNAL);

        MutableByteSpan mic = output.SubSpan(cursorIndex, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);
        cursorIndex += mic.size();

        // Validate that the cursorIndex is within the available output space
        VerifyOrReturnError(cursorIndex <= output.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        // Validate that the cursorIndex matchs the message length
        VerifyOrReturnError(cursorIndex == appData.size() + kMinPayloadSize, CHIP_ERROR_INTERNAL);

        ReturnErrorOnFailure(Crypto::AES_CCM_encrypt(payloadByteSpan.data(), payloadByteSpan.size(), nullptr, 0, aes128KeyHandle,
                                                     output.data(), CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES, payloadByteSpan.data(),
                                                     mic.data(), mic.size()));
    }

    output.reduce_size(appData.size() + kMinPayloadSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckinMessage::ParseCheckinMessagePayload(const Crypto::Aes128KeyHandle & aes128KeyHandle,
                                                      const Crypto::Hmac128KeyHandle & hmacKeyHandle, const ByteSpan & payload,
                                                      CounterType & counter, MutableByteSpan & appData)
{
    size_t appDataSize = GetAppDataSize(payload);

    VerifyOrReturnError(payload.size() >= kMinPayloadSize, CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    // To prevent workbuffer usage, appData size needs to be large enough to hold both the appData and the counter
    VerifyOrReturnError(appData.size() >= sizeof(CounterType) + appDataSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Decrypt received data
    {
        size_t cursorIndex = 0;

        ByteSpan nonce = payload.SubSpan(cursorIndex, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
        cursorIndex += nonce.size();

        ByteSpan encryptedData = payload.SubSpan(cursorIndex, sizeof(CounterType) + appDataSize);
        cursorIndex += encryptedData.size();

        ByteSpan mic = payload.SubSpan(cursorIndex, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);
        cursorIndex += mic.size();

        // Return Invalid message length since the payload isn't the right size
        VerifyOrReturnError(cursorIndex == payload.size(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);

        ReturnErrorOnFailure(Crypto::AES_CCM_decrypt(encryptedData.data(), encryptedData.size(), nullptr, 0, mic.data(), mic.size(),
                                                     aes128KeyHandle, nonce.data(), nonce.size(), appData.data()));
    }

    // Read decrypted counter and application data
    static_assert(sizeof(CounterType) == sizeof(uint32_t), "Expect counter to be 32 bits for correct decoding");
    CounterType tempCounter = Encoding::LittleEndian::Get32(appData.data());

    // Validate that the received nonce is correct
    {
        uint8_t calculatedNonceBuffer[CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES] = { 0 };
        Encoding::LittleEndian::BufferWriter writer(calculatedNonceBuffer, sizeof(calculatedNonceBuffer));

        ReturnErrorOnFailure(GenerateCheckInMessageNonce(hmacKeyHandle, tempCounter, writer));

        // Validate received nonce is the same as the calculated
        ByteSpan nonce = payload.SubSpan(0, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
        VerifyOrReturnError(memcmp(nonce.data(), calculatedNonceBuffer, sizeof(calculatedNonceBuffer)) == 0, CHIP_ERROR_INTERNAL);
    }

    // We have successfully decrypted and validated Check-In message
    // Set output values

    counter = tempCounter;
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
    uint8_t counterBuffer[sizeof(CounterType)];

    // validate that Check-In counter is a uint32_t
    static_assert(sizeof(CounterType) == sizeof(uint32_t), "Expect counter to be 32 bits for correct encoding");
    Encoding::LittleEndian::Put32(counterBuffer, counter);

    chip::Crypto::HMAC_sha shaHandler;
    ReturnErrorOnFailure(
        shaHandler.HMAC_SHA256(hmacKeyHandle, counterBuffer, sizeof(CounterType), hashWorkBuffer, CHIP_CRYPTO_HASH_LEN_BYTES));

    writer.Put(hashWorkBuffer, CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES);
    VerifyOrReturnError(writer.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    return CHIP_NO_ERROR;
}

size_t CheckinMessage::GetAppDataSize(const ByteSpan & payload)
{
    return (payload.size() <= kMinPayloadSize) ? 0 : payload.size() - kMinPayloadSize;
}

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
