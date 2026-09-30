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
#include <provision/flash/ProvisionStorageFlash.h>

#include <headers/ProvisionEncoder.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceError.h>

#include <cstring>

#if SLI_SI91X_MCU_INTERFACE
#define FLASH_ERASE 1
#define FLASH_WRITE 0
#define NWP_FLASH_ADDRESS (0x0000000)
#include <sl_status.h>
extern "C" {
#include <sl_net.h>
#include <sl_net_constants.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_device.h>
}
#else
#include <em_msc.h>
extern uint8_t linker_nvm_end[];
#endif

namespace {
constexpr size_t kPageSize           = FLASH_PAGE_SIZE;
constexpr size_t kMaxBinaryValue     = 1024;
constexpr size_t kArgumentBufferSize = 2 * sizeof(uint16_t) + kMaxBinaryValue;
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
bool sInitialized                 = false;

CHIP_ERROR EnsureInitialized()
{
    if (sInitialized)
    {
        return CHIP_NO_ERROR;
    }
#if SLI_SI91X_MCU_INTERFACE
    const uint32_t address   = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(sReadOnlyPage));
    const sl_status_t status = sl_si91x_command_to_read_common_flash(address, sizeof(sTemporaryPage), sTemporaryPage);
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_READ_FAILED);
    sActivePage = sTemporaryPage;
#else
    sActivePage = sReadOnlyPage;
#endif
    sInitialized = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeTotal(Encoding::Buffer & reader, uint16_t & total)
{
    uint16_t sz = 0;
    ReturnErrorOnFailure(reader.Get(sz));
    total      = (0xffff == sz) ? sizeof(uint16_t) : sz;
    reader.pIn = reader.pBegin + total;
    VerifyOrReturnError(reader.pIn <= reader.pEnd, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "Invalid page, or corrupted data"));
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
    ReturnErrorOnFailure(EnsureInitialized());
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
    ReturnErrorOnFailure(EnsureInitialized());
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
    VerifyOrReturnError(arg.size <= max_size, CHIP_ERROR_BUFFER_TOO_SMALL);
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
    // No room for a NUL-terminated C string; reject before max_size - 1 (avoids underflow) and skip storage.
    VerifyOrReturnError(max_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(Get(id, (uint8_t *) value, max_size - 1, size));
    // Binary Get does not NUL-terminate; DeviceInstanceInfoProvider requires it on success.
    value[size] = '\0';
    return CHIP_NO_ERROR;
}

CHIP_ERROR Initialize(uint32_t flash_addr, uint32_t flash_size)
{
#if SLI_SI91X_MCU_INTERFACE
    (void) flash_addr;
    (void) flash_size;
    Flash::sInitialized = false;
    ReturnErrorOnFailure(Flash::EnsureInitialized());
#else // SLI_SI91X_MCU_INTERFACE
    if (flash_size > 0)
    {
        Flash::sReadOnlyPage = (uint8_t *) (flash_addr + flash_size - kPageSize);
    }
    Flash::sActivePage  = Flash::sReadOnlyPage;
    Flash::sInitialized = true;
    MSC_Init();
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR Commit()
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

CHIP_ERROR GetFlashPageSize(uint32_t & size)
{
    size = kPageSize;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetCredentialsBaseAddress(uint32_t addr)
{
    Flash::sReadOnlyPage = (uint8_t *) addr;
    return CHIP_NO_ERROR;
}

CHIP_ERROR GetCredentialsBaseAddress(uint32_t & addr)
{
    addr = (uint32_t) Flash::sReadOnlyPage;
    return CHIP_NO_ERROR;
}

} // namespace Flash
} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
