/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <ProvisionStorage.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

CHIP_ERROR Storage::Initialize(uint32_t flash_addr, uint32_t flash_size)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::Commit()
{
    return CHIP_NO_ERROR;
}

//
// Generic Interface
//

CHIP_ERROR Storage::Get(uint16_t id, uint8_t & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::Get(uint16_t id, uint16_t & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::Get(uint16_t id, uint32_t & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::Get(uint16_t id, uint64_t & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::Get(uint16_t id, uint8_t * value, size_t max_size, size_t & size)
{
    return CHIP_NO_ERROR;
}

//
// DeviceInstanceInfoProvider
//

CHIP_ERROR Storage::GetSerialNumber(char * value, size_t max)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetVendorId(uint16_t & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetVendorName(char * value, size_t max)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetProductId(uint16_t & productId)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetProductName(char * value, size_t max)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetProductLabel(char * value, size_t max)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetProductURL(char * value, size_t max)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetPartNumber(char * value, size_t max)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetHardwareVersion(uint16_t & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetHardwareVersionString(char * value, size_t max)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetRotatingDeviceIdUniqueId(MutableByteSpan & value)
{
    return CHIP_NO_ERROR;
}

//
// CommissionableDataProvider
//

CHIP_ERROR Storage::GetSetupDiscriminator(uint16_t & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetSpake2pIterationCount(uint32_t & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetSetupPasscode(uint32_t & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetSpake2pSalt(MutableByteSpan & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetSpake2pVerifier(MutableByteSpan & value, size_t & size)
{
    return CHIP_NO_ERROR;
}

//
// DeviceAttestationCredentialsProvider
//

CHIP_ERROR Storage::GetFirmwareInformation(MutableByteSpan & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetCertificationDeclaration(MutableByteSpan & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetProductAttestationIntermediateCert(MutableByteSpan & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetDeviceAttestationCert(MutableByteSpan & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetDeviceAttestationCSR(uint16_t vid, uint16_t pid, const CharSpan & cn, MutableCharSpan & csr)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetCertificationDeclaration(const ByteSpan & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetProductAttestationIntermediateCert(const ByteSpan & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetDeviceAttestationCert(const ByteSpan & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetDeviceAttestationKey(const ByteSpan & value)
{
    return CHIP_NO_ERROR;
}
//
// Other
//

CHIP_ERROR Storage::SetCredentialsBaseAddress(uint32_t addr)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetCredentialsBaseAddress(uint32_t & addr)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetSetupPayload(chip::MutableCharSpan & value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetProvisionRequest(bool value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetProvisionRequest(bool & value)
{
    return CHIP_NO_ERROR;
}

#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
/**
 * @brief Reads the test event trigger key from NVM. If the key isn't present, returns default value if defined.
 *
 * @param[out] keySpan output buffer. Must be at least large enough for 16 bytes (key length)
 * @return CHIP_ERROR
 */
CHIP_ERROR Storage::GetTestEventTriggerKey(MutableByteSpan & keySpan)
{
    return CHIP_NO_ERROR;
}
#endif // SL_MATTER_TEST_EVENT_TRIGGER_ENABLED

// Generic Interface
CHIP_ERROR Storage::Set(uint16_t id, const uint8_t * value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::Set(uint16_t id, const uint16_t * value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::Set(uint16_t id, const uint32_t * value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::Set(uint16_t id, const uint64_t * value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::Set(uint16_t id, const uint8_t * value, size_t size)
{
    return CHIP_NO_ERROR;
}
// DeviceInstanceInfoProvider
CHIP_ERROR Storage::SetSerialNumber(const char * value, size_t len)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetVendorId(uint16_t value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetVendorName(const char * value, size_t len)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetProductId(uint16_t productId)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetProductName(const char * value, size_t len)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetProductLabel(const char * value, size_t len)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetProductURL(const char * value, size_t len)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetPartNumber(const char * value, size_t len)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetHardwareVersion(uint16_t value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetHardwareVersionString(const char * value, size_t len)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetManufacturingDate(const char * value, size_t len)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetManufacturingDate(uint8_t * value, size_t max, size_t & size)
{
    return CHIP_NO_ERROR;
}
// PersistentUniqueId is used to generate the RotatingUniqueId
// This PersistentUniqueId SHALL NOT be the same as the UniqueID attribute exposed in the Basic Information cluster.
CHIP_ERROR Storage::SetPersistentUniqueId(const uint8_t * value, size_t size)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetPersistentUniqueId(uint8_t * value, size_t max, size_t & size)
{
    return CHIP_NO_ERROR;
}
// CommissionableDataProvider
CHIP_ERROR Storage::SetSetupDiscriminator(uint16_t value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetSpake2pIterationCount(uint32_t value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetSetupPasscode(uint32_t value)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetSpake2pSalt(const char * value, size_t size)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetSpake2pSalt(char * value, size_t max, size_t & size)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetSpake2pVerifier(const char * value, size_t size)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetSpake2pVerifier(char * value, size_t max, size_t & size)
{
    return CHIP_NO_ERROR;
}
// DeviceAttestationCredentialsProvider
CHIP_ERROR Storage::SetFirmwareInformation(const ByteSpan & value)
{
    return CHIP_NO_ERROR;
}

// Other
CHIP_ERROR Storage::SetProvisionVersion(const char * value, size_t len)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetProvisionVersion(char * value, size_t max, size_t & size)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::SetSetupPayload(const uint8_t * value, size_t size)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR Storage::GetSetupPayload(uint8_t * value, size_t max, size_t & size)
{
    return CHIP_NO_ERROR;
}

#if OTA_ENCRYPTION_ENABLE
CHIP_ERROR Storage::SetOtaTlvEncryptionKey(const ByteSpan & value)
{
    return CHIP_NO_ERROR;
}
#endif // OTA_ENCRYPTION_ENABLE

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
