/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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
#include <headers/AttestationKey.h>
#include <headers/ProvisionStorage.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceError.h>
#include <platform/Zephyr/CHIPDevicePlatformConfig.h>
#include <platform/Zephyr/ZephyrConfig.h>
#include <psa/crypto.h>
#include <zephyr/settings/settings.h>

#include <cinttypes>
#include <cstdlib>
#include <cstring>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

namespace {

using chip::DeviceLayer::Internal::ZephyrConfig;

#define CONFIG_KEY(key)                                                                                                            \
    (key);                                                                                                                         \
    static_assert(sizeof(key) <= SETTINGS_MAX_NAME_LEN, "Config key too long: " key)

#define NAMESPACE_SL_FACTORY CHIP_DEVICE_CONFIG_SETTINGS_KEY "-sl-fct/"

const ZephyrConfig::Key kConfigKey_VendorId              = CONFIG_KEY(NAMESPACE_SL_FACTORY "vendor-id");
const ZephyrConfig::Key kConfigKey_ProductId             = CONFIG_KEY(NAMESPACE_SL_FACTORY "product-id");
const ZephyrConfig::Key kConfigKey_VendorName            = CONFIG_KEY(NAMESPACE_SL_FACTORY "vendor-name");
const ZephyrConfig::Key kConfigKey_ProductName           = CONFIG_KEY(NAMESPACE_SL_FACTORY "product-name");
const ZephyrConfig::Key kConfigKey_ProductLabel          = CONFIG_KEY(NAMESPACE_SL_FACTORY "product-label");
const ZephyrConfig::Key kConfigKey_ProductUrl            = CONFIG_KEY(NAMESPACE_SL_FACTORY "product-url");
const ZephyrConfig::Key kConfigKey_PartNumber            = CONFIG_KEY(NAMESPACE_SL_FACTORY "part-number");
const ZephyrConfig::Key kConfigKey_HardwareVersionString = CONFIG_KEY(NAMESPACE_SL_FACTORY "hardware-ver-str");
const ZephyrConfig::Key kConfigKey_SetupPayload          = CONFIG_KEY(NAMESPACE_SL_FACTORY "setup-payload");

constexpr psa_key_id_t kDacPsaKeyId = 2;

static constexpr ZephyrConfig::Key kProvisionRequestKey = CONFIG_KEY(NAMESPACE_SL_FACTORY "provision-req");
static constexpr ZephyrConfig::Key kProvisionVersionKey = CONFIG_KEY(NAMESPACE_SL_FACTORY "provision-ver");

constexpr size_t kDateStringLength = 8; // YYYYMMDD

#if SL_PROVISION_GENERATOR == 0
[[maybe_unused]]constexpr size_t kSpake2pSaltB64LengthMax     = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length);
[[maybe_unused]]constexpr size_t kSpake2pVerifierB64LengthMax = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length);
#endif // SL_PROVISION_GENERATOR == 0

CHIP_ERROR ReadConfigBin(ZephyrConfig::Key key, MutableByteSpan & buffer)
{
    size_t dataLen = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValueBin(key, buffer.data(), buffer.size(), dataLen));
    buffer.reduce_size(dataLen);
    return CHIP_NO_ERROR;
}

bool DacPsaKeyExists()
{
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    const psa_status_t status       = psa_get_key_attributes(kDacPsaKeyId, &attributes);
    psa_reset_key_attributes(&attributes);
    return status == PSA_SUCCESS;
}

} // namespace

CHIP_ERROR Storage::Initialize(uint32_t flash_addr, uint32_t flash_size)
{
    // Zephyr settings owns its own flash placement.
    (void) flash_addr;
    (void) flash_size;
    ReturnErrorOnFailure(ZephyrConfig::Init());
#if SL_PROVISION_GENERATOR == 0
    VerifyOrDo(DacPsaKeyExists(), ChipLogError(DeviceLayer, "DAC PSA key id %u missing", kDacPsaKeyId));
#endif // SL_PROVISION_GENERATOR == 0
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::Commit()
{
    // Zephyr settings writes are immediate
    return CHIP_NO_ERROR;
}

//
// DeviceInstanceInfoProvider
//

CHIP_ERROR Storage::SetSerialNumber(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrConfig::kConfigKey_SerialNum, value, len);
}

CHIP_ERROR Storage::GetSerialNumber(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;
    return ZephyrConfig::ReadConfigValueStr(ZephyrConfig::kConfigKey_SerialNum, value, max, size);
}

CHIP_ERROR Storage::SetVendorId(uint16_t value)
{
    return ZephyrConfig::WriteConfigValue(kConfigKey_VendorId, static_cast<uint32_t>(value));
}

CHIP_ERROR Storage::GetVendorId(uint16_t & value)
{
    uint32_t stored = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValue(kConfigKey_VendorId, stored));
    value = static_cast<uint16_t>(stored);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetVendorName(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(kConfigKey_VendorName, value, len);
}

CHIP_ERROR Storage::GetVendorName(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t name_len = 0;

    return ZephyrConfig::ReadConfigValueStr(kConfigKey_VendorName, value, max, name_len);
}

CHIP_ERROR Storage::SetProductId(uint16_t value)
{
    return ZephyrConfig::WriteConfigValue(kConfigKey_ProductId, static_cast<uint32_t>(value));
}

CHIP_ERROR Storage::GetProductId(uint16_t & value)
{
    uint32_t stored = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValue(kConfigKey_ProductId, stored));
    value = static_cast<uint16_t>(stored);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetProductName(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(kConfigKey_ProductName, value, len);
}

CHIP_ERROR Storage::GetProductName(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;

    return ZephyrConfig::ReadConfigValueStr(kConfigKey_ProductName, value, max, size);
}

CHIP_ERROR Storage::SetProductLabel(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(kConfigKey_ProductLabel, value, len);
}

CHIP_ERROR Storage::GetProductLabel(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;
    return ZephyrConfig::ReadConfigValueStr(kConfigKey_ProductLabel, value, max, size);
}

CHIP_ERROR Storage::SetProductURL(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(kConfigKey_ProductUrl, value, len);
}

CHIP_ERROR Storage::GetProductURL(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;
    return ZephyrConfig::ReadConfigValueStr(kConfigKey_ProductUrl, value, max, size);
}

CHIP_ERROR Storage::SetPartNumber(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(kConfigKey_PartNumber, value, len);
}

CHIP_ERROR Storage::GetPartNumber(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;
    return ZephyrConfig::ReadConfigValueStr(kConfigKey_PartNumber, value, max, size);
}

CHIP_ERROR Storage::SetHardwareVersion(uint16_t value)
{
    return ZephyrConfig::WriteConfigValue(ZephyrConfig::kConfigKey_HardwareVersion, static_cast<uint32_t>(value));
}

CHIP_ERROR Storage::GetHardwareVersion(uint16_t & value)
{
    uint32_t stored = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValue(ZephyrConfig::kConfigKey_HardwareVersion, stored));
    value = static_cast<uint16_t>(stored);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetHardwareVersionString(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(kConfigKey_HardwareVersionString, value, len);
}

CHIP_ERROR Storage::GetHardwareVersionString(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;
    return ZephyrConfig::ReadConfigValueStr(kConfigKey_HardwareVersionString, value, max, size);
}

CHIP_ERROR Storage::SetManufacturingDate(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrConfig::kConfigKey_ManufacturingDate, value, len);
}

CHIP_ERROR Storage::GetManufacturingDate(uint8_t * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueStr(ZephyrConfig::kConfigKey_ManufacturingDate, reinterpret_cast<char *>(value), max, size);
}

CHIP_ERROR Storage::SetPersistentUniqueId(const uint8_t * value, size_t size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueBin(ZephyrConfig::kConfigKey_UniqueId, value, size);
}

CHIP_ERROR Storage::GetPersistentUniqueId(uint8_t * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueBin(ZephyrConfig::kConfigKey_UniqueId, value, max, size);
}

//
// CommissionableDataProvider
//

CHIP_ERROR Storage::SetSetupDiscriminator(uint16_t value)
{
    return ZephyrConfig::WriteConfigValue(ZephyrConfig::kConfigKey_SetupDiscriminator, static_cast<uint32_t>(value));
}

CHIP_ERROR Storage::GetSetupDiscriminator(uint16_t & value)
{
    uint32_t stored = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValue(ZephyrConfig::kConfigKey_SetupDiscriminator, stored));
    value = static_cast<uint16_t>(stored);
    VerifyOrReturnLogError(value <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetSpake2pIterationCount(uint32_t value)
{
    return ZephyrConfig::WriteConfigValue(ZephyrConfig::kConfigKey_Spake2pIterationCount, value);
}

CHIP_ERROR Storage::GetSpake2pIterationCount(uint32_t & value)
{
    return ZephyrConfig::ReadConfigValue(ZephyrConfig::kConfigKey_Spake2pIterationCount, value);
}

CHIP_ERROR Storage::SetSetupPasscode(uint32_t value)
{
    (void) value;
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Storage::GetSetupPasscode(uint32_t & value)
{
    (void) value;
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Storage::SetSpake2pSalt(const char * value, size_t size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrConfig::kConfigKey_Spake2pSalt, value, size);
}

CHIP_ERROR Storage::GetSpake2pSalt(char * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueStr(ZephyrConfig::kConfigKey_Spake2pSalt, value, max, size);
}

CHIP_ERROR Storage::SetSpake2pVerifier(const char * value, size_t size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrConfig::kConfigKey_Spake2pVerifier, value, size);
}

CHIP_ERROR Storage::GetSpake2pVerifier(char * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueStr(ZephyrConfig::kConfigKey_Spake2pVerifier, value, max, size);
}

//
// DeviceAttestationCredentialsProvider
//

CHIP_ERROR Storage::SetFirmwareInformation(const ByteSpan & value)
{
    (void) value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetFirmwareInformation(MutableByteSpan & value)
{
    value.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetCertificationDeclaration(const ByteSpan & value)
{
    return ZephyrConfig::WriteConfigValueBin(ZephyrConfig::kConfigKey_CertificationDeclaration, value.data(), value.size());
}

CHIP_ERROR Storage::GetCertificationDeclaration(MutableByteSpan & value)
{
    return ReadConfigBin(ZephyrConfig::kConfigKey_CertificationDeclaration, value);
}

CHIP_ERROR Storage::SetProductAttestationIntermediateCert(const ByteSpan & value)
{
    return ZephyrConfig::WriteConfigValueBin(ZephyrConfig::kConfigKey_MfrDeviceICACerts, value.data(), value.size());
}

CHIP_ERROR Storage::GetProductAttestationIntermediateCert(MutableByteSpan & value)
{
    return ReadConfigBin(ZephyrConfig::kConfigKey_MfrDeviceICACerts, value);
}

CHIP_ERROR Storage::SetDeviceAttestationCert(const ByteSpan & value)
{
    return ZephyrConfig::WriteConfigValueBin(ZephyrConfig::kConfigKey_MfrDeviceCert, value.data(), value.size());
}

CHIP_ERROR Storage::GetDeviceAttestationCert(MutableByteSpan & value)
{
    return ReadConfigBin(ZephyrConfig::kConfigKey_MfrDeviceCert, value);
}

CHIP_ERROR Storage::SetDeviceAttestationKey(const ByteSpan & value)
{
    AttestationKey key;
    ReturnErrorOnFailure(key.Import(value.data(), value.size()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetDeviceAttestationCSR(uint16_t vid, uint16_t pid, const CharSpan & cn, MutableCharSpan & csr)
{
    AttestationKey key;
    ReturnErrorOnFailure(key.GenerateCSR(vid, pid, cn, csr));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    VerifyOrReturnError(DacPsaKeyExists(), CHIP_ERROR_NOT_FOUND,
                        ChipLogError(DeviceLayer, "DAC PSA key id %u missing", kDacPsaKeyId));

    Crypto::P256ECDSASignature rawSignature;
    VerifyOrReturnError(signature.size() >= rawSignature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t outputLen          = 0;
    const psa_status_t status = psa_sign_message(kDacPsaKeyId, PSA_ALG_ECDSA(PSA_ALG_SHA_256), message.data(), message.size(),
                                                 rawSignature.Bytes(), rawSignature.Capacity(), &outputLen);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "psa_sign_message failed: %" PRId32, status));
    VerifyOrReturnError(outputLen == Crypto::kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(rawSignature.SetLength(outputLen));
    return CopySpanToMutableSpan(ByteSpan{ rawSignature.ConstBytes(), rawSignature.Length() }, signature);
}

CHIP_ERROR Storage::SetCredentialsBaseAddress(uint32_t addr)
{
    (void) addr;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetCredentialsBaseAddress(uint32_t & addr)
{
    addr = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetProvisionVersion(const char * value, size_t size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(kProvisionVersionKey, value, size);
}

CHIP_ERROR Storage::GetProvisionVersion(char * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueStr(kProvisionVersionKey, value, max, size);
}

CHIP_ERROR Storage::SetSetupPayload(const uint8_t * value, size_t size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueBin(kConfigKey_SetupPayload, value, size);
}

CHIP_ERROR Storage::GetSetupPayload(uint8_t * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueBin(kConfigKey_SetupPayload, value, max, size);
}

CHIP_ERROR Storage::SetProvisionRequest(bool value)
{
    return ZephyrConfig::WriteConfigValue(kProvisionRequestKey, static_cast<uint32_t>(value ? 1 : 0));
}

CHIP_ERROR Storage::GetProvisionRequest(bool & value)
{
    uint32_t stored = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValue(kProvisionRequestKey, stored));
    value = stored != 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetOtaTlvEncryptionKey(const ByteSpan & value)
{
    (void) value;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR Storage::GetOtaTlvEncryptionKeyId(uint32_t & keyId)
{
    (void) keyId;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR Storage::DecryptUsingOtaTlvEncryptionKey(MutableByteSpan & block, uint32_t & ivOffset)
{
    (void) block;
    (void) ivOffset;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR Storage::SetTestEventTriggerKey(const ByteSpan & value)
{
    (void) value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetTestEventTriggerKey(MutableByteSpan & keySpan)
{
    (void) keySpan;
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

// ProvisionStorage functions for Zephyr settings backend, provided by provision libraries for other ProvisionStorage backends.
#if SL_PROVISION_GENERATOR == 0

CHIP_ERROR Storage::Set(uint16_t id, const uint8_t * value)
{
    (void) id;
    (void) value;
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Get(uint16_t id, uint8_t & value)
{
    (void) id;
    (void) value;
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Set(uint16_t id, const uint16_t * value)
{
    (void) id;
    (void) value;
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Get(uint16_t id, uint16_t & value)
{
    (void) id;
    (void) value;
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Set(uint16_t id, const uint32_t * value)
{
    (void) id;
    (void) value;
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Get(uint16_t id, uint32_t & value)
{
    (void) id;
    (void) value;
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Set(uint16_t id, const uint64_t * value)
{
    (void) id;
    (void) value;
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Get(uint16_t id, uint64_t & value)
{
    (void) id;
    (void) value;
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Get(uint16_t id, uint8_t * value, size_t max_size, size_t & size)
{
    (void) id;
    (void) value;
    (void) max_size;
    (void) size;
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Set(uint16_t id, const uint8_t * value, size_t size)
{
    (void) id;
    (void) value;
    (void) size;
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    constexpr uint8_t kLegacyDateLength        = 10; // YYYY-MM-DD
    char date[kManufacturingDateLengthMax + 1] = { 0 };
    char temp[kManufacturingDateLengthMax + 1] = { 0 };
    size_t date_len                            = 0;
    char * parse_end                           = nullptr;
    CHIP_ERROR err                             = CHIP_NO_ERROR;

    ReturnErrorOnFailure(GetManufacturingDate((uint8_t *) date, sizeof(date), date_len));
    // Convert legacy date format to new date format
    if ((kLegacyDateLength == date_len) && ('-' == date[4]) && ('-' == date[7]))
    {
        date_len = kDateStringLength;
        snprintf(temp, sizeof(temp), "%.4s%.2s%.2s", date, date + 5, date + 8);
        memcpy(date, temp, date_len);
        ReturnErrorOnFailure(SetManufacturingDate(date, date_len));
    }
    VerifyOrExit(date_len >= kDateStringLength, err = CHIP_ERROR_INVALID_ARGUMENT);
    // Year
    memcpy(temp, date, 4); // yyyy
    temp[4] = 0;
    year    = static_cast<uint16_t>(strtoul(temp, &parse_end, 10));
    VerifyOrExit(parse_end == (temp + 4), err = CHIP_ERROR_INVALID_ARGUMENT);
    // Month
    memcpy(temp, &date[4], 2); // mm
    temp[2] = 0;
    month   = static_cast<uint8_t>(strtoul(temp, &parse_end, 10));
    VerifyOrExit(parse_end == (temp + 2), err = CHIP_ERROR_INVALID_ARGUMENT);
    // Day
    memcpy(temp, &date[6], 2); // dd
    temp[2] = 0;
    day     = static_cast<uint8_t>(strtoul(temp, &parse_end, 10));
    VerifyOrExit(parse_end == (temp + 2), err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        printf("Invalid manufacturing date: %s", date);
    }
    return err;
}

CHIP_ERROR Storage::GetManufacturingDateSuffix(MutableCharSpan & suffixBuffer)
{
    char date[kManufacturingDateLengthMax + 1] = { 0 };
    size_t dateLen                             = 0;

    ReturnErrorOnFailure(GetManufacturingDate(reinterpret_cast<uint8_t *>(date), sizeof(date), dateLen));
    if (dateLen <= kDateStringLength)
    {
        suffixBuffer.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    const size_t suffixLen = dateLen - kDateStringLength;
    VerifyOrReturnError(suffixLen <= suffixBuffer.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(suffixBuffer.data(), date + kDateStringLength, suffixLen);
    suffixBuffer.reduce_size(suffixLen);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetRotatingDeviceIdUniqueId(MutableByteSpan & value)
{
    size_t size = 0;

    ReturnErrorOnFailure(GetPersistentUniqueId(value.data(), value.size(), size));
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetSpake2pSalt(MutableByteSpan & value)
{
    char saltB64[kSpake2pSaltB64LengthMax + 1] = { 0 };
    size_t sizeB64                             = 0;

    ReturnErrorOnFailure(GetSpake2pSalt(saltB64, sizeof(saltB64), sizeB64));

    uint8_t salt[chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length] = { 0 };
    const size_t saltLen                                       = chip::Base64Decode32(saltB64, sizeB64, salt);
    VerifyOrReturnError(saltLen != UINT32_MAX, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(saltLen <= value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(value.data(), salt, saltLen);
    value.reduce_size(saltLen);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetSpake2pVerifier(MutableByteSpan & outValue, size_t & outSize)
{
    VerifyOrReturnError(outValue.size() >= chip::Crypto::kSpake2p_VerifierSerialized_Length, CHIP_ERROR_BUFFER_TOO_SMALL);

    char verifierB64[kSpake2pVerifierB64LengthMax + 1] = { 0 };
    size_t sizeB64                                     = 0;

    ReturnErrorOnFailure(GetSpake2pVerifier(verifierB64, sizeof(verifierB64), sizeB64));

    outSize = chip::Base64Decode32(verifierB64, sizeB64, outValue.data());
    VerifyOrReturnError(outSize != UINT32_MAX, CHIP_ERROR_INVALID_ARGUMENT);
    outValue.reduce_size(outSize);
    return CHIP_NO_ERROR;
}

#endif // SL_PROVISION_GENERATOR == 0

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
