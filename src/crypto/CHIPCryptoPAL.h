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

const size_t kMax_ECDSA_Signature_Length = 72;
const size_t kMax_ECDH_Secret_Length     = 32;

/**
 * @brief A function that implements AES-CCM encryption
 * @param plaintext Plaintext to encrypt
 * @param plaintext_length Length of plain_text
 * @param aad Additional authentication data
 * @param aad_length Length of additional authentication data
 * @param key Encryption key
 * @param key_length Length of encryption key (in bytes)
 * @param iv Initial vector
 * @param iv_length Length of initial vector
 * @param ciphertext Buffer to write ciphertext into. Caller must ensure this is large enough to hold the ciphertext
 * @param tag Buffer to write tag into. Caller must ensure this is large enough to hold the tag
 * @param tag_length Expected length of tag
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 * */
CHIP_ERROR AES_CCM_encrypt(const unsigned char * plaintext, size_t plaintext_length, const unsigned char * aad, size_t aad_length,
                           const unsigned char * key, size_t key_length, const unsigned char * iv, size_t iv_length,
                           unsigned char * ciphertext, unsigned char * tag, size_t tag_length);

/**
 * @brief A function that implements AES-CCM decryption
 * @param ciphertext Ciphertext to decrypt
 * @param ciphertext_length Length of ciphertext
 * @param aad Additional authentical data.
 * @param aad_length Length of additional authentication data
 * @param tag Tag to use to decrypt
 * @param tag_length Length of tag
 * @param key Decryption key
 * @param key_length Length of Decryption key (in bytes)
 * @param iv Initial vector
 * @param iv_length Length of initial vector
 * @param plaintext Buffer to write plaintext into
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/

CHIP_ERROR AES_CCM_decrypt(const unsigned char * ciphertext, size_t ciphertext_length, const unsigned char * aad, size_t aad_length,
                           const unsigned char * tag, size_t tag_length, const unsigned char * key, size_t key_length,
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
 * @brief A cryptographically secure random number generator based on NIST SP800-90A
 * @param out_buffer Buffer to write random bytes into
 * @param out_length Number of random bytes to generate
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR DRBG_get_bytes(unsigned char * out_buffer, const size_t out_length);

/**
 * @brief A function to sign a msg using ECDSA
 * @param msg Message that needs to be signed
 * @param msg_length Length of message
 * @param private_key Key to use to sign the message. Private keys are ASN.1 DER encoded as padded big-endian field elements as
 *described in SEC 1: Elliptic Curve Cryptography [https://www.secg.org/sec1-v2.pdf]
 * @param private_key_length Length of private key
 * @param out_signature Buffer that will hold the output signature. The signature consists of: 2 EC elements (r and s), represented
 *as ASN.1 DER integers, plus the ASN.1 sequence Header
 * @param out_signature_length Length of out buffer
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR ECDSA_sign_msg(const unsigned char * msg, const size_t msg_length, const unsigned char * private_key,
                          const size_t private_key_length, unsigned char * out_signature, size_t & out_signature_length);

/**
 * @brief A function to sign a msg using ECDSA
 * @param msg Message that needs to be signed
 * @param msg_length Length of message
 * @param public_key Key to use to verify the message signature. Public keys are ASN.1 DER encoded as uncompressed points as
 *described in SEC 1: Elliptic Curve Cryptography [https://www.secg.org/sec1-v2.pdf]
 * @param private_key_length Length of public key
 * @param signature Signature to use for verification. The signature consists of: 2 EC elements (r and s), represented as ASN.1 DER
 *integers, plus the ASN.1 sequence Header
 * @param signature_length Length of signature
 * @return Returns a CHIP_NO_ERROR on successful verification, a CHIP_ERROR otherwise
 **/
CHIP_ERROR ECDSA_validate_msg_signature(const unsigned char * msg, const size_t msg_length, const unsigned char * public_key,
                                        const size_t public_key_length, const unsigned char * signature,
                                        const size_t signature_length);

/** @brief A function to derive a shared secret using ECDH
 * @param remote_public_key Public key of remote peer with which we are trying to establish secure channel. remote_public_key is
 *ASN.1 DER encoded as padded big-endian field elements as described in SEC 1: Elliptic Curve Cryptography
 *[https://www.secg.org/sec1-v2.pdf]
 * @param remote_public_key_length Length of remote_public_key
 * @param local_private_key Local private key. local_private_key is ASN.1 DER encoded as padded big-endian field elements as
 *described in SEC 1: Elliptic Curve Cryptography [https://www.secg.org/sec1-v2.pdf]
 * @param local_private_key_length Length of private_key_length
 * @param out_secret Buffer to write out secret into. This is a byte array representing the x coordinate of the shared secret.
 * @param out_secret_length Length of out_secret
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR ECDH_derive_secret(const unsigned char * remote_public_key, const size_t remote_public_key_length,
                              const unsigned char * local_private_key, const size_t local_private_key_length,
                              unsigned char * out_secret, size_t & out_secret_length);

/** @brief Entropy callback function
 * @param data Callback-specific data pointer
 * @param output Output data to fill
 * @param len Length of output buffer
 * @param olen The actual amount of data that was written to output buffer
 * @return 0 if success
 */
typedef int (*CHIPEntropySource)(void * data, unsigned char * output, size_t len, size_t * olen);

/** @brief A function to add entropy sources to crypto library
 * @param fn_source Function pointer to the entropy source
 * @param p_source  Data that should be provided when fn_source is called
 * @param threshold Minimum required from source before entropy is released
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR add_entropy_source(CHIPEntropySource fn_source, void * p_source, size_t threshold);
} // namespace Crypto
} // namespace chip

#endif
