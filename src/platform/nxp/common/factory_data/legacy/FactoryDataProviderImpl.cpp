/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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
#include <platform/KeyValueStoreManager.h>
#include <platform/nxp/common/factory_data/legacy/FactoryDataProviderImpl.h>

#include "fsl_adapter_flash.h"
#include <psa/crypto.h>
#include "mcux_psa_s2xx_key_locations.h"

#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
extern "C" WEAK CHIP_ERROR FactoryDataDefaultRestoreMechanism();
#endif

namespace chip {
namespace DeviceLayer {

#if CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION
static constexpr size_t kPrivateKeyBlobLength  = PSA_S200_NON_EL2GO_BLOB_EXPORT_SIZE(PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1), 256);

static_assert (kPrivateKeyBlobLength >= Crypto::kP256_PrivateKey_Length);
static constexpr size_t kSEBlobAdditionalLength = kPrivateKeyBlobLength - Crypto::kP256_PrivateKey_Length;
#endif

uint32_t FactoryDataProvider::kFactoryDataMaxSize = 0x800;

FactoryDataProviderImpl::FactoryDataProviderImpl()
{
#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
    RegisterRestoreMechanism(FactoryDataDefaultRestoreMechanism);
#endif
}

FactoryDataProviderImpl::~FactoryDataProviderImpl()
{
#if CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION
    if (dac_private_key_id != PSA_KEY_ID_NULL)
    {
        psa_destroy_key(dac_private_key_id);
        dac_private_key_id = PSA_KEY_ID_NULL;
    }
#endif
}

CHIP_ERROR FactoryDataProviderImpl::Init()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    mFactoryData = Nv_GetAppFactoryData();
    VerifyOrReturnError(mFactoryData != nullptr, CHIP_ERROR_INTERNAL);

    mConfig.start   = (uint32_t) &mFactoryData->app_factory_data[0];
    mConfig.size    = mFactoryData->extendedDataLength;
    mConfig.payload = mConfig.start + sizeof(FactoryDataProvider::Header);

#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
    mFactoryDataDriver = &FactoryDataDrv();
    ReturnErrorOnFailure(PostResetCheck());
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_API_TEST && CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION
    RunApiTest();
#endif

#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
    error = ValidateWithRestore();
#else
    error = Validate();
#endif
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Factory data init failed with: %" CHIP_ERROR_FORMAT, error.Format());
    }

#ifdef CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION
    ReturnErrorOnFailure(InitContext());
    ReturnErrorOnFailure(ConvertDacKey());
    ReturnErrorOnFailure(ImportPrivateKeyBlob());
#endif

    return error;
}

void FactoryDataProviderImpl::UpdateKeyAttributes(psa_key_attributes_t & attrs)
{
    if (psa_get_key_lifetime(&attrs) == PSA_KEY_LIFETIME_VOLATILE)
    {
        psa_set_key_lifetime(&attrs, PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
                                PSA_KEY_LIFETIME_VOLATILE, PSA_KEY_LOCATION_S200_KEY_STORAGE_NON_EL2GO));
    }
}

#if CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION
CHIP_ERROR FactoryDataProviderImpl::InitContext()
{
    dac_private_key_id = PSA_KEY_ID_NULL;

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::ImportPrivateKeyBlob()
{
    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    uint8_t blob[kPrivateKeyBlobLength] = { 0 };
    uint16_t blobSize                   = 0;

    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, blob, kPrivateKeyBlobLength, blobSize));

    // Define key usage and type
    psa_key_usage_t usage = PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_SIGN_MESSAGE;
    psa_key_type_t key_type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);

    // Set key attributes for P256 private key
    psa_set_key_type(&attributes, key_type);
    psa_set_key_bits(&attributes, 256);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attributes, usage);
    psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
    UpdateKeyAttributes(attributes);

    // Import the key blob
    status = psa_import_key(&attributes, blob, blobSize, &dac_private_key_id);

    psa_reset_key_attributes(&attributes);

    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::ConvertDacKey()
{
    size_t blobSize                     = kPrivateKeyBlobLength;
    size_t newSize                      = sizeof(FactoryDataProvider::Header) + mHeader.size + kSEBlobAdditionalLength;
    uint8_t blob[kPrivateKeyBlobLength] = { 0 };
    extendedAppFactoryData_t * data     = nullptr;
    uint32_t offset = 0;
    bool convNeeded = true;
    uint8_t status  = 0;

     VerifyOrReturnError(mFactoryData != nullptr, CHIP_ERROR_INTERNAL);

     data     = static_cast<extendedAppFactoryData_t *>(
        chip::Platform::MemoryAlloc(offsetof(extendedAppFactoryData_t, app_factory_data) + newSize));
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(ExportBlob(blob, &blobSize, offset, convNeeded));
    if (!convNeeded)
    {
        ChipLogError(DeviceLayer, "DAC private key already converted to blob");
        chip::Platform::MemoryFree(data);
        return CHIP_NO_ERROR;
    }
    ChipLogError(DeviceLayer, "extracted blob from DAC private key");

    memcpy(data, mFactoryData, offsetof(extendedAppFactoryData_t, app_factory_data) + mFactoryData->extendedDataLength);
    ChipLogError(DeviceLayer, "cached factory data in RAM");

    ReturnErrorOnFailure(ReplaceWithBlob(&data->app_factory_data[0], blob, blobSize, offset));
    ChipLogError(DeviceLayer, "replaced DAC private key with secured blob");

    data->extendedDataLength = newSize;
    status                   = Nv_WriteAppFactoryData(data, newSize);
    VerifyOrReturnError(status == 0, CHIP_ERROR_INTERNAL);
    ChipLogError(DeviceLayer, "updated factory data");

    memset(data, 0, newSize);
    chip::Platform::MemoryFree(data);
    ChipLogError(DeviceLayer, "sanitized RAM cache");

    ReturnErrorOnFailure(Validate());

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProviderImpl::ExportBlob(uint8_t * data, size_t * dataLen, uint32_t & offset, bool & isNeeded)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    psa_status_t status = PSA_SUCCESS;

    uint8_t keyBuf[kPrivateKeyBlobLength];
    MutableByteSpan dacPrivateKeySpan(keyBuf);
    uint16_t keySize = 0;
    isNeeded         = true;

    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t temp_key_id = PSA_KEY_ID_NULL;

    // Search for the DAC private key in factory data
    error = SearchForId(FactoryDataId::kDacPrivateKeyId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize, &offset);
    SuccessOrExit(error);
    dacPrivateKeySpan.reduce_size(keySize);

    // Check if already a blob (blob length = plain key length + metadata)
    if (keySize == kPrivateKeyBlobLength)
    {
        isNeeded = false;
        return CHIP_NO_ERROR;
    }

    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, 256);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_EXPORT);
    psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
    UpdateKeyAttributes(attributes);

    // Import the plain key
    status = psa_import_key(&attributes, dacPrivateKeySpan.data(), Crypto::kP256_PrivateKey_Length, &temp_key_id);
    psa_reset_key_attributes(&attributes);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    // Export as blob (platform-specific secure blob format)
    status = psa_export_key(temp_key_id, data, kPrivateKeyBlobLength, dataLen);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

exit:
    // Clean up temporary key if it was created
    if (temp_key_id != PSA_KEY_ID_NULL)
    {
        psa_destroy_key(temp_key_id);
    }

    /* Sanitize temporary buffer */
    memset(keyBuf, 0, Crypto::kP256_PrivateKey_Length);
    return error;
}

CHIP_ERROR FactoryDataProviderImpl::ReplaceWithBlob(uint8_t * data, uint8_t * blob, size_t blobLen, uint32_t offset)
{
    size_t newSize                       = mHeader.size + kSEBlobAdditionalLength;
    FactoryDataProvider::Header * header = reinterpret_cast<FactoryDataProvider::Header *>(data);
    uint8_t * payload                    = data + sizeof(FactoryDataProvider::Header);
    size_t subsequentDataOffset          = offset + kValueOffset + Crypto::kP256_PrivateKey_Length;

    memmove(payload + subsequentDataOffset + kSEBlobAdditionalLength, payload + subsequentDataOffset,
            mHeader.size - subsequentDataOffset);
    header->size = newSize;
    memcpy(payload + offset + kLengthOffset, (uint16_t *) &blobLen, sizeof(uint16_t));
    memcpy(payload + offset + kValueOffset, blob, blobLen);

    uint8_t hash[Crypto::kSHA256_Hash_Length] = { 0 };
    ReturnErrorOnFailure(Crypto::Hash_SHA256(payload, header->size, hash));
    memcpy(header->hash, hash, sizeof(header->hash));

    return CHIP_NO_ERROR;
}

#if CHIP_DEVICE_CONFIG_ENABLE_API_TEST

#define _assert(condition)                                                                                                         \
    if (!condition)                                                                                                                \
    {                                                                                                                              \
        ChipLogError(DeviceLayer, "Condition failed: " #condition);                                                                \
        while (1)                                                                                                                  \
            ;                                                                                                                      \
    }

void FactoryDataProviderImpl::RunApiTest()
{
    uint8_t privateKey[Crypto::kP256_PrivateKey_Length] = { 0x18, 0xfe, 0x9a, 0xd9, 0x30, 0xdd, 0x2f, 0x62, 0xbe, 0x99, 0x43,
                                                            0x93, 0xe8, 0xbe, 0x47, 0x28, 0x7f, 0xda, 0x5a, 0x71, 0x86, 0x1b,
                                                            0x0e, 0x3f, 0x91, 0x27, 0x52, 0xd0, 0xba, 0xa7, 0x40, 0x02 };

    auto error = InitContext();
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

    mConfig.payload = (uint32_t) factoryData + sizeof(FactoryDataProvider::Header);

    uint8_t keyBuf[Crypto::kP256_PrivateKey_Length];
    MutableByteSpan dacPrivateKeySpan(keyBuf);
    uint16_t keySize = 0;
    error            = SearchForId(FactoryDataId::kCertDeclarationId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize);
    _assert((error == CHIP_ERROR_NOT_FOUND));
    error = SearchForId(FactoryDataId::kDacPrivateKeyId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize);
    _assert((error == CHIP_NO_ERROR));
    _assert((memcmp(dacPrivateKeySpan.data(), privateKey, Crypto::kP256_PrivateKey_Length) == 0));

    size_t blobSize                     = kPrivateKeyBlobLength;
    size_t newSize                      = sizeof(FactoryDataProvider::Header) + mHeader.size + kSEBlobAdditionalLength;
    uint8_t blob[kPrivateKeyBlobLength] = { 0 };

    uint32_t offset = 0;
    bool convNeeded = true;
    error           = ExportBlob(blob, &blobSize, offset, convNeeded);
    _assert((error == CHIP_NO_ERROR));
    _assert((convNeeded == true));
    _assert((blobSize == kPrivateKeyBlobLength));

    error = ReplaceWithBlob(factoryData, blob, blobSize, offset);
    _assert((error == CHIP_NO_ERROR));

    FactoryDataProvider::Header * header = reinterpret_cast<FactoryDataProvider::Header *>(factoryData);
    _assert((header->size == (mHeader.size + kSEBlobAdditionalLength)));
    _assert((header->hash[0] != 0xde));
    _assert((header->hash[1] != 0xad));
    _assert((header->hash[2] != 0xbe));
    _assert((header->hash[3] != 0xef));

    // Test importing the blob
    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t test_key_id = PSA_KEY_ID_NULL;

    psa_key_usage_t usage = PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_SIGN_HASH;
    psa_key_type_t key_type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);

    psa_set_key_type(&attributes, key_type);
    psa_set_key_bits(&attributes, 256);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attributes, usage);
    psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
    UpdateKeyAttributes(attributes);

    status = psa_import_key(&attributes, blob, blobSize, &test_key_id);
    psa_reset_key_attributes(&attributes);
    _assert((status == PSA_SUCCESS));
    _assert((test_key_id != PSA_KEY_ID_NULL));

    // Test signing with the imported key
    uint8_t test_hash[Crypto::kSHA256_Hash_Length] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                                        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                                                        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                                        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20 };
    uint8_t signature[PSA_SIGNATURE_MAX_SIZE] = { 0 };
    size_t signature_length = 0;

    status = psa_sign_hash(test_key_id, PSA_ALG_ECDSA(PSA_ALG_SHA_256),
                          test_hash, sizeof(test_hash),
                          signature, sizeof(signature), &signature_length);
    _assert((status == PSA_SUCCESS));
    _assert((signature_length > 0));

    // Clean up test key
    psa_destroy_key(test_key_id);

    // Clean up
    memset(factoryData, 0, size);
    chip::Platform::MemoryFree(factoryData);
    mConfig.payload = mConfig.start + sizeof(FactoryDataProvider::Header);

    ChipLogProgress(DeviceLayer, "RunApiTest: All tests passed!");
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_API_TEST
#endif // CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION

#ifndef CONFIG_CHIP_FACTORY_DATA_PROVIDER_CUSTOM_SINGLETON_IMPL
FactoryDataProvider & FactoryDataPrvdImpl()
{
    static FactoryDataProviderImpl sInstance;
    return sInstance;
}
#endif

} // namespace DeviceLayer
} // namespace chip
