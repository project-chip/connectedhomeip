/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "Driver_Flash.h"
#include "boot_hal.h"
#include "cmsis.h"
#include "flash_layout.h"
#include "region.h"
#include "target_cfg.h"

/* Flash device name must be specified by target */
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_0;

REGION_DECLARE(Image$$, ER_DATA, $$Base)[];
REGION_DECLARE(Image$$, ARM_LIB_HEAP, $$ZI$$Limit)[];
REGION_DECLARE(Image$$, ARM_LIB_STACK, $$ZI$$Base);

int flash_device_base(uint8_t fd_id, uintptr_t * ret)
{
    if (fd_id == FLASH_DEVICE_ID_0)
    {
        *ret = FLASH0_BASE_ADDRESS;
    }
    else if (fd_id == FLASH_DEVICE_ID_1)
    {
        *ret = FLASH0_BASE_ADDRESS;
    }
    else if (fd_id == FLASH_DEVICE_ID_SCRATCH)
    {
        *ret = FLASH0_BASE_ADDRESS;
    }
    else
    {
        return -1;
    }

    return 0;
}

int32_t boot_platform_init(void)
{
    int32_t result;

    /* Initialize stack limit register */
    uint32_t msp_stack_bottom = (uint32_t) &REGION_NAME(Image$$, ARM_LIB_STACK, $$ZI$$Base);

    __set_MSPLIM(msp_stack_bottom);

    result = FLASH_DEV_NAME_0.Initialize(NULL);
    if (result != ARM_DRIVER_OK)
    {
        return 1;
    }

    return 0;
}

void boot_platform_quit(struct boot_arm_vector_table * vt)
{
    /* Clang at O0, stores variables on the stack with SP relative addressing.
     * When manually set the SP then the place of reset vector is lost.
     * Static variables are stored in 'data' or 'bss' section, change of SP has
     * no effect on them.
     */
    static struct boot_arm_vector_table * vt_cpy;
    int32_t result;

    result = FLASH_DEV_NAME_0.Uninitialize();
    if (result != ARM_DRIVER_OK)
    {
        while (1)
            ;
    }

    vt_cpy = vt;

    __set_MSPLIM(0);
    __set_MSP(vt->msp);
    __DSB();
    __ISB();

    boot_jump_to_next_image(vt_cpy->reset);
}
