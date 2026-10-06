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
#include <app/TestEventTriggerDelegate.h>
#include <headers/ProvisionStorage.h>
#include <lib/support/CodeUtils.h>
#include <provision/ProvisionStorageWriter.h>
#include <provision/flash/ProvisionStorageFlash.h>
#if defined(SL_MATTER_ENABLE_OTA_ENCRYPTION) && SL_MATTER_ENABLE_OTA_ENCRYPTION
#include <platform/silabs/multi-ota/OtaTlvEncryptionKey.h>
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
CHIP_ERROR ProvisionStorageWriter::Initialize(uint32_t flashAddress, uint32_t flashSize)
{
    return Flash::Initialize(flashAddress, flashSize);
}
CHIP_ERROR ProvisionStorageWriter::Commit()
{
    return Flash::Commit();
}
CHIP_ERROR ProvisionStorageWriter::GetFlashPageSize(uint32_t & size)
{
    return Flash::GetFlashPageSize(size);
}
CHIP_ERROR ProvisionStorageWriter::SetCredentialsBaseAddress(uint32_t address)
{
    return Flash::SetCredentialsBaseAddress(address);
}
CHIP_ERROR ProvisionStorageWriter::GetCredentialsBaseAddress(uint32_t & address)
{
    return Flash::GetCredentialsBaseAddress(address);
}

CHIP_ERROR ProvisionStorageWriter::SetSerialNumber(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kSerialNumber, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetVendorId(uint16_t value)
{
    return Flash::Set(Parameters::ID::kVendorId, value);
}

CHIP_ERROR ProvisionStorageWriter::SetVendorName(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kVendorName, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetProductId(uint16_t value)
{
    return Flash::Set(Parameters::ID::kProductId, value);
}

CHIP_ERROR ProvisionStorageWriter::SetProductName(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kProductName, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetProductLabel(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kProductLabel, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetProductURL(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kProductUrl, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetPartNumber(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kPartNumber, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetHardwareVersion(uint16_t value)
{
    return Flash::Set(Parameters::ID::kHwVersion, value);
}

CHIP_ERROR ProvisionStorageWriter::SetHardwareVersionString(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kHwVersionStr, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetManufacturingDate(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kManufacturingDate, value, len);
}

CHIP_ERROR ProvisionStorageWriter::SetPersistentUniqueId(const uint8_t * value, size_t size)
{
    return Flash::Set(Parameters::ID::kPersistentUniqueId, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetSetupDiscriminator(uint16_t value)
{
    return Flash::Set(Parameters::ID::kDiscriminator, value);
}

CHIP_ERROR ProvisionStorageWriter::SetSpake2pIterationCount(uint32_t value)
{
    return Flash::Set(Parameters::ID::kSpake2pIterations, value);
}

CHIP_ERROR ProvisionStorageWriter::SetSpake2pSalt(const char * value, size_t size)
{
    return Flash::Set(Parameters::ID::kSpake2pSalt, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetSpake2pVerifier(const char * value, size_t size)
{
    return Flash::Set(Parameters::ID::kSpake2pVerifier, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetFirmwareInformation(const ByteSpan & value)
{
    (void) value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageWriter::SetCertificationDeclaration(const ByteSpan & value)
{
    return Flash::Set(Parameters::ID::kCertification, value.data(), value.size());
}

CHIP_ERROR ProvisionStorageWriter::SetProductAttestationIntermediateCert(const ByteSpan & value)
{
    return Flash::Set(Parameters::ID::kPaiCert, value.data(), value.size());
}

CHIP_ERROR ProvisionStorageWriter::SetDeviceAttestationCert(const ByteSpan & value)
{
    return Flash::Set(Parameters::ID::kDacCert, value.data(), value.size());
}

CHIP_ERROR ProvisionStorageWriter::SetProvisionVersion(const char * value, size_t size)
{
    return Flash::Set(Parameters::ID::kVersion, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetSetupPayload(const uint8_t * value, size_t size)
{
    return Flash::Set(Parameters::ID::kSetupPayload, value, size);
}

CHIP_ERROR ProvisionStorageWriter::SetProvisionRequest(bool value)
{
    // return Flash::Set(Parameters::ID::kProvisionRequest, value);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageWriter::GetProvisionRequest(bool & value)
{
    // return Flash::Set(Parameters::ID::kProvisionRequest, value);
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ProvisionStorageWriter::SetOtaTlvEncryptionKey(const ByteSpan & value)
{
#if defined(SL_MATTER_ENABLE_OTA_ENCRYPTION) && SL_MATTER_ENABLE_OTA_ENCRYPTION
#if defined(SL_MBEDTLS_USE_TINYCRYPT) && SL_MBEDTLS_USE_TINYCRYPT
    return Flash::Set(Parameters::ID::kOtaTlvEncryptionKey, value.data(), value.size());
#else
    Silabs::OtaTlvEncryptionKey key;
    ReturnErrorOnFailure(key.Import(value.data(), value.size()));
    return Flash::Set(Parameters::ID::kOtaTlvEncryptionKey, key.GetId());
#endif
#else
    (void) value;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}
CHIP_ERROR ProvisionStorageWriter::SetTestEventTriggerKey(const ByteSpan & value)
{
#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
    VerifyOrReturnError(value.size() == TestEventTriggerDelegate::kEnableKeyLength, CHIP_ERROR_INVALID_ARGUMENT);
    return Flash::Set(Parameters::ID::kTestEventTriggerKey, value.data(), value.size());
#else
    (void) value;
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
}
} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
