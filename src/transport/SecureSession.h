/*
 *
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
 *      This file defines the CHIP Secure Session object that provides
 *      APIs for encrypting/decryting data using cryptographic keys.
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <crypto/CHIPCryptoPAL.h>
#include <transport/raw/MessageHeader.h>

namespace chip {

class DLL_EXPORT SecureSession
{
public:
    SecureSession();
    SecureSession(SecureSession &&)      = default;
    SecureSession(const SecureSession &) = default;
    SecureSession & operator=(const SecureSession &) = default;
    SecureSession & operator=(SecureSession &&) = default;

    /**
     * @brief
     *   Derive a shared key. The derived key will be used for encryting/decrypting
     *   data exchanged on the secure channel.
     *
     * @param local_keypair      A pointer to local ECP keypair
     * @param remote_public_key  A pointer to peer's public key
     * @param salt               A pointer to the initial salt used for deriving the keys
     * @param salt_length        Length of the initial salt
     * @param info               A pointer to the initial info
     * @param info_length        Length of the initial info
     * @return CHIP_ERROR        The result of key derivation
     */
    CHIP_ERROR Init(const Crypto::P256Keypair & local_keypair, const Crypto::P256PublicKey & remote_public_key,
                    const uint8_t * salt, size_t salt_length, const uint8_t * info, size_t info_length);

    /**
     * @brief
     *   Derive a shared key. The derived key will be used for encryting/decrypting
     *   data exchanged on the secure channel.
     *
     * @param secret             A pointer to the shared secret
     * @param secret_length      Length of the shared secret
     * @param salt               A pointer to the initial salt used for deriving the keys
     * @param salt_length        Length of the initial salt
     * @param info               A pointer to the initial info
     * @param info_length        Length of the initial info
     * @return CHIP_ERROR        The result of key derivation
     */
    CHIP_ERROR InitFromSecret(const uint8_t * secret, size_t secret_length, const uint8_t * salt, size_t salt_length,
                              const uint8_t * info, size_t info_length);

    /**
     * @brief
     *   Encrypt the input data using keys established in the secure channel
     *
     * @param input Unencrypted input data
     * @param input_length Length of the input data
     * @param output Output buffer for encrypted data
     * @param header message header structure. Encryption type will be set on the header.
     * @param mac - output the resulting mac
     *
     * @return CHIP_ERROR The result of encryption
     */
    CHIP_ERROR Encrypt(const uint8_t * input, size_t input_length, uint8_t * output, PacketHeader & header,
                       MessageAuthenticationCode & mac);

    /**
     * @brief
     *   Decrypt the input data using keys established in the secure channel
     *
     * @param input Encrypted input data
     * @param input_length Length of the input data
     * @param output Output buffer for decrypted data
     * @param header message header structure
     * @return CHIP_ERROR The result of decryption
     * @param mac Input mac
     */
    CHIP_ERROR Decrypt(const uint8_t * input, size_t input_length, uint8_t * output, const PacketHeader & header,
                       const MessageAuthenticationCode & mac);

    /**
     * @brief
     *   Memory overhead of encrypting data. The overhead is indepedent of size of
     *   the data being encrypted. The extra space is used for storing the common header.
     *
     * @return number of bytes.
     */
    size_t EncryptionOverhead();

    /**
     * Clears the internal state of secure session back to the state of a new object.
     */
    void Reset();

private:
    static constexpr size_t kAES_CCM128_Key_Length = 16;

    bool mKeyAvailable;
    uint8_t mKey[kAES_CCM128_Key_Length];

    static CHIP_ERROR GetIV(const PacketHeader & header, uint8_t * iv, size_t len);

    // Use unencrypted header as additional authenticated data (AAD) during encryption and decryption.
    // The encryption operations includes AAD when message authentication tag is generated. This tag
    // is used at the time of decryption to integrity check the received data.
    static CHIP_ERROR GetAdditionalAuthData(const PacketHeader & header, uint8_t * aad, uint16_t & len);
};

} // namespace chip
