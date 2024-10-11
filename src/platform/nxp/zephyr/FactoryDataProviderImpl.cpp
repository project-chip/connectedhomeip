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

#include "FactoryDataProviderImpl.h"

/* mbedtls */
#include "mbedtls/aes.h"
#include "mbedtls/sha256.h"

/* -------------------------------------------------------------------------- */
/*                               Private macros                               */
/* -------------------------------------------------------------------------- */

#define HASH_ID 0xCE47BA5E

/* -------------------------------------------------------------------------- */
/*                            Class implementation                            */
/* -------------------------------------------------------------------------- */

namespace chip {
namespace DeviceLayer {

FactoryDataProviderImpl FactoryDataProviderImpl::sInstance;

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
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacCertificateId, NULL, 0, certificateSize, &certificateAddr));
    MutableByteSpan dacCertSpan((uint8_t *) certificateAddr, certificateSize);

    /* Extract Public Key of DAC certificate from itself */
    ReturnErrorOnFailure(Crypto::ExtractPubkeyFromX509Cert(dacCertSpan, dacPublicKey));

    /* Get private key of DAC certificate from reserved section */
    uint16_t keySize = 0;
    uint32_t keyAddr;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, NULL, 0, keySize, &keyAddr));
    MutableByteSpan dacPrivateKeySpan((uint8_t *) keyAddr, keySize);

    ReturnErrorOnFailure(LoadKeypairFromRaw(ByteSpan(dacPrivateKeySpan.data(), dacPrivateKeySpan.size()),
                                            ByteSpan(dacPublicKey.Bytes(), dacPublicKey.Length()), keypair));

    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(digestToSign.data(), digestToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

CHIP_ERROR FactoryDataProviderImpl::LoadKeypairFromRaw(ByteSpan privateKey, ByteSpan publicKey, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serialized_keypair;
    ReturnErrorOnFailure(serialized_keypair.SetLength(privateKey.size() + publicKey.size()));
    memcpy(serialized_keypair.Bytes(), publicKey.data(), publicKey.size());
    memcpy(serialized_keypair.Bytes() + publicKey.size(), privateKey.data(), privateKey.size());
    return keypair.Deserialize(serialized_keypair);
}

CHIP_ERROR FactoryDataProviderImpl::Init(void)
{
    int ret;
    CHIP_ERROR res;
    const struct device * flashDevice;
    uint8_t currentBlock[16];
    off_t factoryDataOffset = FIXED_PARTITION_OFFSET(factory_partition);

    flashDevice = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));

    /* Read the factory data header from flash */
    ret = flash_read(flashDevice, factoryDataOffset, (void *) &mFactoryData, sizeof(FactoryDataProviderImpl::Header));
    if (ret != 0)
    {
        return CHIP_ERROR_READ_FAILED;
    }

    /* Check ID is valid */
    if (mFactoryData.header.hashId != HASH_ID)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    // TODO: add HASH compute + check

    factoryDataOffset += sizeof(FactoryDataProviderImpl::Header);

    /* Load the buffer into RAM by reading each 16 bytes blocks */
    for (int i = 0; (uint32_t) i < (mFactoryData.header.size / 16U); i++)
    {
        ret = flash_read(flashDevice, factoryDataOffset + i * 16, (void *) &currentBlock[0], sizeof(currentBlock));
        if (ret != 0)
        {
            return CHIP_ERROR_READ_FAILED;
        }

        if (pAes128Key != NULL)
        {
            /* Decrypt data if a key has been set */
            res = ReadEncryptedData(&mFactoryData.factoryDataBuffer[i * 16], &currentBlock[0]);
            if (res != CHIP_NO_ERROR)
            {
                return res;
            }
        }
        else
        {
            /* No key was set, copy the data as is */
            memcpy(&mFactoryData.factoryDataBuffer[i * 16], &currentBlock[0], sizeof(currentBlock));
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::SetAes128Key(const uint8_t * keyAes128)
{
    CHIP_ERROR error = CHIP_ERROR_INVALID_ARGUMENT;
    if (keyAes128 != nullptr)
    {
        pAes128Key = keyAes128;
        error      = CHIP_NO_ERROR;
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

CHIP_ERROR FactoryDataProviderImpl::ReadEncryptedData(uint8_t * dest, uint8_t * source)
{
    mbedtls_aes_context aesCtx;

    mbedtls_aes_init(&aesCtx);

    if (mbedtls_aes_setkey_dec(&aesCtx, pAes128Key, 128U) != 0)
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

} // namespace DeviceLayer
} // namespace chip
