/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <credentials/CHIPCert.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/Base64.h>
#include <lib/support/Span.h>
#include <platform/ConfigurationManager.h>
#include <platform/nxp/common/factory_data/legacy/FactoryDataProvider.h>
#include <psa/crypto.h>
#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#endif
#include <cctype>

#ifdef CONFIG_CHIP_NXP_PLATFORM_MCXW72
#include "mcux_psa_s2xx_key_locations.h"
#endif

#ifndef CHIP_USE_DEVICE_CONFIG_CERTIFICATION_DECLARATION
#define CHIP_USE_DEVICE_CONFIG_CERTIFICATION_DECLARATION 0
#endif // CHIP_USE_DEVICE_CONFIG_CERTIFICATION_DECLARATION

#define CBC_INITIAL_VECTOR_SIZE 16

using namespace chip::DeviceLayer::PersistedStorage;

namespace chip {
namespace DeviceLayer {

static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
    BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;
static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;

#ifdef CONFIG_CHIP_NXP_PLATFORM_MCXW72
static constexpr size_t kDacKeyBlobSize =
    PSA_S200_NON_EL2GO_BLOB_EXPORT_SIZE(PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1), 256);
#elif defined(CONFIG_CHIP_NXP_PLATFORM_RW61X)
static constexpr size_t kDacKeyBlobSize = 48;
#else
#define kDacKeyBlobSize Crypto::kP256_PrivateKey_Length
#endif

FactoryDataProvider::~FactoryDataProvider() {}

#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR

CHIP_ERROR FactoryDataProvider::ValidateWithRestore()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrReturnError(mRestoreMechanisms.size() > 0, CHIP_FACTORY_DATA_RESTORE_MECHANISM);

    for (auto & restore : mRestoreMechanisms)
    {
        error = restore();
        if (error != CHIP_NO_ERROR)
        {
            continue;
        }

        error = Validate();
        if (error != CHIP_NO_ERROR)
        {
            continue;
        }

        break;
    }

    if (error == CHIP_NO_ERROR)
    {
        error = mFactoryDataDriver->DeleteBackup();
    }

    return error;
}

void FactoryDataProvider::RegisterRestoreMechanism(RestoreMechanism restore)
{
    mRestoreMechanisms.insert(mRestoreMechanisms.end(), restore);
}

#endif

CHIP_ERROR FactoryDataProvider::SignWithDacKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;
    Crypto::P256SerializedKeypair serializedKeypair;
    uint8_t keyBuf[kDacKeyBlobSize];

    MutableByteSpan dacPrivateKeySpan(keyBuf);
    uint16_t keySize = 0;

    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t key_id             = 0;
    psa_key_lifetime_t lifetime     = PSA_KEY_LIFETIME_VOLATILE;
    psa_algorithm_t alg             = PSA_ALG_ECDSA(PSA_ALG_SHA_256);
    psa_key_usage_t usage           = PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_SIGN_MESSAGE;
    psa_key_type_t key_type         = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);
    size_t bits                     = 256;
    psa_status_t status;
    uint8_t digest[Crypto::kSHA256_Hash_Length];
    unsigned char ecc_signature[PSA_SIGNATURE_MAX_SIZE] = { 0 };
    size_t signature_length                             = sizeof(ecc_signature);

    VerifyOrExit(!outSignBuffer.empty(), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(!messageToSign.empty(), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(outSignBuffer.size() >= signature.Capacity(), error = CHIP_ERROR_BUFFER_TOO_SMALL);

    /* Get private key of DAC certificate from reserved section */
    error = SearchForId(FactoryDataId::kDacPrivateKeyId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize);
    SuccessOrExit(error);
    dacPrivateKeySpan.reduce_size(keySize);

    /* Calculate message HASH to sign */
    memset(&digest[0], 0, sizeof(digest));
    error = chip::Crypto::Hash_SHA256(messageToSign.data(), messageToSign.size(), &digest[0]);
    SuccessOrExit(error);

    psa_set_key_usage_flags(&attributes, usage);
    psa_set_key_algorithm(&attributes, alg);
    psa_set_key_type(&attributes, key_type);
    psa_set_key_bits(&attributes, bits);
    psa_set_key_lifetime(&attributes, lifetime);
    UpdateKeyAttributes(attributes);

    /* Import blob DAC key into PSA */
    status = psa_import_key(&attributes, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), &key_id);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_sign_hash(key_id, alg, digest, sizeof(digest), ecc_signature, sizeof(ecc_signature), &signature_length);

    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    /* Generate MutableByteSpan with ECC signature and ECC signature size */
    error = CopySpanToMutableSpan(ByteSpan{ ecc_signature, signature_length }, outSignBuffer);

exit:
    psa_destroy_key(key_id);
    psa_reset_key_attributes(&attributes);
    memset(keyBuf, 0, sizeof(keyBuf));
    memset(digest, 0, sizeof(digest));
    memset(ecc_signature, 0, sizeof(ecc_signature));
    return error;
}

CHIP_ERROR FactoryDataProvider::Validate()
{
    uint8_t output[Crypto::kSHA256_Hash_Length] = { 0 };

    memcpy(&mHeader, (void *) mConfig.start, sizeof(Header));
    VerifyOrReturnError(mHeader.hashId == kHashId, CHIP_FACTORY_DATA_HASH_ID);

    ReturnErrorOnFailure(Crypto::Hash_SHA256((uint8_t *) mConfig.payload, mHeader.size, output));
    VerifyOrReturnError(memcmp(output, mHeader.hash, kHashLen) == 0, CHIP_FACTORY_DATA_SHA_CHECK);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SearchForId(uint8_t searchedType, uint8_t * pBuf, size_t bufLength, uint16_t & length,
                                            uint32_t * offset)
{
    uint32_t addr = mConfig.payload;
    uint8_t type  = 0;

    while (addr < (mConfig.payload + mHeader.size))
    {
        memcpy(&type, (void *) addr, sizeof(type));
        memcpy(&length, (void *) (addr + 1), sizeof(length));

        if (searchedType == type)
        {
            VerifyOrReturnError(bufLength >= length, CHIP_ERROR_BUFFER_TOO_SMALL);
            memcpy(pBuf, (void *) (addr + kValueOffset), length);

            if (offset)
                *offset = (addr - mConfig.payload);

            return CHIP_NO_ERROR;
        }
        else
        {
            /* Jump past 3 bytes of length and then use length to jump to next data */
            addr = addr + kValueOffset + length;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR

extern "C" WEAK CHIP_ERROR FactoryDataDefaultRestoreMechanism()
{
    CHIP_ERROR error           = CHIP_NO_ERROR;
    FactoryDataDriver * driver = &FactoryDataDrv();

    VerifyOrReturnError(driver != nullptr, CHIP_ERROR_INTERNAL);

    // Check if key related to factory data backup exists.
    // If it does, it means an external event (such as a power loss)
    // interrupted the factory data update process and the section
    // from internal flash is most likely erased and should be restored.
    error = driver->ReadBackupInRam();

    if (error == CHIP_NO_ERROR)
    {
        error = driver->UpdateFactoryData();
        if (error == CHIP_NO_ERROR)
        {
            ChipLogProgress(DeviceLayer, "Factory data was restored successfully");
        }
    }

    ReturnErrorOnFailure(driver->ClearRamBackup());

    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        return CHIP_NO_ERROR;

    return error;
}

CHIP_ERROR FactoryDataProvider::PreResetCheck()
{
    OTARequestorInterface * requestor = nullptr;
    uint32_t targetVersion;
    /* Check integrity of freshly copied data. If validation fails, OTA will be aborted
     * and factory data will be restored to the previous version. Use device instance info
     * provider getter to access the factory data provider instance. The instance is created
     * by the application, so it's easier to access it this way.*/
    ReturnErrorOnFailure(Validate());

    requestor = GetRequestorInstance();
    VerifyOrReturnError(requestor != nullptr, CHIP_ERROR_INVALID_ADDRESS);

    targetVersion = requestor->GetTargetVersion();
    ReturnErrorOnFailure(FactoryDataProvider::SaveTargetVersion(targetVersion));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::PostResetCheck()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t targetVersion, currentVersion;

    err = FactoryDataProvider::GetTargetVersion(targetVersion);
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "Could not get target version");

    err = DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currentVersion);
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "Could not get current version");

    if (targetVersion == currentVersion)
    {
        ChipLogProgress(DeviceLayer, "OTA successfully applied");
        // If this point is reached, it means the new image successfully booted.
        // Delete the factory data backup to stop doing a restore.
        // This ensures that both the factory data and app were updated, otherwise
        // revert to the backed up factory data.
        mFactoryDataDriver->DeleteBackup();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetTargetVersion(uint32_t & version)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint16_t len     = sizeof(uint32_t);
    size_t bytesRead = 0;

    error = KeyValueStoreMgr().Get(FactoryDataProvider::GetTargetVersionKey().KeyName(), (uint8_t *) &version, len, &bytesRead);
    ReturnErrorOnFailure(error);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SaveTargetVersion(uint32_t & version)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    error = KeyValueStoreMgr().Put(FactoryDataProvider::GetTargetVersionKey().KeyName(), (uint8_t *) &version, sizeof(uint32_t));
    ReturnErrorOnFailure(error);

    return CHIP_NO_ERROR;
}
#endif // CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
#if CHIP_USE_DEVICE_CONFIG_CERTIFICATION_DECLARATION
    constexpr uint8_t kCdForAllExamples[] = CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION;

    return CopySpanToMutableSpan(ByteSpan{ kCdForAllExamples }, outBuffer);
#else
    uint16_t declarationSize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kCertDeclarationId, outBuffer.data(), outBuffer.size(), declarationSize));
    outBuffer.reduce_size(declarationSize);

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    uint16_t certificateSize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacCertificateId, outBuffer.data(), outBuffer.size(), certificateSize));
    outBuffer.reduce_size(certificateSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    uint16_t certificateSize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kPaiCertificateId, outBuffer.data(), outBuffer.size(), certificateSize));
    outBuffer.reduce_size(certificateSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    return SignWithDacKey(messageToSign, outSignBuffer);
}

CHIP_ERROR FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    uint32_t discriminator = 0;
    uint16_t temp          = 0;

    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDiscriminatorId, (uint8_t *) &discriminator, sizeof(discriminator), temp));
    setupDiscriminator = (uint16_t) (discriminator & 0x0000FFFF);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    uint16_t temp = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kIcId, (uint8_t *) &iterationCount, sizeof(iterationCount), temp));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    uint16_t saltB64Len                     = 0;

    ReturnErrorOnFailure(SearchForId(FactoryDataId::kSaltId, (uint8_t *) (&saltB64[0]), sizeof(saltB64), saltB64Len));
    size_t saltLen = chip::Base64Decode32(saltB64, saltB64Len, reinterpret_cast<uint8_t *>(saltB64));

    VerifyOrReturnError(saltLen <= saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(saltLen <= sizeof(saltB64), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltBuf.data(), saltB64, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    char verifierB64[kSpake2pSerializedVerifier_MaxBase64Len] = { 0 };
    uint16_t verifierB64Len                                   = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kVerifierId, (uint8_t *) &verifierB64[0], sizeof(verifierB64), verifierB64Len));

    verifierLen = chip::Base64Decode32(verifierB64, verifierB64Len, reinterpret_cast<uint8_t *>(verifierB64));
    VerifyOrReturnError(verifierLen <= verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kSetupPasscodeId, (uint8_t *) &setupPasscode, sizeof(setupPasscode), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kVendorNameId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kVidId, (uint8_t *) &vendorId, sizeof(vendorId), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kProductNameId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kPidId, (uint8_t *) &productId, sizeof(productId), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kPartNumber, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kProductURL, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kProductLabel, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kSerialNumberId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    uint16_t length = 0;
    uint8_t date[ConfigurationManager::kMaxManufacturingDateLength];

    ReturnErrorOnFailure(
        SearchForId(FactoryDataId::kManufacturingDateId, date, ConfigurationManager::kMaxManufacturingDateLength, length));
    date[length] = '\0';

    if (length == 10 && isdigit(date[0]) && isdigit(date[1]) && isdigit(date[2]) && isdigit(date[3]) && date[4] == '-' &&
        isdigit(date[5]) && isdigit(date[6]) && date[7] == '-' && isdigit(date[8]) && isdigit(date[9]))
    {
        year  = (uint16_t) (1000 * (date[0] - '0') + 100 * (date[1] - '0') + 10 * (date[2] - '0') + date[3] - '0');
        month = (uint8_t) (10 * (date[5] - '0') + date[6] - '0');
        day   = (uint8_t) (10 * (date[8] - '0') + date[9] - '0');
    }
    else
    {
        ChipLogError(DeviceLayer, "Manufacturing date is not formatted correctly: YYYY-MM-DD.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(
        SearchForId(FactoryDataId::kHardwareVersionId, (uint8_t *) &hardwareVersion, sizeof(hardwareVersion), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kHardwareVersionStrId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_IMPLEMENTED;
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >= ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                  "Length of unique ID for rotating device ID is smaller than minimum.");
    uint16_t uniqueIdLen = 0;
    err                  = SearchForId(FactoryDataId::kUniqueId, (uint8_t *) uniqueIdSpan.data(), uniqueIdSpan.size(), uniqueIdLen);
#if defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    if (err != CHIP_NO_ERROR)
    {
        constexpr uint8_t uniqueId[] = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID;

        VerifyOrReturnError(sizeof(uniqueId) <= uniqueIdSpan.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(uniqueIdSpan.data(), uniqueId, sizeof(uniqueId));
        uniqueIdLen = sizeof(uniqueId);
        err         = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID
    ReturnErrorOnFailure(err);
    uniqueIdSpan.reduce_size(uniqueIdLen);
#endif

    return err;
}

CHIP_ERROR FactoryDataProvider::GetProductFinish(app::Clusters::BasicInformation::ProductFinishEnum * finish)
{
    uint8_t productFinish = 0;
    uint16_t length       = 0;
    auto err              = SearchForId(FactoryDataId::kProductFinish, &productFinish, sizeof(productFinish), length);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_NOT_IMPLEMENTED);

    *finish = static_cast<app::Clusters::BasicInformation::ProductFinishEnum>(productFinish);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor)
{
    uint8_t color   = 0;
    uint16_t length = 0;
    auto err        = SearchForId(FactoryDataId::kProductPrimaryColor, &color, sizeof(color), length);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_NOT_IMPLEMENTED);

    *primaryColor = static_cast<app::Clusters::BasicInformation::ColorEnum>(color);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SetAesKey(const uint8_t * keyAes, AESKeySize keySize)
{
    CHIP_ERROR error = CHIP_ERROR_INVALID_ARGUMENT;
    if (keyAes != nullptr)
    {
        pAesKey     = keyAes;
        pAESKeySize = keySize;
        error       = CHIP_NO_ERROR;
    }
    return error;
}

CHIP_ERROR FactoryDataProvider::SetEncryptionMode(EncryptionMode mode)
{
    CHIP_ERROR error = CHIP_ERROR_INVALID_ARGUMENT;
    if (mode <= encrypt_cbc)
    {
        encryptMode = mode;
        error       = CHIP_NO_ERROR;
    }
    return error;
}

CHIP_ERROR FactoryDataProvider::SetCbcInitialVector(const uint8_t * iv, uint16_t ivSize)
{
    CHIP_ERROR error = CHIP_ERROR_INVALID_ARGUMENT;
    if (ivSize == CBC_INITIAL_VECTOR_SIZE)
    {
        cbcInitialVector = iv;
        error            = CHIP_NO_ERROR;
    }
    return error;
}

#if CONFIG_CHIP_ENABLE_EL2GO_FACTORY_DATA
CHIP_ERROR FactoryDataProvider::ParseEl2GoBlobs(const uint8_t * blobArea, size_t blobAreaSize, size_t * blobsImported,
                                                uint32_t * dacKeyId, uint32_t * dacCertId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    psa_key_id_t importedKeys[16]; // Adjust size based on expected max blobs
    size_t importedKeyCount = 0;

    const uint8_t * blobAreaEnd    = nullptr;
    const uint8_t * currentBlobPtr = nullptr;

    bool foundDacKey  = false;
    bool foundDacCert = false;

    VerifyOrExit(blobArea != nullptr, {
        ChipLogError(DeviceLayer, "ParseEl2GoBlobs: blobArea is NULL");
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    VerifyOrExit(blobsImported != nullptr, {
        ChipLogError(DeviceLayer, "ParseEl2GoBlobs: blobsImported is NULL");
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    VerifyOrExit(dacKeyId != nullptr, {
        ChipLogError(DeviceLayer, "ParseEl2GoBlobs: dacKeyId is NULL");
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    VerifyOrExit(dacCertId != nullptr, {
        ChipLogError(DeviceLayer, "ParseEl2GoBlobs: dacCertId is NULL");
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    *blobsImported = 0;
    *dacKeyId      = 0;
    *dacCertId     = 0;

    // Now initialize the variables
    blobAreaEnd = blobArea + blobAreaSize;

    VerifyOrExit(IsBlobMagic(blobArea, blobAreaEnd), {
        ChipLogError(DeviceLayer, "ParseEl2GoBlobs: Blob area doesn't start with EL2GO blob magic signature");
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    ChipLogProgress(DeviceLayer, "ParseEl2GoBlobs: Starting blob parsing, area size = %u", blobAreaSize);

    currentBlobPtr = blobArea;

    while (currentBlobPtr < blobAreaEnd)
    {
        // Check if we have enough space for at least a minimal blob
        size_t remainingBytes = blobAreaEnd - currentBlobPtr;
        if (remainingBytes < kBlobMagicTlvSize + 2) // magic + at least tag+length
        {
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlobs: Reached end of blob area, %u bytes remaining", remainingBytes);
            break;
        }

        // Verify this position starts with blob magic
        if (!IsBlobMagic(currentBlobPtr, blobAreaEnd))
        {
            ChipLogError(DeviceLayer, "ParseEl2GoBlobs: Expected blob magic at offset %u", currentBlobPtr - blobArea);
            err = CHIP_ERROR_INVALID_ARGUMENT;
            break;
        }

        // Parse blob and get PSA-relevant fields + size
        BlobContext blobCtx;
        size_t currentBlobSize = 0;

        err = ParseEl2GoBlob(currentBlobPtr, remainingBytes, blobCtx, &currentBlobSize);
        VerifyOrExit(err == CHIP_NO_ERROR, {
            ChipLogError(DeviceLayer, "ParseEl2GoBlobs: Failed to parse blob at offset %u: %" CHIP_ERROR_FORMAT,
                         currentBlobPtr - blobArea, err.Format());
        });

        ChipLogProgress(DeviceLayer, "ParseEl2GoBlobs: Parsed blob with key ID 0x%08" PRIx32 ", size %u", blobCtx.keyId,
                        currentBlobSize);

        // Skip system blobs
        if (blobCtx.keyId == kOemKeyId || blobCtx.keyId == kRkthKeyId || blobCtx.keyId == kOtpDataKeyId ||
            blobCtx.keyId == kBatchFlowKeyId)
        {
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlobs: Skipping system blob with key ID 0x%08" PRIx32, blobCtx.keyId);
            currentBlobPtr += currentBlobSize; // Jump to next blob
            continue;
        }

        // Convert to PSA attributes
        psa_key_attributes_t psaAttrs;
        err = BlobContextToPsaAttributes(blobCtx, psaAttrs);
        VerifyOrExit(err == CHIP_NO_ERROR, {
            ChipLogError(DeviceLayer, "ParseEl2GoBlobs: Failed to convert to PSA attributes: %" CHIP_ERROR_FORMAT, err.Format());
        });

        // Detect DAC key and certificate based on key type and usage
        psa_key_type_t keyType = psa_get_key_type(&psaAttrs);
        psa_key_usage_t usage  = psa_get_key_usage_flags(&psaAttrs);

        // DAC Private Key: ECC key pair with sign usage
        if (PSA_KEY_TYPE_IS_ECC_KEY_PAIR(keyType) && (usage & PSA_KEY_USAGE_SIGN_MESSAGE))
        {
            if (!foundDacKey)
            {
                *dacKeyId   = blobCtx.keyId;
                foundDacKey = true;
                ChipLogProgress(DeviceLayer, "ParseEl2GoBlobs: Found DAC private key with ID 0x%08" PRIx32, blobCtx.keyId);
            }
            else
            {
                ChipLogProgress(DeviceLayer, "ParseEl2GoBlobs: Found additional signing key 0x%08" PRIx32 " (ignoring)",
                                blobCtx.keyId);
            }
        }
        // DAC Certificate: Raw data type with export usage
        else if (keyType == PSA_KEY_TYPE_RAW_DATA && (usage & PSA_KEY_USAGE_EXPORT))
        {
            if (!foundDacCert)
            {
                *dacCertId   = blobCtx.keyId;
                foundDacCert = true;
                ChipLogProgress(DeviceLayer, "ParseEl2GoBlobs: Found DAC certificate with ID 0x%08" PRIx32, blobCtx.keyId);
            }
            else
            {
                ChipLogProgress(DeviceLayer, "ParseEl2GoBlobs: Found additional certificate 0x%08" PRIx32 " (ignoring)",
                                blobCtx.keyId);
            }
        }

        // Import into PSA
        psa_key_id_t keyId  = PSA_KEY_ID_NULL;
        psa_status_t status = psa_import_key(&psaAttrs, currentBlobPtr, currentBlobSize, &keyId);

        if (status == PSA_ERROR_ALREADY_EXISTS)
        {
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlobs: Key 0x%08" PRIx32 " already exists, using existing key", blobCtx.keyId);
            // Key already exists, use it - don't re-import
            keyId  = blobCtx.keyId;
            status = PSA_SUCCESS;
        }

        VerifyOrExit(status == PSA_SUCCESS, {
            ChipLogError(DeviceLayer, "ParseEl2GoBlobs: psa_import_key failed for key 0x%08" PRIx32 ": %" PRId32, blobCtx.keyId,
                         status);
            err = CHIP_ERROR_INTERNAL;
        });

        // Track successfully imported key for cleanup on error
        if (importedKeyCount < sizeof(importedKeys) / sizeof(importedKeys[0]))
        {
            importedKeys[importedKeyCount++] = blobCtx.keyId;
        }
        else
        {
            ChipLogError(DeviceLayer, "ParseEl2GoBlobs: Too many keys imported, cannot track for cleanup");
            err = CHIP_ERROR_NO_MEMORY;
            goto exit;
        }

        ChipLogProgress(DeviceLayer, "ParseEl2GoBlobs: Successfully imported blob with key ID 0x%08" PRIx32, blobCtx.keyId);
        (*blobsImported)++;

        // Move to next blob using the size returned by ParseEl2GoBlob
        currentBlobPtr += currentBlobSize;
    }

    // Verify we found both DAC key and certificate
    VerifyOrExit(foundDacKey, {
        ChipLogError(DeviceLayer, "ParseEl2GoBlobs: DAC private key not found in blobs");
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    VerifyOrExit(foundDacCert, {
        ChipLogError(DeviceLayer, "ParseEl2GoBlobs: DAC certificate not found in blobs");
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    ChipLogProgress(DeviceLayer,
                    "ParseEl2GoBlobs: Successfully imported %u blobs (DAC Key: 0x%08" PRIx32 ", DAC Cert: 0x%08" PRIx32 ")",
                    *blobsImported, *dacKeyId, *dacCertId);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ParseEl2GoBlobs: Failed with error: %" CHIP_ERROR_FORMAT ", cleaning up %u imported keys",
                     err.Format(), importedKeyCount);

        // Destroy all successfully imported keys on error
        for (size_t i = 0; i < importedKeyCount; i++)
        {
            psa_status_t destroyStatus = psa_destroy_key(importedKeys[i]);
            if (destroyStatus != PSA_SUCCESS)
            {
                ChipLogError(DeviceLayer, "ParseEl2GoBlobs: Failed to destroy key 0x%08" PRIx32 " during cleanup: %" PRId32,
                             importedKeys[i], destroyStatus);
            }
            else
            {
                ChipLogProgress(DeviceLayer, "ParseEl2GoBlobs: Destroyed key 0x%08" PRIx32 " during cleanup", importedKeys[i]);
            }
        }

        *blobsImported = 0; // Reset count since we cleaned up
        *dacKeyId      = 0;
        *dacCertId     = 0;
    }

    return err;
}

CHIP_ERROR FactoryDataProvider::ParseEl2GoBlob(const uint8_t * blob, size_t maxSize, BlobContext & blobCtx, size_t * actualSize)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    const uint8_t * currentPtr = nullptr;
    const uint8_t * endPtr     = nullptr;
    size_t blobSize            = 0;
    bool foundMagic            = false;
    bool foundKeyId            = false;

    VerifyOrExit(blob != nullptr, {
        ChipLogError(DeviceLayer, "ParseEl2GoBlob: blob is NULL");
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    VerifyOrExit(maxSize > 0, {
        ChipLogError(DeviceLayer, "ParseEl2GoBlob: maxSize is 0");
        err = CHIP_ERROR_INVALID_ARGUMENT;
    });

    // Initialize output structure
    memset(&blobCtx, 0, sizeof(blobCtx));

    currentPtr = blob;
    endPtr     = blob + maxSize;

    // Parse TLVs until we hit TAG_SIGNATURE (which marks the end of the blob)
    while (currentPtr < endPtr)
    {
        // Ensure we have at least tag + length indicator
        VerifyOrExit(currentPtr + 2 <= endPtr, {
            ChipLogError(DeviceLayer, "ParseEl2GoBlob: Insufficient bytes for TLV header at offset %u", currentPtr - blob);
            err = CHIP_ERROR_BUFFER_TOO_SMALL;
        });

        uint8_t tag            = *currentPtr;
        size_t lengthFieldSize = 0;
        size_t valueLength     = 0;
        size_t remainingBytes  = endPtr - currentPtr;

        // Parse TLV length field
        if ((currentPtr[1] & 0x80) == 0)
        {
            // Short form: length is in bits 0-6 of the first byte
            lengthFieldSize = 1;
            valueLength     = currentPtr[1];
        }
        else
        {
            // Long form: bits 0-6 indicate number of subsequent length bytes
            uint8_t numLengthBytes = currentPtr[1] & 0x7F;

            VerifyOrExit(numLengthBytes > 0 && numLengthBytes <= 4, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid length encoding (numLengthBytes=%u) at offset %u",
                             numLengthBytes, currentPtr - blob);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });

            VerifyOrExit(remainingBytes >= 2 + numLengthBytes, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Insufficient bytes for length field at offset %u", currentPtr - blob);
                err = CHIP_ERROR_BUFFER_TOO_SMALL;
            });

            lengthFieldSize = 1 + numLengthBytes;
            valueLength     = 0;

            for (uint8_t i = 0; i < numLengthBytes; i++)
            {
                valueLength = (valueLength << 8) | currentPtr[2 + i];
            }
        }
        // Verify TLV doesn't exceed buffer bounds safely without overflow
        VerifyOrExit(remainingBytes >= 1 + lengthFieldSize && remainingBytes - (1 + lengthFieldSize) >= valueLength, {
            ChipLogError(DeviceLayer, "ParseEl2GoBlob: TLV size exceeds buffer at offset %u (tag=0x%02X, valueLength=%u)",
                         currentPtr - blob, tag, valueLength);
            err = CHIP_ERROR_BUFFER_TOO_SMALL;
        });

        // Calculate pointer to value
        const uint8_t * valuePtr = currentPtr + 1 + lengthFieldSize;

        // Process tag
        switch (tag)
        {
        case kTagMagic:
            VerifyOrExit(IsBlobMagic(currentPtr, currentPtr + totalTlvSize), {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid magic value");
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            foundMagic = true;
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: Found valid magic");
            break;

        case kTagKeyId:
            VerifyOrExit(valueLength == 4, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid KEY_ID size %u, expected 4", valueLength);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            blobCtx.keyId = GetUint32(valuePtr);
            foundKeyId    = true;
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: Key ID = 0x%08" PRIx32, blobCtx.keyId);
            break;

        case kTagPermittedAlgorithm:
            VerifyOrExit(valueLength == 4, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid PERMITTED_ALGORITHM size %u, expected 4", valueLength);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            blobCtx.psaPermittedAlgorithm = GetUint32(valuePtr);
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: PSA Algorithm = 0x%08" PRIx32, blobCtx.psaPermittedAlgorithm);
            break;

        case kTagKeyUsageFlags:
            VerifyOrExit(valueLength == 4, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid KEY_USAGE_FLAGS size %u, expected 4", valueLength);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            blobCtx.psaUsage = GetUint32(valuePtr);
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: PSA Usage = 0x%08" PRIx32, blobCtx.psaUsage);
            break;

        case kTagKeyType:
            VerifyOrExit(valueLength == 2, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid KEY_TYPE size %u, expected 2", valueLength);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            blobCtx.psaKeyType = GetUint16(valuePtr);
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: PSA Key Type = 0x%04X", blobCtx.psaKeyType);
            break;

        case kTagKeyBits:
            VerifyOrExit(valueLength == 4, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid KEY_BITS size %u, expected 4", valueLength);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            blobCtx.psaKeyBits = GetUint32(valuePtr);
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: PSA Key Bits = %" PRIu32, blobCtx.psaKeyBits);
            break;

        case kTagKeyLifetime:
            VerifyOrExit(valueLength == 4, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid KEY_LIFETIME size %u, expected 4", valueLength);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            blobCtx.psaKeyLifetime = GetUint32(valuePtr);
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: PSA Key Lifetime = 0x%08" PRIx32, blobCtx.psaKeyLifetime);
            break;

        case kTagDeviceLifecycle:
            VerifyOrExit(valueLength == 4, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid PSA_DEVICE_LIFECYCLE size %u, expected 4", valueLength);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: Device Lifecycle = 0x%08" PRIx32 " (validated, not stored)",
                            GetUint32(valuePtr));
            break;

        case kTagWrappingKeyId:
            VerifyOrExit(valueLength == 4, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid WRAPPING_KEY_ID size %u, expected 4", valueLength);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: Wrapping Key ID = 0x%08" PRIx32 " (validated, not stored)",
                            GetUint32(valuePtr));
            break;

        case kTagWrappingAlgorithm:
            VerifyOrExit(valueLength == 4, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid WRAPPING_ALGORITHM size %u, expected 4", valueLength);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: Wrapping Algorithm = 0x%08" PRIx32 " (validated, not stored)",
                            GetUint32(valuePtr));
            break;

        case kTagIv:
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: IV size = %u (validated, not stored)", valueLength);
            break;

        case kTagSignatureKeyId:
            VerifyOrExit(valueLength == 4, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid SIGNATURE_KEY_ID size %u, expected 4", valueLength);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: Signature Key ID = 0x%08" PRIx32 " (validated, not stored)",
                            GetUint32(valuePtr));
            break;

        case kTagSignatureAlgorithm:
            VerifyOrExit(valueLength == 4, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: Invalid SIGNATURE_ALGORITHM size %u, expected 4", valueLength);
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: Signature Algorithm = 0x%08" PRIx32 " (validated, not stored)",
                            GetUint32(valuePtr));
            break;

        case kTagPlain:
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: Plain blob size = %u (validated, not stored)", valueLength);
            break;

        case kTagSignature:
            // Signature marks the END of the blob - this is the termination condition
            blobSize += totalTlvSize;

            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: Signature size = %u, total blob size = %u", valueLength, blobSize);

            if (actualSize != nullptr)
            {
                *actualSize = blobSize;
            }

            // Verify we found required fields
            VerifyOrExit(foundMagic, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: No magic found in blob");
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });

            VerifyOrExit(foundKeyId, {
                ChipLogError(DeviceLayer, "ParseEl2GoBlob: No valid key ID found in blob");
                err = CHIP_ERROR_INVALID_ARGUMENT;
            });

            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: Successfully parsed blob with key ID 0x%08" PRIx32, blobCtx.keyId);
            goto exit; // Exit after signature - blob is complete

        default:
            ChipLogProgress(DeviceLayer, "ParseEl2GoBlob: Skipping unknown tag 0x%02X (size %u)", tag, valueLength);
            break;
        }

        // Move to next TLV
        currentPtr += totalTlvSize;
        blobSize += totalTlvSize;
    }

    // Reached end without finding TAG_SIGNATURE
    ChipLogError(DeviceLayer, "ParseEl2GoBlob: No TAG_SIGNATURE found, blob may be truncated");
    err = CHIP_ERROR_INVALID_ARGUMENT;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ParseEl2GoBlob: Failed with error: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

CHIP_ERROR FactoryDataProvider::BlobContextToPsaAttributes(const BlobContext & blobCtx, psa_key_attributes_t & psaAttrs)
{
    // Initialize PSA attributes
    psaAttrs = psa_key_attributes_init();

    // Map blob fields to PSA attributes
    psa_set_key_id(&psaAttrs, mbedtls_svc_key_id_make(0, static_cast<psa_key_id_t>(blobCtx.keyId)));
    psa_set_key_algorithm(&psaAttrs, static_cast<psa_algorithm_t>(blobCtx.psaPermittedAlgorithm));
    psa_set_key_usage_flags(&psaAttrs, static_cast<psa_key_usage_t>(blobCtx.psaUsage));
    psa_set_key_type(&psaAttrs, static_cast<psa_key_type_t>(blobCtx.psaKeyType));
    psa_set_key_bits(&psaAttrs, static_cast<size_t>(blobCtx.psaKeyBits));
    psa_set_key_lifetime(&psaAttrs, static_cast<psa_key_lifetime_t>(blobCtx.psaKeyLifetime));

    return CHIP_NO_ERROR;
}

bool FactoryDataProvider::IsBlobMagic(const uint8_t * ptr, const uint8_t * endPtr)
{
    if (ptr + kBlobMagicTlvSize > endPtr)
    {
        return false;
    }

    // The magic TLV is 104 bits long, sufficient to not randomly appear inside the blob (for practical purposes)
    return GetUint32(ptr) == kBlobMagicTlv1 && GetUint32(ptr + 4) == kBlobMagicTlv2 && GetUint32(ptr + 8) == kBlobMagicTlv3 &&
        *(ptr + 12) == kBlobMagicTlv4;
}

uint32_t FactoryDataProvider::GetUint32(const uint8_t * input)
{
    // Big-endian (MSB first) - matches EL2GO blob format
    return (static_cast<uint32_t>(input[0]) << 24) | (static_cast<uint32_t>(input[1]) << 16) |
        (static_cast<uint32_t>(input[2]) << 8) | static_cast<uint32_t>(input[3]);
}

uint16_t FactoryDataProvider::GetUint16(const uint8_t * input)
{
    // Big-endian (MSB first) - matches EL2GO blob format
    return (static_cast<uint16_t>(input[0]) << 8) | static_cast<uint16_t>(input[1]);
}
#endif

} // namespace DeviceLayer
} // namespace chip
