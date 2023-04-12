/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifdef SL_WFX_USE_SECURE_LINK

/* Includes */

#include "sl_wfx.h"
#include <stdio.h>

#include "mbedtls/ccm.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/entropy.h"
#include "mbedtls/md.h"
#include "mbedtls/sha256.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

// Secure link MAC key location for WGM160P (in DI page in flash)
#ifdef EFM32GG11B820F2048GM64 // WGM160PX22KGA2
#define SL_WFX_FCCC_BASE_ADDR ((void *) 0x0fe08000ul)
#define SL_WFX_FCCC_DI_OFFSET 0x1B0ul
#define SL_WFX_FCCC_DI_ADDR ((void *) (SL_WFX_FCCC_BASE_ADDR + SL_WFX_FCCC_DI_OFFSET))
#define SL_WFX_SECURE_LINK_MAC_KEY_LOCATION ((void *) (SL_WFX_FCCC_BASE_ADDR + 0x3D0))
#endif
/******************************************************
 *                      Macros
 ******************************************************/
#define MAC_KEY_FAIL_BYTE 0XFF
#define KEY_DIGEST_SIZE 92
#define MEMCMP_FAIL 0
#define MPI_SET 1
#define SUCCESS_STATUS_WIFI_SECURE_LINK_EXCHANGE 0
#define SHA224_0 0
#define HMAC_SIZE 92
#define MEMSET_LEN 1
#define LABLE_LEN 24
#define ADDRESS_LENGTH 0
#define CCM_STATUS_SUCCESS 0
/******************************************************
 *                    Constants
 ******************************************************/

/* Semaphore to signal wfx driver available */
extern TaskHandle_t wfx_securelink_task;
extern SemaphoreHandle_t wfx_securelink_rx_mutex;

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

static inline void reverse_bytes(uint8_t * src, uint8_t length);

/******************************************************
 *               Variable Definitions
 ******************************************************/

#if SL_WFX_SLK_CURVE25519
static mbedtls_ecdh_context mbedtls_host_context;
static mbedtls_ctr_drbg_context host_drbg_context;
#endif
static mbedtls_entropy_context entropy;
uint8_t temp_key_location[SL_WFX_HOST_PUB_KEY_MAC_SIZE];
#ifdef EFM32GG11B820F2048GM64 // WGM160PX22KGA2
static const uint8_t * const secure_link_mac_key = (uint8_t *) SL_WFX_SECURE_LINK_MAC_KEY_LOCATION;
#else
static const uint8_t secure_link_mac_key[SL_WFX_SECURE_LINK_MAC_KEY_LENGTH] = { 0x2B, 0x49, 0xFD, 0x66, 0xCB, 0x74, 0x6D, 0x6B,
                                                                                0x4F, 0xDC, 0xC3, 0x79, 0x4E, 0xC5, 0x9A, 0x86,
                                                                                0xE5, 0x48, 0x2A, 0x41, 0x22, 0x87, 0x8B, 0x12,
                                                                                0x1A, 0x7C, 0x3E, 0xEF, 0xB7, 0x04, 0x9E, 0xB3 };
#endif
/******************************************************
 *               Function Definitions
 ******************************************************/
/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_get_secure_link_mac_key(uint8_t *sl_mac_key)
 * @brief
 * Get secure link mac key
 * @param[in]  sl_mac_key:
 * @return  returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_get_secure_link_mac_key(uint8_t * sl_mac_key)
{
    sl_status_t result = SL_STATUS_WIFI_SECURE_LINK_MAC_KEY_ERROR;

    memcpy(sl_mac_key, secure_link_mac_key, SL_WFX_SECURE_LINK_MAC_KEY_LENGTH);

    for (uint8_t index = 0; index < SL_WFX_SECURE_LINK_MAC_KEY_LENGTH; ++index)
    {
        // Assuming 0xFF... when not written
        if (sl_mac_key[index] != MAC_KEY_FAIL_BYTE)
        {
            result = SL_STATUS_OK;
            break;
        }
    }

    return result;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_compute_pub_key(sl_wfx_securelink_exchange_pub_keys_req_body_t *request,
                                        const uint8_t *sl_mac_key)
 * @brief
 * compute host public key
 * @param[in] request :
 * @param[in] sl_mac_key :
 * @return  returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_compute_pub_key(sl_wfx_securelink_exchange_pub_keys_req_body_t * request, const uint8_t * sl_mac_key)
{
    sl_status_t status = SL_STATUS_OK;

#if SL_WFX_SLK_CURVE25519
    const char identifier[] = "ecdh";

    mbedtls_ecdh_init(&mbedtls_host_context);
    mbedtls_ctr_drbg_init(&host_drbg_context);
    mbedtls_entropy_init(&entropy);
    status = mbedtls_ctr_drbg_seed(&host_drbg_context, mbedtls_entropy_func, &entropy, (const unsigned char *) identifier,
                                   sizeof(identifier));
    status += mbedtls_ecp_group_load(&mbedtls_host_context.grp, MBEDTLS_ECP_DP_CURVE25519);
    status += mbedtls_ecdh_gen_public(&mbedtls_host_context.grp, &mbedtls_host_context.d, &mbedtls_host_context.Q,
                                      mbedtls_ctr_drbg_random, &host_drbg_context);
    status += mbedtls_mpi_write_binary(&mbedtls_host_context.Q.X, request->host_pub_key, SL_WFX_HOST_PUB_KEY_SIZE);
#else
    mbedtls_entropy_init(&entropy);
    status                        = mbedtls_entropy_func(&entropy, request->host_pub_key, SL_WFX_HOST_PUB_KEY_SIZE);
#endif
    reverse_bytes(request->host_pub_key, SL_WFX_HOST_PUB_KEY_SIZE);
    SL_WFX_ERROR_CHECK(status);

    // Generate SHA512 digest of public key
    status = mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA512), sl_mac_key, SL_WFX_HOST_PUB_KEY_SIZE,
                             request->host_pub_key, SL_WFX_HOST_PUB_KEY_SIZE, request->host_pub_key_mac);
    SL_WFX_ERROR_CHECK(status);

error_handler:
    if (status != SL_STATUS_OK)
    {
        return SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;
    }
    return status;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_verify_pub_key(sl_wfx_securelink_exchange_pub_keys_ind_t *response_packet,
                                       const uint8_t *sl_mac_key,
                                       uint8_t *sl_host_pub_key)
 * @brief
 * verify host public key
 * @param[in]  response_packet:
 * @param[in]  sl_mac_key:
 * @param[in]  sl_host_pub_key:
 * @return returns SL_STATUS_OK if successful,
 *         SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_verify_pub_key(sl_wfx_securelink_exchange_pub_keys_ind_t * response_packet, const uint8_t * sl_mac_key,
                                       uint8_t * sl_host_pub_key)
{
    sl_status_t status = SL_STATUS_OK;
    uint8_t shared_key_digest[KEY_DIGEST_SIZE];

    if (xSemaphoreTake(wfx_securelink_rx_mutex, portMAX_DELAY) != pdTRUE)
    {
        return SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;
    }

    // Compute the Hash and verify the public key/hashing
    status = mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA512), sl_mac_key, SL_WFX_NCP_PUB_KEY_SIZE,
                             response_packet->body.ncp_pub_key, SL_WFX_NCP_PUB_KEY_SIZE, temp_key_location);
    SL_WFX_ERROR_CHECK(status);

    // Calculate session key if public key/SHA512 digest matches
    if (memcmp(temp_key_location, response_packet->body.ncp_pub_key_mac, SL_WFX_HOST_PUB_KEY_MAC_SIZE) != MEMCMP_FAIL)
    {
        status = SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;
        goto error_handler;
    }

#if SL_WFX_SLK_CURVE25519
    SL_WFX_UNUSED_PARAMETER(sl_host_pub_key);

    mbedtls_mpi_lset(&mbedtls_host_context.Qp.Z, MPI_SET);

    // Read Ineo public key
    reverse_bytes(response_packet->body.ncp_pub_key, SL_WFX_NCP_PUB_KEY_SIZE);
    mbedtls_mpi_read_binary(&mbedtls_host_context.Qp.X, response_packet->body.ncp_pub_key, SL_WFX_NCP_PUB_KEY_SIZE);

    // Calculate shared secret
    if (mbedtls_ecdh_compute_shared(&mbedtls_host_context.grp, &mbedtls_host_context.z, &mbedtls_host_context.Qp,
                                    &mbedtls_host_context.d, mbedtls_ctr_drbg_random,
                                    &host_drbg_context) != SUCCESS_STATUS_WIFI_SECURE_LINK_EXCHANGE)
    {
        status = SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;
        goto error_handler;
    }

    // Generate session key
    mbedtls_mpi_write_binary(&mbedtls_host_context.z, temp_key_location, SL_WFX_HOST_PUB_KEY_SIZE);
    reverse_bytes(temp_key_location, SL_WFX_HOST_PUB_KEY_SIZE);
    mbedtls_sha256(temp_key_location, SL_WFX_HOST_PUB_KEY_SIZE, shared_key_digest, SHA224_0);
#else
    uint8_t hmac_input[HMAC_SIZE] = { 0 };
    char label[LABLE_LEN]         = "SecureLink!KeyDerivation";

    memset((uint16_t *) &hmac_input[0], (uint16_t) sl_wfx_htole16(1), MEMSET_LEN);
    memcpy((uint8_t *) &hmac_input[2], (uint8_t *) label, LABLE_LEN);
    memcpy((uint8_t *) &hmac_input[26], sl_host_pub_key, SL_WFX_NCP_PUB_KEY_SIZE);
    memcpy((uint8_t *) &hmac_input[58], (uint8_t *) response_packet->body.ncp_pub_key, SL_WFX_NCP_PUB_KEY_SIZE);
    memset((uint16_t *) &hmac_input[90], (uint16_t) sl_wfx_htole16(128), 1);

    // Generate SHA256 digest of hmac_input
    status = mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), sl_mac_key, SL_WFX_HOST_PUB_KEY_SIZE,
                             (uint8_t *) hmac_input, HMAC_SIZE, shared_key_digest);
#endif

    memcpy(sl_wfx_context->secure_link_session_key, shared_key_digest,
           SL_WFX_SECURE_LINK_SESSION_KEY_LENGTH); // Use the lower 16 bytes of the sha256
    sl_wfx_context->secure_link_nonce.hp_packet_count = 0;
    sl_wfx_context->secure_link_nonce.rx_packet_count = 0;
    sl_wfx_context->secure_link_nonce.tx_packet_count = 0;

error_handler:
    if (xSemaphoreGive(wfx_securelink_rx_mutex) != pdTRUE)
    {
        printf("ERROR: sl_wfx_securelink_rx_mutex. unable to post.\n");
    }
    return status;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_free_crypto_context(void)
 * @brief
 * Free host crypto context
 * @param[in] None
 * @return returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_free_crypto_context(void)
{
#if SL_WFX_SLK_CURVE25519
    mbedtls_ecdh_free(&mbedtls_host_context);
    mbedtls_ctr_drbg_free(&host_drbg_context);
#endif
    mbedtls_entropy_free(&entropy);

    return SL_STATUS_OK;
}

/********************************************************************************
 * @fn   sl_status_t sl_wfx_host_decode_secure_link_data(uint8_t *buffer, uint32_t length, uint8_t *session_key)
 * @brief
 * Decode receive data
 * Length excludes size of CCM tag and secure link header
 * @param[in] buffer:
 * @param[in] length:
 * @param[in] session_key:
 * @return returns SL_STATUS_OK if successful,
 *         SL_STATUS_FAIL otherwise
 ********************************************************************************/
sl_status_t sl_wfx_host_decode_secure_link_data(uint8_t * buffer, uint32_t length, uint8_t * session_key)
{
    mbedtls_ccm_context ccm_context;
    sl_status_t status = SL_STATUS_SECURITY_DECRYPT_ERROR;
    int crypto_status;
    sl_wfx_nonce_t nonce = { 0, 0, 0 };

    if (xSemaphoreTake(wfx_securelink_rx_mutex, portMAX_DELAY) != pdTRUE)
    {
        return SL_STATUS_FAIL;
    }

    // Nonce for decryption should have TX and HP counters 0, only use RX counter
    nonce.rx_packet_count = sl_wfx_context->secure_link_nonce.rx_packet_count;

    // Init context
    mbedtls_ccm_init(&ccm_context);

    // Set the crypto key
    crypto_status = mbedtls_ccm_setkey(&ccm_context, MBEDTLS_CIPHER_ID_AES, session_key, SL_WFX_SECURE_LINK_SESSION_KEY_BIT_COUNT);
    SL_WFX_ERROR_CHECK(crypto_status);

    // Decrypt the data
    if (!mbedtls_ccm_auth_decrypt(&ccm_context, length, (uint8_t *) &nonce, SL_WFX_SECURE_LINK_NONCE_SIZE_BYTES, NULL,
                                  ADDRESS_LENGTH, (uint8_t *) buffer, (uint8_t *) buffer, (uint8_t *) buffer + length,
                                  SL_WFX_SECURE_LINK_CCM_TAG_SIZE))
    {
        status = SL_STATUS_OK;
    }

error_handler:
    mbedtls_ccm_free(&ccm_context);
    if (xSemaphoreGive(wfx_securelink_rx_mutex) != pdTRUE)
    {
        printf("ERROR: sl_wfx_securelink_rx_mutex. unable to post.\n");
    }
    return status;
}

/*********************************************************************
 * @fn  sl_status_t sl_wfx_host_encode_secure_link_data(sl_wfx_generic_message_t *buffer,
                                                uint32_t data_length,
                                                uint8_t *session_key,
                                                uint8_t *nonce)
 * @brief
 * Encode transmit data
 * Length excludes size of CCM tag and secure link header
 * @param[in]  buffer:
 * @param[in]  data_length:
 * @param[in]  session_key:
 * @param[in]  nonce:
 * @return  returns SL_STATUS_OK if successful,
 *         SL_STATUS_FAIL otherwise
*************************************************************************/
sl_status_t sl_wfx_host_encode_secure_link_data(sl_wfx_generic_message_t * buffer, uint32_t data_length, uint8_t * session_key,
                                                uint8_t * nonce)
{
    mbedtls_ccm_context ccm_context;
    sl_status_t status = SL_STATUS_FAIL;

    mbedtls_ccm_init(&ccm_context);
    if (mbedtls_ccm_setkey(&ccm_context, MBEDTLS_CIPHER_ID_AES, session_key, SL_WFX_SECURE_LINK_SESSION_KEY_BIT_COUNT) ==
        CCM_STATUS_SUCCESS)
    {
        mbedtls_ccm_encrypt_and_tag(&ccm_context, data_length, nonce, SL_WFX_SECURE_LINK_NONCE_SIZE_BYTES, NULL, ADDRESS_LENGTH,
                                    (uint8_t *) &buffer->header.id, (uint8_t *) &buffer->header.id,
                                    (uint8_t *) &buffer->header.id + data_length, SL_WFX_SECURE_LINK_CCM_TAG_SIZE);
        status = SL_STATUS_OK;
    }

    mbedtls_ccm_free(&ccm_context);

    return status;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_schedule_secure_link_renegotiation(void)
 * @brief
 * Called when the driver needs to schedule secure link renegotiation
 * @param[in] None
 * @returns Returns SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_schedule_secure_link_renegotiation(void)
{
    // call sl_wfx_secure_link_renegotiate_session_key() as soon as it makes sense for the host to do so
    xTaskNotifyGive(wfx_securelink_task);
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  static inline void reverse_bytes(uint8_t *src, uint8_t length)
 * @brief
 *         reverse the bytes
 * @param[in] src: source
 * @param[in] length:
 * @returns None
 *****************************************************************************/
static inline void reverse_bytes(uint8_t * src, uint8_t length)
{
    uint8_t * lo = src;
    uint8_t * hi = src + length - 1;
    uint8_t swap;

    while (lo < hi)
    {
        swap  = *lo;
        *lo++ = *hi;
        *hi-- = swap;
    }
}

/********************************************************************************************************
 ********************************************************************************************************
 *                                       DEPENDENCIES & AVAIL CHECK(S)
 ********************************************************************************************************
 *******************************************************************************************************/

#endif // SL_WFX_USE_SECURE_LINK
