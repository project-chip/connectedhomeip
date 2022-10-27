/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_hal_platform.h"
#include "cmsis.h"
#include "device_definition.h"
#include "target_cfg.h"
#include "tfm_peripherals_def.h"
#include "uart_stdout.h"
#if (CONFIG_TFM_FP == 2) && (TEST_NS_FPU == 1)
#include "tfm_plat_test.h"
#endif

/* Get address of memory regions to configure MPU */
extern const struct memory_region_limits memory_regions;

enum tfm_hal_status_t tfm_hal_platform_init(void)
{
    enum tfm_plat_err_t plat_err                      = TFM_PLAT_ERR_SYSTEM_ERR;
    enum syscounter_armv8_m_cntrl_error_t counter_err = SYSCOUNTER_ARMV8_M_ERR_NONE;

    plat_err = enable_fault_handlers();
    if (plat_err != TFM_PLAT_ERR_SUCCESS)
    {
        return TFM_HAL_ERROR_GENERIC;
    }

    plat_err = system_reset_cfg();
    if (plat_err != TFM_PLAT_ERR_SUCCESS)
    {
        return TFM_HAL_ERROR_GENERIC;
    }

    plat_err = init_debug();
    if (plat_err != TFM_PLAT_ERR_SUCCESS)
    {
        return TFM_HAL_ERROR_GENERIC;
    }

    /* Syscounter enabled by default. This way App-RoT partitions can use
     * systimers without the need to add the syscounter as an mmio devide.
     */
    counter_err = syscounter_armv8_m_cntrl_init(&SYSCOUNTER_CNTRL_ARMV8_M_DEV_S);
    if (counter_err != SYSCOUNTER_ARMV8_M_ERR_NONE)
    {
        return TFM_HAL_ERROR_GENERIC;
    }

    __enable_irq();
    stdio_init();

    plat_err = nvic_interrupt_target_state_cfg();
    if (plat_err != TFM_PLAT_ERR_SUCCESS)
    {
        return TFM_HAL_ERROR_GENERIC;
    }

    plat_err = nvic_interrupt_enable();
    if (plat_err != TFM_PLAT_ERR_SUCCESS)
    {
        return TFM_HAL_ERROR_GENERIC;
    }

#if (CONFIG_TFM_FP == 2) && (TEST_NS_FPU == 1)
    /* Configure secure timer */
    tfm_plat_test_secure_timer_nvic_configure();

    /* Configure non-secure timer */
    tfm_plat_test_non_secure_timer_nvic_configure();
#endif

    return TFM_HAL_SUCCESS;
}

uint32_t tfm_hal_get_ns_VTOR(void)
{
    return memory_regions.non_secure_code_start;
}

uint32_t tfm_hal_get_ns_MSP(void)
{
    return *((uint32_t *) memory_regions.non_secure_code_start);
}

uint32_t tfm_hal_get_ns_entry_point(void)
{
    return *((uint32_t *) (memory_regions.non_secure_code_start + 4));
}
