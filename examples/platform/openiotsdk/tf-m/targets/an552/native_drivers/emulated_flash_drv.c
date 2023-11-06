/*
 * Copyright (c) 2021-2022 Arm Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "emulated_flash_drv.h"
#include <arm_cmse.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static int32_t is_range_valid(struct emulated_flash_dev_t * flash_dev, uint32_t offset)
{
    uint32_t flash_limit = 0;
    int32_t rc           = 0;

    flash_limit = (flash_dev->data->sector_count * flash_dev->data->sector_size) - 1;

    if (offset > flash_limit)
    {
        rc = -1;
    }
    return rc;
}

static int32_t is_write_aligned(struct emulated_flash_dev_t * flash_dev, uint32_t param)
{
    int32_t rc = 0;

    if ((param % flash_dev->data->program_unit) != 0)
    {
        rc = -1;
    }
    return rc;
}

static int32_t is_sector_aligned(struct emulated_flash_dev_t * flash_dev, uint32_t offset)
{
    int32_t rc = 0;

    if ((offset % flash_dev->data->sector_size) != 0)
    {
        rc = -1;
    }
    return rc;
}

static int32_t is_secure_alias_needed(uint32_t addr)
{
    int32_t rc = -1;

    /* Only have to check it if the object is building for secure side */
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    cmse_address_info_t address_info;
    /* Check if address can be accessed from non-secure */
    address_info = cmse_TTA((void *) addr);
    /* We only care about the security of the address here */
    if (address_info.flags.nonsecure_read_ok)
    {
        rc = 0;
    }
    else
    {
        rc = 1;
    }
#else
    rc = 0;
#endif
    return rc;
}

static int32_t is_flash_ready_to_write(const uint8_t * start_addr, uint32_t cnt)
{
    int32_t rc = 0;
    uint32_t i;

    for (i = 0; i < cnt; i++)
    {
        if (start_addr[i] != EMULATED_FLASH_DRV_ERASE_VALUE)
        {
            rc = -1;
            break;
        }
    }

    return rc;
}

enum emulated_flash_error_t emulated_flash_read_data(struct emulated_flash_dev_t * dev, uint32_t addr, void * data, uint32_t cnt)
{
    uint32_t start_addr = 0;
    int32_t rc          = 0;

    /* Check flash memory boundaries */
    rc = is_range_valid(dev, addr + cnt);
    if (rc != 0)
    {
        return EMULATED_FLASH_ERR_INVALID_PARAM;
    }

    /* Check which alias(S or NS) should be used to access the data */
    rc = is_secure_alias_needed(addr + dev->memory_base_ns);
    if (rc == 1)
    {
        start_addr = dev->memory_base_s + addr;
    }
    else if (rc == 0)
    {
        start_addr = dev->memory_base_ns + addr;
    }

    /* Flash interface just emulated over SRAM, use memcpy */
    memcpy(data, (void *) start_addr, cnt);
    return EMULATED_FLASH_ERR_NONE;
}

enum emulated_flash_error_t emulated_flash_program_data(struct emulated_flash_dev_t * dev, uint32_t addr, const void * data,
                                                        uint32_t cnt)
{
    uint32_t start_addr = 0;
    int32_t rc          = 0;

    /* Check flash memory boundaries and alignment with minimal write size */
    rc = is_range_valid(dev, addr + cnt);
    rc |= is_write_aligned(dev, addr);
    rc |= is_write_aligned(dev, cnt);
    if (rc != 0)
    {
        return EMULATED_FLASH_ERR_INVALID_PARAM;
    }

    /* Check which alias(S or NS) should be used to access the data */
    rc = is_secure_alias_needed(addr + dev->memory_base_ns);
    if (rc == 1)
    {
        start_addr = dev->memory_base_s + addr;
    }
    else if (rc == 0)
    {
        start_addr = dev->memory_base_ns + addr;
    }

    /* Check if the flash area to write the data was erased previously */
    rc = is_flash_ready_to_write((const uint8_t *) start_addr, cnt);
    if (rc != 0)
    {
        return EMULATED_FLASH_NOT_READY;
    }

    /* Flash interface just emulated over SRAM, use memcpy */
    memcpy((void *) start_addr, data, cnt);
    return EMULATED_FLASH_ERR_NONE;
}

enum emulated_flash_error_t emulated_flash_erase_sector(struct emulated_flash_dev_t * dev, uint32_t addr)
{
    uint32_t start_addr = 0;
    int32_t rc          = 0;

    rc = is_range_valid(dev, addr);
    rc |= is_sector_aligned(dev, addr);
    if (rc != 0)
    {
        return EMULATED_FLASH_ERR_INVALID_PARAM;
    }

    /* Check which alias(S or NS) should be used to access the data */
    rc = is_secure_alias_needed(addr + dev->memory_base_ns);
    if (rc == 1)
    {
        start_addr = dev->memory_base_s + addr;
    }
    else if (rc == 0)
    {
        start_addr = dev->memory_base_ns + addr;
    }

    /* Flash interface just emulated over SRAM, use memset */
    memset((void *) start_addr, dev->data->erased_value, dev->data->sector_size);
    return EMULATED_FLASH_ERR_NONE;
}

void emulated_flash_erase_chip(struct emulated_flash_dev_t * dev)
{
    uint32_t i;
    uint32_t addr = 0;
    int32_t rc    = 0;

    /* Only check 1 byte, as the whole memory should have the same security */
    rc = is_secure_alias_needed(dev->memory_base_ns);
    if (rc == 1)
    {
        addr = dev->memory_base_s;
    }
    else if (rc == 0)
    {
        addr = dev->memory_base_ns;
    }

    for (i = 0; i < dev->data->sector_count; i++)
    {
        /* Flash interface just emulated over SRAM, use memset */
        memset((void *) addr, dev->data->erased_value, dev->data->sector_size);

        addr += dev->data->sector_size;
    }
}

ARM_FLASH_INFO * emulated_flash_get_info(struct emulated_flash_dev_t * dev)
{
    return dev->data;
}
