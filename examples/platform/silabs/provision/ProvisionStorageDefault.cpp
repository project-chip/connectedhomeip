#include "ProvisionStorageDefault.h"
#ifdef SIWX_917
#include "AttestationKeyMbed.h" // nogncheck
#else
#include "AttestationKeyPSA.h" // nogncheck
#endif
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <em_msc.h>
#include <nvm3.h>
#include <nvm3_default.h>
#include <nvm3_hal_flash.h>
#include "silabs_creds.h"

#ifdef SIWX_917
#include "sl_si91x_common_flash_intf.h"
extern uint8_t linker_nvm_end[];
static uint8_t * _base_address = (uint8_t *) linker_nvm_end;
#endif //SIWX_917

using namespace chip::Credentials;
using namespace chip::DeviceLayer::Internal;

using SilabsConfig = chip::DeviceLayer::Internal::SilabsConfig;
using AttestKey = chip::DeviceLayer::Internal::SilabsConfig::Key;

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

static constexpr size_t kCreds_DAC_Offset_Default        = 0x000;
static constexpr size_t kCreds_PAI_Offset_Default        = 0x200;
static constexpr size_t kCreds_CD_Offset_Default         = 0x400;

static uint8_t _credentials_page[FLASH_PAGE_SIZE] = { 0 };

// Miss-aligned certificates is a common error, and printing the first few bytes is
// useful to verify proper alignment. Eight bytes is enough for this purpose.
static constexpr size_t kDebugLength = 8;



extern "C" __WEAK void setNvm3End(uint32_t addr) {}


CHIP_ERROR DefaultStorage::Initialize(uint32_t flash_addr, uint32_t flash_size)
{
#ifdef SIWX_917
    uint32_t base_addr = (uint32_t)_base_address;
#else
    uint32_t base_addr = flash_addr + flash_size - FLASH_PAGE_SIZE;
#endif
    setNvm3End(base_addr);
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Creds_Base_Addr, base_addr);
}

CHIP_ERROR DefaultStorage::GetBaseAddress(uint32_t & value)
{
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_Base_Addr, value);
}


//
// DeviceInstanceInfoProvider
//

CHIP_ERROR DefaultStorage::SetSerialNumber(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_SerialNum, value, len);
}

CHIP_ERROR DefaultStorage::GetSerialNumber(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_SerialNum, value, max, size);
}

CHIP_ERROR DefaultStorage::SetVendorId(uint16_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_VendorId, value);
}

CHIP_ERROR DefaultStorage::GetVendorId(uint16_t & value)
{
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_VendorId, value);
}

CHIP_ERROR DefaultStorage::SetVendorName(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_VendorName, value, len);
}

CHIP_ERROR DefaultStorage::GetVendorName(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_VendorName, value, max, size);
}

CHIP_ERROR DefaultStorage::SetProductId(uint16_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_ProductId, value);
}

CHIP_ERROR DefaultStorage::GetProductId(uint16_t & value)
{
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_ProductId, value);
}

CHIP_ERROR DefaultStorage::SetProductName(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_ProductName, value, len);
}

CHIP_ERROR DefaultStorage::GetProductName(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_ProductName, value, max, size);
}

CHIP_ERROR DefaultStorage::SetProductLabel(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::KConfigKey_ProductLabel, value, len);
}

CHIP_ERROR DefaultStorage::GetProductLabel(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::KConfigKey_ProductLabel, value, max, size);
}

CHIP_ERROR DefaultStorage::SetProductURL(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_ProductURL, value, len);
}
CHIP_ERROR DefaultStorage::GetProductURL(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_ProductURL, value, max, size);
}

CHIP_ERROR DefaultStorage::SetPartNumber(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_PartNumber, value, len);
}

CHIP_ERROR DefaultStorage::GetPartNumber(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_PartNumber, value, max, size);
}

CHIP_ERROR DefaultStorage::SetHardwareVersion(uint16_t & value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_HardwareVersion, value);
}

CHIP_ERROR DefaultStorage::GetHardwareVersion(uint16_t & value)
{
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_HardwareVersion, value);
}

CHIP_ERROR DefaultStorage::SetHardwareVersionString(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_HardwareVersionString, value, len);
}

CHIP_ERROR DefaultStorage::GetHardwareVersionString(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_HardwareVersionString, value, max, size);
}


CHIP_ERROR DefaultStorage::SetManufacturingDate(const char * value, size_t len)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_ManufacturingDate, value, len);
}

CHIP_ERROR DefaultStorage::GetManufacturingDate(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_ManufacturingDate, value, max, size);
}

CHIP_ERROR DefaultStorage::SetUniqueId(const uint8_t * value, size_t size)
{
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_UniqueId, value, size);
}

CHIP_ERROR DefaultStorage::GetRotatingDeviceIdUniqueId(MutableByteSpan & value)
{
    size_t size = 0;
    ReturnErrorOnFailure(SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_UniqueId, value.data(), value.size(), size));
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}


//
// CommissionableDataProvider
//

CHIP_ERROR DefaultStorage::SetSetupDiscriminator(uint16_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_SetupDiscriminator, value);
}

CHIP_ERROR DefaultStorage::GetSetupDiscriminator(uint16_t & value)
{
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_SetupDiscriminator, value);
}

CHIP_ERROR DefaultStorage::SetSpake2pIterationCount(uint32_t value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Spake2pIterationCount, value);
}

CHIP_ERROR DefaultStorage::GetSpake2pIterationCount(uint32_t & value)
{
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Spake2pIterationCount, value);
}

CHIP_ERROR DefaultStorage::SetSetupPasscode(uint32_t value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DefaultStorage::GetSetupPasscode(uint32_t & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DefaultStorage::SetSpake2pSalt(const char * value, size_t size)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_Spake2pSalt, value, size);
}

CHIP_ERROR DefaultStorage::GetSpake2pSalt(char * value, size_t max, size_t &size)
{
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_Spake2pSalt, value, max, size);
}

CHIP_ERROR DefaultStorage::SetSpake2pVerifier(const char * value, size_t size)
{
    return SilabsConfig::WriteConfigValueStr(SilabsConfig::kConfigKey_Spake2pVerifier, value, size);
}

CHIP_ERROR DefaultStorage::GetSpake2pVerifier(char * value, size_t max, size_t &size)
{
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_Spake2pVerifier, value, max, size);
}

//
// DeviceAttestationCredentialsProvider
//

CHIP_ERROR DefaultStorage::SetFirmwareInformation(const ByteSpan & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DefaultStorage::GetFirmwareInformation(MutableByteSpan & value)
{
    // TODO: We need a real example FirmwareInformation to be populated.
    value.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultStorage::SetCertificationDeclaration(const ByteSpan & value)
{
    uint32_t base_addr = 0;
    ReturnErrorOnFailure(GetBaseAddress(base_addr));
    ReturnErrorOnFailure(WriteFile(SilabsConfig::kConfigKey_Creds_CD_Offset, SilabsConfig::kConfigKey_Creds_CD_Size, base_addr, kCreds_CD_Offset_Default, value));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultStorage::GetCertificationDeclaration(MutableByteSpan & value)
{
    uint32_t base_addr = 0;
    if(CHIP_NO_ERROR == GetBaseAddress(base_addr))
    {
        // Provisioned CD
        return ReadFile("GetCertificationDeclaration", base_addr, SilabsConfig::kConfigKey_Creds_CD_Offset, SILABS_CREDENTIALS_CD_OFFSET,
                        SilabsConfig::kConfigKey_Creds_CD_Size, SILABS_CREDENTIALS_CD_SIZE, value);
    }
    else
    {
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
        // Example CD
        return Examples::GetExampleDACProvider()->GetCertificationDeclaration(value);
#else
        return CHIP_ERROR_NOT_FOUND;
#endif
    }
}

CHIP_ERROR DefaultStorage::SetProductAttestationIntermediateCert(const ByteSpan & value)
{
    uint32_t base_addr = 0;
    ReturnErrorOnFailure(GetBaseAddress(base_addr));
    ReturnErrorOnFailure(WriteFile(SilabsConfig::kConfigKey_Creds_PAI_Offset, SilabsConfig::kConfigKey_Creds_PAI_Size, base_addr, kCreds_PAI_Offset_Default, value));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultStorage::GetProductAttestationIntermediateCert(MutableByteSpan & value)
{
    uint32_t base_addr = 0;
    if(CHIP_NO_ERROR == GetBaseAddress(base_addr))
    {
        // Provisioned PAI
        return ReadFile("GetProductAttestationIntermediateCert", base_addr, SilabsConfig::kConfigKey_Creds_PAI_Offset,
                            SILABS_CREDENTIALS_PAI_OFFSET, SilabsConfig::kConfigKey_Creds_PAI_Size, SILABS_CREDENTIALS_PAI_SIZE, value);
    }
    else
    {
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
        // Example PAI
        return Examples::GetExampleDACProvider()->GetProductAttestationIntermediateCert(value);
#else
        return CHIP_ERROR_NOT_FOUND;
#endif
    }
}

CHIP_ERROR DefaultStorage::SetDeviceAttestationCert(const ByteSpan & value)
{
    uint32_t base_addr = 0;
    ReturnErrorOnFailure(GetBaseAddress(base_addr));
    ReturnErrorOnFailure(WriteFile(SilabsConfig::kConfigKey_Creds_DAC_Offset, SilabsConfig::kConfigKey_Creds_DAC_Size, base_addr, kCreds_DAC_Offset_Default, value));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultStorage::GetDeviceAttestationCert(MutableByteSpan & value)
{
    uint32_t base_addr = 0;
    if(CHIP_NO_ERROR == GetBaseAddress(base_addr))
    {
        // Provisioned DAC
        return ReadFile("GetDeviceAttestationCert", base_addr, SilabsConfig::kConfigKey_Creds_DAC_Offset, SILABS_CREDENTIALS_DAC_OFFSET,
                            SilabsConfig::kConfigKey_Creds_DAC_Size, SILABS_CREDENTIALS_DAC_SIZE, value);
    }
    else
    {
        // Example DAC
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
        return Examples::GetExampleDACProvider()->GetDeviceAttestationCert(value);
#else
        return CHIP_ERROR_NOT_FOUND;
#endif
    }
}

#ifdef SIWX_917
CHIP_ERROR DefaultStorage::SetDeviceAttestationKey(uint32_t kid, const ByteSpan & value)
{
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_Creds_KeyId, value.data(), value.size());
}

CHIP_ERROR DefaultStorage::GetDeviceAttestationCSR(uint32_t kid, uint16_t vid, uint16_t pid, const CharSpan &cn, MutableCharSpan & csr)
{
    AttestationKeyMbed key;
    uint8_t temp[kDeviceAttestationKeySizeMax] = { 0 };
    size_t size = 0;
    ReturnErrorOnFailure(key.GenerateCSR(vid, pid, cn, csr));
    ReturnErrorOnFailure(key.Export(temp, sizeof(temp), size));
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_Creds_KeyId, temp, size);
}

CHIP_ERROR DefaultStorage::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_KeyId))
    {
        AttestationKeyMbed key;
        uint8_t temp[kDeviceAttestationKeySizeMax] = { 0 };
        size_t size = 0;
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

#else
CHIP_ERROR DefaultStorage::SetDeviceAttestationKey(uint32_t kid, const ByteSpan & value)
{
    AttestationKeyPSA key(kid);
    ReturnErrorOnFailure(key.Import(value.data(), value.size()));
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Creds_KeyId, key.GetId());
}

CHIP_ERROR DefaultStorage::GetDeviceAttestationCSR(uint32_t kid, uint16_t vid, uint16_t pid, const CharSpan &cn, MutableCharSpan & csr)
{
    AttestationKeyPSA key(kid);
    ReturnErrorOnFailure(key.GenerateCSR(vid, pid, cn, csr));
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Creds_KeyId, key.GetId());
}

CHIP_ERROR DefaultStorage::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_KeyId))
    {
        uint32_t kid = 0;
        ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_KeyId, kid));
        AttestationKeyPSA key(kid);
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
#endif

//
// Other
//

CHIP_ERROR DefaultStorage::SetSetupPayload(const uint8_t * value, size_t size)
{
    return SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_SetupPayloadBitSet, value, size);
}

CHIP_ERROR DefaultStorage::GetSetupPayload(uint8_t * value, size_t max, size_t &size)
{
    return SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_SetupPayloadBitSet, value, max, size);
}

CHIP_ERROR DefaultStorage::SetProvisionRequest(bool value)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_Provision_Request, value);
}

CHIP_ERROR DefaultStorage::GetProvisionRequest(bool &value)
{
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Provision_Request, value);
}


CHIP_ERROR DefaultStorage::WriteFile(ConfigKey offset_key, ConfigKey size_key, uint32_t base_addr, uint32_t offset, const ByteSpan & value)
{
    // Store file
    memcpy(&_credentials_page[offset], value.data(), value.size());
    // Store file offset
    ReturnErrorOnFailure(SilabsConfig::WriteConfigValue(offset_key, offset));
    // Store file size
    ReturnErrorOnFailure(SilabsConfig::WriteConfigValue(size_key, (uint32_t)value.size()));

    _cd_set  = _cd_set  || (SilabsConfig::kConfigKey_Creds_CD_Offset  == offset_key);
    _pai_set = _pai_set || (SilabsConfig::kConfigKey_Creds_PAI_Offset == offset_key);
    _dac_set = _dac_set || (SilabsConfig::kConfigKey_Creds_DAC_Offset == offset_key);
    if(_cd_set && _pai_set && _dac_set)
    {
#ifdef SIWX_917
        // Erase page
        rsi_flash_erase_sector((uint32_t *)base_addr);

        constexpr size_t WRITE_SIZE = 1024;
        uint32_t offset = 0;
        while(offset < FLASH_PAGE_SIZE)
        {
            // Write to flash
            if (!rsi_flash_write((uint32_t *)(base_addr+offset), &_credentials_page[offset], WRITE_SIZE)) {
                offset += WRITE_SIZE;
            }
        }    
#else
        // Erase page
        MSC_ErasePage((uint32_t *)base_addr);
        // Write to flash
        MSC_WriteWord((uint32_t *)base_addr, _credentials_page, FLASH_PAGE_SIZE);
#endif
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultStorage::ReadFile(const char * description, uint32_t base_addr, uint32_t offset_key, uint32_t offset_default, uint32_t size_key,
                    uint32_t size_default, MutableByteSpan & value)
{
    uint32_t offset    = offset_default;
    uint32_t size      = size_default;

    // Offset
    if (SilabsConfig::ConfigValueExists(offset_key))
    {
        ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(offset_key, offset));
    }

    // Size
    if (SilabsConfig::ConfigValueExists(size_key))
    {
        ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(size_key, size));
    }

    uint8_t * address  = (uint8_t *) (base_addr + offset);
    ByteSpan span(address, size);
    ChipLogProgress(DeviceLayer, "%s, addr:%p, size:%lu", description, address, size);
    //ChipLogByteSpan(DeviceLayer, ByteSpan(span.data(), kDebugLength > span.size() ? span.size() : kDebugLength));
    return CopySpanToMutableSpan(span, value);
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
