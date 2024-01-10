#pragma once

#include "ProvisionStorage.h"
#include <platform/silabs/SilabsConfig.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

using ConfigKey = chip::DeviceLayer::Internal::SilabsConfig::Key;

struct DefaultStorage: public Storage
{
public:
    //
    // Initialization
    //

    CHIP_ERROR Initialize(uint32_t flash_addr, uint32_t flash_size) override;
    CHIP_ERROR GetBaseAddress(uint32_t & value) override;

    //
    // DeviceInstanceInfoProvider
    //

    CHIP_ERROR GetSerialNumber(char * value, size_t max) override;
    CHIP_ERROR GetVendorId(uint16_t & value) override;
    CHIP_ERROR GetVendorName(char * value, size_t max) override;
    CHIP_ERROR GetProductId(uint16_t & productId) override;
    CHIP_ERROR GetProductName(char * value, size_t max) override;
    CHIP_ERROR GetProductLabel(char * value, size_t max) override;
    CHIP_ERROR GetProductURL(char * value, size_t max) override;
    CHIP_ERROR GetPartNumber(char * value, size_t max) override;
    CHIP_ERROR GetHardwareVersion(uint16_t & value) override;
    CHIP_ERROR GetHardwareVersionString(char * value, size_t max) override;
    CHIP_ERROR GetManufacturingDate(char * value, size_t max) override;
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & value) override;

    //
    // CommissionableDataProvider
    //

    CHIP_ERROR GetSetupDiscriminator(uint16_t & value) override;
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & value) override;
    CHIP_ERROR GetSetupPasscode(uint32_t & value) override;

    //
    // DeviceAttestationCredentialsProvider
    //

    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & value) override;
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & value) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & value) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & value) override;
    CHIP_ERROR GetDeviceAttestationCSR(uint32_t kid, uint16_t vid, uint16_t pid, const CharSpan &cn, MutableCharSpan & csr) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature) override;

    // Provision Request
    CHIP_ERROR SetProvisionRequest(bool value) override;
    CHIP_ERROR GetProvisionRequest(bool &value) override;

protected:
    CHIP_ERROR SetSerialNumber(const char * value, size_t len) override;
    CHIP_ERROR SetVendorId(uint16_t value) override;
    CHIP_ERROR SetVendorName(const char * value, size_t len) override;
    CHIP_ERROR SetProductId(uint16_t productId) override;
    CHIP_ERROR SetProductName(const char * value, size_t len) override;
    CHIP_ERROR SetProductLabel(const char * value, size_t len) override;
    CHIP_ERROR SetProductURL(const char * value, size_t len) override;
    CHIP_ERROR SetPartNumber(const char * value, size_t len) override;
    CHIP_ERROR SetHardwareVersion(uint16_t & value) override;
    CHIP_ERROR SetHardwareVersionString(const char * value, size_t len) override;
    CHIP_ERROR SetManufacturingDate(const char * value, size_t len) override;
    CHIP_ERROR SetUniqueId(const uint8_t * value, size_t size) override;

    CHIP_ERROR SetSetupDiscriminator(uint16_t value) override;
    CHIP_ERROR SetSpake2pIterationCount(uint32_t value) override;
    CHIP_ERROR SetSetupPasscode(uint32_t value) override;
    CHIP_ERROR SetSpake2pSalt(const char * value, size_t size) override;
    CHIP_ERROR GetSpake2pSalt(char * value, size_t max, size_t &size) override;
    CHIP_ERROR SetSpake2pVerifier(const char * value, size_t size) override;
    CHIP_ERROR GetSpake2pVerifier(char * value, size_t max, size_t &size) override;

    CHIP_ERROR SetFirmwareInformation(const ByteSpan & value) override;
    CHIP_ERROR SetCertificationDeclaration(const ByteSpan & value) override;
    CHIP_ERROR SetProductAttestationIntermediateCert(const ByteSpan & value) override;
    CHIP_ERROR SetDeviceAttestationCert(const ByteSpan & value) override;
    CHIP_ERROR SetDeviceAttestationKey(uint32_t kid, const ByteSpan & value) override;

    CHIP_ERROR SetSetupPayload(const uint8_t * value, size_t size) override;
    CHIP_ERROR GetSetupPayload(uint8_t * value, size_t max, size_t &size) override;

private:
    CHIP_ERROR WriteFile(ConfigKey offset_key, ConfigKey size_key, uint32_t base_addr, uint32_t offset, const ByteSpan & value);
    CHIP_ERROR ReadFile(const char * description, uint32_t base_addr, uint32_t offset_key, uint32_t offset_default, uint32_t size_key,
                    uint32_t size_default, MutableByteSpan & value);
    bool _cd_set = false;
    bool _pai_set = false;
    bool _dac_set = false;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
