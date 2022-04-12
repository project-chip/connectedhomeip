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
#include <platform/ESP32/ESP32CommissionableDataProvider.h>
#include <platform/ESP32/ESP32Config.h>
#include <platform/ESP32/ScopedNvsHandle.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

CHIP_ERROR ESP32CommissionableDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    uint32_t setupDiscriminator32;
    ReturnErrorOnFailure(ESP32Config::ReadConfigValue(ESP32Config::kConfigKey_SetupDiscriminator, setupDiscriminator32));
    VerifyOrReturnLogError(setupDiscriminator32 <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    setupDiscriminator = static_cast<uint16_t>(setupDiscriminator32);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32CommissionableDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    return ESP32Config::ReadConfigValue(ESP32Config::kConfigKey_Spake2pIterationCount, iterationCount);
}

CHIP_ERROR ESP32CommissionableDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;

    CHIP_ERROR err                          = CHIP_NO_ERROR;
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    size_t saltB64Len                       = 0;

    err = ESP32Config::ReadConfigValueStr(ESP32Config::kConfigKey_Spake2pSalt, saltB64, sizeof(saltB64), saltB64Len);
    ReturnErrorOnFailure(err);

    size_t saltLen = chip::Base64Decode32(saltB64, saltB64Len, reinterpret_cast<uint8_t *>(saltB64));
    ReturnErrorCodeIf(saltLen > saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(saltBuf.data(), saltB64, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32CommissionableDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
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
    ReturnErrorCodeIf(verifierLen > verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
