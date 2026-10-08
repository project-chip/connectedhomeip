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
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Base64.h>
#include <platform/ESP32/ESP32Config.h>
#include <platform/ESP32/ESP32FactoryDataProvider.h>
#include <platform/ESP32/ScopedNvsHandle.h>

namespace chip {
namespace DeviceLayer {

using namespace chip::Credentials;
using namespace chip::DeviceLayer::Internal;

namespace {
static constexpr uint32_t kDACPrivateKeySize = 32;
static constexpr uint32_t kDACPublicKeySize  = 65;

/**
 * @brief Get the manufacturing date or suffix from the ESP32 config.
 *
 * @Note: The manufacturing date is stored in the format YYYY-MM-DD<vendor info> or YYYYMMDD<vendor info>.
 * To retrieve only the manufacturing date, pass nullptr for vendorSuffixSpan.
 * To retrieve only the vendor suffix, pass a non-empty MutableCharSpan for vendorSuffixSpan and nullptr for year, month, and day.
 * @param year The year to store the manufacturing date.
 * @param month The month to store the manufacturing date.
 * @param day The day to store the manufacturing date.
 * @param vendorSuffixSpan The vendor suffix to store the manufacturing date. @Note: It not guaranteed to be null terminated and
 * should be treated as a raw string.
 * @return CHIP_ERROR indicating the success or failure of the operation.
 */
inline CHIP_ERROR GetManufacturingDateOrSuffix(uint16_t * year, uint8_t * month, uint8_t * day, MutableCharSpan * vendorSuffixSpan)
{
    constexpr size_t kMaxManufacturingDateLength  = 18; // (10 + 8) for YYYY-MM-DD<vendor info> or (8 + 8) for YYYYMMDD<vendor info>
    constexpr size_t kMaxDateLength               = 8;  // YYYYMMDD
    char dateStr[kMaxManufacturingDateLength + 1] = {};
    size_t readDateLen                            = kMaxManufacturingDateLength;
    ReturnErrorOnFailure(
        ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_ManufacturingDate, dateStr, sizeof(dateStr), readDateLen));
    VerifyOrReturnError(readDateLen >= kMaxDateLength && readDateLen <= kMaxManufacturingDateLength, CHIP_ERROR_INTERNAL);
    size_t index = 4;
    if (dateStr[index] == '-')
        index++;
    size_t monthIdx = index;
    index += 2;
    if (dateStr[index] == '-')
        index++;
    size_t dayIdx = index;
    index += 2;

    if (year && month && day)
    {
        char buf[5];
        memcpy(buf, dateStr, 4);
        buf[4] = '\0';
        *year  = static_cast<uint16_t>(strtoul(buf, nullptr, 10));
        memcpy(buf, dateStr + monthIdx, 2);
        buf[2] = '\0';
        *month = static_cast<uint8_t>(strtoul(buf, nullptr, 10));
        memcpy(buf, dateStr + dayIdx, 2);
        buf[2] = '\0';
        *day   = static_cast<uint8_t>(strtoul(buf, nullptr, 10));
    }

    if (vendorSuffixSpan)
    {
        size_t suffixLen = readDateLen - index;
        if (suffixLen > 0)
        {
            VerifyOrReturnError(suffixLen <= vendorSuffixSpan->size(), CHIP_ERROR_BUFFER_TOO_SMALL);
            vendorSuffixSpan->reduce_size(suffixLen);
            memcpy(vendorSuffixSpan->data(), dateStr + index, suffixLen);
        }
        else
        {
            vendorSuffixSpan->reduce_size(0);
        }
    }
    return CHIP_NO_ERROR;
}

} // namespace

CHIP_ERROR ESP32FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    uint32_t setupDiscriminator32;
    ReturnErrorOnFailure(ESP32Config::ReadConfigValue(ESP32Config::kConfigKey_SetupDiscriminator, setupDiscriminator32));
    VerifyOrReturnLogError(setupDiscriminator32 <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    setupDiscriminator = static_cast<uint16_t>(setupDiscriminator32);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    return ESP32Config::ReadConfigValue(ESP32Config::kConfigKey_Spake2pIterationCount, iterationCount);
}

CHIP_ERROR ESP32FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;

    CHIP_ERROR err                          = CHIP_NO_ERROR;
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    size_t saltB64Len                       = 0;

    err = ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_Spake2pSalt, saltB64, sizeof(saltB64), saltB64Len);
    ReturnErrorOnFailure(err);

    size_t saltLen = chip::Base64Decode32(saltB64, saltB64Len, reinterpret_cast<uint8_t *>(saltB64));
    VerifyOrReturnError(saltLen <= saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(saltBuf.data(), saltB64, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;

    CHIP_ERROR err                                            = CHIP_NO_ERROR;
    char verifierB64[kSpake2pSerializedVerifier_MaxBase64Len] = { 0 };
    size_t verifierB64Len                                     = 0;

    err =
        ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_Spake2pVerifier, verifierB64, sizeof(verifierB64), verifierB64Len);
    ReturnErrorOnFailure(err);

    verifierLen = chip::Base64Decode32(verifierB64, verifierB64Len, reinterpret_cast<uint8_t *>(verifierB64));
    VerifyOrReturnError(verifierLen <= verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
#ifdef CONFIG_ENABLE_SET_CERT_DECLARATION_API
    return CopySpanToMutableSpan(mCD, outBuffer);
#else
    size_t certSize;
    ReturnErrorOnFailure(
        ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_CertDeclaration, outBuffer.data(), outBuffer.size(), certSize));
    outBuffer.reduce_size(certSize);
    return CHIP_NO_ERROR;
#endif // CONFIG_ENABLE_SET_CERT_DECLARATION_API
}

CHIP_ERROR ESP32FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    // We do not provide any FirmwareInformation.
    out_firmware_info_buffer.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    size_t certSize;
    ReturnErrorOnFailure(
        ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_DACCert, outBuffer.data(), outBuffer.size(), certSize));
    outBuffer.reduce_size(certSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    size_t certSize;
    ReturnErrorOnFailure(
        ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_PAICert, outBuffer.data(), outBuffer.size(), certSize));
    outBuffer.reduce_size(certSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(!outSignBuffer.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!messageToSign.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t privKeyBuf[kDACPrivateKeySize];
    uint8_t pubKeyBuf[kDACPublicKeySize];
    size_t privKeyLen = sizeof(privKeyBuf);
    size_t pubKeyLen  = sizeof(pubKeyBuf);

    ReturnErrorOnFailure(
        ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_DACPrivateKey, privKeyBuf, privKeyLen, privKeyLen));
    ReturnErrorOnFailure(ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_DACPublicKey, pubKeyBuf, pubKeyLen, pubKeyLen));

    ReturnErrorOnFailure(
        keypair.HazardousOperationLoadKeypairFromRaw(ByteSpan(privKeyBuf, privKeyLen), ByteSpan(pubKeyBuf, pubKeyLen)));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

#if CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER
CHIP_ERROR ESP32FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    size_t vendorNameLen = 0; // without counting null-terminator
    return ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_VendorName, buf, bufSize, vendorNameLen);
}

CHIP_ERROR ESP32FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    ChipError err   = CHIP_NO_ERROR;
    uint32_t valInt = 0;

    err = ESP32Config::ReadConfigValue(ESP32Config::kConfigKey_VendorId, valInt);
    ReturnErrorOnFailure(err);
    vendorId = static_cast<uint16_t>(valInt);
    return err;
}

CHIP_ERROR ESP32FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    size_t productNameLen = 0; // without counting null-terminator
    return ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_ProductName, buf, bufSize, productNameLen);
}

CHIP_ERROR ESP32FactoryDataProvider::GetProductId(uint16_t & productId)
{
    ChipError err   = CHIP_NO_ERROR;
    uint32_t valInt = 0;

    err = ESP32Config::ReadConfigValue(ESP32Config::kConfigKey_ProductId, valInt);
    ReturnErrorOnFailure(err);
    productId = static_cast<uint16_t>(valInt);
    return err;
}

CHIP_ERROR ESP32FactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
    CHIP_ERROR err = ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_ProductURL, buf, bufSize, bufSize);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ESP32FactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
    CHIP_ERROR err = ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_ProductLabel, buf, bufSize, bufSize);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ESP32FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    size_t hardwareVersionStringLen = 0; // without counting null-terminator
    return ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_HardwareVersionString, buf, bufSize, hardwareVersionStringLen);
}

CHIP_ERROR ESP32FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    ChipError err = CHIP_ERROR_WRONG_KEY_TYPE;
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >= ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                  "Length of unique ID for rotating device ID is smaller than minimum.");

    size_t uniqueIdLen = 0;
    err = ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_RotatingDevIdUniqueId, uniqueIdSpan.data(), uniqueIdSpan.size(),
                                          uniqueIdLen);
    ReturnErrorOnFailure(err);
    uniqueIdSpan.reduce_size(uniqueIdLen);
#endif
    return err;
}

CHIP_ERROR ESP32FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    return GenericDeviceInstanceInfoProvider<ESP32Config>::GetSerialNumber(buf, bufSize);
}

CHIP_ERROR ESP32FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    return GetManufacturingDateOrSuffix(&year, &month, &day, nullptr);
}

CHIP_ERROR ESP32FactoryDataProvider::GetManufacturingDateSuffix(MutableCharSpan & vendorInfoSpan)
{
    return GetManufacturingDateOrSuffix(nullptr, nullptr, nullptr, &vendorInfoSpan);
}

CHIP_ERROR ESP32FactoryDataProvider::GetProductFinish(app::Clusters::BasicInformation::ProductFinishEnum * finish)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    uint32_t productFinish = 0;

    err = ESP32Config::ReadConfigValue(ESP32Config::kConfigKey_ProductFinish, productFinish);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_NOT_IMPLEMENTED);

    *finish = static_cast<app::Clusters::BasicInformation::ProductFinishEnum>(productFinish);

    return err;
}

CHIP_ERROR ESP32FactoryDataProvider::GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t color = 0;

    err = ESP32Config::ReadConfigValue(ESP32Config::kConfigKey_ProductColor, color);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_NOT_IMPLEMENTED);

    *primaryColor = static_cast<app::Clusters::BasicInformation::ColorEnum>(color);

    return err;
}

CHIP_ERROR ESP32FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    return GenericDeviceInstanceInfoProvider<ESP32Config>::GetHardwareVersion(hardwareVersion);
}

CHIP_ERROR ESP32FactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
    CHIP_ERROR err = ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_PartNumber, buf, bufSize, bufSize);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER

} // namespace DeviceLayer
} // namespace chip
