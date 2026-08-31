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

#include "FactoryDataProvider.h"
#include "CHIPDevicePlatformConfig.h"
#include <crypto/CHIPCryptoPAL.h>

#ifdef CONFIG_CHIP_CERTIFICATION_DECLARATION_STORAGE
#include <credentials/CertificationDeclaration.h>
#include <platform/Zephyr/ZephyrConfig.h>
#endif

#include <lib/support/logging/CHIPLogging.h>

#ifdef CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_KMU
#include <cracen_psa_kmu.h>
#endif

#ifdef CONFIG_CHIP_CRYPTO_PSA
#include <lib/support/ScopedMemoryBuffer.h>
#include <psa/crypto.h>
#include <zephyr/drivers/flash.h>

static const struct device * const kFlashDev = DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_flash_controller));
#endif

namespace chip {
namespace {

CHIP_ERROR GetFactoryDataString(const FactoryDataString & str, char * buf, size_t bufSize)
{
    VerifyOrReturnError(bufSize >= str.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(str.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(buf, str.data, str.len);
    buf[str.len] = 0;

    return CHIP_NO_ERROR;
}

} // namespace

namespace DeviceLayer {

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::Init()
{
    uint8_t * factoryData = nullptr;
    size_t factoryDataSize;

    CHIP_ERROR error = mFlashFactoryData.GetFactoryDataPartition(factoryData, factoryDataSize);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to read factory data partition");
        return error;
    }

    if (!ParseFactoryData(factoryData, static_cast<uint16_t>(factoryDataSize), &mFactoryData))
    {
        ChipLogError(DeviceLayer, "Failed to parse factory data");
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    // Check if factory data version is correct
    if (mFactoryData.version != CONFIG_CHIP_FACTORY_DATA_VERSION)
    {
        ChipLogError(DeviceLayer, "Factory data version mismatch. Flash version: %d vs code version: %d", mFactoryData.version,
                     CONFIG_CHIP_FACTORY_DATA_VERSION);
        return CHIP_ERROR_VERSION_MISMATCH;
    }

#ifdef CONFIG_CHIP_CRYPTO_PSA
    VerifyOrDie(MoveDACPrivateKeyToSecureStorage(factoryData, factoryDataSize) == CHIP_NO_ERROR);
#endif

    error = mFlashFactoryData.ProtectFactoryDataPartitionAgainstWrite();

    // Protection against write for external storage is not supported.
    if (error == CHIP_ERROR_NOT_IMPLEMENTED)
    {
        ChipLogProgress(DeviceLayer, "The device does not support hardware protection against write.");
        error = CHIP_NO_ERROR;
    }
    else if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to protect the factory data partition.");
        return error;
    }

    return error;
}

#ifdef CONFIG_CHIP_CRYPTO_PSA
template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::MoveDACPrivateKeyToSecureStorage(uint8_t * factoryDataPartition,
                                                                                   size_t factoryDataSize)
{

    if (!factoryDataPartition || factoryDataSize == 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (mFactoryData.dac_priv_key.len != kDACPrivateKeyLength)
    {
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    uint8_t clearedDACPrivKey[kDACPrivateKeyLength];
    memset(clearedDACPrivKey, 0x00, sizeof(clearedDACPrivKey));

// If key should be migrated to KMU save the KMU key slot to keyId.
#ifdef CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_KMU
    mDACPrivKeyId = static_cast<psa_key_id_t>(PSA_KEY_ID_FROM_CRACEN_KMU_SLOT(
#ifdef CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_KMU_ENCRYPTED
        CRACEN_KMU_KEY_USAGE_SCHEME_ENCRYPTED,
#else
        CRACEN_KMU_KEY_USAGE_SCHEME_RAW,
#endif // CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_KMU_ENCRYPTED
        CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_KMU_SLOT_ID));
#endif // CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_KMU

    // Check if factory data contains DAC private key
    if (memcmp(mFactoryData.dac_priv_key.data, clearedDACPrivKey, kDACPrivateKeyLength) != 0)
    {
        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
        // If there is the new DAC private key present in the factory data set and also there is
        // the existing one in ITS NVM storage, then skip saving it again.
        if (psa_get_key_attributes(mDACPrivKeyId, &attributes) != PSA_SUCCESS)
        {
            ChipLogProgress(DeviceLayer, "Found DAC Private Key in factory data set. Copying to secure storage...");

            // Remove the key if any exists and can be corrupted.
            psa_destroy_key(mDACPrivKeyId);

            psa_reset_key_attributes(&attributes);
            psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
            psa_set_key_bits(&attributes, kDACPrivateKeyLength * 8);
            psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_ANY_HASH));
            psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
            psa_set_key_id(&attributes, mDACPrivKeyId);
#ifdef CONFIG_CHIP_CRYPTO_PSA_MIGRATE_DAC_PRIV_KEY
#if defined(CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_ITS)
            psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_PERSISTENT);
#elif defined(CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_KMU)
            psa_set_key_lifetime(
                &attributes,
                PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_DEFAULT, PSA_KEY_LOCATION_CRACEN_KMU));
#endif // CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_ITS || CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_KMU
#else
            psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
#endif // CONFIG_CHIP_CRYPTO_PSA_MIGRATE_DAC_PRIV_KEY
            VerifyOrReturnError(psa_import_key(&attributes, reinterpret_cast<uint8_t *>(mFactoryData.dac_priv_key.data),
                                               mFactoryData.dac_priv_key.len, &mDACPrivKeyId) == PSA_SUCCESS,
                                CHIP_ERROR_INTERNAL);
        }

#ifdef CONFIG_CHIP_CRYPTO_PSA_MIGRATE_DAC_PRIV_KEY
#if defined(CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS) && defined(CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_ITS) &&                       \
    !defined(CONFIG_BUILD_WITH_TFM)
#error "Do not use both CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS and CONFIG_CHIP_CRYPTO_PSA_MIGRATE_DAC_PRIV_KEY kconfig options " \
       "while saving the DAC private key to ITS because you will permanently lose the DAC private key from the device."
#endif
        // Check once again if the saved key has attributes set before removing it from the factory data set.
        VerifyOrReturnError(psa_get_key_attributes(mDACPrivKeyId, &attributes) == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

        // Get the actual block size.
        const flash_parameters * flashParameters = flash_get_parameters(kFlashDev);
        VerifyOrReturnError(flashParameters, CHIP_ERROR_INTERNAL);

        // To write zeros directly to the Flash memory without erasing whole page the start address must be aligned to the
        // write_block_size value (alignedDacPrivKeyOffset), then we need align the required buffer size to the write_block_size as
        // well (requiredFlashSpaceSize) to meet Flash write requirements, and we need to calculate how many bytes of the factory
        // data set must be copied to the additional buffer space created after the alignments (bytesToLeftBefore, and
        // bytesToLeftAfter)
        size_t alignedDacPrivKeyOffset = ROUND_DOWN(mFactoryData.dacPrivateKeyOffset, flashParameters->write_block_size);
        size_t bytesToLeftBefore       = mFactoryData.dacPrivateKeyOffset % flashParameters->write_block_size;
        size_t requiredFlashSpaceSize  = ROUND_UP(kDACPrivateKeyLength + bytesToLeftBefore, flashParameters->write_block_size);
        size_t bytesToLeftAfter        = requiredFlashSpaceSize - bytesToLeftBefore - kDACPrivateKeyLength;

        // Allocate the memory buffer for removing DAC private key.
        chip::Platform::ScopedMemoryBuffer<uint8_t> removedPrivKeyBuffer;
        VerifyOrReturnError(removedPrivKeyBuffer.Calloc(requiredFlashSpaceSize), CHIP_ERROR_NO_MEMORY);

        // Copy the existing parts of the aligned memory space to before and after the DAC private key space.
        memcpy(removedPrivKeyBuffer.Get(), factoryDataPartition + alignedDacPrivKeyOffset, bytesToLeftBefore);
        memcpy(removedPrivKeyBuffer.Get() + bytesToLeftBefore + kDACPrivateKeyLength,
               factoryDataPartition + mFactoryData.dacPrivateKeyOffset + kDACPrivateKeyLength, bytesToLeftAfter);

        // Write aligned buffer directly to the Flash without erasing.
        VerifyOrReturnError(0 ==
                                flash_write(kFlashDev, kFactoryDataPartitionAddress + alignedDacPrivKeyOffset,
                                            removedPrivKeyBuffer.Get(), requiredFlashSpaceSize),
                            CHIP_ERROR_INTERNAL);

        // Parse the factory data again and verify if the procedure finished successfully
        VerifyOrReturnError(ParseFactoryData(factoryDataPartition, static_cast<uint16_t>(factoryDataSize), &mFactoryData),
                            CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

        // Verify if the factory data does not contain the DAC private key anymore.
        VerifyOrReturnError(memcmp(mFactoryData.dac_priv_key.data, clearedDACPrivKey, kDACPrivateKeyLength) == 0,
                            CHIP_ERROR_INTERNAL);
#endif // CONFIG_CHIP_CRYPTO_PSA_MIGRATE_DAC_PRIV_KEY
    }

    return CHIP_NO_ERROR;
}
#endif // CONFIG_CHIP_CRYPTO_PSA

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
#ifdef CONFIG_CHIP_CERTIFICATION_DECLARATION_STORAGE
    size_t cdLen = 0;

    if (Internal::ZephyrConfig::ReadConfigValueBin(Internal::ZephyrConfig::kConfigKey_CertificationDeclaration,
                                                   reinterpret_cast<uint8_t *>(outBuffer.data()), outBuffer.size(),
                                                   cdLen) == CHIP_NO_ERROR)
    {
        outBuffer.reduce_size(cdLen);
        return CHIP_NO_ERROR;
    }
#endif
    constexpr uint8_t kCdForAllExamples[] = CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION;

    return CopySpanToMutableSpan(ByteSpan{ kCdForAllExamples }, outBuffer);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    VerifyOrReturnError(outBuffer.size() >= mFactoryData.dac_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.dac_cert.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(outBuffer.data(), mFactoryData.dac_cert.data, mFactoryData.dac_cert.len);

    outBuffer.reduce_size(mFactoryData.dac_cert.len);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    VerifyOrReturnError(outBuffer.size() >= mFactoryData.pai_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.pai_cert.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(outBuffer.data(), mFactoryData.pai_cert.data, mFactoryData.pai_cert.len);

    outBuffer.reduce_size(mFactoryData.pai_cert.len);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::SignWithDeviceAttestationKey(const ByteSpan & messageToSign,
                                                                               MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.dac_cert.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

#ifdef CONFIG_CHIP_CRYPTO_PSA
    size_t outputLen = 0;

    psa_status_t err = psa_sign_message(mDACPrivKeyId, PSA_ALG_ECDSA(PSA_ALG_SHA_256), messageToSign.data(), messageToSign.size(),
                                        signature.Bytes(), signature.Capacity(), &outputLen);

    VerifyOrReturnError(!err, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(outputLen == chip::Crypto::kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(signature.SetLength(outputLen));
#else
    VerifyOrReturnError(mFactoryData.dac_priv_key.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    // Extract public key from DAC cert.
    ByteSpan dacCertSpan{ reinterpret_cast<uint8_t *>(mFactoryData.dac_cert.data), mFactoryData.dac_cert.len };
    chip::Crypto::P256PublicKey dacPublicKey;

    ReturnErrorOnFailure(chip::Crypto::ExtractPubkeyFromX509Cert(dacCertSpan, dacPublicKey));
    ReturnErrorOnFailure(keypair.HazardousOperationLoadKeypairFromRaw(
        ByteSpan(reinterpret_cast<uint8_t *>(mFactoryData.dac_priv_key.data), mFactoryData.dac_priv_key.len),
        ByteSpan(dacPublicKey.Bytes(), dacPublicKey.Length())));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));
#endif // CONFIG_CHIP_CRYPTO_PSA

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    VerifyOrReturnError(mFactoryData.discriminatorPresent, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    setupDiscriminator = mFactoryData.discriminator;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    VerifyOrReturnError(mFactoryData.spake2_it != 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    iterationCount = mFactoryData.spake2_it;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    VerifyOrReturnError(saltBuf.size() >= mFactoryData.spake2_salt.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.spake2_salt.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(saltBuf.data(), mFactoryData.spake2_salt.data, mFactoryData.spake2_salt.len);

    saltBuf.reduce_size(mFactoryData.spake2_salt.len);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    VerifyOrReturnError(verifierBuf.size() >= mFactoryData.spake2_verifier.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.spake2_verifier.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(verifierBuf.data(), mFactoryData.spake2_verifier.data, mFactoryData.spake2_verifier.len);

    verifierLen = mFactoryData.spake2_verifier.len;

    verifierBuf.reduce_size(verifierLen);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSetupPasscode(uint32_t & setupPasscode)
{
    VerifyOrReturnError(mFactoryData.passcode != 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    setupPasscode = mFactoryData.passcode;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetVendorName(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.vendor_name, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetVendorId(uint16_t & vendorId)
{
    VerifyOrReturnError(mFactoryData.vendorIdPresent, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    vendorId = mFactoryData.vendor_id;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductName(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.product_name, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductId(uint16_t & productId)
{
    VerifyOrReturnError(mFactoryData.productIdPresent, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    productId = mFactoryData.product_id;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetPartNumber(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.part_number, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductURL(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.product_url, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductLabel(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.product_label, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSerialNumber(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.sn, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    VerifyOrReturnError(mFactoryData.date_year != 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    year  = mFactoryData.date_year;
    month = mFactoryData.date_month;
    day   = mFactoryData.date_day;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetHardwareVersion(uint16_t & hardwareVersion)
{
    VerifyOrReturnError(mFactoryData.hwVerPresent, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    hardwareVersion = mFactoryData.hw_ver;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetHardwareVersionString(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.hw_ver_str, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    VerifyOrReturnError(uniqueIdSpan.size() >= mFactoryData.rd_uid.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.rd_uid.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(uniqueIdSpan.data(), mFactoryData.rd_uid.data, mFactoryData.rd_uid.len);

    uniqueIdSpan.reduce_size(mFactoryData.rd_uid.len);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetEnableKey(MutableByteSpan & enableKey)
{
    VerifyOrReturnError(mFactoryData.enable_key.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    VerifyOrReturnError(enableKey.size() >= mFactoryData.enable_key.len, CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(enableKey.data(), mFactoryData.enable_key.data, mFactoryData.enable_key.len);

    enableKey.reduce_size(mFactoryData.enable_key.len);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductFinish(app::Clusters::BasicInformation::ProductFinishEnum * finish)
{
    VerifyOrReturnError(finish, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mFactoryData.productFinishPresent, CHIP_ERROR_NOT_IMPLEMENTED);
    *finish = static_cast<app::Clusters::BasicInformation::ProductFinishEnum>(mFactoryData.product_finish);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor)
{
    VerifyOrReturnError(primaryColor, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mFactoryData.primaryColorPresent, CHIP_ERROR_NOT_IMPLEMENTED);

    *primaryColor = static_cast<app::Clusters::BasicInformation::ColorEnum>(mFactoryData.primary_color);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetUserData(MutableByteSpan & userData)
{
    VerifyOrReturnError(mFactoryData.user.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    VerifyOrReturnError(userData.size() >= mFactoryData.user.len, CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(userData.data(), mFactoryData.user.data, mFactoryData.user.len);

    userData.reduce_size(mFactoryData.user.len);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetUserKey(const char * userKey, void * buf, size_t & len)
{
    VerifyOrReturnError(mFactoryData.user.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    VerifyOrReturnError(buf, CHIP_ERROR_BUFFER_TOO_SMALL);

    bool success = FindUserDataEntry(&mFactoryData, userKey, buf, len, &len);

    VerifyOrReturnError(success, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    return CHIP_NO_ERROR;
}

// Fully instantiate the template class in whatever compilation unit includes this file.
template class FactoryDataProvider<InternalFlashFactoryData>;
#if defined(USE_PARTITION_MANAGER) && USE_PARTITION_MANAGER == 1 && (defined(CONFIG_SPI_NOR) || defined(CONFIG_NORDIC_QSPI_NOR))
template class FactoryDataProvider<ExternalFlashFactoryData>;
#endif // if defined(USE_PARTITION_MANAGER) && USE_PARTITION_MANAGER == 1 (defined(CONFIG_SPI_NOR) ||
       // defined(CONFIG_NORDIC_QSPI_NOR))

} // namespace DeviceLayer
} // namespace chip
