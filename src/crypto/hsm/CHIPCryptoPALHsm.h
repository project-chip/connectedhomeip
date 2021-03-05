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

#ifndef _CHIP_CRYPTO_PAL_HSM_H_
#define _CHIP_CRYPTO_PAL_HSM_H_

#include "CHIPCryptoPALHsm_config.h"

#if ENABLE_HSM_AES_CCM_ENCRYPT

/**
 * @brief A function that implements AES-CCM encryption using HSM
 * */
CHIP_ERROR AES_CCM_encrypt_HSM(const uint8_t * plaintext, size_t plaintext_length, const uint8_t * aad, size_t aad_length,
                               const uint8_t * key, size_t key_length, const uint8_t * iv, size_t iv_length, uint8_t * ciphertext,
                               uint8_t * tag, size_t tag_length);

#endif //#if ENABLE_HSM_AES_CCM_ENCRYPT

#if ENABLE_HSM_AES_CCM_DECRYPT

/**
 * @brief A function that implements AES-CCM decryption using HSM
 * */
CHIP_ERROR AES_CCM_decrypt_HSM(const uint8_t * ciphertext, size_t ciphertext_len, const uint8_t * aad, size_t aad_len,
                               const uint8_t * tag, size_t tag_length, const uint8_t * key, size_t key_length, const uint8_t * iv,
                               size_t iv_length, uint8_t * plaintext);

#endif //#if ENABLE_HSM_AES_CCM_DECRYPT

#if ((ENABLE_HSM_HASH_SHA256) || (ENABLE_HSM_GENERATE_EC_KEY))

/**
 * @brief A function that implements SHA-256 hash using HSM
 **/
CHIP_ERROR Hash_SHA256_HSM(const uint8_t * data, const size_t data_length, uint8_t * out_buffer);

#endif //#if ( (ENABLE_HSM_HASH_SHA256) || (ENABLE_HSM_GENERATE_EC_KEY) )

#if ENABLE_HSM_HASH_SHA256_MULTISTEP

/**
 * @brief Init SHA256 multistep using HSM
 **/
CHIP_ERROR Hash_SHA256_stream_Begin_HSM(void);

/**
 * @brief Update data for SHA256 multistep using HSM
 **/
CHIP_ERROR Hash_SHA256_stream_AddData_HSM(const uint8_t * data, const size_t data_length);

/**
 * @brief Finish SHA256 multistep using HSM
 **/
CHIP_ERROR Hash_SHA256_stream_Finish_HSM(uint8_t * out_buffer);

/**
 * @brief Clear SHA256 multistep context
 **/
void Hash_SHA256_stream_Clear_HSM(void);

#endif //#if ENABLE_HSM_HASH_SHA256_MULTISTEP

#if ENABLE_HSM_HKDF_SHA256

/**
 * @brief A function that implements SHA-256 based HKDF using HSM
 **/
CHIP_ERROR HKDF_SHA256_HSM(const uint8_t * secret, const size_t secret_length, const uint8_t * salt, const size_t salt_length,
                           const uint8_t * info, const size_t info_length, uint8_t * out_buffer, size_t out_length);

#endif //#if ENABLE_HSM_HKDF_SHA256

#if ENABLE_HSM_RAND_GEN

/**
 * @brief Generate random numbers using HSM
 **/
CHIP_ERROR DRBG_get_bytes_HSM(uint8_t * out_buffer, const size_t out_length);

#endif //#if ENABLE_HSM_RAND_GEN

#if ENABLE_HSM_GENERATE_EC_KEY

/**
 * @brief Generate NIST256P Signature on input msg using HSM
 **/
CHIP_ERROR P256Keypair_ECDSA_sign_msg_HSM(const uint8_t * msg, const size_t msg_length, uint8_t * out_signature, size_t * siglen,
                                          int keyid);

/**
 * @brief Generate NIST256P Signature on input hash using HSM
 **/
CHIP_ERROR P256Keypair_ECDSA_sign_hash_HSM(const uint8_t * hash, const size_t hash_length, uint8_t * out_signature, size_t * siglen,
                                           int keyid);

/**
 * @brief Verify NIST256P Signature using HSM
 **/
CHIP_ERROR P256PublicKey_ECDSA_validate_msg_signature_HSM(const uint8_t * msg, const size_t msg_length,
                                                          const uint8_t * out_signature, size_t siglen, int keyid);

/**
 * @brief Verify NIST256P Signature using HSM
 **/
CHIP_ERROR P256PublicKey_ECDSA_validate_hash_signature_HSM(const uint8_t * hash, const size_t hash_length,
                                                           const uint8_t * out_signature, size_t siglen, int keyid);

/**
 * @brief ECDH derive key using HSM
 **/
CHIP_ERROR P256Keypair_ECDH_derive_secret_HSM(const uint8_t * pubKey, size_t pubKeyLen, int priv_keyid, uint8_t * out_secret,
                                              size_t * secret_length);

/**
 * @brief Generate NIST256P key inside HSM
 **/
CHIP_ERROR P256Keypair_Initialize_HSM(uint32_t keyid, uint8_t * pubkey, size_t * pbKeyLen);

/**
 * @brief Delete NIST256P key inside HSM
 **/
void P256Keypair_deleteKey_HSM(int keyid);

#endif //#if ENABLE_HSM_GENERATE_EC_KEY

#if ENABLE_HSM_MAC

/**
 * @brief Perform HMAC on input data using HSM
 **/
CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC_Mac_HSM(const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len,
                                                 uint8_t * out);

#endif //#if ENABLE_HSM_MAC

#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))
/**
 * @brief Init Spake2+ on HSM.
 **/
CHIP_ERROR Spake2p_Init_HSM(const uint8_t * context, size_t context_len);

#endif //#if ( (ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER) )

#if ENABLE_HSM_SPAKE_VERIFIER

/**
 * @brief Start the Spake2+ process on HSM as a verifier (i.e. an accessory being provisioned).
 **/
CHIP_ERROR Spake2p_BeginVerifier_HSM(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                     size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                                     size_t Lin_len);

#endif //#if ENABLE_HSM_SPAKE_VERIFIER

#if ENABLE_HSM_SPAKE_PROVER

/**
 * @brief Start the Spake2+ process on HSM as a prover (i.e. a commissioner).
 **/
CHIP_ERROR Spake2p_BeginProver_HSM(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                   size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in,
                                   size_t w1in_len);

#endif //#if ENABLE_HSM_SPAKE_PROVER

#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))

/**
 * @brief Compute the first round of the protocol on HSM.
 **/
CHIP_ERROR Spake2p_ComputeRoundOne_HSM(chip::Crypto::CHIP_SPAKE2P_ROLE role, const uint8_t * pab, size_t pab_len, uint8_t * out,
                                       size_t * out_len);

/**
 * @brief Compute the second round of the protocol on HSM.
 **/
CHIP_ERROR Spake2p_ComputeRoundTwo_HSM(chip::Crypto::CHIP_SPAKE2P_ROLE role, const uint8_t * in, size_t in_len, uint8_t * out,
                                       size_t * out_len, uint8_t * pKeyKe, size_t * pkeyKeLen);

/**
 * @brief Confirm that each party computed the same keys using HSM.
 **/
CHIP_ERROR Spake2p_KeyConfirm_HSM(chip::Crypto::CHIP_SPAKE2P_ROLE role, const uint8_t * in, size_t in_len);

/**
 * @brief Return the shared secret.
 **/
void Spake2p_Finish_HSM(chip::Crypto::CHIP_SPAKE2P_ROLE role);

#endif //#if ( (ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER) )

#endif //#ifndef _CHIP_CRYPTO_PAL_HSM_H_
