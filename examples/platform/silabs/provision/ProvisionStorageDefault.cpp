/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include "AttestationKey.h"
#include "ProvisionStorage.h"
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <em_msc.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <nvm3.h>
#include <nvm3_default.h>
#include <nvm3_hal_flash.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/silabs/SilabsConfig.h>
#include <psa/crypto.h>
#include <silabs_creds.h>
#ifdef SLI_SI91X_MCU_INTERFACE
#include <sl_si91x_common_flash_intf.h>
#endif

#ifdef SL_PROVISION_GENERATOR
extern void setNvm3End(uint32_t addr);
#endif

extern uint8_t linker_nvm_end[];

using namespace chip::Credentials;
using namespace chip::DeviceLayer::Internal;

using SilabsConfig = chip::DeviceLayer::Internal::SilabsConfig;

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

namespace {
// Miss-aligned certificates is a common error, and printing the first few bytes is
// useful to verify proper alignment. Eight bytes is enough for this purpose.
constexpr size_t kDebugLength = 8;
size_t sCredentialsOffset     = 0;

CHIP_ERROR ErasePage(uint32_t addr)
{
#ifdef SLI_SI91X_MCU_INTERFACE
    rsi_flash_erase_sector((uint32_t *) addr);
#else
    MSC_ErasePage((uint32_t *) addr);
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR WritePage(uint32_t addr, const uint8_t * data, size_t size)
{
#ifdef SLI_SI91X_MCU_INTERFACE
    rsi_flash_write((uint32_t *) addr, (unsigned char *) data, size);
#else
    MSC_WriteWord((uint32_t *) addr, data, size);
#endif
    return CHIP_NO_ERROR;
}

size_t RoundNearest(size_t n, size_t multiple)
{
    return (n % multiple) > 0 ? n + (multiple - n % multiple) : n;
}

CHIP_ERROR WriteFile(Storage & store, SilabsConfig::Key offset_key, SilabsConfig::Key size_key, const ByteSpan & value)
{
    uint32_t base_addr = 0;
    ReturnErrorOnFailure(store.GetBaseAddress(base_addr));
    if (0 == sCredentialsOffset)
    {
        ReturnErrorOnFailure(ErasePage(base_addr));
    }

    memcpy(Storage::aux_buffer, value.data(), value.size());
    if (value.size() < Storage::kArgumentSizeMax)
    {
        memset(Storage::aux_buffer + value.size(), 0xff, Storage::kArgumentSizeMax - value.size());
    }

    ChipLogProgress(DeviceLayer, "WriteFile, addr:0x%06x+%03u, size:%u", (unsigned) base_addr, (unsigned) sCredentialsOffset,
                    (unsigned) value.size());
    // ChipLogByteSpan(DeviceLayer, ByteSpan(value.data(), value.size() < kDebugLength ? value.size() : kDebugLength));

    ReturnErrorOnFailure(WritePage(base_addr + sCredentialsOffset, Storage::aux_buffer, Storage::kArgumentSizeMax));

    // Store file offset
    ReturnErrorOnFailure(SilabsConfig::WriteConfigValue(offset_key, (uint32_t) sCredentialsOffset));
    // Store file size
    ReturnErrorOnFailure(SilabsConfig::WriteConfigValue(size_key, (uint32_t) value.size()));
    // Calculate offset for the next file
    sCredentialsOffset = RoundNearest(sCredentialsOffset + value.size(), 64);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadFileByOffset(Storage & store, const char * description, uint32_t offset, uint32_t size, MutableByteSpan & value)
{
    uint32_t base_addr = 0;
    ReturnErrorOnFailure(store.GetBaseAddress(base_addr));

    uint8_t * address = (uint8_t *) (base_addr + offset);
    ByteSpan span(address, size);
    ChipLogProgress(DeviceLayer, "%s, addr:0x%06x+%03u, size:%u", description, (unsigned) base_addr, (unsigned) offset,
                    (unsigned) size);
    // ChipLogByteSpan(DeviceLayer, ByteSpan(span.data(), span.size() < kDebugLength ? span.size() : kDebugLength));
    return CopySpanToMutableSpan(span, value);
}

CHIP_ERROR ReadFileByKey(Storage & store, const char * description, uint32_t offset_key, uint32_t size_key, MutableByteSpan & value)
{
    uint32_t offset = 0;
    uint32_t size   = 0;

    // Offset
    VerifyOrReturnError(SilabsConfig::ConfigValueExists(offset_key), CHIP_ERROR_NOT_FOUND);
    ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(offset_key, offset));

    // Size
    VerifyOrReturnError(SilabsConfig::ConfigValueExists(size_key), CHIP_ERROR_NOT_FOUND);
    ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(size_key, size));

    return ReadFileByOffset(store, description, offset, size, value);
}

} // namespace

//
// Initialization
//

CHIP_ERROR Storage::Initialize(uint32_t flash_addr, uint32_t flash_size)
{
    sCredentialsOffset = 0;

    uint32_t base_addr = (uint32_t) linker_nvm_end;
    if (flash_size > 0)
    {
#ifndef SLI_SI91X_MCU_INTERFACE
        base_addr = (flash_addr + flash_size - FLASH_PAGE_SIZE);
        MSC_Init();
#endif // SLI_SI91X_MCU_INTERFACE
#ifdef SL_PROVISION_GENERATOR
        setNvm3End(base_addr);
#endif
    }
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Creds_Base_Addr, base_addr);
}

CHIP_ERROR Storage::GetBaseAddress(uint32_t & value)
{
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_Base_Addr, value);
}

CHIP_ERROR Storage::Commit()
{
    return CHIP_NO_ERROR;
}

//
// DeviceInstanceInfoProvider
//

CHIP_ERROR Storage::SetSerialNumber(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_SerialNum, value, len);
}

CHIP_ERROR Storage::GetSerialNumber(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_SerialNum, value, max, size);
}

CHIP_ERROR Storage::SetVendorId(uint16_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_VendorId, value);
}

CHIP_ERROR Storage::GetVendorId(uint16_t & value)
{
    CHIP_ERROR err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_VendorId, value);
#if defined(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID) && CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID;
        err   = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR Storage::SetVendorName(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_VendorName, value, len);
}

CHIP_ERROR Storage::GetVendorName(char * value, size_t max)
{
    size_t name_len = 0; // Without counting null-terminator

    CHIP_ERROR err = SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_VendorName, value, max, name_len);
#if defined(CHIP_DEVICE_CONFIG_TEST_VENDOR_NAME)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        VerifyOrReturnError(value != nullptr, CHIP_ERROR_NO_MEMORY);
        VerifyOrReturnError(max > strlen(CHIP_DEVICE_CONFIG_TEST_VENDOR_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
        Platform::CopyString(value, max, CHIP_DEVICE_CONFIG_TEST_VENDOR_NAME);
        err = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR Storage::SetProductId(uint16_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_ProductId, value);
}

CHIP_ERROR Storage::GetProductId(uint16_t & value)
{
    CHIP_ERROR err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_ProductId, value);

#if defined(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID) && CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID;
        err   = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR Storage::SetProductName(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_ProductName, value, len);
}

CHIP_ERROR Storage::GetProductName(char * value, size_t max)
{
    size_t name_len = 0; // Without counting null-terminator

    CHIP_ERROR err = SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_ProductName, value, max, name_len);
#if defined(CHIP_DEVICE_CONFIG_TEST_PRODUCT_NAME)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        VerifyOrReturnError(value != nullptr, CHIP_ERROR_NO_MEMORY);
        VerifyOrReturnError(max > strlen(CHIP_DEVICE_CONFIG_TEST_VENDOR_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
        Platform::CopyString(value, max, CHIP_DEVICE_CONFIG_TEST_PRODUCT_NAME);
        err = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR Storage::SetProductLabel(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::KConfigKey_ProductLabel, value, len);
}

CHIP_ERROR Storage::GetProductLabel(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::KConfigKey_ProductLabel, value, max, size);
}

CHIP_ERROR Storage::SetProductURL(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_ProductURL, value, len);
}
CHIP_ERROR Storage::GetProductURL(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_ProductURL, value, max, size);
}

CHIP_ERROR Storage::SetPartNumber(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_PartNumber, value, len);
}

CHIP_ERROR Storage::GetPartNumber(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_PartNumber, value, max, size);
}

CHIP_ERROR Storage::SetHardwareVersion(uint16_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_HardwareVersion, value);
}

CHIP_ERROR Storage::GetHardwareVersion(uint16_t & value)
{
    CHIP_ERROR err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_HardwareVersion, value);
#if defined(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION;
        err   = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR Storage::SetHardwareVersionString(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_HardwareVersionString, value, len);
}

CHIP_ERROR Storage::GetHardwareVersionString(char * value, size_t max)
{
    size_t hw_version_len = 0; // @ithout counting null-terminator

    CHIP_ERROR err = SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_HardwareVersionString, value, max, hw_version_len);
#if defined(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        VerifyOrReturnError(value != nullptr, CHIP_ERROR_NO_MEMORY);
        VerifyOrReturnError(max > strlen(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
        Platform::CopyString(value, max, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
        err = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR Storage::SetManufacturingDate(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_ManufacturingDate, value, len);
}

CHIP_ERROR Storage::GetManufacturingDate(uint8_t * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_ManufacturingDate, (char *) value, max, size);
}

CHIP_ERROR Storage::SetUniqueId(const uint8_t * value, size_t size)
{
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_UniqueId, value, size);
}

CHIP_ERROR Storage::GetUniqueId(uint8_t * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_UniqueId, value, max, size);
}

//
// CommissionableDataProvider
//

CHIP_ERROR Storage::SetSetupDiscriminator(uint16_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_SetupDiscriminator, value);
}

CHIP_ERROR Storage::GetSetupDiscriminator(uint16_t & value)
{
    CHIP_ERROR err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_SetupDiscriminator, value);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
        err   = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);
    VerifyOrReturnLogError(value <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetSpake2pIterationCount(uint32_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Spake2pIterationCount, value);
}

CHIP_ERROR Storage::GetSpake2pIterationCount(uint32_t & value)
{
    CHIP_ERROR err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Spake2pIterationCount, value);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT;
        err   = CHIP_NO_ERROR;
    }
#endif
    return err;
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
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_Spake2pSalt, value, size);
}

CHIP_ERROR Storage::GetSpake2pSalt(char * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_Spake2pSalt, value, max, size);
}

CHIP_ERROR Storage::SetSpake2pVerifier(const char * value, size_t size)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_Spake2pVerifier, value, size);
}

CHIP_ERROR Storage::GetSpake2pVerifier(char * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_Spake2pVerifier, value, max, size);
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
    // TODO: We need a real example FirmwareInformation to be populated.
    value.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetCertificationDeclaration(const ByteSpan & value)
{
    ReturnErrorOnFailure(WriteFile(*this, SilabsConfig::kConfigKey_Creds_CD_Offset, SilabsConfig::kConfigKey_Creds_CD_Size, value));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetCertificationDeclaration(MutableByteSpan & value)
{
    CHIP_ERROR err = ReadFileByKey(*this, "GetCertificationDeclaration", SilabsConfig::kConfigKey_Creds_CD_Offset,
                                   SilabsConfig::kConfigKey_Creds_CD_Size, value);
#if defined(SL_PROVISION_VERSION_1_0) && SL_PROVISION_VERSION_1_0
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // Reading from the old script's location.
        err = ReadFileByOffset(*this, "GetDeviceAttestationCert", SL_CREDENTIALS_CD_OFFSET, SL_CREDENTIALS_CD_SIZE, value);
    }
#endif
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // Example CD
        err = Examples::GetExampleDACProvider()->GetCertificationDeclaration(value);
    }
#endif
    return err;
}

CHIP_ERROR Storage::SetProductAttestationIntermediateCert(const ByteSpan & value)
{
    ReturnErrorOnFailure(
        WriteFile(*this, SilabsConfig::kConfigKey_Creds_PAI_Offset, SilabsConfig::kConfigKey_Creds_PAI_Size, value));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetProductAttestationIntermediateCert(MutableByteSpan & value)
{
    CHIP_ERROR err = ReadFileByKey(*this, "GetProductAttestationIntermediateCert", SilabsConfig::kConfigKey_Creds_PAI_Offset,
                                   SilabsConfig::kConfigKey_Creds_PAI_Size, value);
#if defined(SL_PROVISION_VERSION_1_0) && SL_PROVISION_VERSION_1_0
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // Reading from the old script's location.
        err = ReadFileByOffset(*this, "GetDeviceAttestationCert", SL_CREDENTIALS_PAI_OFFSET, SL_CREDENTIALS_PAI_SIZE, value);
    }
#endif
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // Example PAI
        err = Examples::GetExampleDACProvider()->GetProductAttestationIntermediateCert(value);
    }
#endif
    return err;
}

CHIP_ERROR Storage::SetDeviceAttestationCert(const ByteSpan & value)
{
    ReturnErrorOnFailure(
        WriteFile(*this, SilabsConfig::kConfigKey_Creds_DAC_Offset, SilabsConfig::kConfigKey_Creds_DAC_Size, value));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetDeviceAttestationCert(MutableByteSpan & value)
{
    CHIP_ERROR err = ReadFileByKey(*this, "GetDeviceAttestationCert", SilabsConfig::kConfigKey_Creds_DAC_Offset,
                                   SilabsConfig::kConfigKey_Creds_DAC_Size, value);
#if defined(SL_PROVISION_VERSION_1_0) && SL_PROVISION_VERSION_1_0
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // Reading from the old script's location.
        err = ReadFileByOffset(*this, "GetDeviceAttestationCert", SL_CREDENTIALS_DAC_OFFSET, SL_CREDENTIALS_DAC_SIZE, value);
    }
#endif
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // Example DAC
        return Examples::GetExampleDACProvider()->GetDeviceAttestationCert(value);
    }
#endif
    return err;
}

#ifdef SLI_SI91X_MCU_INTERFACE
CHIP_ERROR Storage::SetDeviceAttestationKey(const ByteSpan & value)
{
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_Creds_KeyId, value.data(), value.size());
}

CHIP_ERROR Storage::GetDeviceAttestationCSR(uint16_t vid, uint16_t pid, const CharSpan & cn, MutableCharSpan & csr)
{
    AttestationKey key;
    uint8_t temp[kDeviceAttestationKeySizeMax] = { 0 };
    size_t size                                = 0;
    ReturnErrorOnFailure(key.GenerateCSR(vid, pid, cn, csr));
    ReturnErrorOnFailure(key.Export(temp, sizeof(temp), size));
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_Creds_KeyId, temp, size);
}

CHIP_ERROR Storage::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_KeyId))
    {
        AttestationKey key;
        uint8_t temp[kDeviceAttestationKeySizeMax] = { 0 };
        size_t size                                = 0;
        ReturnErrorOnFailure(SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_Creds_KeyId, temp, sizeof(temp), size));
        key.Import(temp, size);
        return key.SignMessage(message, signature);
    }
    else
    {
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
        // Example DAC key
        return Examples::GetExampleDACProvider()->SignWithDeviceAttestationKey(message, signature);
#else
        return CHIP_ERROR_NOT_FOUND;
#endif
    }
}

#else // SLI_SI91X_MCU_INTERFACEX_MCU_INTERFACEX_MCU_INTERFACE

CHIP_ERROR Storage::SetDeviceAttestationKey(const ByteSpan & value)
{
    AttestationKey key;
    ReturnErrorOnFailure(key.Import(value.data(), value.size()));
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Creds_KeyId, key.GetId());
}

CHIP_ERROR Storage::GetDeviceAttestationCSR(uint16_t vid, uint16_t pid, const CharSpan & cn, MutableCharSpan & csr)
{
    AttestationKey key;
    ReturnErrorOnFailure(key.GenerateCSR(vid, pid, cn, csr));
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Creds_KeyId, key.GetId());
}

CHIP_ERROR Storage::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_FOUND;
    uint32_t kid   = 0;

    if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_KeyId))
    {
        ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_KeyId, kid));
        AttestationKey key(kid);
        err = key.SignMessage(message, signature);
    }
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    else
    {
        // Example DAC key
        err = Examples::GetExampleDACProvider()->SignWithDeviceAttestationKey(message, signature);
    }
#endif
    ChipLogProgress(DeviceLayer, "SignWithDeviceAttestationKey, kid:%u, msg_size:%u, sig_size:%u, err:0x%02x", (unsigned) kid,
                    (unsigned) message.size(), (unsigned) signature.size(), (unsigned) err.AsInteger());
    // ChipLogByteSpan(DeviceLayer, ByteSpan(signature.data(), signature.size() < kDebugLength ? signature.size() : kDebugLength));
    return err;
}
#endif // SLI_SI91X_MCU_INTERFACE

//
// Other
//

CHIP_ERROR Storage::SetProvisionVersion(const char * value, size_t size)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_Provision_Version, value, size);
}

CHIP_ERROR Storage::GetProvisionVersion(char * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_Provision_Version, value, max, size);
}

CHIP_ERROR Storage::SetSetupPayload(const uint8_t * value, size_t size)
{
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_SetupPayloadBitSet, value, size);
}

CHIP_ERROR Storage::GetSetupPayload(uint8_t * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_SetupPayloadBitSet, value, max, size);
}

CHIP_ERROR Storage::SetProvisionRequest(bool value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Provision_Request, value);
}

CHIP_ERROR Storage::GetProvisionRequest(bool & value)
{
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Provision_Request, value);
}

#if OTA_ENCRYPTION_ENABLE
CHIP_ERROR Storage::SetOtaTlvEncryptionKey(const ByteSpan & value)
{
    chip::DeviceLayer::Silabs::OtaTlvEncryptionKey::OtaTlvEncryptionKey key;
    ReturnErrorOnFailure(key.Import(value.data(), value.size()));
    return SilabsConfig::WriteConfigValue(SilabsConfig::kOtaTlvEncryption_KeyId, key.GetId());
}
#endif

/**
 * @brief Reads the test event trigger key from NVM. If the key isn't present, returns default value if defined.
 *
 * @param[out] keySpan output buffer. Must be at least large enough for 16 bytes (ken length)
 * @return CHIP_ERROR
 */
CHIP_ERROR Storage::GetTestEventTriggerKey(MutableByteSpan & keySpan)
{
    constexpr size_t kEnableKeyLength = 16; // Expected byte size of the EnableKey
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    size_t keyLength                  = 0;

    VerifyOrReturnError(keySpan.size() >= kEnableKeyLength, CHIP_ERROR_BUFFER_TOO_SMALL);

    err = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_Test_Event_Trigger_Key, keySpan.data(), kEnableKeyLength,
                                           keyLength);
#ifndef NDEBUG
#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {

        constexpr char enableKey[] = SL_MATTER_TEST_EVENT_TRIGGER_ENABLE_KEY;
        if (Encoding::HexToBytes(enableKey, strlen(enableKey), keySpan.data(), kEnableKeyLength) != kEnableKeyLength)
        {
            // enableKey Hex String doesn't have the correct length
            memset(keySpan.data(), 0, keySpan.size());
            return CHIP_ERROR_INTERNAL;
        }
        err = CHIP_NO_ERROR;
    }
#endif // SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
#endif // NDEBUG

    keySpan.reduce_size(kEnableKeyLength);
    return err;
}

} // namespace Provision

void MigrateUint32(uint32_t old_key, uint32_t new_key)
{
    uint32_t value = 0;
    if (SilabsConfig::ConfigValueExists(old_key) && (CHIP_NO_ERROR == SilabsConfig::ReadConfigValue(old_key, value)))
    {
        SilabsConfig::WriteConfigValue(new_key, value);
    }
}

void MigrateDacProvider(void)
{
    constexpr uint32_t kOldKey_Creds_KeyId      = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x21);
    constexpr uint32_t kOldKey_Creds_Base_Addr  = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x22);
    constexpr uint32_t kOldKey_Creds_DAC_Offset = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x23);
    constexpr uint32_t kOldKey_Creds_DAC_Size   = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x24);
    constexpr uint32_t kOldKey_Creds_PAI_Size   = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x26);
    constexpr uint32_t kOldKey_Creds_PAI_Offset = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x25);
    constexpr uint32_t kOldKey_Creds_CD_Offset  = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x27);
    constexpr uint32_t kOldKey_Creds_CD_Size    = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x28);

    MigrateUint32(kOldKey_Creds_KeyId, SilabsConfig::kConfigKey_Creds_KeyId);
    MigrateUint32(kOldKey_Creds_Base_Addr, SilabsConfig::kConfigKey_Creds_Base_Addr);
    MigrateUint32(kOldKey_Creds_DAC_Offset, SilabsConfig::kConfigKey_Creds_DAC_Offset);
    MigrateUint32(kOldKey_Creds_DAC_Size, SilabsConfig::kConfigKey_Creds_DAC_Size);
    MigrateUint32(kOldKey_Creds_PAI_Offset, SilabsConfig::kConfigKey_Creds_PAI_Offset);
    MigrateUint32(kOldKey_Creds_PAI_Size, SilabsConfig::kConfigKey_Creds_PAI_Size);
    MigrateUint32(kOldKey_Creds_CD_Offset, SilabsConfig::kConfigKey_Creds_CD_Offset);
    MigrateUint32(kOldKey_Creds_CD_Size, SilabsConfig::kConfigKey_Creds_CD_Size);
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
