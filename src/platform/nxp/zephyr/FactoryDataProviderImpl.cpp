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

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include <platform/nxp/zephyr/FactoryDataProviderImpl.h>

/* mbedtls */
#include <mbedtls/version.h>
#if (MBEDTLS_VERSION_NUMBER >= 0x04000000)
#include "mbedtls/private/aes.h"
#else
#include "mbedtls/aes.h"
#endif // MBEDTLS_VERSION_NUMBER >= 0x04000000
#include "mbedtls/sha256.h"

#if defined(CONFIG_SOC_SERIES_RW6XX)
#include "els_pkc_driver.h"
#if defined(CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION)
#include "ELSFactoryData.h"
#endif /* defined(CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION) */
#endif /* defined(CONFIG_SOC_SERIES_RW6XX) */

/* -------------------------------------------------------------------------- */
/*                               Private macros                               */
/* -------------------------------------------------------------------------- */

#define HASH_ID 0xCE47BA5E

#if defined(CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION) && defined(CONFIG_SOC_SERIES_RW6XX)
#define DAC_KEY_BLOB_SIZE 48
#endif /* defined(CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION) && defined(CONFIG_SOC_SERIES_RW6XX) */

/* -------------------------------------------------------------------------- */
/*                            Class implementation                            */
/* -------------------------------------------------------------------------- */

namespace chip {
namespace DeviceLayer {

CHIP_ERROR FactoryDataProviderImpl::SearchForId(uint8_t searchedType, uint8_t * pBuf, size_t bufLength, uint16_t & length,
                                                uint32_t * contentAddr)
{
    CHIP_ERROR err               = CHIP_ERROR_NOT_FOUND;
    uint8_t type                 = 0;
    uint32_t index               = 0;
    uint8_t * factoryDataAddress = &mFactoryData.factoryDataBuffer[0];
    uint32_t factoryDataSize     = sizeof(mFactoryData.factoryDataBuffer);
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
            /* If pBuf is null it means that we only want to know if the Type has been found */
            if (pBuf != NULL)
            {
                /* If the buffer given is too small, fill only the available space */
                if (bufLength < currentLen)
                {
                    currentLen = (uint16_t) bufLength;
                }
                memcpy((uint8_t *) pBuf, factoryDataAddress + index, currentLen);
            }
            length = (uint16_t) currentLen;
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
#ifdef CONFIG_SOC_SERIES_RW6XX
    if (psa_get_key_lifetime(&attrs) == PSA_KEY_LIFETIME_VOLATILE)
    {
        psa_set_key_lifetime(&attrs,
                             PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_VOLATILE,
                                                                            PSA_CRYPTO_ELS_PKC_LOCATION_S50_RFC3394_STORAGE));
    }
#endif /* CONFIG_SOC_SERIES_RW6XX */
}

CHIP_ERROR FactoryDataProviderImpl::Init(void)
{
    int ret;
    CHIP_ERROR res;
    uint8_t currentBlock[16];
    uint8_t calculatedHash[Crypto::kSHA256_Hash_Length];
    off_t factoryDataOffset = PARTITION_OFFSET(factory_partition);

    mFlashDevice = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));

    /* Read the factory data header from flash */
    ret = flash_read(mFlashDevice, factoryDataOffset, (void *) &mFactoryData.header, sizeof(Header));
    if (ret != 0)
    {
        return CHIP_ERROR_READ_FAILED;
    }

    /* Check ID is valid */
    if (mFactoryData.header.hashId != HASH_ID)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    factoryDataOffset += sizeof(Header);

    /* First, read data as plaintext */
    for (int i = 0; (uint32_t) i < (mFactoryData.header.size / 16U); i++)
    {
        ret = flash_read(mFlashDevice, factoryDataOffset + i * 16, (void *) &mFactoryData.factoryDataBuffer[i * 16], 16);
        if (ret != 0)
        {
            return CHIP_ERROR_READ_FAILED;
        }
    }

    /* Calculate hash and verify */
    ReturnErrorOnFailure(Crypto::Hash_SHA256(mFactoryData.factoryDataBuffer, mFactoryData.header.size, calculatedHash));

    if (memcmp(calculatedHash, mFactoryData.header.hash, kHashLen) != 0)
    {
        /* Hash didn't match - data might be encrypted, try to decrypt */
        if (pAesKey != NULL)
        {
            ChipLogProgress(DeviceLayer, "Hash mismatch, attempting decryption...");

            /* Re-read and decrypt */
            for (int i = 0; (uint32_t) i < (mFactoryData.header.size / 16U); i++)
            {
                ret = flash_read(mFlashDevice, factoryDataOffset + i * 16, (void *) &currentBlock[0], 16);
                if (ret != 0)
                {
                    return CHIP_ERROR_READ_FAILED;
                }

                res = ReadEncryptedData(&mFactoryData.factoryDataBuffer[i * 16], &currentBlock[0]);
                if (res != CHIP_NO_ERROR)
                {
                    return res;
                }
            }

            /* Verify hash after decryption */
            ReturnErrorOnFailure(Crypto::Hash_SHA256(mFactoryData.factoryDataBuffer, mFactoryData.header.size, calculatedHash));

            if (memcmp(calculatedHash, mFactoryData.header.hash, kHashLen) != 0)
            {
                ChipLogError(DeviceLayer, "Hash verification failed after decryption");
                return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
            }

            ChipLogProgress(DeviceLayer, "Decryption successful, hash verified");
        }
        else
        {
            ChipLogError(DeviceLayer, "Hash verification failed, no encryption key set");
            return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Factory data hash verified successfully");
    }

#if defined(CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION) && defined(CONFIG_SOC_SERIES_RW6XX)
    uint16_t keySize = 0;

    ChipLogProgress(DeviceLayer, "Init: only protect DAC private key");

    /* Check whether the kDacPrivateKeyId data is converted or not */
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, NULL, 0, keySize));
    if (keySize == DAC_KEY_BLOB_SIZE)
    {
        /* The kDacPrivateKeyId data is converted already, do nothing */
        ChipLogProgress(DeviceLayer, "SSS: DAC private key already converted to blob");
    }
    else
    {
        /* Provision the DAC private key into S50 and the returned wrapped key is stored
         * in the previous area of factory data, update the hash and re-write the factory data in Flash */
        ChipLogProgress(DeviceLayer, "SSS: convert DAC private key to blob");
        els_enable();
        ReturnLogErrorOnFailure(ELS_ConvertDacKey());
    }
#endif /* defined(CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION) && defined(CONFIG_SOC_SERIES_RW6XX) */

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

CHIP_ERROR FactoryDataProviderImpl::ReadEncryptedData(uint8_t * dest, uint8_t * source)
{
    mbedtls_aes_context aesCtx;

    mbedtls_aes_init(&aesCtx);

    if (mbedtls_aes_setkey_dec(&aesCtx, pAesKey, pAESKeySize) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (mbedtls_aes_crypt_ecb(&aesCtx, MBEDTLS_AES_DECRYPT, source, dest) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    mbedtls_aes_free(&aesCtx);

    return CHIP_NO_ERROR;
}

#if defined(CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION) && defined(CONFIG_SOC_SERIES_RW6XX)

CHIP_ERROR FactoryDataProviderImpl::ELS_ConvertDacKey()
{
    CHIP_ERROR error                = CHIP_NO_ERROR;
    size_t blobSize                 = DAC_KEY_BLOB_SIZE;
    size_t newSize                  = sizeof(Header) + mFactoryData.header.size + (ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD);
    uint8_t blob[DAC_KEY_BLOB_SIZE] = { 0 };
    uint32_t keyAddr;
    off_t factoryDataOffset = PARTITION_OFFSET(factory_partition);
    size_t factoryDataSize  = PARTITION_SIZE(factory_partition);
    int ret;

    uint8_t * data = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(newSize));
    VerifyOrExit(data != nullptr, error = CHIP_ERROR_NO_MEMORY);

    /* Import plain DAC key and generate the blob */
    SuccessOrExit(error = ELS_ExportBlob(blob, &blobSize, keyAddr));

    ChipLogProgress(DeviceLayer, "SSS: extracted blob from DAC private key");
    VerifyOrExit(blobSize == DAC_KEY_BLOB_SIZE, error = CHIP_ERROR_INTERNAL);

    /* Read header from flash */
    ret = flash_read(mFlashDevice, factoryDataOffset, data, sizeof(Header));
    VerifyOrExit(ret == 0, error = CHIP_ERROR_READ_FAILED);

    /* Copy factory data buffer after header */
    memcpy(data + sizeof(Header), mFactoryData.factoryDataBuffer, mFactoryData.header.size);
    ChipLogProgress(DeviceLayer, "SSS: cached factory data in RAM");

    /* Replace private plain DAC key by the blob into factory data RAM buffer */
    SuccessOrExit(error = ReplaceWithBlob(data, blob, blobSize, keyAddr));
    ChipLogProgress(DeviceLayer, "SSS: replaced DAC private key with secured blob");

    /* Erase flash factory data partition */
    ret = flash_erase(mFlashDevice, factoryDataOffset, factoryDataSize);
    VerifyOrExit(ret == 0, error = CHIP_ERROR_WRITE_FAILED);

    /* Write new factory data into flash */
    ret = flash_write(mFlashDevice, factoryDataOffset, data, newSize);
    VerifyOrExit(ret == 0, error = CHIP_ERROR_WRITE_FAILED);
    ChipLogProgress(DeviceLayer, "SSS: updated factory data");

    /* Update RAM buffer with new data (remove header section) */
    memmove(&data[0], &data[sizeof(Header)], newSize - sizeof(Header));
    memset(mFactoryData.factoryDataBuffer, 0, sizeof(mFactoryData.factoryDataBuffer));
    memcpy(mFactoryData.factoryDataBuffer, data, newSize - sizeof(Header));

    /* Update factory data payload size */
    mFactoryData.header.size = newSize - sizeof(Header);

exit:
    if (data != nullptr)
    {
        chip::Platform::MemoryFree(data);
    }
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

    mcuxClEls_KeyProp_t plain_key_properties = {
        .word = { .value = MCUXCLELS_KEYPROPERTY_VALUE_SECURE | MCUXCLELS_KEYPROPERTY_VALUE_PRIVILEGED |
                      MCUXCLELS_KEYPROPERTY_VALUE_KEY_SIZE_256 | MCUXCLELS_KEYPROPERTY_VALUE_KGSRC }
    };

    mcuxClEls_KeyIndex_t key_index = MCUXCLELS_KEY_SLOTS;

    /* Import plain DAC key into S50 */
    status = import_plain_key_into_els(keySpan.data(), keySpan.size(), plain_key_properties, &key_index);
    VerifyOrReturnError(status == STATUS_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "import_plain_key_into_els failed: 0x%08x", status));

    /* ELS generate key blob. The blob created here is one that can be directly imported into ELS again. */
    status = export_key_from_els(key_index, data, dataLen);
    VerifyOrReturnError(status == STATUS_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "export_key_from_els failed: 0x%08x", status));

    status = els_delete_key(key_index);
    VerifyOrReturnError(status == STATUS_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "els_delete_key failed: 0x%08x", status));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::ReplaceWithBlob(uint8_t * data, uint8_t * blob, size_t blobLen, uint32_t KeyAddr)
{
    size_t newSize                           = mFactoryData.header.size + ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD;
    FactoryDataProviderImpl::Header * header = reinterpret_cast<FactoryDataProviderImpl::Header *>(data);
    uint8_t * payload                        = data + sizeof(FactoryDataProviderImpl::Header);
    uint8_t offset              = (uint8_t *) (KeyAddr - kValueOffset) - (uint8_t *) &mFactoryData.factoryDataBuffer[0];
    size_t subsequentDataOffset = offset + kValueOffset + Crypto::kP256_PrivateKey_Length;

    /* Move subsequent data to make room for the larger blob */
    memmove(payload + subsequentDataOffset + ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD, payload + subsequentDataOffset,
            mFactoryData.header.size - subsequentDataOffset);

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

#endif /* defined(CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION) && defined(CONFIG_SOC_SERIES_RW6XX) */

#ifndef CONFIG_CHIP_FACTORY_DATA_PROVIDER_CUSTOM_SINGLETON_IMPL
FactoryDataProvider & FactoryDataPrvdImpl()
{
    static FactoryDataProviderImpl sInstance;
    return sInstance;
}
#endif

} // namespace DeviceLayer
} // namespace chip
