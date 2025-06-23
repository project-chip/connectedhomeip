/*
 *
 *    Copyright (c) 2020, 2025 Project CHIP Authors
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

#pragma once

#include <string.h>

#include <CHIPCryptoPAL_se05x.h>
#include <crypto/CHIPCryptoPAL.h>
#include <type_traits>

#include <CHIPCryptoPALHsm_se05x_config.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemMutex.h>

/* se05x includes */
#include "ex_sss_boot.h"
#include "fsl_sss_api.h"
#include <fsl_sss_se05x_apis.h>
#include <se05x_APDU.h>

extern ex_sss_boot_ctx_t gex_sss_chip_ctx;

/* SE predefined keyid values. Let kKeyId_node_op_keyid_start be the last entry */
enum keyid_values
{
    kKeyId_NotInitialized           = 0,
    kKeyId_pbkdf2_sha256_hmac_keyid = 0x7D000000,
    kKeyId_hkdf_sha256_hmac_keyid,
    kKeyId_hmac_sha256_keyid,
    kKeyId_sha256_ecc_pub_keyid, // Used for ECDSA verify
    kKeyId_case_ephemeral_keyid, // Used for ECDH
    kKeyId_node_op_keyid_start,  // Node operational key pair
};

// Enable the below macro to make spake HSM imlementation reentrant.
#define ENABLE_REENTRANCY 0

#if ENABLE_REENTRANCY
#define MAX_SPAKE_CRYPTO_OBJECT 20
#define OBJ_ID_TABLE_IDX_OBJID 0
#define OBJ_ID_TABLE_IDX_STATUS 1
#define OBJ_ID_TABLE_OBJID_STATUS_USED 1
#define OBJ_ID_TABLE_OBJID_STATUS_FREE 0
#define LIMIT_CRYPTO_OBJECTS 2
#endif

#if ENABLE_REENTRANCY
/* To keep track of number of spake crypto objects created */
extern int spake_objects_created;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Open session to se05x secure element.
 * Default is the plain session.
 * To open platformSCP03 session, build with option --args="chip_se05x_auth=\"scp03\""
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_session_open(void);

/**
 * @brief Check if key exists in se05x.
 * @param[in] Key id of the key to be checked.
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_check_object_exists(uint32_t keyid);

/**
 * @brief Delete the key in se05x.
 * @param[in] Key id of the key to be deleted.
 */
void se05x_delete_key(uint32_t keyid);

/**
 * @brief Set key that are used for spake operation.
 * @param[in] keyid - Key id of the object.
 * @param[in] key - Buffer with AES / EC key key.
 * @param[in] keylen - Key length.
 * @param[in] keyPart - Type of key.
 * @param[in] cipherType - kSSS_CipherType_EC_NIST_P for ecc and kSSS_CipherType_HMAC for AES key.
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_set_key_for_spake(uint32_t keyid, const uint8_t * key, size_t keylen, sss_key_part_t keyPart,
                                   sss_cipher_type_t cipherType);

/**
 * @brief Set certificate in se05x.
 * The certificate is stored with transient option. The contents are lost on session close.
 * @param[in] keyid - Key id of the object.
 * @param[in] buf - Buffer containing certificate in DER format.
 * @param[in] buflen - Buffer length.
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_set_certificate(uint32_t keyId, const uint8_t * buf, size_t buflen);

/**
 * @brief Set binary data in se05x.
 * The certificate is stored with transient option. The contents are lost on session close.
 * @param[in] keyid - Key id of the object.
 * @param[in] buf - Buffer containing binary data.
 * @param[in] buflen - Buffer length.
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_set_binary_data(uint32_t keyId, const uint8_t * buf, size_t buflen);

/**
 * @brief Perform internal sign in se05x (only on SE051H).
 * Used to Perform ECDSA internal sign
 * ECDSA sign is performed on concatenated data of BinaryFile Secure Object,
 * The objectIDs of which are the contents of a BinaryFile Secure Object called tbsItemList.
 * KeyPair used for Signing should be provided with POLICY_OBJ_INTERNAL_SIGN.
 * This policy requires a 4-byte extension containing the identifier of the tbsItemList Secure Object
 * @param[in] keyid - Key id of ECC key used for internal sign.
 * @param[out] sigBuf - ECDSA Signature
 * @param[out] sigBufLen - Signature length.
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_perform_internal_sign(uint32_t keyId, uint8_t * sigBuf, size_t * sigBufLen);

#if ENABLE_REENTRANCY

/* Init crypto object mutext */
void se05x_init_cryptoObj_mutex(void);

/* Delete all crypto objects in se05x */
void se05x_delete_crypto_objects(void);

/* Get unused object id */
SE05x_CryptoObjectID_t se05x_getCryptoObjID(void);

/* Set object id status */
void se05x_setCryptoObjID(SE05x_CryptoObjectID_t objId, uint8_t status);

#endif // #if ENABLE_REENTRANCY

#ifdef __cplusplus
}
#endif
