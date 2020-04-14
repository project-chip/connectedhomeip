/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      Header that exposes the platform agnostic CHIP crypto primitives
 */

#ifndef _CHIP_CRYPTO_PAL_H_
#define _CHIP_CRYPTO_PAL_H_

#include <core/CHIPError.h>
#include <stddef.h>

namespace chip {
namespace Crypto {

/**
 * @brief A function that implements 256-bit AES-CCM encryption
 * @param plaintext Plaintext to encrypt
 * @param plaintext_length Length of plain_text
 * @param aad Additional authentication data
 * @param aad_length Length of additional authentication data
 * @param key Encryption key
 * @param iv Initial vector
 * @param iv_length Length of initial vector
 * @param ciphertext Buffer to write ciphertext into. Caller must ensure this is large enough to hold the ciphertext
 * @param tag Buffer to write tag into. Caller must ensure this is large enough to hold the tag
 * @param tag_length Expected length of tag
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 * */
CHIP_ERROR AES_CCM_256_encrypt(const unsigned char * plaintext, size_t plaintext_length, const unsigned char * aad,
                               size_t aad_length, const unsigned char * key, const unsigned char * iv, size_t iv_length,
                               unsigned char * ciphertext, unsigned char * tag, size_t tag_length);

/**
 * @brief A function that implements 256-bit AES-CCM decryption
 * @param ciphertext Ciphertext to decrypt
 * @param ciphertext_length Length of ciphertext
 * @param aad Additional authentical data.
 * @param aad_length Length of additional authentication data
 * @param tag Tag to use to decrypt
 * @param tag_length Length of tag
 * @param key Decryption key
 * @param iv Initial vector
 * @param iv_length Length of initial vector
 * @param plaintext Buffer to write plaintext into
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/

CHIP_ERROR AES_CCM_256_decrypt(const unsigned char * ciphertext, size_t ciphertext_length, const unsigned char * aad,
                               size_t aad_length, const unsigned char * tag, size_t tag_length, const unsigned char * key,
                               const unsigned char * iv, size_t iv_length, unsigned char * plaintext);

/**
 * @brief A function that implements SHA-256 based HKDF
 * @param secret The secret to use as the key to the HKDF
 * @param secret_length Length of the secret
 * @param salt Optional salt to use as input to the HKDF
 * @param salt_length Length of the salt
 * @param info Optional info to use as input to the HKDF
 * @param info_length Length of the info
 * @param out_buffer Pointer to buffer to write output into.
 * @param out_length Resulting length of out_buffer
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/

CHIP_ERROR HKDF_SHA256(const unsigned char * secret, const size_t secret_length, const unsigned char * salt,
                       const size_t salt_length, const unsigned char * info, const size_t info_length, unsigned char * out_buffer,
                       size_t out_length);

/**
 * @brief A cryptogaphically secure random number generator based on NIST SP800-90A
 * @param out_buffer Buffer to write random bytes into
 * @param out_length Number of random bytes to generate
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR DRBG_get_bytes(unsigned char * out_buffer, const size_t out_length);

} // namespace Crypto
} // namespace chip
#endif
