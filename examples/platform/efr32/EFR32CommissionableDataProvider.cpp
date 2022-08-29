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

#include <lib/support/Base64.h>

namespace chip {
namespace DeviceLayer {

using namespace chip::DeviceLayer::Internal;

CHIP_ERROR EFR32DeviceDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    uint32_t setupDiscriminator32;
    ReturnErrorOnFailure(EFR32Config::ReadConfigValue(EFR32Config::kConfigKey_SetupDiscriminator, setupDiscriminator32));
    VerifyOrReturnLogError(setupDiscriminator32 <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    setupDiscriminator = static_cast<uint16_t>(setupDiscriminator32);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EFR32DeviceDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    return EFR32Config::ReadConfigValue(EFR32Config::kConfigKey_Spake2pIterationCount, iterationCount);
}

CHIP_ERROR EFR32DeviceDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;

    CHIP_ERROR err                          = CHIP_NO_ERROR;
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    size_t saltB64Len                       = 0;

    err = EFR32Config::ReadConfigValueStr(EFR32Config::kConfigKey_Spake2pSalt, saltB64, sizeof(saltB64), saltB64Len);
    ReturnErrorOnFailure(err);

    size_t saltLen = chip::Base64Decode32(saltB64, saltB64Len, reinterpret_cast<uint8_t *>(saltB64));
    ReturnErrorCodeIf(saltLen > saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(saltBuf.data(), saltB64, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR EFR32DeviceDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;

    CHIP_ERROR err                                            = CHIP_NO_ERROR;
    char verifierB64[kSpake2pSerializedVerifier_MaxBase64Len] = { 0 };
    size_t verifierB64Len                                     = 0;

    err =
        EFR32Config::ReadConfigValueStr(EFR32Config::kConfigKey_Spake2pVerifier, verifierB64, sizeof(verifierB64), verifierB64Len);
    ReturnErrorOnFailure(err);

    verifierLen = chip::Base64Decode32(verifierB64, verifierB64Len, reinterpret_cast<uint8_t *>(verifierB64));
    ReturnErrorCodeIf(verifierLen > verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return CHIP_NO_ERROR;
}

#if ENABLE_DEVICE_DATA_PROVIDER
CHIP_ERROR EFR32DeviceDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    size_t vendorNameLen = 0; // without counting null-terminator
    return EFR32Config::ReadConfigValueStr(EFR32Config::kConfigKey_VendorName, buf, bufSize, vendorNameLen);
}

CHIP_ERROR EFR32DeviceDataProvider::GetVendorId(uint16_t & vendorId)
{
    ChipError err   = CHIP_NO_ERROR;
    uint32_t valInt = 0;

    err = EFR32Config::ReadConfigValue(EFR32Config::kConfigKey_VendorId, valInt);
    ReturnErrorOnFailure(err);
    vendorId = static_cast<uint16_t>(valInt);
    return err;
}

CHIP_ERROR EFR32DeviceDataProvider::GetProductName(char * buf, size_t bufSize)
{
    size_t productNameLen = 0; // without counting null-terminator
    return EFR32Config::ReadConfigValueStr(EFR32Config::kConfigKey_ProductName, buf, bufSize, productNameLen);
}

CHIP_ERROR EFR32DeviceDataProvider::GetProductId(uint16_t & productId)
{
    ChipError err   = CHIP_NO_ERROR;
    uint32_t valInt = 0;

    err = EFR32Config::ReadConfigValue(EFR32Config::kConfigKey_ProductId, valInt);
    ReturnErrorOnFailure(err);
    productId = static_cast<uint16_t>(valInt);
    return err;
}

CHIP_ERROR EFR32DeviceDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    size_t hardwareVersionStringLen = 0; // without counting null-terminator
    return EFR32Config::ReadConfigValueStr(EFR32Config::kConfigKey_HardwareVersionString, buf, bufSize, hardwareVersionStringLen);
}

CHIP_ERROR EFR32DeviceDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    ChipError err = CHIP_ERROR_WRONG_KEY_TYPE;
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >= ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                  "Length of unique ID for rotating device ID is smaller than minimum.");

    size_t uniqueIdLen = 0;
    err = EFR32Config::ReadConfigValueBin(EFR32Config::kConfigKey_UniqueId, uniqueIdSpan.data(), uniqueIdSpan.size(), uniqueIdLen);
    ReturnErrorOnFailure(err);
    uniqueIdSpan.reduce_size(uniqueIdLen);
#endif
    return err;
}
#endif // ENABLE_DEVICE_DATA_PROVIDER

} // namespace DeviceLayer
} // namespace chip
