/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "device_definition.h"
#include "systimer_armv8-m_drv.h"
#include <stdbool.h>

#define NR_FP_REG (32U)
#define REL_VALUE_FP_REGS_INVALIDATED (0xDEADBEEF)
#define REL_VALUE_FP_REGS_NOT_INVALIDATED (0xBEEFDEAD)

void non_secure_timer_set_reload_value(uint32_t value)
{
    systimer_armv8_m_set_autoinc_reload(&SYSTIMER1_ARMV8_M_DEV_NS, value);
}

void non_secure_timer_stop(void)
{
    systimer_armv8_m_uninit(&SYSTIMER1_ARMV8_M_DEV_NS);
    systimer_armv8_m_clear_autoinc_interrupt(&SYSTIMER1_ARMV8_M_DEV_NS);
    systimer_armv8_m_disable_timer(&SYSTIMER1_ARMV8_M_DEV_NS);
}

/*
 * Check whether FP registers are invalidated.
 * Return:
 *   True - FP registers are invalidated
 *   False - FP registers are not invalidated
 */
#if defined(__GNUC__)
__attribute__((noinline))
#endif
bool is_fp_regs_invalidated(void)
{
    static uint32_t fp_buffer[NR_FP_REG] = { 0 };
    uint32_t i;

    /* Dump FP data from FP registers to buffer */
    __asm volatile("vstm      %0, {S0-S31}            \n" : : "r"(fp_buffer) : "memory");

    for (i = 0; i < NR_FP_REG; i++)
    {
        if (fp_buffer[i] != 0)
        {
            return false;
        }
    }
    return true;
}

void TIMER1_Handler()
{
    /* Check whether FP regs is invalidated */
    if (is_fp_regs_invalidated())
    {
        non_secure_timer_set_reload_value(REL_VALUE_FP_REGS_INVALIDATED);
    }
    else
    {
        non_secure_timer_set_reload_value(REL_VALUE_FP_REGS_NOT_INVALIDATED);
    }

    non_secure_timer_stop();
}
