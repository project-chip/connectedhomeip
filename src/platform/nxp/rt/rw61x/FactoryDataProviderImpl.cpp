/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright 2023 NXP
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

#include "FactoryDataProviderImpl.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#include "ELSFactoryData.h"
#include "mflash_drv.h"

#if CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
#include "fsl_adapter_flash.h"
#endif

#ifndef FACTORY_DATA_PROVIDER_RUN_TESTS
#define FACTORY_DATA_PROVIDER_RUN_TESTS 0
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#ifndef FACTORY_DATA_PROVIDER_LOG
#define FACTORY_DATA_PROVIDER_LOG 0
#endif

#if FACTORY_DATA_PROVIDER_LOG
#include "fsl_debug_console.h"
#define FACTORY_DATA_PROVIDER_PRINTF(...)                                                                                          \
    PRINTF("[%s] ", __FUNCTION__);                                                                                                 \
    PRINTF(__VA_ARGS__);                                                                                                           \
    PRINTF("\n\r");
#else
#define FACTORY_DATA_PROVIDER_PRINTF(...)
#endif

/* Grab symbol for the base address from the linker file. */
extern uint32_t __FACTORY_DATA_START_OFFSET[];
extern uint32_t __FACTORY_DATA_SIZE[];

using namespace ::chip::Credentials;
using namespace ::chip::Crypto;

namespace chip {
namespace DeviceLayer {

FactoryDataProviderImpl FactoryDataProviderImpl::sInstance;

CHIP_ERROR FactoryDataProviderImpl::SearchForId(uint8_t searchedType, uint8_t * pBuf, size_t bufLength, uint16_t & length,
                                                uint32_t * contentAddr)
{
    CHIP_ERROR err               = CHIP_ERROR_NOT_FOUND;
    uint8_t type                 = 0;
    uint32_t index               = 0;
    uint8_t * addrContent        = NULL;
    uint8_t * factoryDataAddress = &factoryDataRamBuffer[0];
    uint32_t factoryDataSize     = sizeof(factoryDataRamBuffer);
    uint16_t currentLen          = 0;

    while (index < factoryDataSize)
    {
        /* Read the type */
        memcpy((uint8_t *) &type, factoryDataAddress + index, sizeof(type));
        index += sizeof(type);

        /* Read the len */
        memcpy((uint8_t *) &currentLen, factoryDataAddress + index, sizeof(currentLen));
        index += sizeof(currentLen);

        /* Check if the type gotten is the expected one */
        if (searchedType == type)
        {
            FACTORY_DATA_PROVIDER_PRINTF("type = %d, currentLen = %d, bufLength =%d", type, currentLen, bufLength);
            /* If pBuf is null it means that we only want to know if the Type has been found */
            if (pBuf != NULL)
            {
                /* If the buffer given is too small, fill only the available space */
                if (bufLength < currentLen)
                {
                    currentLen = bufLength;
                }
                memcpy((uint8_t *) pBuf, factoryDataAddress + index, currentLen);
            }
            length = currentLen;
            if (contentAddr != NULL)
            {
                *contentAddr = (uint32_t) factoryDataAddress + index;
            }
            err = CHIP_NO_ERROR;
            break;
        }
        else if (type == 0)
        {
            /* No more type available , break the loop */
            break;
        }
        else
        {
            /* Jump to next data */
            index += currentLen;
        }
    }

    return err;
}

CHIP_ERROR FactoryDataProviderImpl::SignWithDacKey(const ByteSpan & digestToSign, MutableByteSpan & outSignBuffer)
{
    uint8_t els_key_blob[MAX_ELS_KEY_SIZE + ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD];
    uint16_t keySize = 0;
    uint32_t keyAddr;
    status_t status          = STATUS_SUCCESS;
    size_t els_key_blob_size = sizeof(els_key_blob);
    uint8_t digest[kSHA256_Hash_Length];
    uint8_t public_key[64]         = { 0 };
    size_t public_key_size         = sizeof(public_key);
    mcuxClEls_KeyIndex_t key_index = MCUXCLELS_KEY_SLOTS;

    mcuxClEls_KeyProp_t plain_key_properties = {
        .word = { .value = MCUXCLELS_KEYPROPERTY_VALUE_SECURE | MCUXCLELS_KEYPROPERTY_VALUE_PRIVILEGED |
                      MCUXCLELS_KEYPROPERTY_VALUE_KEY_SIZE_256 | MCUXCLELS_KEYPROPERTY_VALUE_KGSRC }
    };

    mcuxClEls_EccSignOption_t sign_options = { 0 };
    mcuxClEls_EccByte_t ecc_signature[MCUXCLELS_ECC_SIGNATURE_SIZE];
    uint8_t psa_import_blob[256];
    size_t psa_import_blob_size = sizeof(psa_import_blob);

    /* Search key ID FactoryDataId::kDacPrivateKeyId */
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, NULL, 0, keySize, &keyAddr));
    memcpy(els_key_blob, (uint8_t *) keyAddr, keySize);

    PLOG_DEBUG_BUFFER("els_key_blob", els_key_blob, els_key_blob_size);

    /* Calculate message HASH to sign */
    memset(&digest[0], 0, sizeof(digest));
    ReturnErrorOnFailure(Hash_SHA256(digestToSign.data(), digestToSign.size(), &digest[0]));

    PLOG_DEBUG_BUFFER("digestToSign", digestToSign.data(), digestToSign.size());

    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, 256);
    psa_set_key_lifetime(
        &attributes,
        PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_PERSISTENT, PSA_KEY_LOCATION_S50_BLOB_STORAGE));
    psa_set_key_id(&attributes, 0x3E000021);

    /*To satisfy PSA APIs and to bind key attributes on PSA level to the key, a TLV structure is created
     containing the els_key_blob and metadata. That TLV structure gets a CMAC. This structure is quasi
     identical to what EdgeLock 2GO is creating. Note that the TLV structure is not used below but
     required in case a blob shall be imported into TF-M for RW61x. */
    status = create_psa_import_blob(els_key_blob, els_key_blob_size, &attributes, psa_import_blob, &psa_import_blob_size);
    STATUS_SUCCESS_OR_EXIT_MSG("export_key_from_els failed: 0x%08x", status);
    PLOG_DEBUG_BUFFER("psa_import_blob", psa_import_blob, psa_import_blob_size);

    /* Import blob DAC key into SE50 (reserved key slot) */
    status = import_die_int_wrapped_key_into_els(els_key_blob, els_key_blob_size, plain_key_properties, &key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("import_die_int_wrapped_key_into_els failed: 0x%08x", status);

    /* For ECC keys that were created from plain key material, there is the
     neceessity to convert them to a key. Converting to a key also yields the public key.
     The conversion can be done either before re-wrapping (when importing the plain key)
     or after (when importing the blob).*/
    status = els_keygen(key_index, &public_key[0], &public_key_size);
    STATUS_SUCCESS_OR_EXIT_MSG("els_keygen failed: 0x%08x", status);

    /* The key is usable for signing. */
    PLOG_DEBUG_BUFFER("public_key", public_key, public_key_size);

    /* ECC sign message hash with the key index slot reserved during the blob importation */
    ELS_sign_hash(digest, ecc_signature, &sign_options, key_index);

    /* Delete SE50 key with the index slot reserved during the blob importation (free key slot) */
    els_delete_key(key_index);

    /* Generate MutableByteSpan with ECC signature and ECC signature size */
    return CopySpanToMutableSpan(ByteSpan{ ecc_signature, MCUXCLELS_ECC_SIGNATURE_SIZE }, outSignBuffer);
exit:
    els_delete_key(key_index);
    return CHIP_ERROR_INVALID_SIGNATURE;
}

CHIP_ERROR FactoryDataProviderImpl::Init(void)
{
#if FACTORY_DATA_PROVIDER_RUN_TESTS
    unittest();
#else
    els_enable();

    uint16_t len;
    status_t status;
    uint32_t factoryDataAddress = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize    = (uint32_t) __FACTORY_DATA_SIZE;
    uint32_t hashId;
    uint8_t currentBlock[BLOCK_SIZE_16_BYTES];
    uint8_t calculatedHash[SHA256_OUTPUT_SIZE];
    uint16_t i;
    CHIP_ERROR res;

    /* Init mflash */
    status = mflash_drv_init();

    if (status != kStatus_Success || factoryDataSize > sizeof(factoryDataRamBuffer))
        return CHIP_ERROR_INTERNAL;

    /* Read hash id saved in flash */
    if (mflash_drv_read(factoryDataAddress, (uint32_t *) &mHeader, sizeof(mHeader)) != kStatus_Success)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (mHeader.hashId != HASH_ID)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    /* Update address to start after hash id to read size of factory data */
    factoryDataAddress += sizeof(mHeader);

    /* Load the buffer into RAM by reading each 16 bytes blocks */
    for (i = 0; i < (factoryDataSize / BLOCK_SIZE_16_BYTES); i++)
    {
        if (mflash_drv_read(factoryDataAddress + i * BLOCK_SIZE_16_BYTES, (uint32_t *) &currentBlock[0], sizeof(currentBlock)) !=
            kStatus_Success)
        {
            return CHIP_ERROR_INTERNAL;
        }

        /* Store the block unencrypted */
        memcpy(&factoryDataRamBuffer[i * BLOCK_SIZE_16_BYTES], &currentBlock[0], sizeof(currentBlock));
    }

    /* Calculate SHA256 value over the factory data and compare with stored value */
    res = Hash256(&factoryDataRamBuffer[0], mHeader.size, &calculatedHash[0]);

    if (res != CHIP_NO_ERROR)
        return res;

    if (memcmp(&calculatedHash[0], &mHeader.hash[0], HASH_LEN) != 0)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    ReturnErrorOnFailure(SearchForId(FactoryDataId::kVerifierId, NULL, 0, len));
    FACTORY_DATA_PROVIDER_PRINTF("[%d] len = %d", FactoryDataId::kVerifierId, len);
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kSaltId, NULL, 0, len));
    FACTORY_DATA_PROVIDER_PRINTF("[%d] len = %d", FactoryDataId::kSaltId, len);
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kIcId, NULL, 0, len));
    FACTORY_DATA_PROVIDER_PRINTF("[%d] len = %d", FactoryDataId::kIcId, len);
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, NULL, 0, len));
    FACTORY_DATA_PROVIDER_PRINTF("[%d] len = %d", FactoryDataId::kDacPrivateKeyId, len);
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacCertificateId, NULL, 0, len));
    FACTORY_DATA_PROVIDER_PRINTF("[%d] len = %d", FactoryDataId::kDacCertificateId, len);
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kPaiCertificateId, NULL, 0, len));
    FACTORY_DATA_PROVIDER_PRINTF("[%d] len = %d", FactoryDataId::kPaiCertificateId, len);
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDiscriminatorId, NULL, 0, len));

#if CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
    ReturnErrorOnFailure(ELS_ConvertDacKey());
#endif
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::Hash256(const uint8_t * input, size_t inputSize, uint8_t * output)
{
    CHIP_ERROR res;
    res = Hash_SHA256(input, inputSize, output);

    return res;
}

#if CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
static inline uint32_t els_get_key_size(mcuxClEls_KeyIndex_t keyIdx)
{
    mcuxClEls_KeyProp_t key_properties;
    key_properties.word.value = ((const volatile uint32_t *) (&ELS->ELS_KS0))[keyIdx];
    return (key_properties.bits.ksize == MCUXCLELS_KEYPROPERTY_KEY_SIZE_256) ? (256U / 8U) : (128U / 8U);
}

static status_t els_export_key(mcuxClEls_KeyIndex_t src_key_index, mcuxClEls_KeyIndex_t wrap_key_index, uint8_t * els_key_out_blob,
                               size_t * els_key_out_blob_size)

{
    uint32_t key_size           = els_get_key_size(src_key_index);
    uint32_t required_blob_size = ELS_BLOB_METADATA_SIZE + key_size + ELS_WRAP_OVERHEAD;
    assert(required_blob_size <= *els_key_out_blob_size);

    *els_key_out_blob_size = required_blob_size;

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_KeyExport_Async(wrap_key_index, src_key_index, els_key_out_blob));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_KeyExport_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_KeyExport_Async failed: 0x%08lx", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08lx", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

static status_t export_key_from_els(mcuxClEls_KeyIndex_t key_index, uint8_t * output, size_t * output_size)
{
    assert(output != NULL);
    status_t status = STATUS_SUCCESS;

    mcuxClEls_KeyIndex_t key_wrap_out_index = MCUXCLELS_KEY_SLOTS;
    PLOG_INFO("Deriving wrapping key for export on ELS...");
    status = els_derive_key(DIE_INT_MK_SK_INDEX, wrap_out_key_prop, ckdf_derivation_data_wrap_out, &key_wrap_out_index);
    STATUS_SUCCESS_OR_EXIT_MSG("derive_key failed: 0x%08x", status);

    PLOG_INFO("Exporting/wrapping key...");
    status = els_export_key(key_index, key_wrap_out_index, output, output_size);
    STATUS_SUCCESS_OR_EXIT_MSG("export_key failed: 0x%08x", status);

    status = els_delete_key(key_wrap_out_index);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
    key_wrap_out_index = MCUXCLELS_KEY_SLOTS;
exit:
    return status;
}

CHIP_ERROR FactoryDataProviderImpl::ELS_ConvertDacKey()
{
    size_t blobSize = MAX_ELS_KEY_SIZE + ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD;
    size_t newSize  = sizeof(Header) + mHeader.size + (ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD);
    uint8_t blob[Crypto::kP256_PrivateKey_Length + (ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD)] = { 0 };
    uint32_t KeyAddr;
    uint32_t factoryDataAddress = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize    = (uint32_t) __FACTORY_DATA_SIZE;
    uint8_t * data              = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(newSize));

    VerifyOrReturnError(factoryDataRamBuffer != nullptr, CHIP_ERROR_INTERNAL);

    /* Import pain DAC key and generate the blob */
    ReturnErrorOnFailure(ELS_ExportBlob(blob, &blobSize, KeyAddr));
    ChipLogError(DeviceLayer, "SSS: extracted blob from DAC private key");

    /* Read all factory data */
    hal_flash_status_t status =
        HAL_FlashRead(factoryDataAddress + MFLASH_BASE_ADDRESS, newSize - (ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD), data);
    VerifyOrReturnError(status == kStatus_HAL_Flash_Success, CHIP_ERROR_INTERNAL);
    ChipLogError(DeviceLayer, "SSS: cached factory data in RAM");

    /* Replace private plain DAC key by the blob into factory data RAM buffer (the blob length is higher then the plain key length)
     */
    ReturnErrorOnFailure(ReplaceWithBlob(data, blob, blobSize, KeyAddr));
    ChipLogError(DeviceLayer, "SSS: replaced DAC private key with secured blob");
    PLOG_DEBUG_BUFFER("ReplaceWithBlob", data, newSize);

    /* Erase flash factory data sectors */
    status = HAL_FlashEraseSector(factoryDataAddress + MFLASH_BASE_ADDRESS, factoryDataSize);
    VerifyOrReturnError(status == kStatus_HAL_Flash_Success, CHIP_ERROR_INTERNAL);
    /* Write new factory data into flash */
    status = HAL_FlashProgramUnaligned(factoryDataAddress + MFLASH_BASE_ADDRESS, newSize, data);
    VerifyOrReturnError(status == kStatus_HAL_Flash_Success, CHIP_ERROR_INTERNAL);
    ChipLogError(DeviceLayer, "SSS: updated factory data");

    return CHIP_NO_ERROR;
}

static status_t els_generate_keypair(mcuxClEls_KeyIndex_t * dst_key_index, uint8_t * public_key, size_t * public_key_size)
{
    if (*public_key_size < 64)
    {
        PLOG_ERROR("insufficient space for public key");
        return STATUS_ERROR_GENERIC;
    }

    mcuxClEls_EccKeyGenOption_t options = { 0 };
    options.bits.kgsrc                  = MCUXCLELS_ECC_OUTPUTKEY_RANDOM;
    options.bits.kgtypedh               = MCUXCLELS_ECC_OUTPUTKEY_KEYEXCHANGE;

    uint32_t keypair_required_keyslots = get_required_keyslots(keypair_prop);
    *dst_key_index                     = (mcuxClEls_KeyIndex_t) els_get_free_keyslot(keypair_required_keyslots);

    if (!(*dst_key_index < MCUXCLELS_KEY_SLOTS))
    {
        PLOG_ERROR("no free keyslot found");
        return STATUS_ERROR_GENERIC;
    }

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(
        result, token,
        mcuxClEls_EccKeyGen_Async(options, (mcuxClEls_KeyIndex_t) 0U, *dst_key_index, keypair_prop, NULL, public_key));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_EccKeyGen_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_EccKeyGen_Async failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    *public_key_size = 64;
    return STATUS_SUCCESS;
}

static status_t els_get_random(unsigned char * out, size_t out_size)
{
    /* Get random IV for sector metadata encryption. */
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClCss_Rng_DrbgRequest_Async(out, out_size));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClCss_Rng_DrbgRequest_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PRINTF("mcuxClCss_Rng_DrbgRequest_Async failed: 0x%08lx\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClCss_WaitForOperation(MCUXCLCSS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PRINTF("Css_EccKeyGen_Async WaitForOperation failed: 0x%08lx\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

static int get_random_mbedtls_callback(void * ctx, unsigned char * out, size_t out_size)
{
    status_t status = els_get_random(out, out_size);
    if (status != STATUS_SUCCESS)
    {
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }
    return 0;
}

static status_t host_perform_key_agreement(const uint8_t * public_key, size_t public_key_size, uint8_t * shared_secret,
                                           size_t * shared_secret_size)
{
    assert(public_key != NULL);
    assert(public_key_size == 64);
    assert(shared_secret != NULL);
    assert(*shared_secret_size >= 32);

    status_t status = STATUS_SUCCESS;

    int ret = 0;
    mbedtls_ecp_group grp;
    mbedtls_ecp_point qB;
    mbedtls_mpi dA, zA;
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&qB);
    mbedtls_mpi_init(&dA);
    mbedtls_mpi_init(&zA);

    unsigned char strbuf[128] = { 0 };
    size_t strlen             = sizeof(strbuf);

    uint8_t public_key_compressed[65] = { 0 };
    public_key_compressed[0]          = 0x04;

    *shared_secret_size = 32;
    ret                 = mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_ecp_group_load failed: 0x%08x", ret);

    ret = mbedtls_mpi_read_binary(&dA, import_die_int_ecdh_sk, sizeof(import_die_int_ecdh_sk));
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_mpi_read_binary failed: 0x%08x", ret);

    memcpy(&public_key_compressed[1], public_key, public_key_size);

    ret = mbedtls_ecp_point_read_binary(&grp, &qB, public_key_compressed, sizeof(public_key_compressed));
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_ecp_point_read_binary failed: 0x%08x", ret);

    ret = mbedtls_ecdh_compute_shared(&grp, &zA, &qB, &dA, &get_random_mbedtls_callback, NULL);
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_ecdh_compute_shared failed: 0x%08x", ret);

    mbedtls_ecp_point_write_binary(&grp, &qB, MBEDTLS_ECP_PF_UNCOMPRESSED, &strlen, &strbuf[0], sizeof(strbuf));
    printf_buffer("public_key", strbuf, strlen);

    mbedtls_mpi_write_binary(&zA, shared_secret, *shared_secret_size);
    PLOG_DEBUG_BUFFER("shared_secret", shared_secret, *shared_secret_size);
exit:
    return status;
}

static status_t host_derive_key(const uint8_t * input_key, size_t input_key_size, const uint8_t * derivation_data,
                                size_t derivation_data_size, uint32_t key_properties, uint8_t * output, size_t * output_size)
{
    status_t status = STATUS_SUCCESS;

    int ret          = 0;
    uint32_t counter = 1;
    mbedtls_cipher_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    bool ctx_valid = false;

    assert(input_key != NULL);
    assert(input_key_size == 32);
    assert(derivation_data != NULL);
    assert(derivation_data_size == 12);
    assert(output != NULL);
    assert(*output_size == 32);

    uint32_t lsbit         = key_properties & 0x01;
    uint32_t length_blocks = 1 + lsbit;
    uint32_t length_bytes  = length_blocks * AES_BLOCK_SIZE;
    assert(*output_size >= length_bytes);
    *output_size = length_bytes;

    // KDF in counter mode implementation as described in Section 5.1
    // of NIST SP 800-108, Recommendation for Key Derivation Using Pseudorandom Functions
    //  Derivation data[191:0](sic!) = software_derivation_data[95:0] || 64'h0 || requested_
    //  properties[31:0 || length[31:0] || counter[31:0]

    uint8_t dd[32] = { 0 };
    memcpy(&dd[0], derivation_data, derivation_data_size);
    memset(&dd[12], 0, 8);
    write_uint32_msb_first(&dd[20], key_properties);
    write_uint32_msb_first(&dd[24], length_bytes * 8); // expected in bits!
    write_uint32_msb_first(&dd[28], counter);

    mbedtls_cipher_type_t mbedtls_cipher_type = MBEDTLS_CIPHER_AES_256_ECB;
    const mbedtls_cipher_info_t * cipher_info = mbedtls_cipher_info_from_type(mbedtls_cipher_type);

    PLOG_DEBUG_BUFFER("input_key", input_key, input_key_size);
    PLOG_DEBUG_BUFFER("dd", dd, sizeof(dd));

    uint8_t * pos = output;
    do
    {
        mbedtls_cipher_init(&ctx);
        ctx_valid = true;

        ret = mbedtls_cipher_setup(&ctx, cipher_info);
        RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_cipher_setup failed: 0x%08x", ret);

        ret = mbedtls_cipher_cmac_starts(&ctx, input_key, input_key_size * 8);
        RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_cipher_cmac_starts failed: 0x%08x", ret);

        ret = mbedtls_cipher_cmac_update(&ctx, dd, sizeof(dd));
        RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_cipher_cmac_update failed: 0x%08x", ret);

        ret = mbedtls_cipher_cmac_finish(&ctx, pos);
        RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_cipher_cmac_finish failed: 0x%08x", ret);

        mbedtls_cipher_free(&ctx);
        ctx_valid = false;

        write_uint32_msb_first(&dd[28], ++counter);
        pos += AES_BLOCK_SIZE;
    } while (counter * AES_BLOCK_SIZE <= length_bytes);

    PLOG_DEBUG_BUFFER("output", output, length_bytes);

exit:
    if (ctx_valid)
    {
        mbedtls_cipher_free(&ctx);
        ctx_valid = false;
    }

    return status;
}

static status_t host_wrap_key(const uint8_t * data, size_t data_size, const uint8_t * key, size_t key_size, uint8_t * output,
                              size_t * output_size)
{
    status_t status = STATUS_SUCCESS;
    int ret         = 0;
    mbedtls_nist_kw_context ctx;
    mbedtls_nist_kw_init(&ctx);
    ret = mbedtls_nist_kw_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, key_size * 8, true);
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_nist_kw_setkey failed: 0x%08x", ret);
    ret = mbedtls_nist_kw_wrap(&ctx, MBEDTLS_KW_MODE_KW, data, data_size, output, output_size, *output_size);
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_nist_kw_wrap failed: 0x%08x", ret);
    PLOG_DEBUG_BUFFER("wrapped buffer", output, *output_size);
exit:
    mbedtls_nist_kw_free(&ctx);
    return status;
}

static status_t create_els_import_keyblob(const uint8_t * plain_key, size_t plain_key_size, mcuxClEls_KeyProp_t plain_key_prop,
                                          const uint8_t * key_wrap_in, size_t key_wrap_in_size, uint8_t * blob, size_t * blob_size)
{
    assert(plain_key_size == 16 || plain_key_size == 32);
    assert(key_wrap_in_size == 16);

    uint8_t buffer[ELS_BLOB_METADATA_SIZE + MAX_ELS_KEY_SIZE] = { 0 };
    size_t buffer_size                                        = ELS_BLOB_METADATA_SIZE + plain_key_size;

    // Enforce the wrpok bit - the key needs to be re-wrappable!
    plain_key_prop.bits.wrpok = MCUXCLELS_KEYPROPERTY_WRAP_TRUE;

    // This is what ELS documentation says. It does not work though??
    // memset(&buffer[0], 0xA6, 8);
    // write_uint32_msb_first(&buffer[8], plain_key_prop.word.value);
    // memset(&buffer[12], 0, 4);
    // memcpy(&buffer[16], plain_key, plain_key_size);

    write_uint32_msb_first(&buffer[0], plain_key_prop.word.value);
    memset(&buffer[4], 0, 4);
    memcpy(&buffer[8], plain_key, plain_key_size);
    PLOG_DEBUG_BUFFER("plain buffer before wrapping for import", buffer, buffer_size);

    status_t status = host_wrap_key(buffer, buffer_size, key_wrap_in, key_wrap_in_size, blob, blob_size);
    return status;
}

static status_t els_perform_key_agreement(mcuxClEls_KeyIndex_t keypair_index, mcuxClEls_KeyProp_t shared_secret_prop,
                                          mcuxClEls_KeyIndex_t * dst_key_index, const uint8_t * public_key, size_t public_key_size)
{
    uint32_t shared_secret_required_keyslots = get_required_keyslots(shared_secret_prop);
    *dst_key_index                           = els_get_free_keyslot(shared_secret_required_keyslots);

    if (!(*dst_key_index < MCUXCLELS_KEY_SLOTS))
    {
        PLOG_ERROR("no free keyslot found");
        return STATUS_ERROR_GENERIC;
    }

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token,
                                     mcuxClEls_EccKeyExchange_Async(keypair_index, public_key, *dst_key_index, shared_secret_prop));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_EccKeyExchange_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_EccKeyExchange_Async failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    return STATUS_SUCCESS;
}

static status_t import_plain_key_into_els(const uint8_t * plain_key, size_t plain_key_size, mcuxClEls_KeyProp_t key_properties,
                                          mcuxClEls_KeyIndex_t * index_output)
{
    status_t status                                       = STATUS_SUCCESS;
    mcuxClEls_KeyIndex_t index_plain                      = MCUXCLELS_KEY_SLOTS;
    mcuxClEls_KeyIndex_t index_shared_secret              = MCUXCLELS_KEY_SLOTS;
    mcuxClEls_KeyIndex_t index_unwrap                     = MCUXCLELS_KEY_SLOTS;
    mcuxClEls_KeyIndex_t * potentially_used_key_indices[] = { &index_plain, &index_shared_secret, &index_unwrap };

    uint8_t els_key_in_blob[ELS_BLOB_METADATA_SIZE + MAX_ELS_KEY_SIZE + ELS_WRAP_OVERHEAD];
    size_t els_key_in_blob_size = sizeof(els_key_in_blob);

    uint8_t shared_secret[32] = { 0 };
    size_t shared_secret_len  = sizeof(shared_secret);

    uint8_t key_wrap_in[32];
    size_t key_wrap_in_size = sizeof(key_wrap_in);

    PLOG_INFO("Generating random ECC keypair...");
    uint8_t public_key[64] = { 0u };
    size_t public_key_size = sizeof(public_key);
    status                 = els_generate_keypair(&index_plain, &public_key[0], &public_key_size);
    STATUS_SUCCESS_OR_EXIT_MSG("generate_keypair failed: 0x%08x", status);

    PLOG_INFO("Calculating shared secret on host...");
    status = host_perform_key_agreement(public_key, public_key_size, &shared_secret[0], &shared_secret_len);
    STATUS_SUCCESS_OR_EXIT_MSG("perform_key_agreement_host failed: 0x%08x", status);

    PLOG_INFO("Deriving wrapping key for import on host...");
    status = host_derive_key(shared_secret, shared_secret_len, ckdf_derivation_data_wrap_in, sizeof(ckdf_derivation_data_wrap_in),
                             wrap_in_key_prop.word.value, &key_wrap_in[0], &key_wrap_in_size);
    STATUS_SUCCESS_OR_EXIT_MSG("ckdf_host failed: 0x%08x", status);

    PLOG_INFO("Creating ELS keyblob for import...");

    status = create_els_import_keyblob(plain_key, plain_key_size, key_properties, key_wrap_in, key_wrap_in_size,
                                       &els_key_in_blob[0], &els_key_in_blob_size);
    STATUS_SUCCESS_OR_EXIT_MSG("create_els_import_keyblob failed: 0x%08x", status);

    PLOG_INFO("Calculating shared secret on ELS...");
    status = els_perform_key_agreement(index_plain, shared_secret_prop, &index_shared_secret, import_die_int_ecdh_pk,
                                       sizeof(import_die_int_ecdh_pk));
    STATUS_SUCCESS_OR_EXIT_MSG("perform_key_agreement failed: 0x%08x", status);

    status = els_delete_key(index_plain);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
    index_plain = MCUXCLELS_KEY_SLOTS;

    PLOG_INFO("Deriving wrapping key for import on ELS...");
    status = els_derive_key(index_shared_secret, wrap_in_key_prop, ckdf_derivation_data_wrap_in, &index_unwrap);
    STATUS_SUCCESS_OR_EXIT_MSG("derive_key failed: 0x%08x", status);

    status = els_delete_key(index_shared_secret);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
    index_shared_secret = MCUXCLELS_KEY_SLOTS;

    PLOG_INFO("Importing wrapped key...");
    status = els_import_key(els_key_in_blob, els_key_in_blob_size, key_properties, index_unwrap, index_output);
    STATUS_SUCCESS_OR_EXIT_MSG("import_wrapped_key failed: 0x%08x", status);

    status = els_delete_key(index_unwrap);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
    index_unwrap = MCUXCLELS_KEY_SLOTS;

exit:
    for (size_t i = 0; i < ARRAY_SIZE(potentially_used_key_indices); i++)
    {
        mcuxClEls_KeyIndex_t key_index = *(potentially_used_key_indices[i]);
        if (key_index < MCUXCLELS_KEY_SLOTS)
        {
            (void) els_delete_key(key_index);
        }
    }
    return status;
}

CHIP_ERROR FactoryDataProviderImpl::ELS_ExportBlob(uint8_t * data, size_t * dataLen, uint32_t & addr)
{
    uint8_t keyBuf[Crypto::kP256_PrivateKey_Length];
    MutableByteSpan dacPrivateKeySpan(keyBuf);
    uint16_t keySize = 0;

    status_t status                 = STATUS_SUCCESS;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, 256);
    psa_set_key_lifetime(
        &attributes,
        PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_PERSISTENT, PSA_KEY_LOCATION_S50_BLOB_STORAGE));
    psa_set_key_id(&attributes, 0x3E000021);

    /* Search key ID FactoryDataId::kDacPrivateKeyId */
    ReturnErrorOnFailure(
        SearchForId(FactoryDataId::kDacPrivateKeyId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize, &addr));
    dacPrivateKeySpan.reduce_size(keySize);
    PLOG_DEBUG_BUFFER("Private DAC key plain", dacPrivateKeySpan.data(), dacPrivateKeySpan.size());

    mcuxClEls_KeyProp_t plain_key_properties = {
        .word = { .value = MCUXCLELS_KEYPROPERTY_VALUE_SECURE | MCUXCLELS_KEYPROPERTY_VALUE_PRIVILEGED |
                      MCUXCLELS_KEYPROPERTY_VALUE_KEY_SIZE_256 | MCUXCLELS_KEYPROPERTY_VALUE_KGSRC }
    };

    mcuxClEls_KeyIndex_t key_index = MCUXCLELS_KEY_SLOTS;
    /* Import plain DAC key into S50 */
    status = import_plain_key_into_els(dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), plain_key_properties, &key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("derive_key failed: 0x%08x", status);

    /* ELS generate key blob. The blob created here is one that can be directly imported into ELS again. */
    status = export_key_from_els(key_index, data, dataLen);
    STATUS_SUCCESS_OR_EXIT_MSG("export_key_from_els failed: 0x%08x", status);

    status = els_delete_key(key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::ReplaceWithBlob(uint8_t * data, uint8_t * blob, size_t blobLen, uint32_t KeyAddr)
{
    size_t newSize                           = mHeader.size + ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD;
    FactoryDataProviderImpl::Header * header = reinterpret_cast<FactoryDataProviderImpl::Header *>(data);
    uint8_t * payload                        = data + sizeof(FactoryDataProviderImpl::Header);
    uint8_t offset                           = (uint8_t *) (KeyAddr - kValueOffset) - (uint8_t *) &factoryDataRamBuffer[0];
    size_t subsequentDataOffset              = offset + kValueOffset + Crypto::kP256_PrivateKey_Length;

    memmove(payload + subsequentDataOffset + ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD, payload + subsequentDataOffset,
            mHeader.size - subsequentDataOffset);
    header->size = newSize;
    /* Update associated TLV length */
    memcpy(payload + offset + kLengthOffset, (uint16_t *) &blobLen, sizeof(uint16_t));
    /* Replace private plain DAC key by the blob */
    memcpy(payload + offset + kValueOffset, blob, blobLen);

    /* Update Header with new hash value */
    uint8_t hash[Crypto::kSHA256_Hash_Length] = { 0 };
    ReturnErrorOnFailure(Crypto::Hash_SHA256(payload, header->size, hash));
    memcpy(header->hash, hash, sizeof(header->hash));

    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY

CHIP_ERROR FactoryDataProviderImpl::unittest(void)
{
#if FACTORY_DATA_PROVIDER_RUN_TESTS
    CHIP_ERROR res;

    uint8_t ecc_message[295] = {
        0x15, 0x30, 0x01, 0xec, 0x30, 0x81, 0xe9, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x81,
        0xdb, 0x30, 0x81, 0xd8, 0x02, 0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04,
        0x02, 0x01, 0x30, 0x45, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x38, 0x04, 0x36, 0x15,
        0x24, 0x00, 0x01, 0x25, 0x01, 0x37, 0x10, 0x36, 0x02, 0x05, 0x26, 0xa2, 0x18, 0x25, 0x03, 0x01, 0x03, 0x2c, 0x04, 0x13,
        0x5a, 0x49, 0x47, 0x32, 0x30, 0x31, 0x34, 0x32, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x33, 0x2d, 0x32, 0x34, 0x24,
        0x05, 0x00, 0x24, 0x06, 0x00, 0x25, 0x07, 0x76, 0x98, 0x24, 0x08, 0x01, 0x18, 0x31, 0x7d, 0x30, 0x7b, 0x02, 0x01, 0x03,
        0x80, 0x14, 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3,
        0x71, 0x60, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a,
        0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x04, 0x47, 0x30, 0x45, 0x02, 0x21, 0x00, 0xc3, 0xbf, 0xd1, 0xcb, 0xed, 0x21,
        0x1e, 0x54, 0x76, 0x81, 0xa6, 0xfa, 0x08, 0x8f, 0x26, 0xce, 0x14, 0x8c, 0x72, 0xae, 0x1b, 0x6f, 0x61, 0x18, 0x0f, 0x6f,
        0x01, 0xc2, 0x75, 0xad, 0x6e, 0x5e, 0x02, 0x20, 0x31, 0x11, 0x00, 0x88, 0xcc, 0xc9, 0x98, 0x55, 0x0e, 0xf1, 0xd2, 0x42,
        0x07, 0x7a, 0xaa, 0x41, 0x0c, 0xd2, 0xd3, 0xd4, 0x76, 0xab, 0xd5, 0xaf, 0x32, 0x2c, 0x45, 0x75, 0xfa, 0xcc, 0x51, 0x5b,
        0x30, 0x02, 0x20, 0x73, 0x09, 0xbb, 0x01, 0xa5, 0xae, 0x2f, 0xfc, 0x0b, 0x7f, 0xee, 0xcb, 0xa0, 0xc4, 0x94, 0xf1, 0xd3,
        0x61, 0xce, 0x4a, 0x83, 0x21, 0x5e, 0x84, 0x07, 0xcf, 0x42, 0xc5, 0xee, 0xea, 0x1a, 0x2e, 0x24, 0x03, 0x00, 0x18, 0x90,
        0x75, 0x48, 0x87, 0x85, 0x5f, 0x73, 0xb0, 0xcb, 0x3e, 0x38, 0xa7, 0xbd, 0xad, 0x22, 0xf4
    };

    const uint8_t kDevelopmentDAC_Cert_FFF1_8002[492] = {
        0x30, 0x82, 0x01, 0xe8, 0x30, 0x82, 0x01, 0x8e, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x52, 0x72, 0x4d, 0x21, 0xe2,
        0xc1, 0x74, 0xaf, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x3d, 0x31, 0x25, 0x30,
        0x23, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x1c, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x44, 0x65, 0x76, 0x20, 0x50,
        0x41, 0x49, 0x20, 0x30, 0x78, 0x46, 0x46, 0x46, 0x31, 0x20, 0x6e, 0x6f, 0x20, 0x50, 0x49, 0x44, 0x31, 0x14, 0x30, 0x12,
        0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c, 0x04, 0x46, 0x46, 0x46, 0x31, 0x30, 0x20,
        0x17, 0x0d, 0x32, 0x32, 0x30, 0x32, 0x30, 0x35, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x18, 0x0f, 0x39, 0x39, 0x39,
        0x39, 0x31, 0x32, 0x33, 0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x53, 0x31, 0x25, 0x30, 0x23, 0x06, 0x03,
        0x55, 0x04, 0x03, 0x0c, 0x1c, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x44, 0x65, 0x76, 0x20, 0x44, 0x41, 0x43, 0x20,
        0x30, 0x78, 0x46, 0x46, 0x46, 0x31, 0x2f, 0x30, 0x78, 0x38, 0x30, 0x30, 0x32, 0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b,
        0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c, 0x04, 0x46, 0x46, 0x46, 0x31, 0x31, 0x14, 0x30, 0x12, 0x06,
        0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x02, 0x0c, 0x04, 0x38, 0x30, 0x30, 0x32, 0x30, 0x59, 0x30,
        0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07,
        0x03, 0x42, 0x00, 0x04, 0xda, 0x93, 0xf1, 0x67, 0x36, 0x25, 0x67, 0x50, 0xd9, 0x03, 0xb0, 0x34, 0xba, 0x45, 0x88, 0xab,
        0xaf, 0x58, 0x95, 0x4f, 0x77, 0xaa, 0x9f, 0xd9, 0x98, 0x9d, 0xfd, 0x40, 0x0d, 0x7a, 0xb3, 0xfd, 0xc9, 0x75, 0x3b, 0x3b,
        0x92, 0x1b, 0x29, 0x4c, 0x95, 0x0f, 0xd9, 0xd2, 0x80, 0xd1, 0x4c, 0x43, 0x86, 0x2f, 0x16, 0xdc, 0x85, 0x4b, 0x00, 0xed,
        0x39, 0xe7, 0x50, 0xba, 0xbf, 0x1d, 0xc4, 0xca, 0xa3, 0x60, 0x30, 0x5e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01,
        0x01, 0xff, 0x04, 0x02, 0x30, 0x00, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02,
        0x07, 0x80, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0xef, 0x06, 0x56, 0x11, 0x9c, 0x1c, 0x91,
        0xa7, 0x9a, 0x94, 0xe6, 0xdc, 0xf3, 0x79, 0x79, 0xdb, 0xd0, 0x7f, 0xf8, 0xa3, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23,
        0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x63, 0x54, 0x0e, 0x47, 0xf6, 0x4b, 0x1c, 0x38, 0xd1, 0x38, 0x84, 0xa4, 0x62, 0xd1,
        0x6c, 0x19, 0x5d, 0x8f, 0xfb, 0x3c, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48,
        0x00, 0x30, 0x45, 0x02, 0x20, 0x46, 0x86, 0x81, 0x07, 0x33, 0xbf, 0x0d, 0xc8, 0xff, 0x4c, 0xb5, 0x14, 0x5a, 0x6b, 0xfa,
        0x1a, 0xec, 0xff, 0xa8, 0xb6, 0xda, 0xb6, 0xc3, 0x51, 0xaa, 0xee, 0xcd, 0xaf, 0xb8, 0xbe, 0x95, 0x7d, 0x02, 0x21, 0x00,
        0xe8, 0xc2, 0x8d, 0x6b, 0xfc, 0xc8, 0x7a, 0x7d, 0x54, 0x2e, 0xad, 0x6e, 0xda, 0xca, 0x14, 0x8d, 0x5f, 0xa5, 0x06, 0x1e,
        0x51, 0x7c, 0xbe, 0x4f, 0x24, 0xa7, 0x20, 0xe1, 0xc0, 0x59, 0xde, 0x1a,
    };
    const uint8_t kDevelopmentDAC_PublicKey_FFF1_8002[65] = {
        0x04, 0xda, 0x93, 0xf1, 0x67, 0x36, 0x25, 0x67, 0x50, 0xd9, 0x03, 0xb0, 0x34, 0xba, 0x45, 0x88, 0xab,
        0xaf, 0x58, 0x95, 0x4f, 0x77, 0xaa, 0x9f, 0xd9, 0x98, 0x9d, 0xfd, 0x40, 0x0d, 0x7a, 0xb3, 0xfd, 0xc9,
        0x75, 0x3b, 0x3b, 0x92, 0x1b, 0x29, 0x4c, 0x95, 0x0f, 0xd9, 0xd2, 0x80, 0xd1, 0x4c, 0x43, 0x86, 0x2f,
        0x16, 0xdc, 0x85, 0x4b, 0x00, 0xed, 0x39, 0xe7, 0x50, 0xba, 0xbf, 0x1d, 0xc4, 0xca,
    };

    /* Sign using the example attestation private key */
    P256ECDSASignature da_signature;
    MutableByteSpan out_sig_span(da_signature.Bytes(), da_signature.Capacity());
    CHIP_ERROR err = SignWithDacKey(ByteSpan{ ecc_message, sizeof(ecc_message) }, out_sig_span);
    assert(err == CHIP_NO_ERROR);

    assert(out_sig_span.size() == kP256_ECDSA_Signature_Length_Raw);
    da_signature.SetLength(out_sig_span.size());

    /* Get DAC from the provider */
    uint8_t dac_cert_buf[kMaxDERCertLength];
    MutableByteSpan dac_cert_span(dac_cert_buf);

    memcpy(dac_cert_span.data(), kDevelopmentDAC_Cert_FFF1_8002, 492);

    /* Extract public key from DAC, prior to signature verification */
    P256PublicKey dac_public_key;
    err = ExtractPubkeyFromX509Cert(dac_cert_span, dac_public_key);
    assert(err == CHIP_NO_ERROR);
    assert(dac_public_key.Length() == 65);
    assert(0 == memcmp(dac_public_key.ConstBytes(), kDevelopmentDAC_PublicKey_FFF1_8002, 65));

    /* Verify round trip signature */
    err = dac_public_key.ECDSA_validate_msg_signature(&ecc_message[0], sizeof(ecc_message), da_signature);
    assert(err == CHIP_NO_ERROR);
    PRINTF("ECDSA signature validated with SUCCESS \n");
#endif
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
