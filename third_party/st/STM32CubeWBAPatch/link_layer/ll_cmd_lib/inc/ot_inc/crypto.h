/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/ot_inc/crypto.h#1 $*/
/*
 *  Copyright (c) 2021, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * @brief
 *   This file includes the platform abstraction for Crypto operations.
 */

#ifndef OPENTHREAD_PLATFORM_CRYPTO_H_
#define OPENTHREAD_PLATFORM_CRYPTO_H_

#include <stdint.h>
#include <stdlib.h>

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup plat-crypto
 *
 * @brief
 *   This module includes the platform abstraction for Crypto.
 *
 * @{
 *
 */

/**
 * Defines the key types.
 *
 */
typedef enum
{
    OT_CRYPTO_KEY_TYPE_RAW,   ///< Key Type: Raw Data.
    OT_CRYPTO_KEY_TYPE_AES,   ///< Key Type: AES.
    OT_CRYPTO_KEY_TYPE_HMAC,  ///< Key Type: HMAC.
    OT_CRYPTO_KEY_TYPE_ECDSA, ///< Key Type: ECDSA.
} otCryptoKeyType;

/**
 * Defines the key algorithms.
 *
 */
typedef enum
{
    OT_CRYPTO_KEY_ALG_VENDOR,       ///< Key Algorithm: Vendor Defined.
    OT_CRYPTO_KEY_ALG_AES_ECB,      ///< Key Algorithm: AES ECB.
    OT_CRYPTO_KEY_ALG_HMAC_SHA_256, ///< Key Algorithm: HMAC SHA-256.
    OT_CRYPTO_KEY_ALG_ECDSA,        ///< Key Algorithm: ECDSA.
} otCryptoKeyAlgorithm;

/**
 * Defines the key usage flags.
 *
 */
enum
{
    OT_CRYPTO_KEY_USAGE_NONE        = 0,      ///< Key Usage: Key Usage is empty.
    OT_CRYPTO_KEY_USAGE_EXPORT      = 1 << 0, ///< Key Usage: Key can be exported.
    OT_CRYPTO_KEY_USAGE_ENCRYPT     = 1 << 1, ///< Key Usage: Encryption (vendor defined).
    OT_CRYPTO_KEY_USAGE_DECRYPT     = 1 << 2, ///< Key Usage: AES ECB.
    OT_CRYPTO_KEY_USAGE_SIGN_HASH   = 1 << 3, ///< Key Usage: Sign Hash.
    OT_CRYPTO_KEY_USAGE_VERIFY_HASH = 1 << 4, ///< Key Usage: Verify Hash.
};

/**
 * Defines the key storage types.
 *
 */
typedef enum
{
    OT_CRYPTO_KEY_STORAGE_VOLATILE,   ///< Key Persistence: Key is volatile.
    OT_CRYPTO_KEY_STORAGE_PERSISTENT, ///< Key Persistence: Key is persistent.
} otCryptoKeyStorage;

/**
 * This datatype represents the key reference.
 *
 */
typedef uint32_t otCryptoKeyRef;

/**
 * @struct otCryptoKey
 *
 * Represents the Key Material required for Crypto operations.
 *
 */
typedef struct otCryptoKey
{
    const uint8_t *mKey;       ///< Pointer to the buffer containing key. NULL indicates to use `mKeyRef`.
    uint16_t       mKeyLength; ///< The key length in bytes (applicable when `mKey` is not NULL).
    uint32_t       mKeyRef;    ///< The PSA key ref (requires `mKey` to be NULL).
} otCryptoKey;

/**
 * @struct otCryptoContext
 *
 * Stores the context object for platform APIs.
 *
 */
typedef struct otCryptoContext
{
    void    *mContext;     ///< Pointer to the context.
    uint16_t mContextSize; ///< The length of the context in bytes.
} otCryptoContext;

/**
 * Length of SHA256 hash (in bytes).
 *
 */
#define OT_CRYPTO_SHA256_HASH_SIZE 32

/**
 * @struct otPlatCryptoSha256Hash
 *
 * Represents a SHA-256 hash.
 *
 */
OT_TOOL_PACKED_BEGIN
struct otPlatCryptoSha256Hash
{
    uint8_t m8[OT_CRYPTO_SHA256_HASH_SIZE]; ///< Hash bytes.
} OT_TOOL_PACKED_END;

/**
 * Represents a SHA-256 hash.
 *
 */
typedef struct otPlatCryptoSha256Hash otPlatCryptoSha256Hash;

/**
 * Max buffer size (in bytes) for representing the EDCSA key-pair in DER format.
 *
 */
#define OT_CRYPTO_ECDSA_MAX_DER_SIZE 125

/**
 * @struct otPlatCryptoEcdsaKeyPair
 *
 * Represents an ECDSA key pair (public and private keys).
 *
 * The key pair is stored using Distinguished Encoding Rules (DER) format (per RFC 5915).
 *
 */
typedef struct otPlatCryptoEcdsaKeyPair
{
    uint8_t mDerBytes[OT_CRYPTO_ECDSA_MAX_DER_SIZE];
    uint8_t mDerLength;
} otPlatCryptoEcdsaKeyPair;

/**
 * Buffer size (in bytes) for representing the EDCSA public key.
 *
 */
#define OT_CRYPTO_ECDSA_PUBLIC_KEY_SIZE 64

/**
 * @struct otPlatCryptoEcdsaPublicKey
 *
 * Represents a ECDSA public key.
 *
 * The public key is stored as a byte sequence representation of an uncompressed curve point (RFC 6605 - sec 4).
 *
 */
OT_TOOL_PACKED_BEGIN
struct otPlatCryptoEcdsaPublicKey
{
    uint8_t m8[OT_CRYPTO_ECDSA_PUBLIC_KEY_SIZE];
} OT_TOOL_PACKED_END;

typedef struct otPlatCryptoEcdsaPublicKey otPlatCryptoEcdsaPublicKey;

/**
 * Buffer size (in bytes) for representing the EDCSA signature.
 *
 */
#define OT_CRYPTO_ECDSA_SIGNATURE_SIZE 64

/**
 * @struct otPlatCryptoEcdsaSignature
 *
 * Represents an ECDSA signature.
 *
 * The signature is encoded as the concatenated binary representation of two MPIs `r` and `s` which are calculated
 * during signing (RFC 6605 - section 4).
 *
 */
OT_TOOL_PACKED_BEGIN
struct otPlatCryptoEcdsaSignature
{
    uint8_t m8[OT_CRYPTO_ECDSA_SIGNATURE_SIZE];
} OT_TOOL_PACKED_END;

typedef struct otPlatCryptoEcdsaSignature otPlatCryptoEcdsaSignature;

/**
 * Max PBKDF2 SALT length: salt prefix (6) + extended panid (8) + network name (16)
 *
 */
#define OT_CRYPTO_PBDKF2_MAX_SALT_SIZE 30

/**
 * Initialize the Crypto module.
 *
 */
void otPlatCryptoInit(void);

/**
 * Import a key into PSA ITS.
 *
 * @param[in,out] aKeyRef           Pointer to the key ref to be used for crypto operations.
 * @param[in]     aKeyType          Key Type encoding for the key.
 * @param[in]     aKeyAlgorithm     Key algorithm encoding for the key.
 * @param[in]     aKeyUsage         Key Usage encoding for the key (combinations of `OT_CRYPTO_KEY_USAGE_*`).
 * @param[in]     aKeyPersistence   Key Persistence for this key
 * @param[in]     aKey              Actual key to be imported.
 * @param[in]     aKeyLen           Length of the key to be imported.
 *
 * @retval OT_ERROR_NONE          Successfully imported the key.
 * @retval OT_ERROR_FAILED        Failed to import the key.
 * @retval OT_ERROR_INVALID_ARGS  @p aKey was set to NULL.
 *
 * @note If OT_CRYPTO_KEY_STORAGE_PERSISTENT is passed for aKeyPersistence then @p aKeyRef is input and platform
 *       should use the given aKeyRef and MUST not change it.
 *
 *       If OT_CRYPTO_KEY_STORAGE_VOLATILE is passed for aKeyPersistence then @p aKeyRef is output, the initial
 *       value does not matter and platform API MUST update it to return the new key ref.
 *
 *       This API is only used by OT core when `OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE` is enabled.
 *
 */
otError otPlatCryptoImportKey(otCryptoKeyRef      *aKeyRef,
                              otCryptoKeyType      aKeyType,
                              otCryptoKeyAlgorithm aKeyAlgorithm,
                              int                  aKeyUsage,
                              otCryptoKeyStorage   aKeyPersistence,
                              const uint8_t       *aKey,
                              size_t               aKeyLen);

/**
 * Export a key stored in PSA ITS.
 *
 * @param[in]   aKeyRef           The key ref to be used for crypto operations.
 * @param[out]  aBuffer           Pointer to the buffer where key needs to be exported.
 * @param[in]   aBufferLen        Length of the buffer passed to store the exported key.
 * @param[out]  aKeyLen           Pointer to return the length of the exported key.
 *
 * @retval OT_ERROR_NONE          Successfully exported  @p aKeyRef.
 * @retval OT_ERROR_FAILED        Failed to export @p aKeyRef.
 * @retval OT_ERROR_INVALID_ARGS  @p aBuffer was NULL
 *
 * @note This API is only used by OT core when `OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE` is enabled.
 *
 */
otError otPlatCryptoExportKey(otCryptoKeyRef aKeyRef, uint8_t *aBuffer, size_t aBufferLen, size_t *aKeyLen);

/**
 * Destroy a key stored in PSA ITS.
 *
 * @param[in]   aKeyRef          The key ref to be destroyed
 *
 * @retval OT_ERROR_NONE          Successfully destroyed the key.
 * @retval OT_ERROR_FAILED        Failed to destroy the key.
 *
 * @note This API is only used by OT core when `OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE` is enabled.
 *
 */
otError otPlatCryptoDestroyKey(otCryptoKeyRef aKeyRef);

/**
 * Check if the key ref passed has an associated key in PSA ITS.
 *
 * @param[in]  aKeyRef          The Key Ref to check.
 *
 * @retval TRUE                 There is an associated key with @p aKeyRef.
 * @retval FALSE                There is no associated key with @p aKeyRef.
 *
 * @note This API is only used by OT core when `OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE` is enabled.
 *
 */
bool otPlatCryptoHasKey(otCryptoKeyRef aKeyRef);

/**
 * Initialize the HMAC operation.
 *
 * @param[in]  aContext          Context for HMAC operation.
 *
 * @retval OT_ERROR_NONE          Successfully initialized HMAC operation.
 * @retval OT_ERROR_FAILED        Failed to initialize HMAC operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 * @note The platform driver shall point the context to the correct object such as psa_mac_operation_t or
 *       mbedtls_md_context_t.
 *
 */
otError otPlatCryptoHmacSha256Init(otCryptoContext *aContext);

/**
 * Uninitialize the HMAC operation.
 *
 * @param[in]  aContext          Context for HMAC operation.
 *
 * @retval OT_ERROR_NONE          Successfully uninitialized HMAC operation.
 * @retval OT_ERROR_FAILED        Failed to uninitialized HMAC operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 */
otError otPlatCryptoHmacSha256Deinit(otCryptoContext *aContext);

/**
 * Start HMAC operation.
 *
 * @param[in]  aContext           Context for HMAC operation.
 * @param[in]  aKey               Key material to be used for HMAC operation.
 *
 * @retval OT_ERROR_NONE          Successfully started HMAC operation.
 * @retval OT_ERROR_FAILED        Failed to start HMAC operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aKey was NULL
 *
 */
otError otPlatCryptoHmacSha256Start(otCryptoContext *aContext, const otCryptoKey *aKey);

/**
 * Update the HMAC operation with new input.
 *
 * @param[in]  aContext           Context for HMAC operation.
 * @param[in]  aBuf               A pointer to the input buffer.
 * @param[in]  aBufLength         The length of @p aBuf in bytes.
 *
 * @retval OT_ERROR_NONE          Successfully updated HMAC with new input operation.
 * @retval OT_ERROR_FAILED        Failed to update HMAC operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aBuf was NULL
 *
 */
otError otPlatCryptoHmacSha256Update(otCryptoContext *aContext, const void *aBuf, uint16_t aBufLength);

/**
 * Complete the HMAC operation.
 *
 * @param[in]  aContext           Context for HMAC operation.
 * @param[out] aBuf               A pointer to the output buffer.
 * @param[in]  aBufLength         The length of @p aBuf in bytes.
 *
 * @retval OT_ERROR_NONE          Successfully completed HMAC operation.
 * @retval OT_ERROR_FAILED        Failed to complete HMAC operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aBuf was NULL
 *
 */
otError otPlatCryptoHmacSha256Finish(otCryptoContext *aContext, uint8_t *aBuf, size_t aBufLength);

/**
 * Initialise the AES operation.
 *
 * @param[in]  aContext           Context for AES operation.
 *
 * @retval OT_ERROR_NONE          Successfully Initialised AES operation.
 * @retval OT_ERROR_FAILED        Failed to Initialise AES operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 * @retval OT_ERROR_NO_BUFS       Cannot allocate the context.
 *
 * @note The platform driver shall point the context to the correct object such as psa_key_id
 *       or mbedtls_aes_context_t.
 *
 */
otError otPlatCryptoAesInit(otCryptoContext *aContext);

/**
 * Set the key for AES operation.
 *
 * @param[in]  aContext           Context for AES operation.
 * @param[out] aKey               Key to use for AES operation.
 *
 * @retval OT_ERROR_NONE          Successfully set the key for AES operation.
 * @retval OT_ERROR_FAILED        Failed to set the key for AES operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aKey was NULL
 *
 */
otError otPlatCryptoAesSetKey(otCryptoContext *aContext, const otCryptoKey *aKey);

/**
 * Encrypt the given data.
 *
 * @param[in]  aContext           Context for AES operation.
 * @param[in]  aInput             Pointer to the input buffer.
 * @param[in]  aOutput            Pointer to the output buffer.
 *
 * @retval OT_ERROR_NONE          Successfully encrypted @p aInput.
 * @retval OT_ERROR_FAILED        Failed to encrypt @p aInput.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aKey or @p aOutput were NULL
 *
 */
otError otPlatCryptoAesEncrypt(otCryptoContext *aContext, const uint8_t *aInput, uint8_t *aOutput);

/**
 * Free the AES context.
 *
 * @param[in]  aContext           Context for AES operation.
 *
 * @retval OT_ERROR_NONE          Successfully freed AES context.
 * @retval OT_ERROR_FAILED        Failed to free AES context.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 */
otError otPlatCryptoAesFree(otCryptoContext *aContext);

/**
 * Initialise the HKDF context.
 *
 * @param[in]  aContext           Context for HKDF operation.
 *
 * @retval OT_ERROR_NONE          Successfully Initialised AES operation.
 * @retval OT_ERROR_FAILED        Failed to Initialise AES operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 * @note The platform driver shall point the context to the correct object such as psa_key_derivation_operation_t
 *       or HmacSha256::Hash
 *
 */
otError otPlatCryptoHkdfInit(otCryptoContext *aContext);

/**
 * Perform HKDF Expand step.
 *
 * @param[in]  aContext           Operation context for HKDF operation.
 * @param[in]  aInfo              Pointer to the Info sequence.
 * @param[in]  aInfoLength        Length of the Info sequence.
 * @param[out] aOutputKey         Pointer to the output Key.
 * @param[in]  aOutputKeyLength   Size of the output key buffer.
 *
 * @retval OT_ERROR_NONE          HKDF Expand was successful.
 * @retval OT_ERROR_FAILED        HKDF Expand failed.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 */
otError otPlatCryptoHkdfExpand(otCryptoContext *aContext,
                               const uint8_t   *aInfo,
                               uint16_t         aInfoLength,
                               uint8_t         *aOutputKey,
                               uint16_t         aOutputKeyLength);

/**
 * Perform HKDF Extract step.
 *
 * @param[in]  aContext           Operation context for HKDF operation.
 * @param[in]  aSalt              Pointer to the Salt for HKDF.
 * @param[in]  aSaltLength        Length of Salt.
 * @param[in]  aInputKey          Pointer to the input key.
 *
 * @retval OT_ERROR_NONE          HKDF Extract was successful.
 * @retval OT_ERROR_FAILED        HKDF Extract failed.
 *
 */
otError otPlatCryptoHkdfExtract(otCryptoContext   *aContext,
                                const uint8_t     *aSalt,
                                uint16_t           aSaltLength,
                                const otCryptoKey *aInputKey);

/**
 * Uninitialize the HKDF context.
 *
 * @param[in]  aContext           Context for HKDF operation.
 *
 * @retval OT_ERROR_NONE          Successfully un-initialised HKDF operation.
 * @retval OT_ERROR_FAILED        Failed to un-initialised HKDF operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 */
otError otPlatCryptoHkdfDeinit(otCryptoContext *aContext);

/**
 * Initialise the SHA-256 operation.
 *
 * @param[in]  aContext           Context for SHA-256 operation.
 *
 * @retval OT_ERROR_NONE          Successfully initialised SHA-256 operation.
 * @retval OT_ERROR_FAILED        Failed to initialise SHA-256 operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 *
 * @note The platform driver shall point the context to the correct object such as psa_hash_operation_t
 *       or mbedtls_sha256_context.
 */
otError otPlatCryptoSha256Init(otCryptoContext *aContext);

/**
 * Uninitialize the SHA-256 operation.
 *
 * @param[in]  aContext           Context for SHA-256 operation.
 *
 * @retval OT_ERROR_NONE          Successfully un-initialised SHA-256 operation.
 * @retval OT_ERROR_FAILED        Failed to un-initialised SHA-256 operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 */
otError otPlatCryptoSha256Deinit(otCryptoContext *aContext);

/**
 * Start SHA-256 operation.
 *
 * @param[in]  aContext           Context for SHA-256 operation.
 *
 * @retval OT_ERROR_NONE          Successfully started SHA-256 operation.
 * @retval OT_ERROR_FAILED        Failed to start SHA-256 operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 */
otError otPlatCryptoSha256Start(otCryptoContext *aContext);

/**
 * Update SHA-256 operation with new input.
 *
 * @param[in]  aContext           Context for SHA-256 operation.
 * @param[in]  aBuf               A pointer to the input buffer.
 * @param[in]  aBufLength         The length of @p aBuf in bytes.
 *
 * @retval OT_ERROR_NONE          Successfully updated SHA-256 with new input operation.
 * @retval OT_ERROR_FAILED        Failed to update SHA-256 operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aBuf was NULL
 *
 */
otError otPlatCryptoSha256Update(otCryptoContext *aContext, const void *aBuf, uint16_t aBufLength);

/**
 * Finish SHA-256 operation.
 *
 * @param[in]  aContext           Context for SHA-256 operation.
 * @param[in]  aHash              A pointer to the output buffer, where hash needs to be stored.
 * @param[in]  aHashSize          The length of @p aHash in bytes.
 *
 * @retval OT_ERROR_NONE          Successfully completed the SHA-256 operation.
 * @retval OT_ERROR_FAILED        Failed to complete SHA-256 operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aHash was NULL
 *
 */
otError otPlatCryptoSha256Finish(otCryptoContext *aContext, uint8_t *aHash, uint16_t aHashSize);

/**
 * Initialize cryptographically-secure pseudorandom number generator (CSPRNG).
 *
 */
void otPlatCryptoRandomInit(void);

/**
 * Deinitialize cryptographically-secure pseudorandom number generator (CSPRNG).
 *
 */
void otPlatCryptoRandomDeinit(void);

/**
 * Fills a given buffer with cryptographically secure random bytes.
 *
 * @param[out] aBuffer            A pointer to a buffer to fill with the random bytes.
 * @param[in]  aSize              Size of buffer (number of bytes to fill).
 *
 * @retval OT_ERROR_NONE          Successfully filled buffer with random values.
 * @retval OT_ERROR_FAILED        Operation failed.
 *
 */
otError otPlatCryptoRandomGet(uint8_t *aBuffer, uint16_t aSize);

/**
 * Generate and populate the output buffer with a new ECDSA key-pair.
 *
 * @param[out] aKeyPair           A pointer to an ECDSA key-pair structure to store the generated key-pair.
 *
 * @retval OT_ERROR_NONE          A new key-pair was generated successfully.
 * @retval OT_ERROR_NO_BUFS       Failed to allocate buffer for key generation.
 * @retval OT_ERROR_NOT_CAPABLE   Feature not supported.
 * @retval OT_ERROR_FAILED        Failed to generate key-pair.
 *
 */
otError otPlatCryptoEcdsaGenerateKey(otPlatCryptoEcdsaKeyPair *aKeyPair);

/**
 * Get the associated public key from the input context.
 *
 * @param[in]  aKeyPair           A pointer to an ECDSA key-pair structure where the key-pair is stored.
 * @param[out] aPublicKey         A pointer to an ECDSA public key structure to store the public key.
 *
 * @retval OT_ERROR_NONE          Public key was retrieved successfully, and @p aBuffer is updated.
 * @retval OT_ERROR_PARSE         The key-pair DER format could not be parsed (invalid format).
 * @retval OT_ERROR_INVALID_ARGS  The @p aContext is NULL.
 *
 */
otError otPlatCryptoEcdsaGetPublicKey(const otPlatCryptoEcdsaKeyPair *aKeyPair, otPlatCryptoEcdsaPublicKey *aPublicKey);

/**
 * Calculate the ECDSA signature for a hashed message using the private key from the input context.
 *
 * Uses the deterministic digital signature generation procedure from RFC 6979.
 *
 * @param[in]  aKeyPair           A pointer to an ECDSA key-pair structure where the key-pair is stored.
 * @param[in]  aHash              A pointer to a SHA-256 hash structure where the hash value for signature calculation
 *                                is stored.
 * @param[out] aSignature         A pointer to an ECDSA signature structure to output the calculated signature.
 *
 * @retval OT_ERROR_NONE          The signature was calculated successfully, @p aSignature was updated.
 * @retval OT_ERROR_PARSE         The key-pair DER format could not be parsed (invalid format).
 * @retval OT_ERROR_NO_BUFS       Failed to allocate buffer for signature calculation.
 * @retval OT_ERROR_INVALID_ARGS  The @p aContext is NULL.
 *
 */
otError otPlatCryptoEcdsaSign(const otPlatCryptoEcdsaKeyPair *aKeyPair,
                              const otPlatCryptoSha256Hash   *aHash,
                              otPlatCryptoEcdsaSignature     *aSignature);

/**
 * Use the key from the input context to verify the ECDSA signature of a hashed message.
 *
 * @param[in]  aPublicKey         A pointer to an ECDSA public key structure where the public key for signature
 *                                verification is stored.
 * @param[in]  aHash              A pointer to a SHA-256 hash structure where the hash value for signature verification
 *                                is stored.
 * @param[in]  aSignature         A pointer to an ECDSA signature structure where the signature value to be verified is
 *                                stored.
 *
 * @retval OT_ERROR_NONE          The signature was verified successfully.
 * @retval OT_ERROR_SECURITY      The signature is invalid.
 * @retval OT_ERROR_INVALID_ARGS  The key or hash is invalid.
 * @retval OT_ERROR_NO_BUFS       Failed to allocate buffer for signature verification.
 *
 */
otError otPlatCryptoEcdsaVerify(const otPlatCryptoEcdsaPublicKey *aPublicKey,
                                const otPlatCryptoSha256Hash     *aHash,
                                const otPlatCryptoEcdsaSignature *aSignature);

/**
 * Calculate the ECDSA signature for a hashed message using the Key reference passed.
 *
 * Uses the deterministic digital signature generation procedure from RFC 6979.
 *
 * @param[in]  aKeyRef            Key Reference to the slot where the key-pair is stored.
 * @param[in]  aHash              A pointer to a SHA-256 hash structure where the hash value for signature calculation
 *                                is stored.
 * @param[out] aSignature         A pointer to an ECDSA signature structure to output the calculated signature.
 *
 * @retval OT_ERROR_NONE          The signature was calculated successfully, @p aSignature was updated.
 * @retval OT_ERROR_PARSE         The key-pair DER format could not be parsed (invalid format).
 * @retval OT_ERROR_NO_BUFS       Failed to allocate buffer for signature calculation.
 * @retval OT_ERROR_INVALID_ARGS  The @p aContext is NULL.
 *
 * @note This API is only used by OT core when `OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE` is enabled.
 *
 */
otError otPlatCryptoEcdsaSignUsingKeyRef(otCryptoKeyRef                aKeyRef,
                                         const otPlatCryptoSha256Hash *aHash,
                                         otPlatCryptoEcdsaSignature   *aSignature);

/**
 * Get the associated public key from the key reference passed.
 *
 * The public key is stored differently depending on the crypto backend library being used
 * (OPENTHREAD_CONFIG_CRYPTO_LIB).
 *
 * This API must make sure to return the public key as a byte sequence representation of an
 * uncompressed curve point (RFC 6605 - sec 4)
 *
 * @param[in]  aKeyRef            Key Reference to the slot where the key-pair is stored.
 * @param[out] aPublicKey         A pointer to an ECDSA public key structure to store the public key.
 *
 * @retval OT_ERROR_NONE          Public key was retrieved successfully, and @p aBuffer is updated.
 * @retval OT_ERROR_PARSE         The key-pair DER format could not be parsed (invalid format).
 * @retval OT_ERROR_INVALID_ARGS  The @p aContext is NULL.
 *
 * @note This API is only used by OT core when `OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE` is enabled.
 *
 */
otError otPlatCryptoEcdsaExportPublicKey(otCryptoKeyRef aKeyRef, otPlatCryptoEcdsaPublicKey *aPublicKey);

/**
 * Generate and import a new ECDSA key-pair at reference passed.
 *
 * @param[in]  aKeyRef            Key Reference to the slot where the key-pair is stored.
 *
 * @retval OT_ERROR_NONE          A new key-pair was generated successfully.
 * @retval OT_ERROR_NO_BUFS       Failed to allocate buffer for key generation.
 * @retval OT_ERROR_NOT_CAPABLE   Feature not supported.
 * @retval OT_ERROR_FAILED        Failed to generate key-pair.
 *
 * @note This API is only used by OT core when `OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE` is enabled.
 *
 */
otError otPlatCryptoEcdsaGenerateAndImportKey(otCryptoKeyRef aKeyRef);

/**
 * Use the keyref to verify the ECDSA signature of a hashed message.
 *
 * @param[in]  aKeyRef            Key Reference to the slot where the key-pair is stored.
 * @param[in]  aHash              A pointer to a SHA-256 hash structure where the hash value for signature verification
 *                                is stored.
 * @param[in]  aSignature         A pointer to an ECDSA signature structure where the signature value to be verified is
 *                                stored.
 *
 * @retval OT_ERROR_NONE          The signature was verified successfully.
 * @retval OT_ERROR_SECURITY      The signature is invalid.
 * @retval OT_ERROR_INVALID_ARGS  The key or hash is invalid.
 * @retval OT_ERROR_NO_BUFS       Failed to allocate buffer for signature verification.
 *
 * @note This API is only used by OT core when `OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE` is enabled.
 *
 */
otError otPlatCryptoEcdsaVerifyUsingKeyRef(otCryptoKeyRef                    aKeyRef,
                                           const otPlatCryptoSha256Hash     *aHash,
                                           const otPlatCryptoEcdsaSignature *aSignature);

/**
 * Perform PKCS#5 PBKDF2 using CMAC (AES-CMAC-PRF-128).
 *
 * @param[in]     aPassword          Password to use when generating key.
 * @param[in]     aPasswordLen       Length of password.
 * @param[in]     aSalt              Salt to use when generating key.
 * @param[in]     aSaltLen           Length of salt.
 * @param[in]     aIterationCounter  Iteration count.
 * @param[in]     aKeyLen            Length of generated key in bytes.
 * @param[out]    aKey               A pointer to the generated key.
 *
 * @retval OT_ERROR_NONE          A new key-pair was generated successfully.
 * @retval OT_ERROR_NO_BUFS       Failed to allocate buffer for key generation.
 * @retval OT_ERROR_NOT_CAPABLE   Feature not supported.
 * @retval OT_ERROR_FAILED        Failed to generate key.
 */
otError otPlatCryptoPbkdf2GenerateKey(const uint8_t *aPassword,
                                      uint16_t       aPasswordLen,
                                      const uint8_t *aSalt,
                                      uint16_t       aSaltLen,
                                      uint32_t       aIterationCounter,
                                      uint16_t       aKeyLen,
                                      uint8_t       *aKey);

/**
 * @}
 *
 */

#ifdef __cplusplus
} // end of extern "C"
#endif
#endif // OPENTHREAD_PLATFORM_CRYPTO_H_
