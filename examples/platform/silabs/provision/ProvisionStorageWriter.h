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

#include <headers/ProvisionStorageInterfaces.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

/**
 * Silicon Labs factory-data storage writer.
 *
 * The selected storage backend implements this class. Provisioning
 * applications such as GFW consume this writable capability.
 */
class ProvisionStorageWriter final : public IProvisionStorageWriter
{
public:
    static ProvisionStorageWriter & GetInstance();

    ProvisionStorageWriter(const ProvisionStorageWriter &)             = delete;
    ProvisionStorageWriter & operator=(const ProvisionStorageWriter &) = delete;

    CHIP_ERROR Initialize(uint32_t flash_addr = 0, uint32_t flash_size = 0) override;
    CHIP_ERROR Commit() override;
    CHIP_ERROR GetFlashPageSize(uint32_t & size) override;

    CHIP_ERROR SetSerialNumber(const char * value, size_t len) override;
    CHIP_ERROR SetVendorId(uint16_t value) override;
    CHIP_ERROR SetVendorName(const char * value, size_t len) override;
    CHIP_ERROR SetProductId(uint16_t value) override;
    CHIP_ERROR SetProductName(const char * value, size_t len) override;
    CHIP_ERROR SetProductLabel(const char * value, size_t len) override;
    CHIP_ERROR SetProductURL(const char * value, size_t len) override;
    CHIP_ERROR SetPartNumber(const char * value, size_t len) override;
    CHIP_ERROR SetHardwareVersion(uint16_t value) override;
    CHIP_ERROR SetHardwareVersionString(const char * value, size_t len) override;
    CHIP_ERROR SetManufacturingDate(const char * value, size_t len) override;
    CHIP_ERROR SetPersistentUniqueId(const uint8_t * value, size_t size) override;

    CHIP_ERROR SetSetupDiscriminator(uint16_t value) override;
    CHIP_ERROR SetSpake2pIterationCount(uint32_t value) override;
    CHIP_ERROR SetSpake2pSalt(const char * value, size_t size) override;
    CHIP_ERROR SetSpake2pVerifier(const char * value, size_t size) override;
    CHIP_ERROR SetSetupPayload(const uint8_t * value, size_t size) override;

    CHIP_ERROR SetFirmwareInformation(const ByteSpan & value) override;
    CHIP_ERROR SetCertificationDeclaration(const ByteSpan & value) override;
    CHIP_ERROR SetProductAttestationIntermediateCert(const ByteSpan & value) override;
    CHIP_ERROR SetDeviceAttestationCert(const ByteSpan & value) override;

    CHIP_ERROR SetProvisionVersion(const char * value, size_t len) override;
    CHIP_ERROR SetTestEventTriggerKey(const ByteSpan & value) override;
    CHIP_ERROR SetOtaTlvEncryptionKey(const ByteSpan & value) override;

    // Provisioning/platform lifecycle controls.
    CHIP_ERROR SetCredentialsBaseAddress(uint32_t addr) override;
    CHIP_ERROR SetProvisionRequest(bool value) override;
    CHIP_ERROR GetCredentialsBaseAddress(uint32_t & addr) override;
    CHIP_ERROR GetProvisionRequest(bool & value) override;

private:
    ProvisionStorageWriter() = default;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
