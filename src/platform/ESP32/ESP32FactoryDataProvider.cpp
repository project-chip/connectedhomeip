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

// Helper function to remove dashes from a string in-place
// Returns the new length after removing dashes
inline size_t RemoveDashesInPlace(char * str, size_t length)
{
    size_t writePos = 0;
    for (size_t readPos = 0; readPos < length; readPos++)
    {
        if (str[readPos] != '-')
        {
            str[writePos++] = str[readPos];
        }
    }
    str[writePos] = '\0';
    return writePos;
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
    CHIP_ERROR err                               = CHIP_NO_ERROR;
    constexpr size_t kMaxManufacturingDateLength = 16; // YYYY-MM-DD<vendor info> or YYYYMMDD<vendor info>
    constexpr size_t kMaxDateLength              = 8;  // YYYYMMDD
    char dateStr[kMaxManufacturingDateLength + 1];
    size_t dateLen;
    err = ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_ManufacturingDate, dateStr, sizeof(dateStr), dateLen);
    SuccessOrExit(err);
    VerifyOrExit(dateLen <= kMaxManufacturingDateLength, err = CHIP_ERROR_INVALID_ARGUMENT);

    dateLen = RemoveDashesInPlace(dateStr, dateLen);

    VerifyOrExit(dateLen >= kMaxDateLength && dateLen <= kMaxManufacturingDateLength, err = CHIP_ERROR_INVALID_ARGUMENT);

    char yearStr[5]  = { dateStr[0], dateStr[1], dateStr[2], dateStr[3], '\0' };
    char monthStr[3] = { dateStr[4], dateStr[5], '\0' };
    char dayStr[3]   = { dateStr[6], dateStr[7], '\0' };

    year  = static_cast<uint16_t>(atoi(yearStr));
    month = static_cast<uint8_t>(atoi(monthStr));
    day   = static_cast<uint8_t>(atoi(dayStr));

    VerifyOrExit(year >= 1000 && year <= 9999, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(month >= 1 && month <= 12, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(day >= 1 && day <= 31, err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ChipLogError(DeviceLayer, "Invalid manufacturing date: %s", dateStr);
    }
    return err;
}

CHIP_ERROR ESP32FactoryDataProvider::GetManufacturingDateSuffix(MutableCharSpan & vendorInfoSpan)
{
    VerifyOrReturnError(!vendorInfoSpan.empty(), CHIP_ERROR_BUFFER_TOO_SMALL);
    CHIP_ERROR err                               = CHIP_NO_ERROR;
    constexpr size_t kMaxManufacturingDateLength = 16; // YYYY-MM-DD<vendor info> or YYYYMMDD<vendor info>
    constexpr size_t kMaxDateLength              = 8;  // YYYYMMDD
    constexpr size_t kMaxVendorInfoLength        = kMaxManufacturingDateLength - kMaxDateLength;
    char dateStr[kMaxManufacturingDateLength + 1];
    size_t dateLen;
    size_t vendorInfoLen;
    err = ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_ManufacturingDate, dateStr, sizeof(dateStr), dateLen);
    SuccessOrExit(err);
    VerifyOrExit(dateLen <= kMaxManufacturingDateLength, err = CHIP_ERROR_INVALID_ARGUMENT);

    dateLen = RemoveDashesInPlace(dateStr, dateLen);

    VerifyOrExit(dateLen >= kMaxDateLength && dateLen <= kMaxManufacturingDateLength, err = CHIP_ERROR_INVALID_ARGUMENT);
    vendorInfoLen = dateLen - kMaxDateLength;
    VerifyOrExit(vendorInfoLen <= kMaxVendorInfoLength, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(vendorInfoSpan.size() >= vendorInfoLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(vendorInfoSpan.data(), dateStr + kMaxDateLength, vendorInfoLen);
    vendorInfoSpan.reduce_size(vendorInfoLen);

exit:
    if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ChipLogError(DeviceLayer, "Invalid manufacturing date: %s", dateStr);
    }
    return err;
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
