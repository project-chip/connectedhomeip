/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cmsis.h"
#include "device_definition.h"
#include "systimer_armv8-m_drv.h"
#include "tfm_plat_test.h"

/* Interrupt interval is set to 1 ms */
#define TIMER_RELOAD_VALUE (SYSTIMER0_ARMV8M_DEFAULT_FREQ_HZ / 1000)

void tfm_plat_test_secure_timer_start(void)
{
    systimer_armv8_m_init(&SYSTIMER0_ARMV8_M_DEV_S);
    systimer_armv8_m_set_autoinc_reload(&SYSTIMER0_ARMV8_M_DEV_S, TIMER_RELOAD_VALUE);
    systimer_armv8_m_enable_autoinc(&SYSTIMER0_ARMV8_M_DEV_S);
    systimer_armv8_m_enable_interrupt(&SYSTIMER0_ARMV8_M_DEV_S);
}

void tfm_plat_test_secure_timer_clear_intr(void)
{
    systimer_armv8_m_clear_autoinc_interrupt(&SYSTIMER0_ARMV8_M_DEV_S);
}

void tfm_plat_test_secure_timer_stop(void)
{
    systimer_armv8_m_uninit(&SYSTIMER0_ARMV8_M_DEV_S);
    systimer_armv8_m_clear_autoinc_interrupt(&SYSTIMER0_ARMV8_M_DEV_S);
}
