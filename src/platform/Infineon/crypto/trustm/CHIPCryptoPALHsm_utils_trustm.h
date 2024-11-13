/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <CHIPCryptoPAL.h>
#include <type_traits>

#include "optiga_crypt.h"
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemMutex.h>
/* trustm includes */
#include "optiga_util.h"
#include <CHIPCryptoPALHsm_config_trustm.h>

#ifdef __cplusplus
extern "C" {
#endif

extern optiga_crypt_t * p_local_crypt;
extern optiga_util_t * p_local_util;

#define TRUSTM_HKDF_OID_KEY (0xF1D8)
#define TRUSTM_HMAC_OID_KEY (0xF1D9)
#define TRUSTM_P256_PUBKEY_OID_KEY (0xF1DA)
#define TRUSTM_ECDH_OID_KEY (0xE100)
#define TRUSTM_NODE_OID_KEY_START (0xE0F2)

#define IFX_CRYPTO_KEY_MAGIC                                                                                                       \
    {                                                                                                                              \
        0xA0, 0x10, 0xA0, 0x10                                                                                                     \
    }

static const uint8_t trustm_magic_no[] = IFX_CRYPTO_KEY_MAGIC;
static const uint8_t DA_KEY_ID[]       = { 0xF0, 0xE0 }; // OID --> 0xE0F0
/* Open session to trustm */
void trustm_Open(void);
void read_certificate_from_optiga(uint16_t optiga_oid, char * cert_pem, uint16_t * cert_pem_length);
void write_data(uint16_t optiga_oid, const uint8_t * p_data, uint16_t length);
void write_metadata(uint16_t optiga_oid, const uint8_t * p_data, uint8_t length);
void trustmGetKey(uint16_t optiga_oid, uint8_t * pubkey, uint16_t * pubKeyLen);
optiga_lib_status_t deriveKey_HKDF(const uint8_t * salt, uint16_t salt_length, const uint8_t * info, uint16_t info_length,
                                   uint16_t derived_key_length, bool_t export_to_host, uint8_t * derived_key);
optiga_lib_status_t hmac_sha256(optiga_hmac_type_t type, const uint8_t * input_data, uint32_t input_data_length, uint8_t * mac,
                                uint32_t * mac_length);
optiga_lib_status_t trustm_ecc_keygen(uint16_t optiga_key_id, uint8_t key_type, optiga_ecc_curve_t curve_id, uint8_t * pubkey,
                                      uint16_t * pubkey_length);
optiga_lib_status_t trustm_hash(uint8_t * msg, uint16_t msg_length, uint8_t * digest, uint8_t digest_length);
optiga_lib_status_t trustm_ecdsa_sign(optiga_key_id_t optiga_key_id, uint8_t * digest, uint8_t digest_length, uint8_t * signature,
                                      uint16_t * signature_length);
void ecc_public_key_in_bit(const uint8_t * q_buffer, uint8_t q_length, uint8_t * pub_key_buffer, uint16_t pub_key_length);
optiga_lib_status_t trustm_ecdsa_verify(uint8_t * digest, uint8_t digest_length, uint8_t * signature, uint16_t signature_length,
                                        uint8_t * ecc_pubkey, uint8_t ecc_pubkey_length);
/* Close session to trustm */
void trustm_close(void);
CHIP_ERROR trustmGetCertificate(uint16_t optiga_oid, uint8_t * buf, uint16_t * buflen);
optiga_lib_status_t trustm_ecdh_derive_secret(optiga_key_id_t optiga_key_id, uint8_t * public_key, uint16_t public_key_length,
                                              uint8_t * shared_secret, uint8_t shared_secret_length);
optiga_lib_status_t optiga_crypt_rng(uint8_t * random_data, uint16_t random_data_length);
optiga_lib_status_t trustm_PBKDF2_HMAC(const unsigned char * salt, size_t slen, unsigned int iteration_count, uint32_t key_length,
                                       unsigned char * output);
#ifdef __cplusplus
}
#endif
