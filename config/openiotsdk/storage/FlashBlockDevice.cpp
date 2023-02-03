/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *          Utilities for interacting with the the Open IoT SDK key-value storage.
 */

#include <string.h>

#include "FlashBlockDevice.h"
#include <OpenIoTSDKPort.h>

extern "C" {
#include <platform_base_address.h>
}

#define FLASH_BASE_ADDRESS (SRAM_BASE_NS)     /* 0x01000000 */
#define FLASH_TOTAL_SIZE (SRAM_SIZE)          /* 2 MB */
#define FLASH_AREA_IMAGE_SECTOR_SIZE (0x1000) /* 4 KB */
#define FLASH_PAGE_SIZE 256
#define FLASH_END_ADDRESS (FLASH_BASE_ADDRESS + FLASH_TOTAL_SIZE)
#define FLASH_READ_SIZE 1
#define FLASH_ERASE_VALUE 0xFFU

namespace iotsdk {
namespace storage {

FlashBlockDevice::FlashBlockDevice(uint32_t address, uint32_t size) : _base(address), _size(size), _is_initialized(false) {}

FlashBlockDevice::~FlashBlockDevice() {}

bd_status FlashBlockDevice::init()
{
    if (!_base)
    {
        _base = FLASH_BASE_ADDRESS;
    }

    if (!_size)
    {
        _size = FLASH_TOTAL_SIZE - (_base - FLASH_BASE_ADDRESS);
    }

    if (_size + _base > FLASH_TOTAL_SIZE + FLASH_BASE_ADDRESS)
    {
        return bd_status::INCORRECT_SIZE;
    }

    if (_base < FLASH_BASE_ADDRESS)
    {
        return bd_status::INCORRECT_ADDRESS;
    }

    _is_initialized = true;
    return bd_status::OK;
}

bd_status FlashBlockDevice::deinit()
{
    if (!_is_initialized)
    {
        return bd_status::OK;
    }

    _is_initialized = false;

    return bd_status::OK;
}

bd_status FlashBlockDevice::read(void * buffer, bd_addr_t virtual_address, bd_size_t size)
{
    if (!_is_initialized)
    {
        return bd_status::DEVICE_NOT_INITIALIZED;
    }

    /* Check that the address and size are properly aligned and fit. */
    bd_status status = is_valid_read(virtual_address, size);
    if (status != bd_status::OK)
    {
        return status;
    }

    /* Convert virtual address to the physical address for the device. */
    const auto physical_address = static_cast<uint32_t>(_base + virtual_address);

    /* Read data */
    memcpy(static_cast<uint8_t *>(buffer), (void *) physical_address, static_cast<uint32_t>(size));

    return bd_status::OK;
}

bd_status FlashBlockDevice::program(const void * buffer, bd_addr_t virtual_address, bd_size_t size)
{
    if (!_is_initialized)
    {
        return bd_status::DEVICE_NOT_INITIALIZED;
    }

    /* Check that the address and size are properly aligned and fit. */
    bd_status status = is_valid_program(virtual_address, size);
    if (status != bd_status::OK)
    {
        return status;
    }

    /* Convert virtual address to the physical address for the device. */
    auto physical_address = static_cast<uint32_t>(_base + virtual_address);
    auto remaining_size   = static_cast<uint32_t>(size);
    const auto * buf      = static_cast<const uint8_t *>(buffer);

    while (remaining_size > 0)
    {
        /* Multiple pages can be programmed at once but cannot cross sector boundaries */
        const auto sector_size = FLASH_AREA_IMAGE_SECTOR_SIZE;
        const auto sector_end  = (physical_address / sector_size + 1) * sector_size;
        const auto chunk = (physical_address + remaining_size > sector_end) ? (sector_end - physical_address) : remaining_size;
        /* Write data */
        memcpy((void *) physical_address, buf, chunk);
        physical_address += chunk;
        remaining_size -= chunk;
        buf += chunk;
    }

    return bd_status::OK;
}

bd_status FlashBlockDevice::erase(bd_addr_t virtual_address, bd_size_t size)
{
    if (!_is_initialized)
    {
        return bd_status::DEVICE_NOT_INITIALIZED;
    }

    /* Check that the address and size are properly aligned and fit. */
    const bd_status status = is_valid_erase(virtual_address, size);
    if (status != bd_status::OK)
    {
        return status;
    }

    /* Convert virtual address to the physical address for the device. */
    auto physical_address   = static_cast<uint32_t>(_base + virtual_address);
    const auto sector_size  = FLASH_AREA_IMAGE_SECTOR_SIZE;
    const auto erase_region = (physical_address + size) - sector_size;

    while (physical_address <= erase_region)
    {
        memset((void *) physical_address, FLASH_ERASE_VALUE, FLASH_AREA_IMAGE_SECTOR_SIZE);
        physical_address += sector_size;
    }

    return bd_status::OK;
}

bd_size_t FlashBlockDevice::get_read_size() const
{
    return FLASH_READ_SIZE;
}

bd_size_t FlashBlockDevice::get_program_size() const
{
    if (!_is_initialized)
    {
        return 0;
    }

    return FLASH_PAGE_SIZE;
}

bd_size_t FlashBlockDevice::get_erase_size() const
{
    return 0;
}

bd_size_t FlashBlockDevice::get_erase_size(bd_addr_t addr) const
{
    if (!_is_initialized)
    {
        return 0;
    }

    if (static_cast<uint64_t>(_base) + addr > UINT32_MAX)
    {
        return 0;
    }

    return FLASH_AREA_IMAGE_SECTOR_SIZE;
}

int FlashBlockDevice::get_erase_value() const
{
    if (!_is_initialized)
    {
        return -1;
    }

    return FLASH_ERASE_VALUE;
}

bd_size_t FlashBlockDevice::size() const
{
    return _size;
}

const char * FlashBlockDevice::get_type() const
{
    return "FLASH_BD";
}

bd_status FlashBlockDevice::is_valid_read(bd_addr_t addr, bd_size_t size) const
{
    if (static_cast<uint64_t>(_base) + addr > UINT32_MAX)
    {
        return bd_status::INCORRECT_ADDRESS;
    }

    if (size > UINT32_MAX)
    {
        return bd_status::INCORRECT_SIZE;
    }

    return BlockDevice::is_valid_read(addr, size);
}

bd_status FlashBlockDevice::is_valid_program(bd_addr_t addr, bd_size_t size) const
{
    if (static_cast<uint64_t>(_base) + addr > UINT32_MAX)
    {
        return bd_status::INCORRECT_ADDRESS;
    }

    if (size > UINT32_MAX)
    {
        return bd_status::INCORRECT_SIZE;
    }

    return BlockDevice::is_valid_program(addr, size);
}

bd_status FlashBlockDevice::is_valid_erase(bd_addr_t addr, bd_size_t size) const
{
    if (static_cast<uint64_t>(_base) + addr > UINT32_MAX)
    {
        return bd_status::INCORRECT_ADDRESS;
    }

    if (size > UINT32_MAX)
    {
        return bd_status::INCORRECT_SIZE;
    }

    return BlockDevice::is_valid_erase(addr, size);
}

} // namespace storage
} // namespace iotsdk

namespace chip {
namespace DeviceLayer {
namespace Internal {

static iotsdk::storage::FlashBlockDevice gBlockDevice(0, 0);

iotsdk::storage::BlockDevice * GetBlockDevice()
{
    return &gBlockDevice;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
