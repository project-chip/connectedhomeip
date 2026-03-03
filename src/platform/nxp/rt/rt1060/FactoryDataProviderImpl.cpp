/*
 *
 *    Copyright (c) 2022-2024, 2026 Project CHIP Authors
 *    Copyright 2023, 2024, 2026 NXP
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

#include "fsl_dcp.h"
#include "mflash_drv.h"

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

#define BLOCK_SIZE_16_BYTES 16
#define SHA256_OUTPUT_SIZE 32
#define HASH_ID 0xCE47BA5E
#define HASH_LEN 4

/* Grab symbol for the base address from the linker file. */
extern uint32_t __FACTORY_DATA_START_OFFSET[];
extern uint32_t __FACTORY_DATA_SIZE[];

namespace chip {
namespace DeviceLayer {

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

CHIP_ERROR FactoryDataProviderImpl::Init(void)
{
    uint16_t len;
    status_t status;
    uint32_t factoryDataAddress = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize    = (uint32_t) __FACTORY_DATA_SIZE;
    uint8_t currentBlock[BLOCK_SIZE_16_BYTES];
    uint8_t calculatedHash[SHA256_OUTPUT_SIZE];
    size_t outputHashSize = sizeof(calculatedHash);
    uint16_t i;
    CHIP_ERROR res;

    VerifyOrReturnError(pAESKeySize == aes_128, CHIP_ERROR_INVALID_ARGUMENT);

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

        /* Decrypt data if an encryptMode is set */
        if (encryptMode != encrypt_none)
        {
            res = ReadEncryptedData(&factoryDataRamBuffer[i * BLOCK_SIZE_16_BYTES], &currentBlock[0], sizeof(currentBlock));
            if (res != CHIP_NO_ERROR)
                return res;
        }
        else
        {
            /* Store the block unencrypted */
            memcpy(&factoryDataRamBuffer[i * BLOCK_SIZE_16_BYTES], &currentBlock[0], sizeof(currentBlock));
        }
    }

    /* Calculate SHA256 value over the factory data and compare with stored value */
    res = Hash256(&factoryDataRamBuffer[0], mHeader.size, &calculatedHash[0], &outputHashSize);

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

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::SetKeyType(KeyType type)
{
    if (type == kHwKey)
    {
        // by default if hw key is selected, kDCP_OTPMKKeyLow is choosen
        selectedKey = kDCP_OTPMKKeyLow;
    }
    else
    {
        selectedKey = kDCP_UseSoftKey;
    }
    return CHIP_NO_ERROR;
}

void FactoryDataProviderImpl::SetDCP_OTPKeySelect(void)
{
    switch (selectedKey)
    {
    case kDCP_OTPMKKeyLow:
        IOMUXC_GPR->GPR3 &= ~(1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 &= ~(1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
        break;
    case kDCP_OTPMKKeyHigh:
        IOMUXC_GPR->GPR3 |= (1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 &= ~(1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
        break;
    case kDCP_OCOTPKeyLow:
        IOMUXC_GPR->GPR3 &= ~(1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 |= (1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
        break;
    case kDCP_OCOTPKeyHigh:
        IOMUXC_GPR->GPR3 |= (1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 |= (1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
        break;
    default:
        break;
    }
}

CHIP_ERROR FactoryDataProviderImpl::ReadEncryptedData(uint8_t * desBuff, uint8_t * sourceAddr, uint16_t sizeToRead)
{
    status_t status;
    dcp_handle_t m_handle;
    dcp_config_t dcpConfig;

    /* Check that the length is aligned on 16 bytes */
    if ((sizeToRead % 16) != 0)
        return CHIP_ERROR_INVALID_ARGUMENT;

    /* Check that the soft key has been correclty provisioned */
    if (selectedKey == kDCP_UseSoftKey && pAesKey == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    /* Check if the initial vector has been provisioned if CBC mode is chosen */
    if (encryptMode == encrypt_cbc && cbcInitialVector == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    if (!dcpDriverIsInitialized)
    {
        /* Initialize DCP */
        DCP_GetDefaultConfig(&dcpConfig);
        SetDCP_OTPKeySelect();
        /* Reset and initialize DCP */
        DCP_Init(DCP, &dcpConfig);
        dcpDriverIsInitialized = true;
    }

    m_handle.channel    = kDCP_Channel0;
    m_handle.swapConfig = kDCP_NoSwap;

    if (selectedKey == kDCP_UseSoftKey)
        m_handle.keySlot = kDCP_KeySlot0;
    else
        m_handle.keySlot = kDCP_OtpKey;

    status = DCP_AES_SetKey(DCP, &m_handle, pAesKey, 16);
    if (status != kStatus_Success)
        return CHIP_ERROR_INTERNAL;

    if (encryptMode == encrypt_ecb)
        DCP_AES_DecryptEcb(DCP, &m_handle, sourceAddr, desBuff, sizeToRead);
    else
        DCP_AES_DecryptCbc(DCP, &m_handle, sourceAddr, desBuff, sizeToRead, cbcInitialVector);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::Hash256(const uint8_t * input, size_t inputSize, uint8_t * output, size_t * outputSize)
{
    status_t status;
    dcp_handle_t m_handle;

    m_handle.channel    = kDCP_Channel0;
    m_handle.swapConfig = kDCP_NoSwap;
    m_handle.keySlot    = kDCP_KeySlot0;

    status = DCP_HASH(DCP, &m_handle, kDCP_Sha256, input, inputSize, output, outputSize);

    if (status != kStatus_Success)
        return CHIP_ERROR_INTERNAL;

    return CHIP_NO_ERROR;
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
