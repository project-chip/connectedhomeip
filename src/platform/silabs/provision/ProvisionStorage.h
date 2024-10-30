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
#pragma once

#include "ProvisionStorageGeneric.h"
#include <credentials/DeviceAttestationCredsProvider.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>

#include <app/data-model/Nullable.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Base64.h>
#include <lib/support/Span.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

namespace Parameters {

enum ID : uint16_t
{
    // Internal,
    kFlashAddress  = 0x0101,
    kFlashSize     = 0x0102,
    kFlashPageSize = 0x0103,
    kCredsAddress  = 0x0104,
    kCsrFile       = 0x0105,
    // Options,
    kVersion       = 0x0111,
    kAction        = 0x0112,
    kExtra         = 0x0113,
    kStop          = 0x0114,
    kParamsPath    = 0x0121,
    kInputsPath    = 0x0122,
    kOutputPath    = 0x0123,
    kTemporaryDir  = 0x0124,
    kTargetDevice  = 0x0131,
    kChannel       = 0x0132,
    kGenerateCreds = 0x0133,
    kCsrMode       = 0x0134,
    kGeneratorFW   = 0x0135,
    kProductionFW  = 0x0136,
    kCertToolPath  = 0x0137,
    kPylinkLib     = 0x013a,
    kBufferSize    = 0x013b,
    // Instance Info,
    kSerialNumber       = 0x0141,
    kVendorId           = 0x0142,
    kVendorName         = 0x0143,
    kProductId          = 0x0144,
    kProductName        = 0x0145,
    kProductLabel       = 0x0146,
    kProductUrl         = 0x0147,
    kPartNumber         = 0x0148,
    kHwVersion          = 0x0151,
    kHwVersionStr       = 0x0152,
    kManufacturingDate  = 0x0153,
    kPersistentUniqueId = 0x0154,
    // Commissionable Data,
    kDiscriminator     = 0x0161,
    kSpake2pPasscode   = 0x0162,
    kSpake2pIterations = 0x0163,
    kSpake2pSalt       = 0x0164,
    kSpake2pVerifier   = 0x0165,
    kSetupPayload      = 0x0166,
    kCommissioningFlow = 0x0167,
    kRendezvousFlags   = 0x0168,
    // Attestation Credentials,
    kFirmwareInfo        = 0x0181,
    kCertification       = 0x0182,
    kCdCert              = 0x0183,
    kCdKey               = 0x0184,
    kPaaCert             = 0x0191,
    kPaaKey              = 0x0192,
    kPaiCert             = 0x0193,
    kPaiKey              = 0x0194,
    kDacCert             = 0x0195,
    kDacKey              = 0x0196,
    kKeyId               = 0x0197,
    kKeyPass             = 0x0198,
    kPKCS12              = 0x0199,
    kCommonName          = 0x01a1,
    kOtaTlvEncryptionKey = 0x01a2,
};

} // namespace Parameters

struct CustomStorage : public GenericStorage
{
    CHIP_ERROR Set(uint16_t id, const uint8_t * value) override;
    CHIP_ERROR Get(uint16_t id, uint8_t & value) override;
    CHIP_ERROR Set(uint16_t id, const uint16_t * value) override;
    CHIP_ERROR Get(uint16_t id, uint16_t & value) override;
    CHIP_ERROR Set(uint16_t id, const uint32_t * value) override;
    CHIP_ERROR Get(uint16_t id, uint32_t & value) override;
    CHIP_ERROR Set(uint16_t id, const uint64_t * value) override;
    CHIP_ERROR Get(uint16_t id, uint64_t & value) override;
    CHIP_ERROR Get(uint16_t id, uint8_t * value, size_t max_size, size_t & size) override;
    CHIP_ERROR Set(uint16_t id, const uint8_t * value, size_t size) override;
};

namespace {
constexpr size_t kVersionFieldLengthInBits              = 3;
constexpr size_t kVendorIDFieldLengthInBits             = 16;
constexpr size_t kProductIDFieldLengthInBits            = 16;
constexpr size_t kCommissioningFlowFieldLengthInBits    = 2;
constexpr size_t kRendezvousInfoFieldLengthInBits       = 8;
constexpr size_t kPayloadDiscriminatorFieldLengthInBits = 12;
constexpr size_t kSetupPINCodeFieldLengthInBits         = 27;
constexpr size_t kPaddingFieldLengthInBits              = 4;
} // namespace

struct Storage : public GenericStorage,
                 public chip::DeviceLayer::DeviceInstanceInfoProvider,
                 public chip::DeviceLayer::CommissionableDataProvider,
                 public chip::Credentials::DeviceAttestationCredentialsProvider
{
    static constexpr size_t kArgumentSizeMax             = 512;
    static constexpr size_t kVersionLengthMax            = 16;
    static constexpr size_t kSerialNumberLengthMax       = 32;
    static constexpr size_t kVendorNameLengthMax         = 32;
    static constexpr size_t kProductNameLengthMax        = 32;
    static constexpr size_t kProductLabelLengthMax       = 32;
    static constexpr size_t kProductUrlLengthMax         = 32;
    static constexpr size_t kPartNumberLengthMax         = 32;
    static constexpr size_t kHardwareVersionStrLengthMax = 32;
    static constexpr size_t kManufacturingDateLengthMax  = 11; // yyyy-mm-dd + \0
    static constexpr size_t kPersistentUniqueIdMaxLength = 16;
    static constexpr size_t kSpake2pVerifierB64LengthMax = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;
    static constexpr size_t kSpake2pSaltB64LengthMax     = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;
    static constexpr size_t kFirmwareInfoSizeMax         = 32;
    static constexpr size_t kCertificationSizeMax        = 350;
    static constexpr size_t kDeviceAttestationKeySizeMax = 128;
    static constexpr size_t kSetupPayloadSizeMax         = 32;
    static constexpr size_t kCsrLengthMax                = 512;
    static constexpr size_t kCommonNameMax               = 128;
    static constexpr size_t kTotalPayloadDataSizeInBits =
        (kVersionFieldLengthInBits + kVendorIDFieldLengthInBits + kProductIDFieldLengthInBits +
         kCommissioningFlowFieldLengthInBits + kRendezvousInfoFieldLengthInBits + kPayloadDiscriminatorFieldLengthInBits +
         kSetupPINCodeFieldLengthInBits + kPaddingFieldLengthInBits);
    static constexpr size_t kTotalPayloadDataSize = kTotalPayloadDataSizeInBits / 8;

public:
    friend class Manager;
    friend class Protocol1;
    friend class Command;
    friend class CsrCommand;
    friend class ReadCommand;
    friend class WriteCommand;

    //
    // Initialization
    //

    CHIP_ERROR Initialize(uint32_t flash_addr = 0, uint32_t flash_size = 0);
    CHIP_ERROR Commit();

    //
    // Generic Interface
    //

    CHIP_ERROR Get(uint16_t id, uint8_t & value) override;
    CHIP_ERROR Get(uint16_t id, uint16_t & value) override;
    CHIP_ERROR Get(uint16_t id, uint32_t & value) override;
    CHIP_ERROR Get(uint16_t id, uint64_t & value) override;
    CHIP_ERROR Get(uint16_t id, uint8_t * value, size_t max_size, size_t & size) override;

    //
    // DeviceInstanceInfoProvider
    //

    CHIP_ERROR GetSerialNumber(char * value, size_t max);
    CHIP_ERROR GetVendorId(uint16_t & value);
    CHIP_ERROR GetVendorName(char * value, size_t max);
    CHIP_ERROR GetProductId(uint16_t & productId);
    CHIP_ERROR GetProductName(char * value, size_t max);
    CHIP_ERROR GetProductLabel(char * value, size_t max);
    CHIP_ERROR GetProductURL(char * value, size_t max);
    CHIP_ERROR GetPartNumber(char * value, size_t max);
    CHIP_ERROR GetHardwareVersion(uint16_t & value);
    CHIP_ERROR GetHardwareVersionString(char * value, size_t max);
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day);
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & value);

    //
    // CommissionableDataProvider
    //

    CHIP_ERROR GetSetupDiscriminator(uint16_t & value);
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & value);
    CHIP_ERROR GetSetupPasscode(uint32_t & value);
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & value);
    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & value, size_t & size);

    //
    // DeviceAttestationCredentialsProvider
    //

    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & value);
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & value);
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & value);
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & value);
    CHIP_ERROR GetDeviceAttestationCSR(uint16_t vid, uint16_t pid, const CharSpan & cn, MutableCharSpan & csr);
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature);

    CHIP_ERROR SetCertificationDeclaration(const ByteSpan & value);
    CHIP_ERROR SetProductAttestationIntermediateCert(const ByteSpan & value);
    CHIP_ERROR SetDeviceAttestationCert(const ByteSpan & value);
    CHIP_ERROR SetDeviceAttestationKey(const ByteSpan & value);
    //
    // Other
    //

    CHIP_ERROR SetCredentialsBaseAddress(uint32_t addr);
    CHIP_ERROR GetCredentialsBaseAddress(uint32_t & addr);
    CHIP_ERROR GetSetupPayload(chip::MutableCharSpan & value);
    CHIP_ERROR SetProvisionRequest(bool value);
    CHIP_ERROR GetProvisionRequest(bool & value);
    CHIP_ERROR GetTestEventTriggerKey(MutableByteSpan & keySpan);
    void SetBufferSize(size_t size) { mBufferSize = size > 0 ? size : kArgumentSizeMax; }
    size_t GetBufferSize() { return mBufferSize; }

private:
    // Generic Interface
    CHIP_ERROR Set(uint16_t id, const uint8_t * value) override;
    CHIP_ERROR Set(uint16_t id, const uint16_t * value) override;
    CHIP_ERROR Set(uint16_t id, const uint32_t * value) override;
    CHIP_ERROR Set(uint16_t id, const uint64_t * value) override;
    CHIP_ERROR Set(uint16_t id, const uint8_t * value, size_t size) override;
    // DeviceInstanceInfoProvider
    CHIP_ERROR SetSerialNumber(const char * value, size_t len);
    CHIP_ERROR SetVendorId(uint16_t value);
    CHIP_ERROR SetVendorName(const char * value, size_t len);
    CHIP_ERROR SetProductId(uint16_t productId);
    CHIP_ERROR SetProductName(const char * value, size_t len);
    CHIP_ERROR SetProductLabel(const char * value, size_t len);
    CHIP_ERROR SetProductURL(const char * value, size_t len);
    CHIP_ERROR SetPartNumber(const char * value, size_t len);
    CHIP_ERROR SetHardwareVersion(uint16_t value);
    CHIP_ERROR SetHardwareVersionString(const char * value, size_t len);
    CHIP_ERROR SetManufacturingDate(const char * value, size_t len);
    CHIP_ERROR GetManufacturingDate(uint8_t * value, size_t max, size_t & size);
    // PersistentUniqueId is used to generate the RotatingUniqueId
    // This PersistentUniqueId SHALL NOT be the same as the UniqueID attribute exposed in the Basic Information cluster.
    CHIP_ERROR SetPersistentUniqueId(const uint8_t * value, size_t size);
    CHIP_ERROR GetPersistentUniqueId(uint8_t * value, size_t max, size_t & size);
    // CommissionableDataProvider
    CHIP_ERROR SetSetupDiscriminator(uint16_t value);
    CHIP_ERROR SetSpake2pIterationCount(uint32_t value);
    CHIP_ERROR SetSetupPasscode(uint32_t value);
    CHIP_ERROR SetSpake2pSalt(const char * value, size_t size);
    CHIP_ERROR GetSpake2pSalt(char * value, size_t max, size_t & size);
    CHIP_ERROR SetSpake2pVerifier(const char * value, size_t size);
    CHIP_ERROR GetSpake2pVerifier(char * value, size_t max, size_t & size);
    // DeviceAttestationCredentialsProvider
    CHIP_ERROR SetFirmwareInformation(const ByteSpan & value);

    // Other
    CHIP_ERROR SetProvisionVersion(const char * value, size_t len);
    CHIP_ERROR GetProvisionVersion(char * value, size_t max, size_t & size);
    CHIP_ERROR SetSetupPayload(const uint8_t * value, size_t size);
    CHIP_ERROR GetSetupPayload(uint8_t * value, size_t max, size_t & size);
#if OTA_ENCRYPTION_ENABLE
    CHIP_ERROR SetOtaTlvEncryptionKey(const ByteSpan & value);
#endif

    uint16_t mVendorId               = 0;
    uint16_t mProductId              = 0;
    uint16_t mHwVersion              = 0;
    uint16_t mDiscriminator          = 0; // 12-bit
    uint32_t mCommissioningFlow      = 0;
    uint32_t mRendezvousFlags        = 0;
    uint32_t mPasscode               = 0;
    uint32_t mKeyId                  = 0;
    uint32_t mBufferSize             = kArgumentSizeMax;
    char mCommonName[kCommonNameMax] = { 0 };
    CustomStorage mCustom;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
