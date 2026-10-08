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

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Zephyr/ZephyrConfig.h>
#include <provision/ProvisionStorageWriter.h>
#include <provision/zephyr/ProvisionStorageZephyr.h>
#if defined(SL_PROVISION_GENERATOR) && SL_PROVISION_GENERATOR
#include <settings/settings_nvs.h>
#endif
namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {
using chip::DeviceLayer::Internal::ZephyrConfig;

namespace {
bool sInitialized = false;
} // namespace

ProvisionStorageWriter & ProvisionStorageWriter::GetInstance()
{
    static ProvisionStorageWriter instance;
    return instance;
}
CHIP_ERROR ProvisionStorageWriter::Initialize(uint32_t flashAddress, uint32_t flashSize)
{
#if defined(SL_PROVISION_GENERATOR) && SL_PROVISION_GENERATOR
    // flashAddress is the offset to the storage area; flashSize is its total size.
    VerifyOrReturnError(settings_nvs_set_runtime_geometry(static_cast<off_t>(flashAddress), flashSize) == 0,
                        CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    ReturnErrorOnFailure(ZephyrConfig::Init());
#else
    (void) flashAddress;
    (void) flashSize;
    ReturnErrorOnFailure(ZephyrConfig::Init());
    VerifyOrDo(ZephyrStorage::DacPsaKeyExists(),
               ChipLogError(DeviceLayer, "DAC PSA key id %u missing", ZephyrStorage::GetDacPsaKeyId()));
#endif
    sInitialized = true;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ProvisionStorageWriter::Commit()
{
    // Zephyr settings writes are immediate.
    return CHIP_NO_ERROR;
}
CHIP_ERROR ProvisionStorageWriter::GetFlashPageSize(uint32_t & size)
{
    (void) size;
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR ProvisionStorageWriter::SetCredentialsBaseAddress(uint32_t address)
{
    (void) address;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ProvisionStorageWriter::GetCredentialsBaseAddress(uint32_t & address)
{
    address = 0;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ProvisionStorageWriter::SetProvisionRequest(bool value)
{
    return sInitialized
        ? ZephyrConfig::WriteConfigValue(ZephyrStorage::kConfigKeyProvisionRequest, static_cast<uint32_t>(value ? 1 : 0))
        : CHIP_NO_ERROR;
}
CHIP_ERROR ProvisionStorageWriter::GetProvisionRequest(bool & value)
{
    if (!sInitialized)
    {
        // Generator firmware has not initialized NVS yet, so assume provisioning is required.
        value = true;
        return CHIP_NO_ERROR;
    }
    uint32_t stored = 0;
    ReturnErrorOnFailure(ZephyrConfig::ReadConfigValue(ZephyrStorage::kConfigKeyProvisionRequest, stored));
    value = stored != 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageWriter::SetSerialNumber(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrConfig::kConfigKey_SerialNum, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetVendorId(uint16_t value)
{
    return ZephyrConfig::WriteConfigValue(ZephyrStorage::kConfigKeyVendorId, static_cast<uint32_t>(value));
}

CHIP_ERROR ProvisionStorageWriter::SetVendorName(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrStorage::kConfigKeyVendorName, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetProductId(uint16_t value)
{
    return ZephyrConfig::WriteConfigValue(ZephyrStorage::kConfigKeyProductId, static_cast<uint32_t>(value));
}

CHIP_ERROR ProvisionStorageWriter::SetProductName(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrStorage::kConfigKeyProductName, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetProductLabel(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrStorage::kConfigKeyProductLabel, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetProductURL(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrStorage::kConfigKeyProductUrl, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetPartNumber(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrStorage::kConfigKeyPartNumber, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetHardwareVersion(uint16_t value)
{
    return ZephyrConfig::WriteConfigValue(ZephyrConfig::kConfigKey_HardwareVersion, static_cast<uint32_t>(value));
}

CHIP_ERROR ProvisionStorageWriter::SetHardwareVersionString(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrStorage::kConfigKeyHardwareVersionString, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetManufacturingDate(const char * value, size_t len)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrConfig::kConfigKey_ManufacturingDate, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetPersistentUniqueId(const uint8_t * value, size_t size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueBin(ZephyrConfig::kConfigKey_UniqueId, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetSetupDiscriminator(uint16_t value)
{
    return ZephyrConfig::WriteConfigValue(ZephyrConfig::kConfigKey_SetupDiscriminator, static_cast<uint32_t>(value));
}

CHIP_ERROR ProvisionStorageWriter::SetSpake2pIterationCount(uint32_t value)
{
    return ZephyrConfig::WriteConfigValue(ZephyrConfig::kConfigKey_Spake2pIterationCount, value);
}

CHIP_ERROR ProvisionStorageWriter::SetSpake2pSalt(const char * value, size_t size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrConfig::kConfigKey_Spake2pSalt, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetSpake2pVerifier(const char * value, size_t size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrConfig::kConfigKey_Spake2pVerifier, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetFirmwareInformation(const ByteSpan & value)
{
    (void) value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageWriter::SetCertificationDeclaration(const ByteSpan & value)
{
    return ZephyrConfig::WriteConfigValueBin(ZephyrConfig::kConfigKey_CertificationDeclaration, value.data(), value.size());
}

CHIP_ERROR ProvisionStorageWriter::SetProductAttestationIntermediateCert(const ByteSpan & value)
{
    return ZephyrConfig::WriteConfigValueBin(ZephyrConfig::kConfigKey_MfrDeviceICACerts, value.data(), value.size());
}

CHIP_ERROR ProvisionStorageWriter::SetDeviceAttestationCert(const ByteSpan & value)
{
    return ZephyrConfig::WriteConfigValueBin(ZephyrConfig::kConfigKey_MfrDeviceCert, value.data(), value.size());
}

CHIP_ERROR ProvisionStorageWriter::SetProvisionVersion(const char * value, size_t size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueStr(ZephyrStorage::kConfigKeyProvisionVersion, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetSetupPayload(const uint8_t * value, size_t size)
{
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ZephyrConfig::WriteConfigValueBin(ZephyrStorage::kConfigKeySetupPayload, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetOtaTlvEncryptionKey(const ByteSpan & value)
{
    (void) value;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}
CHIP_ERROR ProvisionStorageWriter::SetTestEventTriggerKey(const ByteSpan & value)
{
    (void) value;
    return CHIP_NO_ERROR;
}
} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
