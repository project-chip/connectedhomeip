/*
 *
 *    Copyright (c) 2023, 2026 Project CHIP Authors
 *    Copyright 2023, 2026 NXP
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

#include "FactoryDataProviderDefaultImpl.h"
#include <psa/crypto.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#include "mflash_drv.h"

#include "fsl_adapter_flash.h"

#if defined(__cplusplus)
}
#endif /* __cplusplus */


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

CHIP_ERROR FactoryDataProviderImpl::Init(void)
{
    ReturnLogErrorOnFailure(ReadAndCheckFactoryDataInFlash());
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

#ifndef CONFIG_CHIP_FACTORY_DATA_PROVIDER_CUSTOM_SINGLETON_IMPL
FactoryDataProvider & FactoryDataPrvdImpl()
{
    static FactoryDataProviderImpl sInstance;
    return sInstance;
}
#endif

} // namespace DeviceLayer
} // namespace chip