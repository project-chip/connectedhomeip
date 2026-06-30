/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright 2024, 2026 NXP
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
#include <inttypes.h>
#include <psa/crypto.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#include "fsl_adapter_flash.h"
#include "fusemap.h"
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

#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
#error("OTA FACTORY DATA PROCESSOR NOT SUPPORTED WITH THIS FACTORY DATA PRVD IMPL")
#endif

/* Grab symbol for the base address from the linker file. */
extern uint32_t __FACTORY_DATA_START_OFFSET[];
extern uint32_t __FACTORY_DATA_SIZE[];

#include <credentials/CHIPCert.h>

using namespace ::chip::Credentials;
using namespace ::chip::Crypto;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR FactoryDataProviderImpl::Init(void)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    uint16_t blobAreaSize     = 0;
    uint32_t blobAreaAddr     = 0;
    size_t blobsImported      = 0;
    uint16_t keyIdSize        = 0;
    uint16_t certIdSize       = 0;
    uint32_t factoryDacKeyId  = 0;
    uint32_t factoryDacCertId = 0;
    uint32_t blobDacKeyId     = 0;
    uint32_t blobDacCertId    = 0;

    // Step 0: Verify secure boot is enabled
    VerifyOrExit(OTP_SECURE_BOOT_EN_FUSE_VALUE() != 0, {
        ChipLogError(DeviceLayer, "Init: Secure boot disabled - EL2GO requires secure boot");
        err = CHIP_ERROR_INCORRECT_STATE;
    });

    // Step 1: Load and validate factory data from flash
    err = ReadAndCheckFactoryDataInFlash();
    VerifyOrExit(err == CHIP_NO_ERROR,
                 { ChipLogError(DeviceLayer, "Init: ReadAndCheckFactoryDataInFlash failed: %" CHIP_ERROR_FORMAT, err.Format()); });

    // Step 2: Get DAC Key ID and Certificate ID from factory data
    err = SearchForId(FactoryDataId::kEl2GoDacKeyId, (uint8_t *) &factoryDacKeyId, sizeof(factoryDacKeyId), keyIdSize);
    VerifyOrExit(err == CHIP_NO_ERROR, {
        ChipLogError(DeviceLayer, "Init: Failed to find DAC Key ID in factory data: %" CHIP_ERROR_FORMAT, err.Format());
    });

    err = SearchForId(FactoryDataId::kEl2GoDacCertId, (uint8_t *) &factoryDacCertId, sizeof(factoryDacCertId), certIdSize);
    VerifyOrExit(err == CHIP_NO_ERROR, {
        ChipLogError(DeviceLayer, "Init: Failed to find DAC Certificate ID in factory data: %" CHIP_ERROR_FORMAT, err.Format());
    });

    ChipLogProgress(DeviceLayer, "Init: Factory data specifies DAC Key ID 0x%08" PRIx32 " and Certificate ID 0x%08" PRIx32,
                    factoryDacKeyId, factoryDacCertId);

    // Step 3: Validate factory data IDs are in EL2GO range
    VerifyOrExit(
        factoryDacKeyId >= CHIP_CONFIG_CRYPTO_PSA_KEY_ID_EL2GO_BASE && factoryDacKeyId <= CHIP_CONFIG_CRYPTO_PSA_KEY_ID_EL2GO_END, {
            ChipLogError(DeviceLayer, "Init: Factory DAC Key ID 0x%08" PRIx32 " outside EL2GO range [0x%08X, 0x%08X]",
                         factoryDacKeyId, CHIP_CONFIG_CRYPTO_PSA_KEY_ID_EL2GO_BASE, CHIP_CONFIG_CRYPTO_PSA_KEY_ID_EL2GO_END);
            err = CHIP_ERROR_INVALID_ARGUMENT;
        });

    VerifyOrExit(
        factoryDacCertId >= CHIP_CONFIG_CRYPTO_PSA_KEY_ID_EL2GO_BASE && factoryDacCertId <= CHIP_CONFIG_CRYPTO_PSA_KEY_ID_EL2GO_END,
        {
            ChipLogError(DeviceLayer, "Init: Factory DAC Certificate ID 0x%08" PRIx32 " outside EL2GO range [0x%08X, 0x%08X]",
                         factoryDacCertId, CHIP_CONFIG_CRYPTO_PSA_KEY_ID_EL2GO_BASE, CHIP_CONFIG_CRYPTO_PSA_KEY_ID_EL2GO_END);
            err = CHIP_ERROR_INVALID_ARGUMENT;
        });

    // Step 4: Get EL2GO blob area address and size from factory data
    err = SearchForId(FactoryDataId::kEl2GoBlob, NULL, 0, blobAreaSize, &blobAreaAddr);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 { ChipLogError(DeviceLayer, "Init: SearchForId failed: %" CHIP_ERROR_FORMAT, err.Format()); });

    ChipLogProgress(DeviceLayer, "Init: Found EL2GO blob area at 0x%08" PRIx32 " (size: %u bytes)", blobAreaAddr, blobAreaSize);

    // Step 5: Import EL2GO blobs and get DAC IDs from blobs
    err = ParseEl2GoBlobs((const uint8_t *) blobAreaAddr, blobAreaSize, &blobsImported, &blobDacKeyId, &blobDacCertId);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 { ChipLogError(DeviceLayer, "Init: ParseEl2GoBlobs failed: %" CHIP_ERROR_FORMAT, err.Format()); });

    ChipLogProgress(DeviceLayer, "Init: Successfully imported %u EL2GO blobs (DAC Key: 0x%08" PRIx32 ", DAC Cert: 0x%08" PRIx32 ")",
                    blobsImported, blobDacKeyId, blobDacCertId);

    // Step 6: Verify factory data IDs match blob IDs
    VerifyOrExit(factoryDacKeyId == blobDacKeyId, {
        ChipLogError(DeviceLayer, "Init: DAC Key ID mismatch - Factory: 0x%08" PRIx32 ", Blob: 0x%08" PRIx32, factoryDacKeyId,
                     blobDacKeyId);
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    VerifyOrExit(factoryDacCertId == blobDacCertId, {
        ChipLogError(DeviceLayer, "Init: DAC Certificate ID mismatch - Factory: 0x%08" PRIx32 ", Blob: 0x%08" PRIx32,
                     factoryDacCertId, blobDacCertId);
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    // Step 7: Cache the validated IDs
    mEl2GoDacKeyId  = blobDacKeyId;
    mEl2GoDacCertId = blobDacCertId;
    mKeyIdsCached   = true;

    ChipLogProgress(DeviceLayer, "Init: DAC Key ID 0x%08" PRIx32 " and Certificate ID 0x%08" PRIx32 " validated and cached",
                    mEl2GoDacKeyId, mEl2GoDacCertId);

exit:
    if (err != CHIP_NO_ERROR)
    {
        mKeyIdsCached = false;
    }
    return err;
}

#ifndef CONFIG_CHIP_FACTORY_DATA_PROVIDER_CUSTOM_SINGLETON_IMPL
FactoryDataProvider & FactoryDataPrvdImpl()
{
    static FactoryDataProviderImpl sInstance;
    return sInstance;
}
#endif

CHIP_ERROR FactoryDataProviderImpl::ReadAndCheckFactoryDataInFlash(void)
{
    status_t status;
    uint32_t factoryDataAddress = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize    = (uint32_t) __FACTORY_DATA_SIZE;
    uint8_t calculatedHash[kSHA256_Hash_Length];
    CHIP_ERROR res;

    /* Init mflash */
    status = mflash_drv_init();

    if (status != kStatus_Success || factoryDataSize > sizeof(factoryDataRamBuffer) || factoryDataSize < sizeof(mHeader))
    {
        return CHIP_ERROR_INTERNAL;
    }

    /* Load the factory data into RAM buffer */
    if (mflash_drv_read(factoryDataAddress, (uint32_t *) &factoryDataRamBuffer[0], factoryDataSize) != kStatus_Success)
    {
        return CHIP_ERROR_INTERNAL;
    }

    memcpy(&mHeader, factoryDataRamBuffer, sizeof(mHeader));
    if (mHeader.hashId != kFactoryDataHashId)
    {
        return CHIP_FACTORY_DATA_HASH_ID;
    }
    /* remove the header section */
    memmove(&factoryDataRamBuffer[0], &factoryDataRamBuffer[sizeof(mHeader)], mHeader.size);

    /* Calculate SHA256 value over the factory data and compare with stored value */
    res = Hash_SHA256(&factoryDataRamBuffer[0], mHeader.size, &calculatedHash[0]);
    if (res != CHIP_NO_ERROR)
    {
        return res;
    }

    if (memcmp(&calculatedHash[0], &mHeader.hash[0], kHashLength) != 0)
    {
        return CHIP_FACTORY_DATA_HASH_ID;
    }

    ChipLogProgress(DeviceLayer, "factory data hash check is successful!");
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

CHIP_ERROR FactoryDataProviderImpl::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    psa_status_t psaStatus = PSA_SUCCESS;
    size_t certLen         = 0;

    // Use cached certificate ID
    VerifyOrExit(mKeyIdsCached, {
        ChipLogError(DeviceLayer, "GetDeviceAttestationCert: Key IDs not cached, Init() may have failed");
        err = CHIP_ERROR_INCORRECT_STATE;
    });

    // Export certificate directly to output buffer
    psaStatus = psa_export_key(mEl2GoDacCertId, outBuffer.data(), outBuffer.size(), &certLen);
    VerifyOrExit(psaStatus == PSA_SUCCESS, {
        ChipLogError(DeviceLayer, "GetDeviceAttestationCert: psa_export_key failed: %" PRId32, psaStatus);
        err = CHIP_ERROR_INTERNAL;
    });

    // Validate certificate size
    VerifyOrExit(certLen > 0 && certLen <= chip::Credentials::kMaxDERCertLength, {
        ChipLogError(DeviceLayer, "GetDeviceAttestationCert: Invalid certificate size: %u (max: %u)", certLen,
                     static_cast<size_t>(chip::Credentials::kMaxDERCertLength));
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    // Resize output buffer to actual certificate size
    outBuffer.reduce_size(certLen);

    ChipLogProgress(DeviceLayer, "GetDeviceAttestationCert: Successfully exported %u bytes", certLen);

exit:
    return err;
}

CHIP_ERROR FactoryDataProviderImpl::SignWithDacKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    CHIP_ERROR err                                     = CHIP_NO_ERROR;
    psa_status_t psa_status                            = PSA_SUCCESS;
    psa_algorithm_t alg                                = PSA_ALG_ECDSA(PSA_ALG_SHA_256);
    unsigned char eccSignature[PSA_SIGNATURE_MAX_SIZE] = { 0 };
    size_t eccSignatureLength                          = 0;

    // Step 1: Validate input parameters
    VerifyOrExit(!outSignBuffer.empty(), {
        ChipLogError(DeviceLayer, "SignWithDacKey: Output buffer is empty");
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    VerifyOrExit(!messageToSign.empty(), {
        ChipLogError(DeviceLayer, "SignWithDacKey: Message to sign is empty");
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    VerifyOrExit(outSignBuffer.size() >= Crypto::kP256_ECDSA_Signature_Length_Raw, {
        ChipLogError(DeviceLayer, "SignWithDacKey: Output buffer too small (%u < %u)", outSignBuffer.size(),
                     Crypto::kP256_ECDSA_Signature_Length_Raw);
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
    });

    // Step 2: Use cached DAC key ID
    VerifyOrExit(mKeyIdsCached, {
        ChipLogError(DeviceLayer, "SignWithDacKey: Key IDs not cached, Init() may have failed");
        err = CHIP_ERROR_INCORRECT_STATE;
    });

    ChipLogProgress(DeviceLayer, "SignWithDacKey: Using cached DAC key PSA ID = 0x%08" PRIx32, mEl2GoDacKeyId);

    // Step 3: Sign the message directly using PSA (secure element handles hashing internally)
    psa_status = psa_sign_message(mEl2GoDacKeyId, alg, messageToSign.data(), messageToSign.size(), eccSignature,
                                  sizeof(eccSignature), &eccSignatureLength);
    VerifyOrExit(psa_status == PSA_SUCCESS, {
        ChipLogError(DeviceLayer, "SignWithDacKey: psa_sign_message failed: %" PRId32, psa_status);
        err = CHIP_ERROR_INTERNAL;
    });

    // Step 4: Validate signature size
    VerifyOrExit(eccSignatureLength == Crypto::kP256_ECDSA_Signature_Length_Raw, {
        ChipLogError(DeviceLayer, "SignWithDacKey: Invalid signature size: %u (expected: %u)", eccSignatureLength,
                     Crypto::kP256_ECDSA_Signature_Length_Raw);
        err = CHIP_ERROR_INTERNAL;
    });

    // Step 5: Copy signature to output buffer
    err = CopySpanToMutableSpan(ByteSpan{ eccSignature, eccSignatureLength }, outSignBuffer);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 { ChipLogError(DeviceLayer, "SignWithDacKey: CopySpanToMutableSpan failed: %" CHIP_ERROR_FORMAT, err.Format()); });

    ChipLogProgress(DeviceLayer, "SignWithDacKey: Successfully signed message (%u bytes)", eccSignatureLength);

exit:
    // Clean up sensitive data
    memset(eccSignature, 0, sizeof(eccSignature));
    return err;
}

CHIP_ERROR FactoryDataProviderImpl::FactoryReset()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    psa_status_t status;

    if (mKeyIdsCached)
    {
        // Destroy DAC Private Key
        status = psa_destroy_key(mEl2GoDacKeyId);
        if (status != PSA_SUCCESS && status != PSA_ERROR_DOES_NOT_EXIST)
        {
            ChipLogError(DeviceLayer, "FactoryResetKeys: Failed to destroy DAC Private Key: %" PRId32, status);
        }

        // Destroy DAC Certificate Key
        status = psa_destroy_key(mEl2GoDacCertId);
        if (status != PSA_SUCCESS && status != PSA_ERROR_DOES_NOT_EXIST)
        {
            ChipLogError(DeviceLayer, "FactoryResetKeys: Failed to destroy DAC Certificate: %" PRId32, status);
            err = CHIP_ERROR_INTERNAL;
        }

        // Clear cached values
        mEl2GoDacKeyId  = 0;
        mEl2GoDacCertId = 0;
        mKeyIdsCached   = false;
    }

    return err;
}

} // namespace DeviceLayer
} // namespace chip
