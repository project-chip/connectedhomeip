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
#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#endif
#include <cctype>

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
/* Secure subsystem private key blob size is 32 + 24 = 56.
 * DAC private key may be used to store an SSS exported blob instead of the private key.
 */
static constexpr size_t kDacPrivateKey_MaxLen = Crypto::kP256_PrivateKey_Length + 24;

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
    uint8_t keyBuf[Crypto::kP256_PrivateKey_Length];
    MutableByteSpan dacPrivateKeySpan(keyBuf);
    uint16_t keySize = 0;

    VerifyOrExit(!outSignBuffer.empty(), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(!messageToSign.empty(), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(outSignBuffer.size() >= signature.Capacity(), error = CHIP_ERROR_BUFFER_TOO_SMALL);

    /* Get private key of DAC certificate from reserved section */
    error = SearchForId(FactoryDataId::kDacPrivateKeyId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize);
    SuccessOrExit(error);
    dacPrivateKeySpan.reduce_size(keySize);

    /* Only the private key is used when signing */
    error = serializedKeypair.SetLength(Crypto::kP256_PublicKey_Length + dacPrivateKeySpan.size());
    SuccessOrExit(error);
    memcpy(serializedKeypair.Bytes() + Crypto::kP256_PublicKey_Length, dacPrivateKeySpan.data(), dacPrivateKeySpan.size());

    error = keypair.Deserialize(serializedKeypair);
    SuccessOrExit(error);

    error = keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature);
    SuccessOrExit(error);

    error = CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);

exit:
    /* Sanitize temporary buffer */
    memset(keyBuf, 0, Crypto::kP256_PrivateKey_Length);
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

CHIP_ERROR FactoryDataProvider::GetSoftwareVersionString(char * buf, size_t bufSize)
{
    /* The FactoryDataProvider instance is registered as a DeviceInstanceInfoProvider, which requires SoftwareVersionString support.
     This information is not stored in the factory data, as it may change after an OTA update. */
    CHIP_ERROR err = ConfigurationMgr().GetSoftwareVersionString(buf, bufSize);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get software version string from ConfigurationMgr: %s", ErrorStr(err));
    }
    return err;
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
    uint8_t productFinish;
    uint16_t length = 0;
    auto err        = SearchForId(FactoryDataId::kProductFinish, &productFinish, sizeof(productFinish), length);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_NOT_IMPLEMENTED);

    *finish = static_cast<app::Clusters::BasicInformation::ProductFinishEnum>(productFinish);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor)
{
    uint8_t color;
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

} // namespace DeviceLayer
} // namespace chip
