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
#pragma once

#include <credentials/DeviceAttestationCredsProvider.h>
#include <crypto/CHIPCryptoPAL.h>
#include <headers/ProvisionStorageInterfaces.h>
#include <lib/support/Base64.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

/**
 * Silicon Labs factory-data storage reader.
 *
 * The selected storage backend implements this class. It is both the read-only
 * persistence capability consumed by Provision Core and the Matter-facing
 * provider used by normal applications.
 */
class ProvisionStorageReader final : public IProvisionStorageReader,
                                     public chip::DeviceLayer::DeviceInstanceInfoProvider,
                                     public chip::DeviceLayer::CommissionableDataProvider,
                                     public chip::Credentials::DeviceAttestationCredentialsProvider
{
public:
    static ProvisionStorageReader & GetInstance();

    ProvisionStorageReader(const ProvisionStorageReader &)             = delete;
    ProvisionStorageReader & operator=(const ProvisionStorageReader &) = delete;

    // Methods whose signatures are shared by the persistence and Matter APIs.
    CHIP_ERROR GetSerialNumber(char * value, size_t max) override;
    CHIP_ERROR GetVendorId(uint16_t & value) override;
    CHIP_ERROR GetVendorName(char * value, size_t max) override;
    CHIP_ERROR GetProductId(uint16_t & value) override;
    CHIP_ERROR GetProductName(char * value, size_t max) override;
    CHIP_ERROR GetProductLabel(char * value, size_t max) override;
    CHIP_ERROR GetProductURL(char * value, size_t max) override;
    CHIP_ERROR GetPartNumber(char * value, size_t max) override;
    CHIP_ERROR GetHardwareVersion(uint16_t & value) override;
    CHIP_ERROR GetHardwareVersionString(char * value, size_t max) override;
    CHIP_ERROR GetManufacturingDate(uint8_t * value, size_t max, size_t & size) override;
    CHIP_ERROR GetPersistentUniqueId(uint8_t * value, size_t max, size_t & size) override;

    CHIP_ERROR GetSetupDiscriminator(uint16_t & value) override;
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & value) override;
    CHIP_ERROR GetSpake2pSalt(char * value, size_t max, size_t & size) override;
    CHIP_ERROR GetSpake2pVerifier(char * value, size_t max, size_t & size) override;
    CHIP_ERROR GetSetupPayload(uint8_t * value, size_t max, size_t & size) override;

    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & value) override;
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & value) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & value) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & value) override;

    CHIP_ERROR GetProvisionVersion(char * value, size_t max, size_t & size) override;

    // Matter-facing interpretation of persisted factory data.
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override;
    CHIP_ERROR GetManufacturingDateSuffix(MutableCharSpan & suffixBuffer) override;
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & value) override;
    CHIP_ERROR SetSetupDiscriminator(uint16_t value) override;
    CHIP_ERROR GetSetupPasscode(uint32_t & value) override;
    CHIP_ERROR SetSetupPasscode(uint32_t value) override;
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & value) override;
    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & value, size_t & size) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature) override;

    // Common method to read the setup payload as mutable byte span.
    CHIP_ERROR GetSetupPayload(MutableCharSpan & value);

    // Silicon Labs provisioned-data access used by optional platform features.
    CHIP_ERROR GetTestEventTriggerKey(MutableByteSpan & keySpan) override;
    CHIP_ERROR GetOtaTlvEncryptionKeyId(uint32_t & value) override;
    CHIP_ERROR DecryptUsingOtaTlvEncryptionKey(MutableByteSpan & block, uint32_t & ivOffset) override;

private:
    ProvisionStorageReader() = default;

    // Provisioning stores this as YYYYMMDD followed by an optional hhmmssxx suffix.
    static constexpr size_t kManufacturingDateBufferSize = sizeof("yyyymmddhhmmssxx");
    static constexpr size_t kSpake2pSaltB64BufferSize    = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;
    static constexpr size_t kSpake2pVerifierB64BufferSize =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;
    static constexpr size_t kSpake2pSaltDecodedBufferSize     = BASE64_MAX_DECODED_LEN(kSpake2pSaltB64BufferSize);
    static constexpr size_t kSpake2pVerifierDecodedBufferSize = BASE64_MAX_DECODED_LEN(kSpake2pVerifierB64BufferSize);
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
