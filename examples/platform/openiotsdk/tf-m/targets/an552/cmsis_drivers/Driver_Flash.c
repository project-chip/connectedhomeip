/*
 * Copyright (c) 2013-2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Driver_Flash.h"
#include "RTE_Device.h"
#include "emulated_flash_drv.h"
#include "platform_base_address.h"
#include <stdint.h>
#include <string.h>

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg) ((void) arg)
#endif

#define FLASH0_BASE_S QSPI_SRAM_BASE_S
#define FLASH0_BASE_NS QSPI_SRAM_BASE_NS
#define FLASH0_SIZE QSPI_SRAM_SIZE
#define FLASH0_SECTOR_SIZE 0x00001000 /* 4 kB */
#define FLASH0_PAGE_SIZE 0x00001000   /* 4 kB */
#define FLASH0_PROGRAM_UNIT 0x1       /* Minimum write size */

/* Driver version */
#define ARM_FLASH_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1, 1)

/**
 * Data width values for ARM_FLASH_CAPABILITIES::data_width
 * \ref ARM_FLASH_CAPABILITIES
 */
enum
{
    DATA_WIDTH_8BIT = 0u,
    DATA_WIDTH_16BIT,
    DATA_WIDTH_32BIT,
    DATA_WIDTH_ENUM_SIZE
};

static const uint32_t data_width_byte[DATA_WIDTH_ENUM_SIZE] = {
    sizeof(uint8_t),
    sizeof(uint16_t),
    sizeof(uint32_t),
};

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = { ARM_FLASH_API_VERSION, ARM_FLASH_DRV_VERSION };

/* Driver Capabilities */
static const ARM_FLASH_CAPABILITIES DriverCapabilities = {
    0, /* event_ready */
    0, /* data_width = 0:8-bit, 1:16-bit, 2:32-bit */
    1, /* erase_chip */
    0  /* reserved */
};

static ARM_DRIVER_VERSION ARM_Flash_GetVersion(void)
{
    return DriverVersion;
}

static ARM_FLASH_CAPABILITIES ARM_Flash_GetCapabilities(void)
{
    return DriverCapabilities;
}

static int32_t ARM_Flash_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
    ARG_UNUSED(cb_event);

    if (DriverCapabilities.data_width >= DATA_WIDTH_ENUM_SIZE)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_Uninitialize(void)
{
    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_PowerControl(ARM_POWER_STATE state)
{
    switch (state)
    {
    case ARM_POWER_FULL:
        /* Nothing to be done */
        return ARM_DRIVER_OK;

    case ARM_POWER_OFF:
    case ARM_POWER_LOW:
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

#if (RTE_FLASH0)

/* Flash Status */
static ARM_FLASH_STATUS FlashStatus_FLASH0 = { 0, 0, 0 };

static ARM_FLASH_INFO ARM_FLASH0_DEV_INFO = { .sector_info  = NULL, /* Uniform sector layout */
                                              .sector_count = FLASH0_SIZE / FLASH0_SECTOR_SIZE,
                                              .sector_size  = FLASH0_SECTOR_SIZE,
                                              .page_size    = FLASH0_PAGE_SIZE,
                                              .program_unit = FLASH0_PROGRAM_UNIT,
                                              .erased_value = EMULATED_FLASH_DRV_ERASE_VALUE };

static struct emulated_flash_dev_t ARM_FLASH0_DEV = {
#if (defined(__DOMAIN_NS) && (__DOMAIN_NS == 1))
    .memory_base_ns = FLASH0_BASE_NS,
#else
    .memory_base_ns = FLASH0_BASE_NS,
    .memory_base_s  = FLASH0_BASE_S,
#endif /* __DOMAIN_NS == 1 */
    .data = &(ARM_FLASH0_DEV_INFO)
};

struct emulated_flash_dev_t * FLASH0_DEV = &ARM_FLASH0_DEV;

/*
 * Functions
 */

static int32_t ARM_Flash_FLASH0_ReadData(uint32_t addr, void * data, uint32_t cnt)
{
    /* Conversion between data items and bytes */
    cnt *= data_width_byte[DriverCapabilities.data_width];

    enum emulated_flash_error_t rc = emulated_flash_read_data(FLASH0_DEV, addr, data, cnt);
    if (EMULATED_FLASH_ERR_NONE == rc)
    {
        cnt /= data_width_byte[DriverCapabilities.data_width];
        return cnt;
    }
    else if (EMULATED_FLASH_ERR_INVALID_PARAM == rc)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    else
    {
        return ARM_DRIVER_ERROR;
    }
}

static int32_t ARM_Flash_FLASH0_ProgramData(uint32_t addr, const void * data, uint32_t cnt)
{
    /* Conversion between data items and bytes */
    cnt *= data_width_byte[DriverCapabilities.data_width];
    enum emulated_flash_error_t rc = emulated_flash_program_data(FLASH0_DEV, addr, data, cnt);

    if (EMULATED_FLASH_ERR_NONE == rc)
    {
        cnt /= data_width_byte[DriverCapabilities.data_width];
        return (int32_t) cnt;
    }
    else if (EMULATED_FLASH_ERR_INVALID_PARAM == rc)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    else if (EMULATED_FLASH_NOT_READY == rc)
    {
        return ARM_DRIVER_ERROR;
    }
    else
    {
        return ARM_DRIVER_ERROR;
    }
}

static int32_t ARM_Flash_FLASH0_EraseSector(uint32_t addr)
{
    enum emulated_flash_error_t rc = emulated_flash_erase_sector(FLASH0_DEV, addr);

    if (EMULATED_FLASH_ERR_NONE == rc)
    {
        return ARM_DRIVER_OK;
    }
    else if (EMULATED_FLASH_ERR_INVALID_PARAM == rc)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    else
    {
        return ARM_DRIVER_ERROR;
    }
}

static int32_t ARM_Flash_FLASH0_EraseChip(void)
{
    emulated_flash_erase_chip(FLASH0_DEV);
    return ARM_DRIVER_OK;
}

static ARM_FLASH_INFO * ARM_Flash_FLASH0_GetInfo(void)
{
    return &ARM_FLASH0_DEV_INFO;
}

static ARM_FLASH_STATUS ARM_Flash_FLASH0_GetStatus(void)
{
    return FlashStatus_FLASH0;
}

ARM_DRIVER_FLASH Driver_FLASH0 = { ARM_Flash_GetVersion,         ARM_Flash_GetCapabilities,    ARM_Flash_Initialize,
                                   ARM_Flash_Uninitialize,       ARM_Flash_PowerControl,       ARM_Flash_FLASH0_ReadData,
                                   ARM_Flash_FLASH0_ProgramData, ARM_Flash_FLASH0_EraseSector, ARM_Flash_FLASH0_EraseChip,
                                   ARM_Flash_FLASH0_GetStatus,   ARM_Flash_FLASH0_GetInfo };
#endif /* RTE_FLASH0 */
