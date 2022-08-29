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

#pragma once

#include <string.h>

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#include <type_traits>

#include <crypto/hsm/CHIPCryptoPALHsm_config.h>
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

/* SE predefined keyid values */
enum keyid_values
{
    kKeyId_NotInitialized           = 0,
    kKeyId_pbkdf2_sha256_hmac_keyid = 0x7D000000,
    kKeyId_hkdf_sha256_hmac_keyid,
    kKeyId_hmac_sha256_keyid,
    kKeyId_sha256_ecc_pub_keyid,
    kKeyId_case_ephemeral_keyid,
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

/* Open session to se05x */
void se05x_sessionOpen(void);

/* Delete key in se05x */
void se05x_delete_key(uint32_t keyid);

/* Set key in se05x */
CHIP_ERROR se05x_set_key_for_spake(uint32_t keyid, const uint8_t * key, size_t keylen, sss_key_part_t keyPart,
                                   sss_cipher_type_t cipherType);

#if ENABLE_REENTRANCY

/* Init crypto object mutext */
void init_cryptoObj_mutex(void);

/* Delete all crypto objects in se05x */
void delete_crypto_objects(void);

/* Get unused object id */
SE05x_CryptoObjectID_t getObjID(void);

/* Set object id status */
void setObjID(SE05x_CryptoObjectID_t objId, uint8_t status);

#endif //#if ENABLE_REENTRANCY

#ifdef __cplusplus
}
#endif
