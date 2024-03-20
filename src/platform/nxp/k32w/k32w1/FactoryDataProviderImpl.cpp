/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <platform/nxp/k32w/k32w1/FactoryDataProviderImpl.h>

#if CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
#include "fsl_adapter_flash.h"
#endif

namespace chip {
namespace DeviceLayer {

// SSS adds 24 bytes of metadata when creating the blob
static constexpr size_t kSssBlobMetadataLength = 24;
static constexpr size_t kPrivateKeyBlobLength  = Crypto::kP256_PrivateKey_Length + kSssBlobMetadataLength;

FactoryDataProviderImpl::~FactoryDataProviderImpl()
{
    SSS_KEY_OBJ_FREE(&mContext);
}

CHIP_ERROR FactoryDataProviderImpl::Init()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

#if CHIP_DEVICE_CONFIG_ENABLE_SSS_API_TEST
    SSS_RunApiTest();
#endif

    error = Validate();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Factory data init failed with: %s", ErrorStr(error));
    }

    ReturnErrorOnFailure(SSS_InitContext());
#if CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
    ReturnErrorOnFailure(SSS_ConvertDacKey());
    ReturnErrorOnFailure(Validate());
#endif
    ReturnErrorOnFailure(SSS_ImportPrivateKeyBlob());

    return error;
}

CHIP_ERROR FactoryDataProviderImpl::SignWithDacKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;

    VerifyOrReturnError(IsSpanUsable(outSignBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(messageToSign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(messageToSign.data() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(messageToSign.size() > 0, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[Crypto::kSHA256_Hash_Length];
    memset(&digest[0], 0, sizeof(digest));

    ReturnErrorOnFailure(Crypto::Hash_SHA256(messageToSign.data(), messageToSign.size(), digest));
    ReturnErrorOnFailure(SSS_Sign(digest, signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

CHIP_ERROR FactoryDataProviderImpl::SSS_InitContext()
{
    auto res = sss_sscp_key_object_init(&mContext, &g_keyStore);
    VerifyOrReturnError(res == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);
    res = sss_sscp_key_object_allocate_handle(&mContext, 0x0u, kSSS_KeyPart_Private, kSSS_CipherType_EC_NIST_P,
                                              Crypto::kP256_PrivateKey_Length, SSS_KEYPROP_OPERATION_ASYM);
    VerifyOrReturnError(res == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::SSS_ImportPrivateKeyBlob()
{
    uint8_t blob[kPrivateKeyBlobLength] = { 0 };
    uint16_t blobSize                   = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, blob, kPrivateKeyBlobLength, blobSize));

    auto res = sss_sscp_key_store_import_key(&g_keyStore, &mContext, blob, kPrivateKeyBlobLength, kPrivateKeyBlobLength * 8,
                                             kSSS_blobType_ELKE_blob);
    VerifyOrReturnError(res == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::SSS_Sign(uint8_t * digest, Crypto::P256ECDSASignature & signature)
{
    CHIP_ERROR error     = CHIP_NO_ERROR;
    size_t signatureSize = Crypto::kP256_ECDSA_Signature_Length_Raw;
    sss_status_t res     = kStatus_SSS_Fail;
    sss_sscp_asymmetric_t asyc;

    res = sss_sscp_asymmetric_context_init(&asyc, &g_sssSession, &mContext, kAlgorithm_SSS_ECDSA_SHA256, kMode_SSS_Sign);
    VerifyOrExit(res == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    res = sss_sscp_asymmetric_sign_digest(&asyc, digest, Crypto::kP256_PrivateKey_Length, signature.Bytes(), &signatureSize);
    VerifyOrExit(res == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    signature.SetLength(signatureSize);

exit:
    sss_sscp_asymmetric_context_free(&asyc);
    return error;
}

#if CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
CHIP_ERROR FactoryDataProviderImpl::SSS_ConvertDacKey()
{
    size_t blobSize                     = kPrivateKeyBlobLength;
    size_t newSize                      = sizeof(FactoryDataProvider::Header) + mHeader.size + kSssBlobMetadataLength;
    uint8_t blob[kPrivateKeyBlobLength] = { 0 };
    uint8_t * data                      = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(newSize));
    uint32_t offset                     = 0;

    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(SSS_ExportBlob(blob, &blobSize, offset));
    ChipLogError(DeviceLayer, "SSS: extracted blob from DAC private key");

    hal_flash_status_t status = HAL_FlashRead(kFactoryDataStart, newSize - kSssBlobMetadataLength, data);
    VerifyOrReturnError(status == kStatus_HAL_Flash_Success, CHIP_ERROR_INTERNAL);
    ChipLogError(DeviceLayer, "SSS: cached factory data in RAM");

    ReturnErrorOnFailure(ReplaceWithBlob(data, blob, blobSize, offset));
    ChipLogError(DeviceLayer, "SSS: replaced DAC private key with secured blob");

    status = HAL_FlashEraseSector(kFactoryDataStart, kFactoryDataSize);
    VerifyOrReturnError(status == kStatus_HAL_Flash_Success, CHIP_ERROR_INTERNAL);
    status = HAL_FlashProgramUnaligned(kFactoryDataStart, newSize, data);
    VerifyOrReturnError(status == kStatus_HAL_Flash_Success, CHIP_ERROR_INTERNAL);
    ChipLogError(DeviceLayer, "SSS: updated factory data");

    memset(data, 0, newSize);
    chip::Platform::MemoryFree(data);
    ChipLogError(DeviceLayer, "SSS: sanitized RAM cache");

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::SSS_ExportBlob(uint8_t * data, size_t * dataLen, uint32_t & offset)
{
    uint8_t keyBuf[Crypto::kP256_PrivateKey_Length];
    MutableByteSpan dacPrivateKeySpan(keyBuf);
    uint16_t keySize = 0;

    ReturnErrorOnFailure(
        SearchForId(FactoryDataId::kDacPrivateKeyId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize, &offset));
    dacPrivateKeySpan.reduce_size(keySize);

    auto res = SSS_KEY_STORE_SET_KEY(&mContext, dacPrivateKeySpan.data(), Crypto::kP256_PrivateKey_Length, keySize * 8,
                                     kSSS_KeyPart_Private);
    VerifyOrReturnError(res == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    res = sss_sscp_key_store_export_key(&g_keyStore, &mContext, data, dataLen, kSSS_blobType_ELKE_blob);
    VerifyOrReturnError(res == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::ReplaceWithBlob(uint8_t * data, uint8_t * blob, size_t blobLen, uint32_t offset)
{
    size_t newSize                       = mHeader.size + kSssBlobMetadataLength;
    FactoryDataProvider::Header * header = reinterpret_cast<FactoryDataProvider::Header *>(data);
    uint8_t * payload                    = data + sizeof(FactoryDataProvider::Header);
    size_t subsequentDataOffset          = offset + kValueOffset + Crypto::kP256_PrivateKey_Length;

    memmove(payload + subsequentDataOffset + kSssBlobMetadataLength, payload + subsequentDataOffset,
            mHeader.size - subsequentDataOffset);
    header->size = newSize;
    memcpy(payload + offset + kLengthOffset, (uint16_t *) &blobLen, sizeof(uint16_t));
    memcpy(payload + offset + kValueOffset, blob, blobLen);

    uint8_t hash[Crypto::kSHA256_Hash_Length] = { 0 };
    ReturnErrorOnFailure(Crypto::Hash_SHA256(payload, header->size, hash));
    memcpy(header->hash, hash, sizeof(header->hash));

    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY

#if CHIP_DEVICE_CONFIG_ENABLE_SSS_API_TEST

#define _assert(condition)                                                                                                         \
    if (!condition)                                                                                                                \
    {                                                                                                                              \
        ChipLogError(DeviceLayer, "Condition failed" #condition);                                                                  \
        while (1)                                                                                                                  \
            ;                                                                                                                      \
    }

void FactoryDataProviderImpl::SSS_RunApiTest()
{
    uint8_t privateKey[Crypto::kP256_PrivateKey_Length] = { 0x18, 0xfe, 0x9a, 0xd9, 0x30, 0xdd, 0x2f, 0x62, 0xbe, 0x99, 0x43,
                                                            0x93, 0xe8, 0xbe, 0x47, 0x28, 0x7f, 0xda, 0x5a, 0x71, 0x86, 0x1b,
                                                            0x0e, 0x3f, 0x91, 0x27, 0x52, 0xd0, 0xba, 0xa7, 0x40, 0x02 };

    auto error = SSS_InitContext();
    _assert((error == CHIP_NO_ERROR));

    // Simulate factory data in RAM: create the header + dummy data + DAC private key entry + dummy data
    uint8_t type             = FactoryDataProvider::FactoryDataId::kDacPrivateKeyId;
    uint8_t dummyType        = FactoryDataProvider::FactoryDataId::kProductLabel;
    uint16_t length          = Crypto::kP256_PrivateKey_Length;
    uint16_t dummyLength     = 3;
    uint32_t numberOfDummies = 10;
    uint32_t dummySize       = numberOfDummies * (sizeof(dummyType) + sizeof(dummyLength) + dummyLength);
    uint32_t size =
        sizeof(FactoryDataProvider::Header) + dummySize + sizeof(type) + sizeof(length) + kPrivateKeyBlobLength + dummySize;
    uint8_t dummyData[3]                      = { 0xab };
    uint8_t hash[Crypto::kSHA256_Hash_Length] = { 0 };
    mHeader.hashId                            = FactoryDataProvider::kHashId;
    mHeader.size                              = size - sizeof(FactoryDataProvider::Header);
    mHeader.hash[0]                           = 0xde;
    mHeader.hash[1]                           = 0xad;
    mHeader.hash[2]                           = 0xbe;
    mHeader.hash[3]                           = 0xef;

    uint8_t * factoryData = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(size));
    _assert((factoryData != nullptr));

    uint8_t * entry = factoryData + sizeof(mHeader);
    for (auto i = 0; i < numberOfDummies; i++)
    {
        memcpy(entry, (void *) &dummyType, sizeof(dummyType));
        entry += sizeof(type);
        memcpy(entry, (void *) &dummyLength, sizeof(dummyLength));
        entry += sizeof(length);
        memcpy(entry, dummyData, dummyLength);
        entry += dummyLength;
    }
    memcpy(entry, (void *) &type, sizeof(type));
    entry += sizeof(type);
    memcpy(entry, (void *) &length, sizeof(length));
    entry += sizeof(length);
    memcpy(entry, privateKey, Crypto::kP256_PrivateKey_Length);
    entry += Crypto::kP256_PrivateKey_Length;
    for (auto i = 0; i < numberOfDummies; i++)
    {
        memcpy(entry, (void *) &dummyType, sizeof(dummyType));
        entry += sizeof(type);
        memcpy(entry, (void *) &dummyLength, sizeof(dummyLength));
        entry += sizeof(length);
        memcpy(entry, dummyData, dummyLength);
        entry += dummyLength;
    }

    FactoryDataProvider::kFactoryDataPayloadStart = (uint32_t) factoryData + sizeof(FactoryDataProvider::Header);

    uint8_t keyBuf[Crypto::kP256_PrivateKey_Length];
    MutableByteSpan dacPrivateKeySpan(keyBuf);
    uint16_t keySize = 0;
    error            = SearchForId(FactoryDataId::kCertDeclarationId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize);
    _assert((error == CHIP_ERROR_NOT_FOUND));
    error = SearchForId(FactoryDataId::kDacPrivateKeyId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize);
    _assert((error == CHIP_NO_ERROR));
    _assert((memcmp(dacPrivateKeySpan.data(), privateKey, Crypto::kP256_PrivateKey_Length) == 0));

    size_t blobSize                     = kPrivateKeyBlobLength;
    size_t newSize                      = sizeof(FactoryDataProvider::Header) + mHeader.size + kSssBlobMetadataLength;
    uint8_t blob[kPrivateKeyBlobLength] = { 0 };

    uint32_t offset = 0;
    error           = SSS_ExportBlob(blob, &blobSize, offset);
    _assert((error == CHIP_NO_ERROR));
    error = ReplaceWithBlob(factoryData, blob, blobSize, offset);
    _assert((error == CHIP_NO_ERROR));
    FactoryDataProvider::Header * header = reinterpret_cast<FactoryDataProvider::Header *>(factoryData);
    _assert((header->size == (mHeader.size + kSssBlobMetadataLength)));
    _assert((header->hash[0] != 0xde));
    _assert((header->hash[1] != 0xad));
    _assert((header->hash[2] != 0xbe));
    _assert((header->hash[3] != 0xef));

    memset(factoryData, 0, size);
    chip::Platform::MemoryFree(factoryData);
    FactoryDataProvider::kFactoryDataPayloadStart = FactoryDataProvider::kFactoryDataStart + sizeof(FactoryDataProvider::Header);
    SSS_KEY_OBJ_FREE(&mContext);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_SSS_API_TEST

} // namespace DeviceLayer
} // namespace chip
