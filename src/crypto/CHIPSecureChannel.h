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
 *      This file defines the CHIP Secure Channel object that provides
 *      APIs for encrypting/decryting data using cryptographic keys.
 *
 */

#ifndef __CHIPSECURECHANNEL_H__
#define __CHIPSECURECHANNEL_H__

#include <core/CHIPConfig.h>
#include <core/CHIPError.h>
#include <system/SystemLayer.h>

namespace chip {

class DLL_EXPORT ChipSecureChannel
{
public:
    /**
     * @brief
     *   Initialization parameters of secure channel
     *
     * @param remote_public_key  A pointer to peer's public key
     * @param public_key_length  Length of remote_public_key
     * @param local_private_key  A pointer to local private key
     * @param private_key_length Length of local_private_key
     * @param salt               Salt used for key derivation
     * @param salt_length        Length of salt
     * @param info               Info used for key derivation
     * @param info_length        Length of info
     */
    typedef struct
    {
        const unsigned char * remote_public_key;
        size_t public_key_length;
        const unsigned char * local_private_key;
        size_t private_key_length;
        const unsigned char * salt;
        size_t salt_length;
        const unsigned char * info;
        size_t info_length;
    } secure_channel_params_t;

    /**
     * @brief
     *   Derive a shared key. The derived key will be used for encryting/decrypting
     *   data exchanged on the secure channel.
     *
     * @param parameters  A pointer to channel parameters
     * @return CHIP_ERROR        The result of key derivation
     */
    CHIP_ERROR Init(const secure_channel_params_t * parameters);

    /**
     * @brief
     *   Close the channel, and release its resources. The released channel can be
     *   reused for a new security context by calling Init().
     */
    void Close(void);

    /**
     * @brief
     *   Encrypt the input data using keys established in the secure channel
     *
     * @param input Unencrypted input data
     * @param input_length Length of the input data
     * @param output Output buffer for encrypted data
     * @param output_length Length of the output buffer
     * @return CHIP_ERROR The result of encryption
     */
    CHIP_ERROR Encrypt(const unsigned char * input, size_t input_length, unsigned char * output, size_t output_length);

    /**
     * @brief
     *   Decrypt the input data using keys established in the secure channel
     *
     * @param input Encrypted input data
     * @param input_length Length of the input data
     * @param output Output buffer for decrypted data
     * @param output_length Length of the output buffer
     * @return CHIP_ERROR The result of decryption
     */
    CHIP_ERROR Decrypt(const unsigned char * input, size_t input_length, unsigned char * output, size_t & output_length);

    /**
     * @brief
     *   Memory overhead of encrypting data. The overhead is indepedent of size of
     *   the data being encrypted. The extra space is used for storing the common header.
     *
     * @return number of bytes.
     */
    size_t EncryptionOverhead(void);

    ChipSecureChannel(void);

private:
    static const size_t kAES_CCM128_Key_Length = 16;

    typedef struct
    {
        size_t payload_length;
        uint64_t IV;
        uint64_t tag;
    } security_header_t;

    bool mKeyAvailable;
    uint64_t mNextIV;
    uint8_t mKey[kAES_CCM128_Key_Length];
};

} // namespace chip

#endif // __CHIPSECURECHANNEL_H__
