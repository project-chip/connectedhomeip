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

/**
 *    @file
 *      HSM based implementation of CHIP crypto primitives
 *      Based on configurations in CHIPCryptoPALHsm_config.h file,
 *      chip crypto apis use either HSM or rollback to software implementation.
 */

/* OPTIGA(TM) Trust M includes */
#include "CHIPCryptoPALHsm_utils_trustm.h"
#include "ifx_i2c_config.h"
#include "mbedtls/base64.h"
#include "optiga_crypt.h"
#include "optiga_lib_types.h"
#include "optiga_util.h"
#include "pal.h"
#include "pal_ifx_i2c_config.h"
#include "pal_os_event.h"
#include "pal_os_memory.h"
#include "pal_os_timer.h"
#include <FreeRTOS.h>

optiga_crypt_t * me_crypt = NULL;
optiga_util_t * me_util   = NULL;
static bool trustm_isOpen = false;
#define ENABLE_HMAC_MULTI_STEP (0)
#define OPTIGA_UTIL_DER_BITSTRING_TAG (0x03)
#define OPTIGA_UTIL_DER_NUM_UNUSED_BITS (0x00)

#if ENABLE_HMAC_MULTI_STEP
#define MAX_MAC_DATA_LEN 640
#endif

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================

/* This is a place from which we can poll the status of operation */

void vApplicationTickHook(void);

void vApplicationTickHook(void) {}

#define WAIT_FOR_COMPLETION(ret)                                                                                                   \
    if (OPTIGA_LIB_SUCCESS != ret)                                                                                                 \
    {                                                                                                                              \
        break;                                                                                                                     \
    }                                                                                                                              \
    while (optiga_lib_status == OPTIGA_LIB_BUSY)                                                                                   \
    {                                                                                                                              \
    }                                                                                                                              \
    if (OPTIGA_LIB_SUCCESS != optiga_lib_status)                                                                                   \
    {                                                                                                                              \
        ret = optiga_lib_status;                                                                                                   \
        printf("Error: 0x%02X \r\n", optiga_lib_status);                                                                           \
        break;                                                                                                                     \
    }

#define CHECK_RESULT(expr)                                                                                                         \
    optiga_lib_status_t return_status = (int32_t) OPTIGA_DEVICE_ERROR;                                                             \
                                                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        optiga_lib_status = OPTIGA_LIB_BUSY;                                                                                       \
        return_status     = expr;                                                                                                  \
        WAIT_FOR_COMPLETION(return_status);                                                                                        \
    } while (0);                                                                                                                   \
                                                                                                                                   \
    return return_status;

static volatile optiga_lib_status_t optiga_lib_status;

static void optiga_util_callback(void * context, optiga_lib_status_t return_status)
{
    optiga_lib_status = return_status;
}

// lint --e{818} suppress "argument "context" is not used in the sample provided"
static void optiga_crypt_callback(void * context, optiga_lib_status_t return_status)
{
    optiga_lib_status = return_status;
    if (NULL != context)
    {
        // callback to upper layer here
    }
}

/* Open session to trustm */
/**********************************************************************
 * trustm_Open()
 **********************************************************************/
static bool init = false;
void trustm_Open(void)
{
    uint16_t dOptigaOID = 0xE0C4;
    // Maximum Power, Minimum Current limitation
    uint8_t cCurrentLimit = 15;

    if (!trustm_isOpen)
    {
        optiga_lib_status_t return_status;
        do
        {
            // Create Optiga crypt instance
            if (me_crypt == NULL)
            {
                me_crypt = optiga_crypt_create(0, optiga_crypt_callback, NULL);
                if (NULL == me_crypt)
                {
                    break;
                }
            }
            else
            {
                printf("Error: me_crypt already initialised\n");
            }
            // Create Optiga Util instance
            if (me_util == NULL)
            {
                me_util = optiga_util_create(0, optiga_util_callback, NULL);
                if (NULL == me_util)
                {
                    break;
                }
            }
            else
            {
                printf("Error: me_crypt already initialised\n");
            }
            /**
             * Open the application on OPTIGA which is a precondition to perform any other operations
             * using optiga_util_open_application
             */
            optiga_lib_status = OPTIGA_LIB_BUSY;
            return_status     = optiga_util_open_application(me_util, 0); // skip restore
            if (OPTIGA_LIB_SUCCESS != return_status)
            {
                printf("optiga_util_open_application api returns error %02X\n", return_status);
                break;
            }
            // Wait until the optiga_util_open_application is completed
            WAIT_FOR_COMPLETION(return_status);

            if (OPTIGA_LIB_SUCCESS != return_status)
            {
                // optiga_util_open_application failed
                printf("optiga_util_open_application failed\n");
                break;
            }
            trustm_isOpen = true;

            // Only run once for initialisation
            if (!init)
            {
                optiga_lib_status = OPTIGA_LIB_BUSY;
                return_status     = optiga_util_write_data(me_util, dOptigaOID, OPTIGA_UTIL_ERASE_AND_WRITE, 0, &cCurrentLimit, 1);
                if (OPTIGA_LIB_SUCCESS != return_status)
                {
                    printf("optiga_util_write_data api returns error %02X\n", return_status);
                    break;
                }
                WAIT_FOR_COMPLETION(return_status);
                if (OPTIGA_LIB_SUCCESS != return_status)
                {
                    printf("optiga_util_write_data returns error\n");
                    break;
                }
                // Set init to true
                init = true;
            }
        } while (0);
    }
}

void trustm_close(void)
{
    optiga_lib_status_t return_status = OPTIGA_DEVICE_ERROR;

    do
    {
        /**
         * Close the application on OPTIGA after all the operations are executed
         * using optiga_util_close_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status     = optiga_util_close_application(me_util, 0);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            printf("optiga_util_close_application api returns error %02X\n", return_status);
            break;
        }

        WAIT_FOR_COMPLETION(return_status);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_util_close_application failed
            printf("optiga_util_close_application failed\n");
            break;
        }

        // destroy util and crypt instances
        optiga_util_destroy(me_util);
        optiga_crypt_destroy(me_crypt);
        pal_os_event_destroy(NULL);
        me_util       = NULL;
        me_crypt      = NULL;
        trustm_isOpen = false;
        return_status = OPTIGA_LIB_SUCCESS;
    } while (0);
}

void read_certificate_from_optiga(uint16_t optiga_oid, char * cert_pem, uint16_t * cert_pem_length)
{
    size_t ifx_cert_b64_len = 0;
    uint8_t ifx_cert_b64_temp[1200];
    uint16_t offset_to_write = 0, offset_to_read = 0;
    uint16_t size_to_copy = 0;
    optiga_lib_status_t return_status;

    uint8_t ifx_cert_hex[1024];
    uint16_t ifx_cert_hex_len = sizeof(ifx_cert_hex);

    do
    {
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status     = optiga_util_read_data(me_util, optiga_oid, 0, ifx_cert_hex, &ifx_cert_hex_len);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            printf("optiga_util_read_data api returns error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_util_read_data returns error
            printf("read_certificate_from_optiga failed\n");
            break;
        }
        // convert to PEM format
        // If the first byte is TLS Identity Tag, than we need to skip 9 first bytes
        offset_to_read = ifx_cert_hex[0] == 0xc0 ? 9 : 0;
        mbedtls_base64_encode((unsigned char *) ifx_cert_b64_temp, sizeof(ifx_cert_b64_temp), &ifx_cert_b64_len,
                              ifx_cert_hex + offset_to_read, ifx_cert_hex_len - offset_to_read);

        memcpy(cert_pem, "-----BEGIN CERTIFICATE-----\n", 28);
        offset_to_write += 28;

        // Properly copy certificate and format it as pkcs expects
        for (offset_to_read = 0; offset_to_read < (uint16_t) ifx_cert_b64_len;)
        {
            // The last block of data usually is less than 64, thus we need to find the leftover
            if ((offset_to_read + 64) >= (uint16_t) ifx_cert_b64_len)
                size_to_copy = (uint16_t) ifx_cert_b64_len - offset_to_read;
            else
                size_to_copy = 64;
            memcpy(cert_pem + offset_to_write, ifx_cert_b64_temp + offset_to_read, size_to_copy);
            offset_to_write += size_to_copy;
            offset_to_read += size_to_copy;
            cert_pem[offset_to_write] = '\n';
            offset_to_write++;
        }

        memcpy(cert_pem + offset_to_write, "-----END CERTIFICATE-----\n\0", 27);

        *cert_pem_length = offset_to_write + 27;

    } while (0);
}

void write_data(uint16_t optiga_oid, const uint8_t * p_data, uint16_t length)
{
    optiga_lib_status_t return_status;

    do
    {
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status     = optiga_util_write_data(me_util, optiga_oid, OPTIGA_UTIL_ERASE_AND_WRITE, 0, p_data, length);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            printf("optiga_util_write_data api returns error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            printf("write_data failed\n");
            return_status = optiga_lib_status;
            break;
        }
    } while (0);
}

void write_metadata(uint16_t optiga_oid, const uint8_t * p_data, uint8_t length)
{
    optiga_lib_status_t return_status;

    do
    {
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status     = optiga_util_write_metadata(me_util, optiga_oid, p_data, length);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            printf("optiga_util_write_metadata api returns error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            printf("optiga_util_write_metadata failed\n");
            break;
        }
    } while (0);
}

optiga_lib_status_t deriveKey_HKDF(const uint8_t * salt, uint16_t salt_length, const uint8_t * info, uint16_t info_length,
                                   uint16_t derived_key_length, bool_t export_to_host, uint8_t * derived_key)
{
    optiga_lib_status_t return_status;

    do
    {
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status     = optiga_crypt_hkdf(me_crypt, OPTIGA_HKDF_SHA_256, TRUSTM_HKDF_OID_KEY, /* Input secret OID */
                                              salt, salt_length, info, info_length, derived_key_length, TRUE, derived_key);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            printf("optiga_crypt_hkdf api returns error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            printf("optiga_crypt_hkdf failed\n");
            break;
        }
    } while (0);

    return return_status;
}

optiga_lib_status_t hmac_sha256(optiga_hmac_type_t type, const uint8_t * input_data, uint32_t input_data_length, uint8_t * mac,
                                uint32_t * mac_length)
{
    optiga_lib_status_t return_status;

    do
    {
        optiga_lib_status = OPTIGA_LIB_BUSY;
#if ENABLE_HMAC_MULTI_STEP
        // If the size is less than the max length supported
        if (input_data_length <= MAX_MAC_DATA_LEN)
        {
            return_status = optiga_crypt_hmac(me_crypt, type, TRUSTM_HMAC_OID_KEY, input_data, input_data_length, mac, mac_length);
            if (OPTIGA_LIB_SUCCESS != return_status)
            {
                // optiga_crypt_hmac returns error !!!
                printf("optiga_crypt_hmac api error %02X\n", return_status);
                break;
            }

            WAIT_FOR_COMPLETION(return_status);
            if (OPTIGA_LIB_SUCCESS != return_status)
            {
                // optiga_crypt_hmac returns error !!!
                printf("optiga_crypt_hmac returns error\n");
                break;
            }
        }
        else
        {
            // Calculate HMAC in multiple steps
            uint32_t dataLenTemp  = 0;
            uint32_t remainingLen = input_data_length;
            // Start the HMAC Operation
            return_status = optiga_crypt_hmac_start(me_crypt, type, TRUSTM_HMAC_OID_KEY, input_data, MAX_MAC_DATA_LEN);
            if (OPTIGA_LIB_SUCCESS != return_status)
            {
                // optiga_crypt_hmac returns error !!!
                printf("optiga_crypt_hmac_start api error %02X\n", return_status);
                break;
            }
            WAIT_FOR_COMPLETION(return_status);
            if (OPTIGA_LIB_SUCCESS != return_status)
            {
                // optiga_crypt_hmac_start returns error !!!
                printf("optiga_crypt_hmac_start returns error\n");
                break;
            }
            remainingLen = input_data_length - MAX_MAC_DATA_LEN;

            while (remainingLen > 0)
            {
                dataLenTemp = (remainingLen > MAX_MAC_DATA_LEN) ? MAX_MAC_DATA_LEN : remainingLen;

                if (remainingLen > MAX_MAC_DATA_LEN)
                {
                    optiga_lib_status = OPTIGA_LIB_BUSY;
                    return_status =
                        optiga_crypt_hmac_update(me_crypt, (input_data + (input_data_length - remainingLen)), dataLenTemp);
                    if (OPTIGA_LIB_SUCCESS != return_status)
                    {
                        // optiga_crypt_hmac_update returns error !!!
                        printf("optiga_crypt_hmac_update api error %02X\n", return_status);
                        break;
                    }
                    WAIT_FOR_COMPLETION(return_status);
                    remainingLen = remainingLen - dataLenTemp;
                    if (OPTIGA_LIB_SUCCESS != return_status)
                    {
                        // optiga_crypt_hmac_update returns error !!!
                        printf("optiga_crypt_hmac_update returns error\n");
                        break;
                    }
                }
                else
                {
                    // End HMAC sequence and return the MAC generated
                    // printf("HMAC Finalize\n");
                    optiga_lib_status = OPTIGA_LIB_BUSY;
                    return_status     = optiga_crypt_hmac_finalize(me_crypt, (input_data + (input_data_length - remainingLen)),
                                                                   dataLenTemp, mac, mac_length);
                    if (OPTIGA_LIB_SUCCESS != return_status)
                    {
                        // optiga_crypt_hmac_finalize returns error !!!
                        printf("optiga_crypt_hmac_finalize api error %02X\n", return_status);
                        break;
                    }
                    WAIT_FOR_COMPLETION(return_status);
                    if (OPTIGA_LIB_SUCCESS != return_status)
                    {
                        // optiga_crypt_hmac_finalize returns error !!!
                        printf("optiga_crypt_hmac_finalize returns error\n");
                        break;
                    }
                }
            }
        }
#else

        return_status = optiga_crypt_hmac(me_crypt, type, TRUSTM_HMAC_OID_KEY, input_data, input_data_length, mac, mac_length);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_hmac returns error !!!
            printf("optiga_crypt_hmac api error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_hmac returns error !!!
            printf("optiga_crypt_hmac returns error\n");
            break;
        }
#endif
    } while (0);

    return return_status;
}

optiga_lib_status_t optiga_crypt_rng(uint8_t * random_data, uint16_t random_data_length)
{
    optiga_lib_status_t return_status;
    do
    {
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status     = optiga_crypt_random(me_crypt, OPTIGA_RNG_TYPE_DRNG, random_data, random_data_length);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_random returns error !!!
            printf("optiga_crypt_random api error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_random failed
            printf("optiga_crypt_random returns error\n");
            break;
        }
    } while (0);

    return return_status;
}
optiga_lib_status_t trustm_ecc_keygen(uint16_t optiga_key_id, uint8_t key_type, optiga_ecc_curve_t curve_id, uint8_t * pubkey,
                                      uint16_t * pubkey_length)
{
    optiga_lib_status_t return_status;
    uint8_t header256[] = { 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02,
                            0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07 };
    uint16_t i;
    for (i = 0; i < sizeof(header256); i++)
    {
        pubkey[i] = header256[i];
    }
    do
    {
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status =
            optiga_crypt_ecc_generate_keypair(me_crypt, curve_id, key_type, FALSE, &optiga_key_id, (pubkey + i), pubkey_length);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_ecc_generate_keypair api returns error !!!
            printf("optiga_crypt_ecc_generate_keypair api error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_ecc_generate_keypair returns error !!!
            printf("optiga_crypt_ecc_generate_keypair returns error\n");
            break;
        }
    } while (0);

    *pubkey_length += sizeof(header256);
    return return_status;
}
void trustmGetKey(uint16_t optiga_oid, uint8_t * pubkey, uint16_t * pubkeyLen)
{
    optiga_lib_status_t return_status;
    uint16_t offset = 0;
    do
    {
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status     = optiga_util_read_data(me_util, optiga_oid, offset, pubkey, pubkeyLen);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_util_read_data api returns error !!!
            printf("optiga_util_read_data api error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_util_read_pubkey returns error !!!
            printf("optiga_util_read_pubkey returns error\n");
            break;
        }
    } while (0);
}
optiga_lib_status_t trustm_hash(uint8_t * msg, uint16_t msg_length, uint8_t * digest, uint8_t digest_length)
{
    optiga_lib_status_t return_status;
    hash_data_from_host_t hash_data_host;
    do
    {
        hash_data_host.buffer = msg;
        hash_data_host.length = msg_length;
        optiga_lib_status     = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_hash(me_crypt, OPTIGA_HASH_TYPE_SHA_256, OPTIGA_CRYPT_HOST_DATA, &hash_data_host, digest);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_hash api returns error !!!
            printf("optiga_crypt_hash api error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_hash api returns error !!!
            printf("optiga_crypt_hash returns error\n");
            break;
        }
    } while (0);

    return return_status;
}
optiga_lib_status_t trustm_ecdsa_sign(optiga_key_id_t optiga_key_id, uint8_t * digest, uint8_t digest_length, uint8_t * signature,
                                      uint16_t * signature_length)
{
    optiga_lib_status_t return_status;
    int i;
    do
    {
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status     = optiga_crypt_ecdsa_sign(me_crypt, digest, digest_length, optiga_key_id, signature, signature_length);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_ecdsa_sign api returns error !!!
            printf("optiga_crypt_ecdsa_sign api error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_ecdsa_sign returns error !!!
            printf("optiga_crypt_ecdsa_sign returns error\n");
            break;
        }
        for (i = (*signature_length - 1); i >= 0; i--)
        {
            signature[i + 2] = signature[i];
        }

        signature[0]      = 0x30;                          // Insert SEQUENCE
        signature[1]      = (uint8_t) (*signature_length); // insert length
        *signature_length = *signature_length + 2;

    } while (0);

    return return_status;
}

void ecc_pub_key_bit(uint8_t * q_buffer, uint8_t q_length, uint8_t * pub_key_buffer, uint16_t * pub_key_length)
{
#define OPTIGA_UTIL_ECC_DER_ADDITIONAL_LENGTH (0x01)

    uint16_t index = 0;

    pub_key_buffer[index++] = OPTIGA_UTIL_DER_BITSTRING_TAG;
    pub_key_buffer[index++] = q_length + OPTIGA_UTIL_ECC_DER_ADDITIONAL_LENGTH;
    pub_key_buffer[index++] = OPTIGA_UTIL_DER_NUM_UNUSED_BITS;

    pal_os_memcpy(&pub_key_buffer[index], q_buffer, q_length);
    index += q_length;

    *pub_key_length = index;

#undef OPTIGA_UTIL_ECC_DER_ADDITIONAL_LENGTH
}
optiga_lib_status_t trustm_ecdsa_verify(uint8_t * digest, uint8_t digest_length, uint8_t * signature, uint16_t signature_length,
                                        uint8_t * ecc_pubkey, uint8_t ecc_pubkey_length)
{
    optiga_lib_status_t return_status;
    uint8_t ecc_public_key[70] = { 0x00 };
    uint16_t i;
    uint16_t ecc_public_key_length = 0;

    ecc_pub_key_bit(ecc_pubkey, ecc_pubkey_length, ecc_public_key, &ecc_public_key_length);

    public_key_from_host_t public_key_details = { ecc_public_key, ecc_public_key_length, (uint8_t) OPTIGA_ECC_CURVE_NIST_P_256 };
    do
    {
        signature_length = signature[1];
        for (i = 0; i < signature_length; i++)
        {
            signature[i] = signature[i + 2];
        }

        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status     = optiga_crypt_ecdsa_verify(me_crypt, digest, digest_length, signature, signature_length,
                                                      OPTIGA_CRYPT_HOST_DATA, &public_key_details);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_ecdsa_verify api returns error !!!
            printf("optiga_crypt_ecdsa_verify api error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_ecdsa_verify returns error !!!
            printf("optiga_crypt_ecdsa_verify returns error\n");
            break;
        }
    } while (0);

    return return_status;
}

CHIP_ERROR trustmGetCertificate(uint16_t optiga_oid, uint8_t * buf, uint16_t * buflen)
{
    optiga_lib_status_t return_status;
    VerifyOrReturnError(buf != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(buflen != nullptr, CHIP_ERROR_INTERNAL);

    uint8_t ifx_cert_hex[1024];
    uint16_t ifx_cert_hex_len = sizeof(ifx_cert_hex);

    trustm_Open();
    do
    {
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status     = optiga_util_read_data(me_util, optiga_oid, 0, ifx_cert_hex, &ifx_cert_hex_len);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_util_read_data api returns error !!!
            printf("optiga_util_read_data api error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_util_read_data failed
            printf("trustmGetCertificate failed\n");
            break;
        }
        memcpy(buf, ifx_cert_hex, ifx_cert_hex_len);
        *buflen = ifx_cert_hex_len;
    } while (0);

    if (return_status == OPTIGA_LIB_SUCCESS)
    {
        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_INTERNAL;
    }
}

optiga_lib_status_t trustm_ecdh_derive_secret(optiga_key_id_t optiga_key_id, uint8_t * public_key, uint16_t public_key_length,
                                              uint8_t * shared_secret, uint8_t shared_secret_length)
{
    optiga_lib_status_t return_status;
    static public_key_from_host_t public_key_details = {
        (uint8_t *) public_key,
        public_key_length,
        (uint8_t) OPTIGA_ECC_CURVE_NIST_P_256,
    };
    do
    {
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status     = optiga_crypt_ecdh(me_crypt, optiga_key_id, &public_key_details, TRUE, shared_secret);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_util_read_data api returns error !!!
            printf("optiga_crypt_ecdh api error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_ecdh returns error !!!
            printf("optiga_crypt_ecdh returns error\n");
            break;
        }
    } while (0);

    return return_status;
}

optiga_lib_status_t trustm_PBKDF2_HMAC(const unsigned char * salt, size_t slen, unsigned int iteration_count, uint32_t key_length,
                                       unsigned char * output)
{
    optiga_lib_status_t return_status;
    uint8_t md1[32];
    uint32_t md1_len = sizeof(md1);
    uint8_t work[32];
    uint32_t work_len = sizeof(work);

    unsigned char * out_p = output;
    do
    {
        // Calculate U1, U1 ends up in work
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status =
            optiga_crypt_hmac(me_crypt, OPTIGA_HMAC_SHA_256, TRUSTM_HMAC_OID_KEY, salt, (uint32_t) slen, work, &work_len);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            // optiga_crypt_hmac api returns error !!!
            printf("optiga_crypt_hmac api 1 error %02X\n", return_status);
            break;
        }
        WAIT_FOR_COMPLETION(return_status);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            printf("optiga_crypt_hmac 1 returns error\n");
            break;
        }
        memcpy(md1, work, md1_len);
        for (unsigned int i = 1; i < iteration_count; i++)
        {
            optiga_lib_status = OPTIGA_LIB_BUSY;
            // Calculated subsequent U, which ends up in md1
            return_status = optiga_crypt_hmac(me_crypt, OPTIGA_HMAC_SHA_256, TRUSTM_HMAC_OID_KEY, md1, md1_len, md1, &md1_len);
            if (OPTIGA_LIB_SUCCESS != return_status)
            {
                // optiga_crypt_hmac api returns error !!!
                printf("optiga_crypt_hmac api 2 error %02X\n", return_status);
                break;
            }
            WAIT_FOR_COMPLETION(return_status);
            if (OPTIGA_LIB_SUCCESS != return_status)
            {
                printf("optiga_crypt_hmac 2 returns error\n");
                break;
            }
            // U1 xor U2
            for (int j = 0; j < (int) md1_len; j++)
            {
                work[j] ^= md1[j];
            }
        }

        memcpy(out_p, work, key_length);
    } while (0);

    return return_status;
}
