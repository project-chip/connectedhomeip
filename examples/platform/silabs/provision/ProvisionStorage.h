#pragma once

#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/CommissionableDataProvider.h>
#include <credentials/DeviceAttestationCredsProvider.h>

#include <lib/support/Span.h>
#include <lib/core/CHIPError.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Base64.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

static constexpr size_t kSerialNumberLengthMax          = 32;
static constexpr size_t kVendorNameLengthMax            = 32;
static constexpr size_t kProductNameLengthMax           = 32;
static constexpr size_t kProductLabelLengthMax          = 32;
static constexpr size_t kProductUrlLengthMax            = 32;
static constexpr size_t kPartNumberLengthMax            = 32;
static constexpr size_t kHardwareVersionStrLengthMax    = 32;
static constexpr size_t kManufacturingDateLengthMax     = 11; // yyyy-mm-dd + \0
static constexpr size_t kUniqueIdLengthMax              = 16;
static constexpr size_t kSpake2pVerifierB64LengthMax    = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;
static constexpr size_t kSpake2pSaltB64LengthMax        = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;
static constexpr size_t kFirmwareInfoSizeMax            = 32;
static constexpr size_t kCertificateSizeMax             = 500;
static constexpr size_t kCertificationSizeMax           = 350;
static constexpr size_t kDeviceAttestationKeySizeMax    = 128;
static constexpr size_t kSetupPayloadSizeMax            = 32;

static constexpr size_t kVersionFieldLengthInBits = 3;
static constexpr size_t kVendorIDFieldLengthInBits = 16;
static constexpr size_t kProductIDFieldLengthInBits = 16;
static constexpr size_t kCommissioningFlowFieldLengthInBits = 2;
static constexpr size_t kRendezvousInfoFieldLengthInBits = 8;
static constexpr size_t kPayloadDiscriminatorFieldLengthInBits = 12;
static constexpr size_t kSetupPINCodeFieldLengthInBits = 27;
static constexpr size_t kPaddingFieldLengthInBits = 4;
static constexpr size_t kTotalPayloadDataSizeInBits = (
                            kVersionFieldLengthInBits + kVendorIDFieldLengthInBits +
                            kProductIDFieldLengthInBits + kCommissioningFlowFieldLengthInBits +
                            kRendezvousInfoFieldLengthInBits + kPayloadDiscriminatorFieldLengthInBits +
                            kSetupPINCodeFieldLengthInBits + kPaddingFieldLengthInBits);
static constexpr size_t kTotalPayloadDataSize = kTotalPayloadDataSizeInBits / 8;

struct Storage: public DeviceInstanceInfoProvider,
                public CommissionableDataProvider,
                public chip::Credentials::DeviceAttestationCredentialsProvider
{
    friend class Manager;
    friend class CsrCommand;
    friend class ImportCommand;
    friend class SetupCommand;
public:

    virtual ~Storage() = default;

    //
    // Initialization
    //

    virtual CHIP_ERROR Initialize(uint32_t flash_addr, uint32_t flash_size) = 0;
    virtual CHIP_ERROR GetBaseAddress(uint32_t & value) = 0;

    //
    // DeviceInstanceInfoProvider
    //

    virtual CHIP_ERROR GetSerialNumber(char * value, size_t max) = 0;
    virtual CHIP_ERROR GetVendorId(uint16_t & value) = 0;
    virtual CHIP_ERROR GetVendorName(char * value, size_t max) = 0;
    virtual CHIP_ERROR GetProductId(uint16_t & productId) = 0;
    virtual CHIP_ERROR GetProductName(char * value, size_t max) = 0;
    virtual CHIP_ERROR GetProductLabel(char * value, size_t max) = 0;
    virtual CHIP_ERROR GetProductURL(char * value, size_t max) = 0;
    virtual CHIP_ERROR GetPartNumber(char * value, size_t max) = 0;
    virtual CHIP_ERROR GetHardwareVersion(uint16_t & value) = 0;
    virtual CHIP_ERROR GetHardwareVersionString(char * value, size_t max) = 0;
    virtual CHIP_ERROR GetManufacturingDate(char * value, size_t max) = 0;
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day);
    virtual CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & value) = 0;

    //
    // CommissionableDataProvider
    //

    virtual CHIP_ERROR GetSetupDiscriminator(uint16_t & value) = 0;
    virtual CHIP_ERROR GetSpake2pIterationCount(uint32_t & value) = 0;
    virtual CHIP_ERROR GetSetupPasscode(uint32_t & value) = 0;
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & value);
    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & value, size_t & size);

    //
    // DeviceAttestationCredentialsProvider
    //

    virtual CHIP_ERROR GetFirmwareInformation(MutableByteSpan & value) = 0;
    virtual CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & value) = 0;
    virtual CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & value) = 0;
    virtual CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & value) = 0;
    virtual CHIP_ERROR GetDeviceAttestationCSR(uint32_t kid, uint16_t vid, uint16_t pid, const CharSpan &cn, MutableCharSpan & csr) = 0;
    virtual CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature) = 0;

    //
    // Other
    //

    // Setup Payload
    CHIP_ERROR GetSetupPayload(chip::MutableCharSpan &value);

    // Provision Request
    virtual CHIP_ERROR SetProvisionRequest(bool value) = 0;
    virtual CHIP_ERROR GetProvisionRequest(bool &value) = 0;

protected:
    virtual CHIP_ERROR SetSerialNumber(const char * value, size_t len) = 0;
    virtual CHIP_ERROR SetVendorId(uint16_t value) = 0;
    virtual CHIP_ERROR SetVendorName(const char * value, size_t len) = 0;
    virtual CHIP_ERROR SetProductId(uint16_t productId) = 0;
    virtual CHIP_ERROR SetProductName(const char * value, size_t len) = 0;
    virtual CHIP_ERROR SetProductLabel(const char * value, size_t len) = 0;
    virtual CHIP_ERROR SetProductURL(const char * value, size_t len) = 0;
    virtual CHIP_ERROR SetPartNumber(const char * value, size_t len) = 0;
    virtual CHIP_ERROR SetHardwareVersion(uint16_t & value) = 0;
    virtual CHIP_ERROR SetHardwareVersionString(const char * value, size_t len) = 0;
    virtual CHIP_ERROR SetManufacturingDate(const char * value, size_t len) = 0;
    virtual CHIP_ERROR SetUniqueId(const uint8_t * value, size_t size) = 0;

    virtual CHIP_ERROR SetSetupDiscriminator(uint16_t value) = 0;
    virtual CHIP_ERROR SetSpake2pIterationCount(uint32_t value) = 0;
    virtual CHIP_ERROR SetSetupPasscode(uint32_t value) = 0;
    virtual CHIP_ERROR SetSpake2pSalt(const char * value, size_t size) = 0;
    virtual CHIP_ERROR GetSpake2pSalt(char * value, size_t max, size_t &size) = 0;
    virtual CHIP_ERROR SetSpake2pVerifier(const char * value, size_t size) = 0;
    virtual CHIP_ERROR GetSpake2pVerifier(char * value, size_t max, size_t &size) = 0;

    virtual CHIP_ERROR SetFirmwareInformation(const ByteSpan & value) = 0;
    virtual CHIP_ERROR SetCertificationDeclaration(const ByteSpan & value) = 0;
    virtual CHIP_ERROR SetProductAttestationIntermediateCert(const ByteSpan & value) = 0;
    virtual CHIP_ERROR SetDeviceAttestationCert(const ByteSpan & value) = 0;
    virtual CHIP_ERROR SetDeviceAttestationKey(uint32_t kid, const ByteSpan & value) = 0;

    virtual CHIP_ERROR SetSetupPayload(const uint8_t * value, size_t size) = 0;
    virtual CHIP_ERROR GetSetupPayload(uint8_t * value, size_t max, size_t &size) = 0;

};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
