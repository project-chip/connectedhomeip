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
#include "ProvisionEncoder.h"
#include "ProvisionStorage.h"
#include <algorithm>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/silabs/SilabsConfig.h>
#include <string.h>
#ifdef OTA_ENCRYPTION_ENABLE
#include <platform/silabs/multi-ota/OtaTlvEncryptionKey.h>
#endif // OTA_ENCRYPTION_ENABLE

using namespace chip::Credentials;

#if SLI_SI91X_MCU_INTERFACE
// TODO: Remove this once the flash header integrates these definitions
#define FLASH_ERASE 1 // flash_sector_erase_enable value for erase operation
#define FLASH_WRITE 0 // flash_sector_erase_enable value for write operation

#define NWP_FLASH_ADDRESS (0x0000000)
#define NWP_FLASH_SIZE (0x0004E20)

#include <sl_status.h>
extern "C" {
#include <sl_net.h>
#include <sl_net_constants.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_device.h>
}

#else // SLI_SI91X_MCU_INTERFACE
#include <em_msc.h>
extern uint8_t linker_nvm_end[];
#endif // SLI_SI91X_MCU_INTERFACE

namespace {
constexpr size_t kPageSize           = FLASH_PAGE_SIZE;
constexpr size_t kMaxBinaryValue     = 1024;
constexpr size_t kArgumentBufferSize = 2 * sizeof(uint16_t) + kMaxBinaryValue; // ID(2) + Size(2) + Value(n)
} // namespace

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {
namespace Flash {

#if SLI_SI91X_MCU_INTERFACE
static uint8_t * sReadOnlyPage = reinterpret_cast<uint8_t *>(NWP_FLASH_ADDRESS);
#else
static uint8_t * sReadOnlyPage = reinterpret_cast<uint8_t *>(linker_nvm_end);
#endif // SLI_SI91X_MCU_INTERFACE
uint8_t sTemporaryPage[kPageSize] = { 0 };
uint8_t * sActivePage             = sReadOnlyPage;

CHIP_ERROR DecodeTotal(Encoding::Buffer & reader, uint16_t & total)
{
    uint16_t sz = 0;
    ReturnErrorOnFailure(reader.Get(sz));
    total     = (0xffff == sz) ? sizeof(uint16_t) : sz;
    reader.in = reader.begin + total;
    VerifyOrReturnError(reader.in <= reader.end, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ActivateWrite(uint8_t *& active)
{
#if !(SLI_SI91X_MCU_INTERFACE)
    if (sActivePage == sReadOnlyPage)
    {
        memcpy(sTemporaryPage, sReadOnlyPage, sizeof(sTemporaryPage));
    }
    active = sActivePage = sTemporaryPage;
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR Set(uint16_t id, Encoding::Buffer & in)
{
    uint8_t * page = sActivePage;
    uint16_t total = 0;
    Encoding::Buffer reader(page, kPageSize, true);
    uint8_t temp[kArgumentBufferSize] = { 0 };
    Encoding::Version2::Argument found(temp, sizeof(temp));

    // Decode total
    ReturnErrorOnFailure(DecodeTotal(reader, total));
    // Search entry
    CHIP_ERROR err = Encoding::Version2::Find(reader, id, found);
    if ((CHIP_ERROR_NOT_FOUND != err) && (CHIP_NO_ERROR != err))
    {
        // Memory corruption, write at the last correct address
        return err;
    }
    ReturnErrorOnFailure(ActivateWrite(page));

    Encoding::Buffer writer(page, kPageSize);
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // New entry
        size_t temp_total = found.offset;
        VerifyOrReturnError(temp_total + in.Size() <= kPageSize, CHIP_ERROR_INVALID_ARGUMENT);
        // Copy entry
        ReturnErrorOnFailure(in.Get(page + temp_total, in.Size()));
        // Update total
        total = temp_total + in.Size();
        ReturnErrorOnFailure(writer.Add(total));
    }
    else
    {
        // Existing entry
        if (in.Size() == found.encoded_size)
        {
            // Same size, keep in place
            memset(page + found.offset, 0xff, found.encoded_size);
            ReturnErrorOnFailure(in.Get(page + found.offset, in.Size()));
        }
        else
        {
            // Size change, move to the end
            uint16_t temp_total = total - found.encoded_size;
            VerifyOrReturnError(temp_total + in.Size() <= kPageSize, CHIP_ERROR_INVALID_ARGUMENT);
            // Remove the entry
            memmove(page + found.offset, page + found.offset + found.encoded_size, temp_total);
            // Add the entry
            ReturnErrorOnFailure(in.Get(page + temp_total, in.Size()));
            // Update total
            total = temp_total + in.Size();
            ReturnErrorOnFailure(writer.Add(total));
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Get(uint16_t id, Encoding::Version2::Argument & arg)
{
    uint16_t total = 0;

    Encoding::Buffer reader(sActivePage, kPageSize, true);
    ReturnErrorOnFailure(DecodeTotal(reader, total));
    CHIP_ERROR err = Encoding::Version2::Find(reader, id, arg);
    // ProvisionStorage expects CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND
    VerifyOrReturnError(CHIP_ERROR_NOT_FOUND != err, CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    return err;
}

CHIP_ERROR Set(uint16_t id, uint8_t value)
{
    uint8_t temp[kArgumentBufferSize] = { 0 };
    Encoding::Version2::Argument arg(temp, sizeof(temp));
    ReturnErrorOnFailure(Encoding::Version2::Encode(id, &value, arg));
    return Set(id, arg);
}

CHIP_ERROR Get(uint16_t id, uint8_t & value)
{
    uint8_t temp[kArgumentBufferSize] = { 0 };
    Encoding::Version2::Argument arg(temp, sizeof(temp));
    ReturnErrorOnFailure(Get(id, arg));
    VerifyOrReturnError(Encoding::Version2::Type_Int8u == arg.type, CHIP_ERROR_INVALID_ARGUMENT);
    value = arg.value.u8;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Set(uint16_t id, uint16_t value)
{
    uint8_t temp[kArgumentBufferSize] = { 0 };
    Encoding::Version2::Argument arg(temp, sizeof(temp));
    ReturnErrorOnFailure(Encoding::Version2::Encode(id, &value, arg));
    return Set(id, arg);
}

CHIP_ERROR Get(uint16_t id, uint16_t & value)
{
    uint8_t temp[kArgumentBufferSize] = { 0 };
    Encoding::Version2::Argument arg(temp, sizeof(temp));
    ReturnErrorOnFailure(Get(id, arg));
    VerifyOrReturnError(Encoding::Version2::Type_Int16u == arg.type, CHIP_ERROR_INVALID_ARGUMENT);
    value = arg.value.u16;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Set(uint16_t id, uint32_t value)
{
    uint8_t temp[kArgumentBufferSize] = { 0 };
    Encoding::Version2::Argument arg(temp, sizeof(temp));
    ReturnErrorOnFailure(Encoding::Version2::Encode(id, &value, arg));
    return Set(id, arg);
}

CHIP_ERROR Get(uint16_t id, uint32_t & value)
{
    uint8_t temp[kArgumentBufferSize] = { 0 };
    Encoding::Version2::Argument arg(temp, sizeof(temp));
    ReturnErrorOnFailure(Get(id, arg));
    VerifyOrReturnError(Encoding::Version2::Type_Int32u == arg.type, CHIP_ERROR_INVALID_ARGUMENT);
    value = arg.value.u32;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Set(uint16_t id, const uint8_t * value, size_t size)
{
    uint8_t temp[kArgumentBufferSize] = { 0 };
    Encoding::Version2::Argument arg(temp, sizeof(temp));
    ReturnErrorOnFailure(Encoding::Version2::Encode(id, value, size, arg));
    return Set(id, arg);
}

CHIP_ERROR Get(uint16_t id, uint8_t * value, size_t max_size, size_t & size)
{

    uint8_t temp[kArgumentBufferSize] = { 0 };
    Encoding::Version2::Argument arg(temp, sizeof(temp));
    ReturnErrorOnFailure(Get(id, arg));
    VerifyOrReturnError(Encoding::Version2::Type_Binary == arg.type, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(arg.size <= max_size, CHIP_ERROR_INTERNAL);
    memcpy(value, arg.value.b, arg.size);
    size = arg.size;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Set(uint16_t id, const char * value, size_t size)
{
    return Set(id, (const uint8_t *) value, size);
}

CHIP_ERROR Get(uint16_t id, char * value, size_t max_size, size_t & size)
{
    return Get(id, (uint8_t *) value, max_size - 1, size);
}

} // namespace Flash

//
// Initialization
//

CHIP_ERROR Storage::Initialize(uint32_t flash_addr, uint32_t flash_size)
{
#if SLI_SI91X_MCU_INTERFACE
    sl_status_t status = sl_si91x_command_to_read_common_flash((uint32_t) (Flash::sReadOnlyPage), sizeof(Flash::sTemporaryPage),
                                                               Flash::sTemporaryPage);
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_INVALID_ARGUMENT);
    Flash::sActivePage = Flash::sTemporaryPage;

#else // SLI_SI91X_MCU_INTERFACE
    if (flash_size > 0)
    {
        Flash::sReadOnlyPage = (uint8_t *) (flash_addr + flash_size - kPageSize);
    }
    Flash::sActivePage = Flash::sReadOnlyPage;
    MSC_Init();
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::Commit()
{
    if (Flash::sActivePage == Flash::sTemporaryPage)
    {
#if SLI_SI91X_MCU_INTERFACE
        // Erase page
        sl_status_t status = sl_si91x_command_to_write_common_flash((uint32_t) (Flash::sReadOnlyPage), Flash::sTemporaryPage,
                                                                    kPageSize, FLASH_ERASE);
        VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_WRITE_FAILED);
        // Write to flash
        status = sl_si91x_command_to_write_common_flash((uint32_t) (Flash::sReadOnlyPage), Flash::sTemporaryPage, kPageSize,
                                                        FLASH_WRITE);
        VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_WRITE_FAILED);
#else
        // Erase page
        MSC_ErasePage((uint32_t *) Flash::sReadOnlyPage);
        // Write to flash
        MSC_WriteWord((uint32_t *) Flash::sReadOnlyPage, Flash::sTemporaryPage, kPageSize);
#endif // SLI_SI91X_MCU_INTERFACE
    }
    return CHIP_NO_ERROR;
}

//
// DeviceInstanceInfoProvider
//

CHIP_ERROR Storage::SetSerialNumber(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kSerialNumber, value, len);
}

CHIP_ERROR Storage::GetSerialNumber(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kSerialNumber, value, max, size);
}

CHIP_ERROR Storage::SetVendorId(uint16_t value)
{
    return Flash::Set(Parameters::ID::kVendorId, value);
}

CHIP_ERROR Storage::GetVendorId(uint16_t & value)
{
    CHIP_ERROR err = Flash::Get(Parameters::ID::kVendorId, value);
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
    return Flash::Set(Parameters::ID::kVendorName, value, len);
}

CHIP_ERROR Storage::GetVendorName(char * value, size_t max)
{
    size_t size    = 0;
    CHIP_ERROR err = Flash::Get(Parameters::ID::kVendorName, value, max, size);
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
    return Flash::Set(Parameters::ID::kProductId, value);
}

CHIP_ERROR Storage::GetProductId(uint16_t & value)
{
    CHIP_ERROR err = Flash::Get(Parameters::ID::kProductId, value);
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
    return Flash::Set(Parameters::ID::kProductName, value, len);
}

CHIP_ERROR Storage::GetProductName(char * value, size_t max)
{
    size_t size    = 0;
    CHIP_ERROR err = Flash::Get(Parameters::ID::kProductName, value, max, size);
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
    return Flash::Set(Parameters::ID::kProductLabel, value, len);
}

CHIP_ERROR Storage::GetProductLabel(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kProductLabel, value, max, size);
}

CHIP_ERROR Storage::SetProductURL(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kProductUrl, value, len);
}
CHIP_ERROR Storage::GetProductURL(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kProductUrl, value, max, size);
}

CHIP_ERROR Storage::SetPartNumber(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kPartNumber, value, len);
}

CHIP_ERROR Storage::GetPartNumber(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kPartNumber, value, max, size);
}

CHIP_ERROR Storage::SetHardwareVersion(uint16_t value)
{
    return Flash::Set(Parameters::ID::kHwVersion, value);
}

CHIP_ERROR Storage::GetHardwareVersion(uint16_t & value)
{
    CHIP_ERROR err = Flash::Get(Parameters::ID::kHwVersion, value);
#if defined(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION;
        err   = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION
    return err;
}

CHIP_ERROR Storage::SetHardwareVersionString(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kHwVersionStr, value, len);
}

CHIP_ERROR Storage::GetHardwareVersionString(char * value, size_t max)
{
    size_t size    = 0;
    CHIP_ERROR err = Flash::Get(Parameters::ID::kHwVersionStr, value, max, size);
#if defined(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        VerifyOrReturnError(value != nullptr, CHIP_ERROR_NO_MEMORY);
        VerifyOrReturnError(max > strlen(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
        Platform::CopyString(value, max, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
        err = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING
    return err;
}

CHIP_ERROR Storage::SetManufacturingDate(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kManufacturingDate, value, len);
}

CHIP_ERROR Storage::GetManufacturingDate(uint8_t * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kManufacturingDate, value, max, size);
}

CHIP_ERROR Storage::SetPersistentUniqueId(const uint8_t * value, size_t size)
{
    return Flash::Set(Parameters::ID::kPersistentUniqueId, value, size);
}

CHIP_ERROR Storage::GetPersistentUniqueId(uint8_t * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kPersistentUniqueId, value, max, size);
}

//
// CommissionableDataProvider
//

CHIP_ERROR Storage::SetSetupDiscriminator(uint16_t value)
{
    return Flash::Set(Parameters::ID::kDiscriminator, value);
}

CHIP_ERROR Storage::GetSetupDiscriminator(uint16_t & value)
{
    CHIP_ERROR err = Flash::Get(Parameters::ID::kDiscriminator, value);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
        err   = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    ReturnErrorOnFailure(err);
    VerifyOrReturnLogError(value <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetSpake2pIterationCount(uint32_t value)
{
    return Flash::Set(Parameters::ID::kSpake2pIterations, value);
}

CHIP_ERROR Storage::GetSpake2pIterationCount(uint32_t & value)
{
    CHIP_ERROR err = Flash::Get(Parameters::ID::kSpake2pIterations, value);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT;
        err   = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    return err;
}

CHIP_ERROR Storage::SetSetupPasscode(uint32_t value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Storage::GetSetupPasscode(uint32_t & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Storage::SetSpake2pSalt(const char * value, size_t size)
{
    return Flash::Set(Parameters::ID::kSpake2pSalt, value, size);
}

CHIP_ERROR Storage::GetSpake2pSalt(char * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kSpake2pSalt, value, max, size);
}

CHIP_ERROR Storage::SetSpake2pVerifier(const char * value, size_t size)
{
    return Flash::Set(Parameters::ID::kSpake2pVerifier, value, size);
}

CHIP_ERROR Storage::GetSpake2pVerifier(char * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kSpake2pVerifier, value, max, size);
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
    return Flash::Set(Parameters::ID::kCertification, value.data(), value.size());
}

CHIP_ERROR Storage::GetCertificationDeclaration(MutableByteSpan & value)
{
    size_t size    = 0;
    CHIP_ERROR err = (Flash::Get(Parameters::ID::kCertification, value.data(), value.size(), size));
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        // Example CD
        return Examples::GetExampleDACProvider()->GetCertificationDeclaration(value);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    ReturnErrorOnFailure(err);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetProductAttestationIntermediateCert(const ByteSpan & value)
{
    return Flash::Set(Parameters::ID::kPaiCert, value.data(), value.size());
}

CHIP_ERROR Storage::GetProductAttestationIntermediateCert(MutableByteSpan & value)
{
    size_t size    = 0;
    CHIP_ERROR err = (Flash::Get(Parameters::ID::kPaiCert, value.data(), value.size(), size));
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        // Example PAI
        return Examples::GetExampleDACProvider()->GetProductAttestationIntermediateCert(value);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    ReturnErrorOnFailure(err);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetDeviceAttestationCert(const ByteSpan & value)
{
    return Flash::Set(Parameters::ID::kDacCert, value.data(), value.size());
}

CHIP_ERROR Storage::GetDeviceAttestationCert(MutableByteSpan & value)
{
    size_t size    = 0;
    CHIP_ERROR err = (Flash::Get(Parameters::ID::kDacCert, value.data(), value.size(), size));
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        // Example DAC
        return Examples::GetExampleDACProvider()->GetDeviceAttestationCert(value);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    ReturnErrorOnFailure(err);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetDeviceAttestationKey(const ByteSpan & value)
{
    return Flash::Set(Parameters::ID::kDacKey, value.data(), value.size());
}

CHIP_ERROR Storage::GetDeviceAttestationCSR(uint16_t vid, uint16_t pid, const CharSpan & cn, MutableCharSpan & csr)
{
    AttestationKey key;
    uint8_t temp[kDeviceAttestationKeySizeMax] = { 0 };
    size_t size                                = 0;
    ReturnErrorOnFailure(key.GenerateCSR(vid, pid, cn, csr));
    ReturnErrorOnFailure(key.Export(temp, sizeof(temp), size));
    return Flash::Set(Parameters::ID::kDacKey, temp, size);
}

CHIP_ERROR Storage::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    uint8_t temp[kDeviceAttestationKeySizeMax] = { 0 };
    size_t size                                = 0;
    CHIP_ERROR err                             = Flash::Get(Parameters::ID::kDacKey, temp, sizeof(temp), size);
#ifdef CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        // Example DAC key
        return Examples::GetExampleDACProvider()->SignWithDeviceAttestationKey(message, signature);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS
    ReturnErrorOnFailure(err);
#if (defined(SLI_SI91X_MCU_INTERFACE) && SLI_SI91X_MCU_INTERFACE)
    uint8_t key_buffer[kDeviceAttestationKeySizeMax] = { 0 };
    MutableByteSpan private_key(key_buffer);
    AttestationKey::Unwrap(temp, size, private_key);
    return AttestationKey::SignMessageWithKey((const uint8_t *) key_buffer, message, signature);
#else
    AttestationKey key;
    ReturnErrorOnFailure(key.Import(temp, size));
    return key.SignMessage(message, signature);
#endif // SLI_SI91X_MCU_INTERFACE
}

//
// Other
//

CHIP_ERROR Storage::SetCredentialsBaseAddress(uint32_t addr)
{
    Flash::sReadOnlyPage = (uint8_t *) addr;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetCredentialsBaseAddress(uint32_t & addr)
{
    addr = (uint32_t) Flash::sReadOnlyPage;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetProvisionVersion(const char * value, size_t size)
{
    return Flash::Set(Parameters::ID::kVersion, value, size);
}

CHIP_ERROR Storage::GetProvisionVersion(char * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kVersion, value, max, size);
}

CHIP_ERROR Storage::SetSetupPayload(const uint8_t * value, size_t size)
{
    return Flash::Set(Parameters::ID::kSetupPayload, value, size);
}

CHIP_ERROR Storage::GetSetupPayload(uint8_t * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kSetupPayload, value, max, size);
}

CHIP_ERROR Storage::SetProvisionRequest(bool value)
{
    // return Flash::Set(Parameters::ID::kProvisionRequest, value);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Storage::GetProvisionRequest(bool & value)
{
    // return Flash::Set(Parameters::ID::kProvisionRequest, value);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
#if OTA_ENCRYPTION_ENABLE
CHIP_ERROR Storage::SetOtaTlvEncryptionKey(const ByteSpan & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
#endif // OTA_ENCRYPTION_ENABLE

CHIP_ERROR Storage::GetTestEventTriggerKey(MutableByteSpan & keySpan)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Provision

void MigrateDacProvider(void) {}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
