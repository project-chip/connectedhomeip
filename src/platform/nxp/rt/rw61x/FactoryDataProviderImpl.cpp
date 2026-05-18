/*
 *
 *    Copyright (c) 2020-2022, 2026 Project CHIP Authors
 *    Copyright 2023-2026 NXP
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

#include "mflash_drv.h"

#include "fsl_adapter_flash.h"

#include "els_pkc_driver.h"
#include "psa/crypto.h"

#ifdef CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION
#include "ELSFactoryData.h"

#define DAC_KEY_BLOB_SIZE 48
#endif /* CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION */

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

#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
#error("OTA FACTORY DATA PROCESSOR NOT SUPPORTED WITH THIS FACTORY DATA PRVD IMPL")
#endif

#define HASH_ID 0xCE47BA5E
#define HASH_LEN 4

/* Grab symbol for the base address from the linker file. */
extern uint32_t __FACTORY_DATA_START_OFFSET[];
extern uint32_t __FACTORY_DATA_SIZE[];

using namespace ::chip::Credentials;
using namespace ::chip::Crypto;

namespace chip {
namespace DeviceLayer {


CHIP_ERROR FactoryDataProviderImpl::DecryptAesEcb(uint8_t * dest, uint8_t * source)
{
    psa_status_t status;
    psa_key_attributes_t key_attr = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t key_id = 0;

    // Configure key attributes
    psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&key_attr, PSA_ALG_ECB_NO_PADDING);
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);
    psa_set_key_bits(&key_attr, pAESKeySize);

    // Import AES key into PSA
    status = psa_import_key(&key_attr, pAesKey, PSA_BITS_TO_BYTES(pAESKeySize), &key_id);
    if (status != PSA_SUCCESS)
    {
        return CHIP_ERROR_INTERNAL;
    }

    // Perform AES-ECB decrypt
    size_t out_len = 0;
    status = psa_cipher_decrypt(
        key_id,
        PSA_ALG_ECB_NO_PADDING,
        source,
        16,        // ECB always processes 16‑byte blocks
        dest,
        16,
        &out_len);

    // Clean up key regardless of outcome
    psa_destroy_key(key_id);

    if (status != PSA_SUCCESS || out_len != 16)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}


CHIP_ERROR FactoryDataProviderImpl::SearchForId(uint8_t searchedType, uint8_t * pBuf, size_t bufLength, uint16_t & length,
                                                uint32_t * contentAddr)
{
    CHIP_ERROR err               = CHIP_ERROR_NOT_FOUND;
    uint8_t type                 = 0;
    uint32_t index               = 0;
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

void FactoryDataProviderImpl::UpdateKeyAttributes(psa_key_attributes_t & attrs)
{
    if (psa_get_key_lifetime(&attrs) == PSA_KEY_LIFETIME_VOLATILE)
    {
        psa_set_key_lifetime(&attrs,
                             PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_VOLATILE,
                                                                            PSA_CRYPTO_ELS_PKC_LOCATION_S50_RFC3394_STORAGE));
    }
}

CHIP_ERROR FactoryDataProviderImpl::ReadAndCheckFactoryDataInFlash(void)
{
    status_t status;
    uint32_t factoryDataAddress = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize    = (uint32_t) __FACTORY_DATA_SIZE;
    uint8_t calculatedHash[kSHA256_Hash_Length];
    CHIP_ERROR res;
    uint8_t currentBlock[16];

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
        return CHIP_ERROR_NOT_FOUND;
    }
    /* remove the header section */
    memmove(&factoryDataRamBuffer[0], &factoryDataRamBuffer[sizeof(mHeader)], mHeader.size);

    /* Calculate SHA256 value over the factory data and compare with stored value */
    res = Hash_SHA256(&factoryDataRamBuffer[0], mHeader.size, &calculatedHash[0]);

    if (res != CHIP_NO_ERROR)
        return res;

    if (memcmp(&calculatedHash[0], &mHeader.hash[0], HASH_LEN) != 0)
    {
        /* HASH value didn't match, test if factory data are encrypted */

        /* try to decrypt factory data, reset factory data buffer content*/
        memset(factoryDataRamBuffer, 0, sizeof(factoryDataRamBuffer));
        memset(calculatedHash, 0, sizeof(calculatedHash));

        factoryDataAddress += sizeof(Header);

        /* Load the buffer into RAM by reading each 16 bytes blocks */
        for (int i = 0; i < (mHeader.size / 16); i++)
        {
            if (mflash_drv_read(factoryDataAddress + i * 16, (uint32_t *) &currentBlock[0], sizeof(currentBlock)) !=
                kStatus_Success)
            {
                return CHIP_ERROR_INTERNAL;
            }
            ReturnErrorOnFailure(DecryptAesEcb(&factoryDataRamBuffer[i * 16], &currentBlock[0]));
        }

        /* Calculate SHA256 value over the factory data and compare with stored value */
        res = Hash_SHA256(&factoryDataRamBuffer[0], mHeader.size, &calculatedHash[0]);
        if (memcmp(&calculatedHash[0], &mHeader.hash[0], HASH_LEN) != 0)
        {
            return CHIP_ERROR_NOT_FOUND;
        }
    }

    ChipLogProgress(DeviceLayer, "factory data hash check is successful!");
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::SetEncryptionMode(EncryptionMode mode)
{
    CHIP_ERROR error = CHIP_ERROR_INVALID_ARGUMENT;

    /*
     * Currently the fwk_factory_data_provider module supports only ecb mode.
     * Therefore return an error if encrypt mode is not ecb
     */
    if (mode == encrypt_ecb)
    {
        encryptMode = mode;
        error       = CHIP_NO_ERROR;
    }
    return error;
}

CHIP_ERROR FactoryDataProviderImpl::Init(void)
{
    ReturnLogErrorOnFailure(ReadAndCheckFactoryDataInFlash());

#ifdef CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION

    uint16_t keySize = 0;

    ChipLogProgress(DeviceLayer, "init: only protect DAC private key\n");

    /* check whether the kDacPrivateKeyId data is converted or not*/
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, NULL, 0, keySize));
    if (keySize == DAC_KEY_BLOB_SIZE)
    {
        /* the kDacPrivateKeyId data is converted already, do nothing */
        ChipLogProgress(DeviceLayer, "SSS: DAC private key already converted to blob");
        return CHIP_NO_ERROR;
    }
    else
    {
        /* provison the dac private key into Edge Lock and the returned wrapped key is stored the previous area of factory data,
         update the hash and re-write the factory data in Flash */
        ChipLogProgress(DeviceLayer, "SSS: convert DAC private key to blob");
        els_enable();
        ReturnLogErrorOnFailure(ELS_ConvertDacKey());
        ChipLogProgress(DeviceLayer, "System restarting");
    }

#endif

    return CHIP_NO_ERROR;
}

#ifdef CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION
CHIP_ERROR FactoryDataProviderImpl::ELS_ConvertDacKey()
{
    CHIP_ERROR error                = CHIP_NO_ERROR;
    size_t blobSize                 = DAC_KEY_BLOB_SIZE;
    size_t newSize                  = sizeof(Header) + mHeader.size + (ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD);
    uint8_t blob[DAC_KEY_BLOB_SIZE] = { 0 };
    uint32_t KeyAddr;
    uint32_t factoryDataAddress = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize    = (uint32_t) __FACTORY_DATA_SIZE;
    hal_flash_status_t status;

    uint8_t * data = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(newSize));
    /* Import pain DAC key and generate the blob */
    SuccessOrExit(ELS_ExportBlob(blob, &blobSize, KeyAddr));

    ChipLogProgress(DeviceLayer, "SSS: extracted blob from DAC private key");
    PLOG_DEBUG_BUFFER("blob", blob, blobSize);
    VerifyOrExit(blobSize == DAC_KEY_BLOB_SIZE, error = CHIP_ERROR_INTERNAL);

    /* Read all factory data */
    status = HAL_FlashRead(factoryDataAddress + MFLASH_BASE_ADDRESS, sizeof(Header), data);
    memcpy(data + sizeof(Header), factoryDataRamBuffer, mHeader.size);
    VerifyOrExit(status == kStatus_HAL_Flash_Success, error = CHIP_ERROR_INTERNAL);
    ChipLogError(DeviceLayer, "SSS: cached factory data in RAM");

    /* Replace private plain DAC key by the blob into factory data RAM buffer (the blob length is higher then the plain key length)
     */
    SuccessOrExit(ReplaceWithBlob(data, blob, blobSize, KeyAddr));
    ChipLogError(DeviceLayer, "SSS: replaced DAC private key with secured blob");

    /* Erase flash factory data sectors */
    status = HAL_FlashEraseSector(factoryDataAddress + MFLASH_BASE_ADDRESS, factoryDataSize);
    VerifyOrExit(status == kStatus_HAL_Flash_Success, error = CHIP_ERROR_INTERNAL);
    /* Write new factory data into flash */
    status = HAL_FlashProgramUnaligned(factoryDataAddress + MFLASH_BASE_ADDRESS, newSize, data);
    VerifyOrExit(status == kStatus_HAL_Flash_Success, error = CHIP_ERROR_INTERNAL);
    ChipLogError(DeviceLayer, "SSS: updated factory data");

    /* remove the header section as it will no longer be used */
    memmove(&data[0], &data[sizeof(mHeader)], newSize);
    memset(factoryDataRamBuffer, 0, sizeof(factoryDataRamBuffer));
    memcpy(factoryDataRamBuffer, data, newSize);
    /* Actualisation of the factory data payload size */
    mHeader.size = newSize;

exit:
    chip::Platform::MemoryFree(data);
    return error;
}

CHIP_ERROR FactoryDataProviderImpl::ELS_ExportBlob(uint8_t * data, size_t * dataLen, uint32_t & addr)
{
    status_t status = STATUS_SUCCESS;
    uint8_t keyBuf[Crypto::kP256_PrivateKey_Length];
    uint16_t keySize = 0;
    MutableByteSpan keySpan(keyBuf);

    /* Search key ID FactoryDataId::kDacPrivateKeyId */
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, keySpan.data(), keySpan.size(), keySize, &addr));
    keySpan.reduce_size(keySize);
    PLOG_DEBUG_BUFFER("Private DAC key plain", keySpan.data(), keySpan.size());
    mcuxClEls_KeyProp_t plain_key_properties = {
        .word = { .value = MCUXCLELS_KEYPROPERTY_VALUE_SECURE | MCUXCLELS_KEYPROPERTY_VALUE_PRIVILEGED |
                      MCUXCLELS_KEYPROPERTY_VALUE_KEY_SIZE_256 | MCUXCLELS_KEYPROPERTY_VALUE_KGSRC }
    };

    mcuxClEls_KeyIndex_t key_index = MCUXCLELS_KEY_SLOTS;
    /* Import plain DAC key into S50 */
    status = import_plain_key_into_els(keySpan.data(), keySpan.size(), plain_key_properties, &key_index);
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

#endif /* CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION */

#ifndef CONFIG_CHIP_FACTORY_DATA_PROVIDER_CUSTOM_SINGLETON_IMPL
FactoryDataProvider & FactoryDataPrvdImpl()
{
    static FactoryDataProviderImpl sInstance;
    return sInstance;
}
#endif

} // namespace DeviceLayer
} // namespace chip
