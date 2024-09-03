/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "FactoryDataProviderEncImpl.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#include "ELSFactoryData.h"
#include "mflash_drv.h"

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
extern uint32_t __FACTORY_DATA_END[];

using namespace ::chip::Credentials;
using namespace ::chip::Crypto;

namespace chip {
namespace DeviceLayer {

FactoryDataProviderImpl FactoryDataProviderImpl::sInstance;

static constexpr size_t kAesKeyBlobLength = Crypto::kP256_PrivateKey_Length + ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD;

#define TAG_ID_FOR_AES_KEY_BOLB 0xFE
#define MEM_ALIGN_4(size) ((size + 4) / 4 * 4)

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

CHIP_ERROR FactoryDataProviderImpl::ELS_ImportWrappedKeyAndDecrypt(MutableByteSpan & key, uint8_t * encrypt, uint16_t size,
                                                                   uint8_t * decrypt)
{
    uint8_t els_key_blob[kAesKeyBlobLength];
    size_t els_key_blob_size       = sizeof(els_key_blob);
    status_t status                = STATUS_SUCCESS;
    mcuxClEls_KeyIndex_t key_index = MCUXCLELS_KEY_SLOTS;
    uint8_t public_key[64]         = { 0 };
    size_t public_key_size         = sizeof(public_key);

    mcuxClEls_KeyProp_t plain_key_properties = {
        .word = { .value = MCUXCLELS_KEYPROPERTY_VALUE_SECURE | MCUXCLELS_KEYPROPERTY_VALUE_PRIVILEGED |
                      MCUXCLELS_KEYPROPERTY_VALUE_KEY_SIZE_256 | MCUXCLELS_KEYPROPERTY_VALUE_AES }
    };

    if (key.data() == NULL)
        return CHIP_ERROR_INTERNAL;

    memcpy(els_key_blob, key.data(), key.size());

    PLOG_DEBUG_BUFFER("els key blob", els_key_blob, els_key_blob_size);

    /* Import blob DAC key into SE50 (reserved key slot) */
    status = import_die_int_wrapped_key_into_els(els_key_blob, els_key_blob_size, plain_key_properties, &key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("import_die_int_wrapped_key_into_els failed: 0x%08x", status);

    ELS_Cipher_Aes_Ecb_Decrypt(key_index, encrypt, size, decrypt);

    els_delete_key(key_index);

    return CHIP_NO_ERROR;
exit:
    els_delete_key(key_index);
    return CHIP_ERROR_INVALID_SIGNATURE;
}

CHIP_ERROR FactoryDataProviderImpl::LoadKeypairFromRaw(ByteSpan privateKey, ByteSpan publicKey, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serialized_keypair;
    ReturnErrorOnFailure(serialized_keypair.SetLength(privateKey.size() + publicKey.size()));
    memcpy(serialized_keypair.Bytes(), publicKey.data(), publicKey.size());
    memcpy(serialized_keypair.Bytes() + publicKey.size(), privateKey.data(), privateKey.size());
    return keypair.Deserialize(serialized_keypair);
}

CHIP_ERROR FactoryDataProviderImpl::SignWithDacKey(const ByteSpan & digestToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(IsSpanUsable(outSignBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(digestToSign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // In a non-exemplary implementation, the public key is not needed here. It is used here merely because
    // Crypto::P256Keypair is only (currently) constructable from raw keys if both private/public keys are present.
    Crypto::P256PublicKey dacPublicKey;
    uint16_t certificateSize = 0;
    uint32_t certificateAddr;
    ReturnLogErrorOnFailure(SearchForId(FactoryDataId::kDacCertificateId, NULL, 0, certificateSize, &certificateAddr));
    MutableByteSpan dacCertSpan((uint8_t *) certificateAddr, certificateSize);

    /* Extract Public Key of DAC certificate from itself */
    ReturnLogErrorOnFailure(Crypto::ExtractPubkeyFromX509Cert(dacCertSpan, dacPublicKey));

    /* Get private key of DAC certificate from reserved section */
    uint16_t keySize = 0;
    uint32_t keyAddr;
    ReturnLogErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, NULL, 0, keySize, &keyAddr));
    MutableByteSpan dacPrivateKeySpan((uint8_t *) keyAddr, keySize);

    ReturnLogErrorOnFailure(LoadKeypairFromRaw(ByteSpan(dacPrivateKeySpan.data(), dacPrivateKeySpan.size()),
                                               ByteSpan(dacPublicKey.Bytes(), dacPublicKey.Length()), keypair));

    ReturnLogErrorOnFailure(keypair.ECDSA_sign_msg(digestToSign.data(), digestToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

CHIP_ERROR FactoryDataProviderImpl::ReadAndCheckFactoryDataInFlash(void)
{
    status_t status;
    uint32_t factoryDataAddress = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize    = (uint32_t) __FACTORY_DATA_SIZE;
    uint32_t hashId;

    uint16_t i;

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

    return CHIP_NO_ERROR;
}

/* the factory data must be sencrypted by AES-256 */
CHIP_ERROR FactoryDataProviderImpl::DecryptAndCheckFactoryData(void)
{
    status_t status;
    bool encrypted           = false;
    uint16_t i               = 0;
    uint32_t factoryDataSize = mHeader.size;
    uint8_t calculatedHash[SHA256_OUTPUT_SIZE];
    uint8_t currentBlock[BLOCK_SIZE_16_BYTES];
    CHIP_ERROR res;
    uint8_t type;
    uint16_t len;

    while (i < factoryDataSize)
    {
        type = factoryDataRamBuffer[i];
        len  = factoryDataRamBuffer[i + 1] + (factoryDataRamBuffer[i + 2] << 8);
        i += len;
        if ((type > kMaxId) || (i > factoryDataSize))
        {
            ChipLogProgress(DeviceLayer, "factory data is encrypted\n");
            encrypted = true;
            break;
        }
    }

    if (encrypted)
    {
        mbedtls_aes_context aesCtx;
        int status = 0;
        do
        {
            mbedtls_aes_init(&aesCtx);
            if (mbedtls_aes_setkey_dec(&aesCtx, pAesKey, 256U) != 0)
            {
                status = -1;
                break;
            }
            for (i = 0; i < factoryDataSize / BLOCK_SIZE_16_BYTES; i++)
            {
                if (mbedtls_aes_crypt_ecb(&aesCtx, MBEDTLS_AES_DECRYPT, &factoryDataRamBuffer[i * 16], &currentBlock[0]) != 0)
                {
                    status = -2;
                    break;
                }
                memcpy(&factoryDataRamBuffer[i * 16], &currentBlock[0], sizeof(currentBlock));
            }
            mbedtls_aes_free(&aesCtx);
        } while (false);

        if (status < 0)
        {
            ChipLogError(DeviceLayer, "factory data decryption Failure, status:%d", status);
            return CHIP_ERROR_WRONG_ENCRYPTION_TYPE;
        }
        ChipLogProgress(DeviceLayer, "factory data decryption is successful!");
    }
    else
    {
        ChipLogError(DeviceLayer, "factory data is in plain text!");
    }

    /* Calculate SHA256 value over the factory data and compare with stored value */
    res = Hash_SHA256(&factoryDataRamBuffer[0], mHeader.size, &calculatedHash[0]);

    if (res != CHIP_NO_ERROR)
        return res;

    if (memcmp(&calculatedHash[0], &mHeader.hash[0], HASH_LEN) != 0)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    ChipLogProgress(DeviceLayer, "factory data hash check is successful!");

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

CHIP_ERROR FactoryDataProviderImpl::Init(void)
{
    uint8_t type;
    ReturnLogErrorOnFailure(ReadAndCheckFactoryDataInFlash());

    els_enable();

    ChipLogProgress(DeviceLayer, "init: protect whole factory data\n");

    /* check if AES Key blob attached in the tail of factory data*/
    uint16_t size = MEM_ALIGN_4(sizeof(uint8_t) + sizeof(uint16_t) + kAesKeyBlobLength);
    uint16_t len;
    type = factoryDataRamBuffer[FACTORY_DATA_MAX_SIZE - size];
    len  = factoryDataRamBuffer[FACTORY_DATA_MAX_SIZE - size + kLengthOffset] +
        (factoryDataRamBuffer[FACTORY_DATA_MAX_SIZE - size + kLengthOffset + 1] << 8);
    ChipLogProgress(DeviceLayer, "aes key blob type:%x, len:%d", type, len);
    if ((type == TAG_ID_FOR_AES_KEY_BOLB) && (len == kAesKeyBlobLength))
    {
        MutableByteSpan key(&factoryDataRamBuffer[FACTORY_DATA_MAX_SIZE - size + kValueOffset], len);
        mcuxClEls_KeyIndex_t index;

        uint8_t * data = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(mHeader.size));
        memset(data, 0, mHeader.size);

        /* will provision the wrapping key to edgelock and decrypt the factory data */
        ReturnLogErrorOnFailure(ELS_ImportWrappedKeyAndDecrypt(key, &factoryDataRamBuffer[0], mHeader.size, data));

        memcpy(factoryDataRamBuffer, data, FACTORY_DATA_MAX_SIZE);
        chip::Platform::MemoryFree(data);
    }
    else
    {
        /* This situation is usually in production mode, the AES key is passed through Uart and only runs once.
        The AES Key is provisioined into Edge Lock, and the returned wrapped key is store in the end the factory data in TLV mode.
        Here we take the precondition that we already got the AES Key, and check whether the decrypted factory data is right and
        provision it into Edge Lock if right. */
        /* pAesKey should be pointed to a real key in advance on app layer, so should not be NULL.
        Currently the fwk_factory_data_provider module supports only ecb mode. Therefore return an error if encrypt mode is not ecb
      */
        if (pAesKey == NULL || encryptMode != encrypt_ecb)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        ReturnLogErrorOnFailure(DecryptAndCheckFactoryData());
        ReturnLogErrorOnFailure(ELS_SaveAesKeyBlob());

        ChipLogProgress(DeviceLayer, "System restarting");
        // Restart the system.
        NVIC_SystemReset();
        while (1)
        {
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::ELS_SaveAesKeyBlob()
{
    size_t blobSize                 = kAesKeyBlobLength;
    uint8_t blob[kAesKeyBlobLength] = { 0 };
    uint32_t factoryDataAddress     = (uint32_t) __FACTORY_DATA_START_OFFSET;
    uint32_t factoryDataSize        = (uint32_t) __FACTORY_DATA_SIZE;

    VerifyOrReturnError(factoryDataRamBuffer != nullptr, CHIP_ERROR_INTERNAL);

    uint8_t type = TAG_ID_FOR_AES_KEY_BOLB;
    ReturnErrorOnFailure(ELS_ExportBlob(blob, &blobSize));
    PLOG_DEBUG_BUFFER("els key blob", blob, blobSize);
    ChipLogProgress(DeviceLayer, "SSS: extracted blob from factory data AES key");

    /* Read all factory data */
    hal_flash_status_t status =
        HAL_FlashRead(factoryDataAddress + MFLASH_BASE_ADDRESS, FACTORY_DATA_MAX_SIZE, &factoryDataRamBuffer[0]);
    VerifyOrReturnError(status == kStatus_HAL_Flash_Success, CHIP_ERROR_INTERNAL);
    ChipLogError(DeviceLayer, "SSS: cached factory data in RAM");

    /* save the Aes Key Blob in the end of factory data region */
    uint16_t size = MEM_ALIGN_4(sizeof(uint8_t) + sizeof(uint16_t) + kAesKeyBlobLength);
    memcpy(&factoryDataRamBuffer[factoryDataSize - size], &type, sizeof(uint8_t));
    memcpy(&factoryDataRamBuffer[factoryDataSize - size + kLengthOffset], &blobSize, sizeof(uint16_t));
    memcpy(&factoryDataRamBuffer[factoryDataSize - size + kValueOffset], blob, blobSize);
    ChipLogError(DeviceLayer, "SSS: attach wrapped key in the end of factory data in tlv format");

    /* Erase flash factory data sectors */
    status = HAL_FlashEraseSector(factoryDataAddress + MFLASH_BASE_ADDRESS, FACTORY_DATA_MAX_SIZE);
    VerifyOrReturnError(status == kStatus_HAL_Flash_Success, CHIP_ERROR_INTERNAL);
    /* Write new factory data into flash */
    status = HAL_FlashProgramUnaligned(factoryDataAddress + MFLASH_BASE_ADDRESS, FACTORY_DATA_MAX_SIZE, factoryDataRamBuffer);
    VerifyOrReturnError(status == kStatus_HAL_Flash_Success, CHIP_ERROR_INTERNAL);
    ChipLogError(DeviceLayer, "SSS: updated factory data");

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::ELS_ExportBlob(uint8_t * data, size_t * dataLen)
{
    status_t status                          = STATUS_SUCCESS;
    mcuxClEls_KeyProp_t plain_key_properties = {
        .word = { .value = MCUXCLELS_KEYPROPERTY_VALUE_SECURE | MCUXCLELS_KEYPROPERTY_VALUE_PRIVILEGED |
                      MCUXCLELS_KEYPROPERTY_VALUE_KEY_SIZE_256 | MCUXCLELS_KEYPROPERTY_VALUE_AES }
    };

    mcuxClEls_KeyIndex_t key_index = MCUXCLELS_KEY_SLOTS;
    /* Import plain DAC key into S50 */
    status = import_plain_key_into_els(pAesKey, Crypto::kP256_PrivateKey_Length, plain_key_properties, &key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("derive_key failed: 0x%08x", status);

    /* ELS generate key blob. The blob created here is one that can be directly imported into ELS again. */
    status = export_key_from_els(key_index, data, dataLen);
    STATUS_SUCCESS_OR_EXIT_MSG("export_key_from_els failed: 0x%08x", status);

    status = els_delete_key(key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::SetAes256Key(const uint8_t * keyAes256)
{
    CHIP_ERROR error = CHIP_ERROR_INVALID_ARGUMENT;
    if (keyAes256 != nullptr)
    {
        pAesKey = keyAes256;
        error   = CHIP_NO_ERROR;
    }

    return error;
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

} // namespace DeviceLayer
} // namespace chip
