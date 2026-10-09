/*
 *    Copyright (c) 2024-2026 Project CHIP Authors
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
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/silabs/SilabsConfig.h>
#include <platform/silabs/multi-ota/OtaTlvEncryptionKey.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>
#include <provision/ProvisionStorageWriter.h>
#include <silabs_creds.h>

#include <cstring>

using SilabsConfig = chip::DeviceLayer::Internal::SilabsConfig;

#if defined(SL_PROVISION_GENERATOR) && SL_PROVISION_GENERATOR
extern void setNvm3End(uint32_t addr);
#endif // SL_PROVISION_GENERATOR
extern uint8_t linker_nvm_end[];
#ifdef _SILICON_LABS_32B_SERIES_3
extern uint8_t linker_static_secure_tokens_begin;
#endif

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

ProvisionStorageWriter & ProvisionStorageWriter::GetInstance()
{
    static ProvisionStorageWriter instance;
    return instance;
}

namespace {
size_t sCredentialsOffset = 0;

CHIP_ERROR ErasePage(uint32_t addr)
{
    return chip::DeviceLayer::Silabs::GetPlatform().FlashErasePage(addr);
}

size_t RoundNearest(size_t n, size_t multiple)
{
    return (n % multiple) > 0 ? n + (multiple - n % multiple) : n;
}

/**
 * Writes "size" bytes to the flash page. The data is padded with 0xff
 * up to the nearest 32-bit boundary.
 */
CHIP_ERROR WritePage(uint32_t addr, const uint8_t * data, size_t size)
{
    // The flash driver fails if the size is not a multiple of 4 (32-bits)
    size_t size_32 = RoundNearest(size, 4);
    if (size_32 == size)
    {
        return chip::DeviceLayer::Silabs::GetPlatform().FlashWritePage(addr, data, size);
    }

    // Create a temporary buffer, and pad it with "0xff"
    uint8_t * p = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(size_32));
    VerifyOrReturnError(p != nullptr, CHIP_ERROR_INTERNAL);
    memcpy(p, data, size);
    memset(p + size, 0xff, size_32 - size);
    CHIP_ERROR err = chip::DeviceLayer::Silabs::GetPlatform().FlashWritePage(addr, p, size_32);
    chip::Platform::MemoryFree(p);
    return err;
}

CHIP_ERROR WriteFile(ProvisionStorageWriter & store, size_t & credentialsOffset, SilabsConfig::Key offset_key,
                     SilabsConfig::Key size_key, const ByteSpan & value)
{
    uint32_t base_addr = 0;
    ReturnErrorOnFailure(store.GetCredentialsBaseAddress(base_addr));
    if (0 == credentialsOffset)
    {
        ReturnErrorOnFailure(ErasePage(base_addr));
    }

    ReturnErrorOnFailure(WritePage(base_addr + credentialsOffset, value.data(), value.size()));

    // Store file offset
    ReturnErrorOnFailure(SilabsConfig::WriteConfigValue(offset_key, (uint32_t) credentialsOffset));
    // Store file size
    ReturnErrorOnFailure(SilabsConfig::WriteConfigValue(size_key, (uint32_t) value.size()));
    // Calculate offset for the next file
    credentialsOffset = RoundNearest(credentialsOffset + value.size(), 64);
    return CHIP_NO_ERROR;
}

} // namespace

//
// DeviceInstanceInfoProvider
//

CHIP_ERROR ProvisionStorageWriter::SetSerialNumber(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_SerialNum, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetVendorId(uint16_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_VendorId, value);
}

CHIP_ERROR ProvisionStorageWriter::SetVendorName(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_VendorName, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetProductId(uint16_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_ProductId, value);
}

CHIP_ERROR ProvisionStorageWriter::SetProductName(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_ProductName, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetProductLabel(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_ProductLabel, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetProductURL(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_ProductURL, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetPartNumber(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_PartNumber, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetHardwareVersion(uint16_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_HardwareVersion, value);
}

CHIP_ERROR ProvisionStorageWriter::SetHardwareVersionString(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_HardwareVersionString, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetManufacturingDate(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_ManufacturingDate, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetPersistentUniqueId(const uint8_t * value, size_t size)
{
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_PersistentUniqueId, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetSetupDiscriminator(uint16_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_SetupDiscriminator, value);
}

CHIP_ERROR ProvisionStorageWriter::SetSpake2pIterationCount(uint32_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Spake2pIterationCount, value);
}

CHIP_ERROR ProvisionStorageWriter::SetSpake2pSalt(const char * value, size_t size)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_Spake2pSalt, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetSpake2pVerifier(const char * value, size_t size)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_Spake2pVerifier, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetFirmwareInformation(const ByteSpan & value)
{
    (void) value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageWriter::SetCertificationDeclaration(const ByteSpan & value)
{
    return WriteFile(*this, sCredentialsOffset, SilabsConfig::kConfigKey_Creds_CD_Offset, SilabsConfig::kConfigKey_Creds_CD_Size,
                     value);
}

CHIP_ERROR ProvisionStorageWriter::SetProductAttestationIntermediateCert(const ByteSpan & value)
{
    return WriteFile(*this, sCredentialsOffset, SilabsConfig::kConfigKey_Creds_PAI_Offset, SilabsConfig::kConfigKey_Creds_PAI_Size,
                     value);
}

CHIP_ERROR ProvisionStorageWriter::SetDeviceAttestationCert(const ByteSpan & value)
{
    return WriteFile(*this, sCredentialsOffset, SilabsConfig::kConfigKey_Creds_DAC_Offset, SilabsConfig::kConfigKey_Creds_DAC_Size,
                     value);
}

CHIP_ERROR ProvisionStorageWriter::SetProvisionVersion(const char * value, size_t size)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_Provision_Version, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetTestEventTriggerKey(const ByteSpan & value)
{
#if defined(SL_MATTER_TEST_EVENT_TRIGGER_ENABLED) && SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
    constexpr size_t kEnableKeyLength = 16;
    VerifyOrReturnError(value.size() == kEnableKeyLength, CHIP_ERROR_INVALID_ARGUMENT);
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_Test_Event_Trigger_Key, value.data(), value.size());
#else
    (void) value;
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
}

CHIP_ERROR ProvisionStorageWriter::SetOtaTlvEncryptionKey(const ByteSpan & value)
{
#if defined(SL_MATTER_ENABLE_OTA_ENCRYPTION) && SL_MATTER_ENABLE_OTA_ENCRYPTION
#if defined(SL_MBEDTLS_USE_TINYCRYPT) && SL_MBEDTLS_USE_TINYCRYPT
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kOtaTlvEncryption_KeyId, value.data(), value.size());
#else
    Silabs::OtaTlvEncryptionKey key;
    ReturnErrorOnFailure(key.Import(value.data(), value.size()));
    return SilabsConfig::WriteConfigValue(SilabsConfig::kOtaTlvEncryption_KeyId, key.GetId());
#endif
#else
    (void) value;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR ProvisionStorageWriter::SetSetupPayload(const uint8_t * value, size_t size)
{
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_SetupPayloadBitSet, value, size);
}

CHIP_ERROR ProvisionStorageWriter::Initialize(uint32_t flash_addr, uint32_t flash_size)
{
    sCredentialsOffset = 0;

    uint32_t base_addr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(linker_nvm_end));
    if (flash_size > 0)
    {
#ifndef SLI_SI91X_MCU_INTERFACE
        base_addr = (flash_addr + flash_size - FLASH_PAGE_SIZE);
#endif
#ifdef _SILICON_LABS_32B_SERIES_3
        uint32_t tokenStartAddr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&linker_static_secure_tokens_begin));
        base_addr               = tokenStartAddr + FLASH_PAGE_SIZE;
#endif
        TEMPORARY_RETURN_IGNORED chip::DeviceLayer::Silabs::GetPlatform().FlashInit();
#if defined(SL_PROVISION_GENERATOR) && SL_PROVISION_GENERATOR
        // TODO: Might be ok to move this to gfw only using Reader.GetCredentialsBaseAddress().
        setNvm3End(base_addr);
#endif // SL_PROVISION_GENERATOR
    }
    return SetCredentialsBaseAddress(base_addr);
}

CHIP_ERROR ProvisionStorageWriter::Commit()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageWriter::GetFlashPageSize(uint32_t & size)
{
    size = FLASH_PAGE_SIZE;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageWriter::SetCredentialsBaseAddress(uint32_t addr)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Creds_Base_Addr, addr);
}

CHIP_ERROR ProvisionStorageWriter::GetCredentialsBaseAddress(uint32_t & addr)
{
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_Base_Addr, addr);
}

CHIP_ERROR ProvisionStorageWriter::SetProvisionRequest(bool value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Provision_Request, value);
}

CHIP_ERROR ProvisionStorageWriter::GetProvisionRequest(bool & value)
{
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Provision_Request, value);
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
