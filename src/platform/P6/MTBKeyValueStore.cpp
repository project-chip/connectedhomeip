/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *          Implementation of a key-value store using mtb_kvstore lib
 *
 */

#include "MTBKeyValueStore.h"

namespace {
cyhal_flash_block_info_t block_info;
cyhal_flash_t flash_obj;
mtb_kvstore_bd_t block_device;
} // namespace

cy_rslt_t mtb_key_value_store_init(mtb_kvstore_t * kvstore_obj)
{
    cyhal_flash_info_t flash_info;

    // Initialize the block device
    cy_rslt_t result = cyhal_flash_init(&flash_obj);
    if (CY_RSLT_SUCCESS != result)
    {
        return result;
    }

    block_device.read         = bd_read;
    block_device.program      = bd_program;
    block_device.erase        = bd_erase;
    block_device.read_size    = bd_read_size;
    block_device.program_size = bd_program_size;
    block_device.erase_size   = bd_erase_size;
    block_device.context      = &flash_obj;

    cyhal_flash_get_info(&flash_obj, &flash_info);
    block_info = flash_info.blocks[flash_info.block_count - 1];

    // Initialize the kv-store library
    result = mtb_kvstore_init(kvstore_obj, block_info.start_address, block_info.size, &block_device);
    if (CY_RSLT_SUCCESS != result)
    {
        cyhal_flash_free(&flash_obj);
    }

    return result;
}

uint32_t bd_read_size(void * context, uint32_t addr)
{
    return 1;
}

uint32_t bd_program_size(void * context, uint32_t addr)
{
    return block_info.page_size;
}

uint32_t bd_erase_size(void * context, uint32_t addr)
{
    return block_info.sector_size;
}

cy_rslt_t bd_read(void * context, uint32_t addr, uint32_t length, uint8_t * buf)
{
    memcpy(buf, (const uint8_t *) (addr), length);
    return CY_RSLT_SUCCESS;
}

cy_rslt_t bd_program(void * context, uint32_t addr, uint32_t length, const uint8_t * buf)
{
    uint32_t prog_size = bd_program_size(context, addr);
    CY_ASSERT(0 == (length % prog_size));
    cy_rslt_t result = CY_RSLT_SUCCESS;
    for (uint32_t loc = addr; (result == CY_RSLT_SUCCESS) && (loc < addr + length); loc += prog_size, buf += prog_size)
    {
        result = cyhal_flash_program((cyhal_flash_t *) context, loc, (const uint32_t *) buf);
    }
    return result;
}

cy_rslt_t bd_erase(void * context, uint32_t addr, uint32_t length)
{
    uint32_t erase_size = bd_erase_size(context, addr);
    CY_ASSERT(0 == (length % erase_size));
    cy_rslt_t result = CY_RSLT_SUCCESS;
    for (uint32_t loc = addr; (result == CY_RSLT_SUCCESS) && (loc < addr + length); loc += erase_size)
    {
        result = cyhal_flash_erase((cyhal_flash_t *) context, loc);
    }
    return result;
}
