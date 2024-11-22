/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright 2024 NXP
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

#include "FactoryDataProviderEl2GoImpl.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#include "ELSFactoryData.h"
#include "mflash_drv.h"

#if defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT)
#include "els_pkc_mbedtls.h"
#endif /* defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT) */

#include "fsl_adapter_flash.h"

/* mbedtls */
#include "mbedtls/aes.h"
#include "mbedtls/sha256.h"

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

CHIP_ERROR FactoryDataProviderImpl::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    status_t status                         = STATUS_SUCCESS;
    uint8_t el2go_blob[EL2GO_MAX_BLOB_SIZE] = { 0U };
    size_t el2go_blob_size                  = 0U;
    size_t outBufferSize                    = 0U;
    uint16_t CertificateIdSize              = 0;
    uint16_t BlobSize                       = 0;
    uint32_t Addr;
    uint32_t el2go_dac_cert_id = 0;

    /* Search key ID FactoryDataId::kEl2GoBlob address */
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kEl2GoBlob, NULL, 0, BlobSize, &Addr));
    ReturnErrorOnFailure(
        SearchForId(FactoryDataId::kEl2GoDacCertId, (uint8_t *) &el2go_dac_cert_id, sizeof(el2go_dac_cert_id), CertificateIdSize));

    /* Read DAC certificate from EL2GO data */
    status =
        read_el2go_blob((uint8_t *) Addr, (size_t) BlobSize, el2go_dac_cert_id, el2go_blob, EL2GO_MAX_BLOB_SIZE, &el2go_blob_size);

    STATUS_SUCCESS_OR_EXIT_MSG("DAC Private key not found: 0x%08x", status);

    // Import EL2GO blobs in ELS
    status = decrypt_el2go_cert_blob(el2go_blob, el2go_blob_size, outBuffer.data(), outBuffer.size(), &outBufferSize);
    outBuffer.reduce_size(outBufferSize);
    STATUS_SUCCESS_OR_EXIT_MSG("decrypt_el2go_cert_blob failed: 0x%08x", status);

    return CHIP_NO_ERROR;

exit:
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR FactoryDataProviderImpl::ReadAndCheckFactoryDataInFlash(void)
{
    status_t status;
    uint32_t factoryDataAddress = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize    = (uint32_t) __FACTORY_DATA_SIZE;
    uint32_t hashId;
    uint8_t calculatedHash[SHA256_OUTPUT_SIZE];
    CHIP_ERROR res;

    /* Init mflash */
    status = mflash_drv_init();

    if (status != kStatus_Success || factoryDataSize > sizeof(factoryDataRamBuffer))
        return CHIP_ERROR_INTERNAL;

    /* Load the factory data into RAM buffer */
    if (mflash_drv_read(factoryDataAddress, (uint32_t *) &factoryDataRamBuffer[0], factoryDataSize) != kStatus_Success)
    {
        return CHIP_ERROR_INTERNAL;
    }
    memcpy(&mHeader, factoryDataRamBuffer, sizeof(mHeader));
    if (mHeader.hashId != HASH_ID)
    {
        return CHIP_FACTORY_DATA_HASH_ID;
    }
    /* remove the header section */
    memmove(&factoryDataRamBuffer[0], &factoryDataRamBuffer[sizeof(mHeader)], mHeader.size);

    /* Calculate SHA256 value over the factory data and compare with stored value */
    res = Hash_SHA256(&factoryDataRamBuffer[0], mHeader.size, &calculatedHash[0]);

    if (res != CHIP_NO_ERROR)
        return res;

    if (memcmp(&calculatedHash[0], &mHeader.hash[0], HASH_LEN) != 0)
    {
        return CHIP_FACTORY_DATA_HASH_ID;
    }

    ChipLogProgress(DeviceLayer, "factory data hash check is successful!");
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::SignWithDacKey(const ByteSpan & digestToSign, MutableByteSpan & outSignBuffer)
{
    CHIP_ERROR res                                   = CHIP_NO_ERROR;
    status_t status                                  = STATUS_SUCCESS;
    uint8_t el2go_blob[EL2GO_MAX_BLOB_SIZE]          = { 0U };
    size_t el2go_blob_size                           = 0U;
    mcuxClEls_EccSignOption_t sign_options           = { 0 };
    uint8_t public_key[MCUXCLELS_ECC_PUBLICKEY_SIZE] = { 0 };
    size_t public_key_size                           = sizeof(public_key);
    uint8_t hash[MCUXCLHASH_OUTPUT_SIZE_SHA_256]     = { 0 };
    mcuxClEls_KeyIndex_t key_index                   = MCUXCLELS_KEY_SLOTS;
    mcuxClEls_EccByte_t ecc_signature[MCUXCLELS_ECC_SIGNATURE_SIZE];
    uint8_t digest[kSHA256_Hash_Length];
    uint16_t BlobSize  = 0;
    uint16_t KeyIdSize = 0;
    uint32_t Addr;
    uint32_t el2go_dac_key_id = 0;

    /* Search key ID FactoryDataId::kEl2GoBlob */
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kEl2GoBlob, NULL, 0, BlobSize, &Addr));
    ReturnErrorOnFailure(
        SearchForId(FactoryDataId::kEl2GoDacKeyId, (uint8_t *) &el2go_dac_key_id, sizeof(el2go_dac_key_id), KeyIdSize));

    /* Calculate message HASH to sign */
    memset(&digest[0], 0, sizeof(digest));
    res = Hash_SHA256(digestToSign.data(), digestToSign.size(), &digest[0]);
    if (res != CHIP_NO_ERROR)
    {
        return res;
    }

#if defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT)
    (void) mcux_els_mutex_lock();
#endif

    /* Read DAC key from EL2GO data*/
    status =
        read_el2go_blob((uint8_t *) Addr, (size_t) BlobSize, el2go_dac_key_id, el2go_blob, EL2GO_MAX_BLOB_SIZE, &el2go_blob_size);
    STATUS_SUCCESS_OR_EXIT_MSG("DAC Provate key not found: 0x%08x", status);

    // Import EL2GO blobs in ELS
    status = import_el2go_key_in_els(el2go_blob, el2go_blob_size, &key_index);

    // Generate key in ELS
    status = els_keygen(key_index, public_key, &public_key_size);
    STATUS_SUCCESS_OR_EXIT_MSG("els_keygen failed: 0x%08x", status);

    // Compute signature
    status = ELS_sign_hash(hash, ecc_signature, &sign_options, key_index);
    CopySpanToMutableSpan(ByteSpan{ ecc_signature, MCUXCLELS_ECC_SIGNATURE_SIZE }, outSignBuffer);
    STATUS_SUCCESS_OR_EXIT_MSG("ELS_sign_hash failed: 0x%08x", status);

    status = els_delete_key(key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("Deletion of el2goimport_auth failed", status);

#if defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT)
    (void) mcux_els_mutex_unlock();
#endif
    return CHIP_NO_ERROR;
exit:
#if defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT)
    (void) mcux_els_mutex_unlock();
#endif
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR FactoryDataProviderImpl::Init(void)
{
    uint16_t len;
    uint8_t type;
    uint16_t keySize = 0;
    status_t status  = STATUS_SUCCESS;

    ReturnLogErrorOnFailure(ReadAndCheckFactoryDataInFlash());

    els_enable();

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
