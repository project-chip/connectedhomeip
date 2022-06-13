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
#include "cy_serial_flash_qspi.h"
#include "cybsp.h"
#include "cycfg_qspi_memslot.h"
#include "cyhal.h"

namespace {
mtb_kvstore_bd_t block_device;
} // namespace

#define MEM_SLOT_NUM (0u) /* Slot number of the memory to use */
#define QSPI_BUS_FREQUENCY_HZ (50000000lu)
cy_rslt_t mtb_key_value_store_init(mtb_kvstore_t * kvstore_obj)
{
    uint32_t ext_mem_address = 0x00;
    size_t sectorSize;
    // Initialize the block device
    cy_rslt_t result =
        cy_serial_flash_qspi_init(smifMemConfigs[MEM_SLOT_NUM], CYBSP_QSPI_D0, CYBSP_QSPI_D1, CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC,
                                  NC, NC, CYBSP_QSPI_SCK, CYBSP_QSPI_SS, QSPI_BUS_FREQUENCY_HZ);
    if (CY_RSLT_SUCCESS != result)
    {
        return result;
    }
    sectorSize      = cy_serial_flash_qspi_get_erase_size(ext_mem_address);
    uint32_t length = sectorSize * 2;
#ifdef P6_OTA
    // If using OTA set the start address towards the end of the flash area so
    // an update doesn't clobber the persistent storage.
    uint32_t start_addr = 0x3000000U;
#else
    uint32_t start_addr = 0;
#endif

    block_device.read         = bd_read;
    block_device.program      = bd_program;
    block_device.erase        = bd_erase;
    block_device.read_size    = bd_read_size;
    block_device.program_size = bd_program_size;
    block_device.erase_size   = bd_erase_size;
    block_device.context      = NULL;

    // Initialize the kv-store library
    result = mtb_kvstore_init(kvstore_obj, start_addr, length, &block_device);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_serial_flash_qspi_deinit();
    }

    return result;
}

uint32_t bd_read_size(void * context, uint32_t addr)
{
    return 1;
}

uint32_t bd_program_size(void * context, uint32_t addr)
{
    return cy_serial_flash_qspi_get_prog_size(addr);
}

uint32_t bd_erase_size(void * context, uint32_t addr)
{
    return cy_serial_flash_qspi_get_erase_size(addr);
}

cy_rslt_t bd_read(void * context, uint32_t addr, uint32_t length, uint8_t * buf)
{
    return cy_serial_flash_qspi_read(addr, length, buf);
}

cy_rslt_t bd_program(void * context, uint32_t addr, uint32_t length, const uint8_t * buf)
{
    return cy_serial_flash_qspi_write(addr, length, buf);
}

cy_rslt_t bd_erase(void * context, uint32_t addr, uint32_t length)
{
    return cy_serial_flash_qspi_erase(addr, length);
}
