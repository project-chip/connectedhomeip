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

#include <cinttypes>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Zephyr/ZephyrConfig.h>
#include <provision/ProvisionStorageReader.h>
#include <provision/zephyr/ProvisionStorageZephyr.h>
#include <psa/crypto.h>
namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {
using chip::DeviceLayer::Internal::ZephyrConfig;

namespace {

CHIP_ERROR ReadConfigBin(ZephyrConfig::Key key, MutableByteSpan & buffer)
{
    size_t dataLength = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValueBin(key, buffer.data(), buffer.size(), dataLength));
    buffer.reduce_size(dataLength);
    return CHIP_NO_ERROR;
}

} // namespace

ProvisionStorageReader & ProvisionStorageReader::GetInstance()
{
    static ProvisionStorageReader instance;
    return instance;
}

CHIP_ERROR ProvisionStorageReader::GetSerialNumber(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;
    return ZephyrConfig::ReadConfigValueStr(ZephyrConfig::kConfigKey_SerialNum, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetVendorId(uint16_t & value)
{
    uint32_t stored = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValue(ZephyrStorage::kConfigKeyVendorId, stored));
    value = static_cast<uint16_t>(stored);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetVendorName(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t name_len = 0;

    return ZephyrConfig::ReadConfigValueStr(ZephyrStorage::kConfigKeyVendorName, value, max, name_len);
}

CHIP_ERROR ProvisionStorageReader::GetProductId(uint16_t & value)
{
    uint32_t stored = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValue(ZephyrStorage::kConfigKeyProductId, stored));
    value = static_cast<uint16_t>(stored);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetProductName(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;

    return ZephyrConfig::ReadConfigValueStr(ZephyrStorage::kConfigKeyProductName, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetProductLabel(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;
    return ZephyrConfig::ReadConfigValueStr(ZephyrStorage::kConfigKeyProductLabel, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetProductURL(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;
    return ZephyrConfig::ReadConfigValueStr(ZephyrStorage::kConfigKeyProductUrl, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetPartNumber(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;
    return ZephyrConfig::ReadConfigValueStr(ZephyrStorage::kConfigKeyPartNumber, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetHardwareVersion(uint16_t & value)
{
    uint32_t stored = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValue(ZephyrConfig::kConfigKey_HardwareVersion, stored));
    value = static_cast<uint16_t>(stored);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetHardwareVersionString(char * value, size_t max)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = 0;
    return ZephyrConfig::ReadConfigValueStr(ZephyrStorage::kConfigKeyHardwareVersionString, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetManufacturingDate(uint8_t * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueStr(ZephyrConfig::kConfigKey_ManufacturingDate, reinterpret_cast<char *>(value), max, size);
}

CHIP_ERROR ProvisionStorageReader::GetPersistentUniqueId(uint8_t * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueBin(ZephyrConfig::kConfigKey_UniqueId, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetSetupDiscriminator(uint16_t & value)
{
    uint32_t stored = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValue(ZephyrConfig::kConfigKey_SetupDiscriminator, stored));
    value = static_cast<uint16_t>(stored);
    VerifyOrReturnLogError(value <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetSpake2pIterationCount(uint32_t & value)
{
    return ZephyrConfig::ReadConfigValue(ZephyrConfig::kConfigKey_Spake2pIterationCount, value);
}

CHIP_ERROR ProvisionStorageReader::GetSpake2pSalt(char * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueStr(ZephyrConfig::kConfigKey_Spake2pSalt, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetSpake2pVerifier(char * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueStr(ZephyrConfig::kConfigKey_Spake2pVerifier, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetFirmwareInformation(MutableByteSpan & value)
{
    value.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetCertificationDeclaration(MutableByteSpan & value)
{
    return ReadConfigBin(ZephyrConfig::kConfigKey_CertificationDeclaration, value);
}

CHIP_ERROR ProvisionStorageReader::GetProductAttestationIntermediateCert(MutableByteSpan & value)
{
    return ReadConfigBin(ZephyrConfig::kConfigKey_MfrDeviceICACerts, value);
}

CHIP_ERROR ProvisionStorageReader::GetDeviceAttestationCert(MutableByteSpan & value)
{
    return ReadConfigBin(ZephyrConfig::kConfigKey_MfrDeviceCert, value);
}

CHIP_ERROR ProvisionStorageReader::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    const uint32_t keyId = ZephyrStorage::GetDacPsaKeyId();
    VerifyOrReturnError(ZephyrStorage::DacPsaKeyExists(), CHIP_ERROR_NOT_FOUND,
                        ChipLogError(DeviceLayer, "DAC PSA key id %u missing", keyId));

    Crypto::P256ECDSASignature rawSignature;
    VerifyOrReturnError(signature.size() >= rawSignature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t outputLen          = 0;
    const psa_status_t status = psa_sign_message(static_cast<psa_key_id_t>(keyId), PSA_ALG_ECDSA(PSA_ALG_SHA_256), message.data(),
                                                 message.size(), rawSignature.Bytes(), rawSignature.Capacity(), &outputLen);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "psa_sign_message failed: %" PRId32, status));
    VerifyOrReturnError(outputLen == Crypto::kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(rawSignature.SetLength(outputLen));
    return CopySpanToMutableSpan(ByteSpan{ rawSignature.ConstBytes(), rawSignature.Length() }, signature);
}

CHIP_ERROR ProvisionStorageReader::GetProvisionVersion(char * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueStr(ZephyrStorage::kConfigKeyProvisionVersion, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetSetupPayload(uint8_t * value, size_t max, size_t & size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::ReadConfigValueBin(ZephyrStorage::kConfigKeySetupPayload, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetOtaTlvEncryptionKeyId(uint32_t & keyId)
{
    (void) keyId;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ProvisionStorageReader::DecryptUsingOtaTlvEncryptionKey(MutableByteSpan & block, uint32_t & ivOffset)
{
    (void) block;
    (void) ivOffset;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ProvisionStorageReader::GetTestEventTriggerKey(MutableByteSpan & keySpan)
{
    (void) keySpan;
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
