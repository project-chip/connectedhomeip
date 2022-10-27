/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cmsis.h"
#include "device_definition.h"
#include "systimer_armv8-m_drv.h"
#include "tfm_peripherals_def.h"
#include "tfm_plat_test.h"

#ifdef TEST_NS_FPU
/* Interrupt interval is set to 1 s */
#define TIMER0_RELOAD_VALUE (SYSTIMER0_ARMV8M_DEFAULT_FREQ_HZ)
#define TIMER1_RELOAD_VALUE (SYSTIMER1_ARMV8M_DEFAULT_FREQ_HZ)
#else
/* Interrupt interval is set to 1 ms */
#define TIMER0_RELOAD_VALUE (SYSTIMER0_ARMV8M_DEFAULT_FREQ_HZ / 1000)
#define TIMER1_RELOAD_VALUE (SYSTIMER1_ARMV8M_DEFAULT_FREQ_HZ / 1000)
#endif

void tfm_plat_test_secure_timer_start(void)
{
    systimer_armv8_m_init(&SYSTIMER0_ARMV8_M_DEV_S);
    systimer_armv8_m_set_autoinc_reload(&SYSTIMER0_ARMV8_M_DEV_S, TIMER0_RELOAD_VALUE);
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

void tfm_plat_test_secure_timer_set_reload_value(uint32_t value)
{
    systimer_armv8_m_set_autoinc_reload(&SYSTIMER0_ARMV8_M_DEV_S, value);
}

uint32_t tfm_plat_test_secure_timer_get_reload_value(void)
{
    return systimer_armv8_m_get_autoinc_reload(&SYSTIMER0_ARMV8_M_DEV_S);
}

void tfm_plat_test_secure_timer_nvic_configure(void)
{
    NVIC_SetPriority(TFM_TIMER0_IRQ, DEFAULT_IRQ_PRIORITY);
    NVIC_ClearTargetState(TFM_TIMER0_IRQ);
    NVIC_EnableIRQ(TFM_TIMER0_IRQ);
}

void tfm_plat_test_non_secure_timer_start(void)
{
    systimer_armv8_m_init(&SYSTIMER1_ARMV8_M_DEV_NS);
    systimer_armv8_m_set_autoinc_reload(&SYSTIMER1_ARMV8_M_DEV_NS, TIMER1_RELOAD_VALUE);
    systimer_armv8_m_enable_autoinc(&SYSTIMER1_ARMV8_M_DEV_NS);
    systimer_armv8_m_enable_interrupt(&SYSTIMER1_ARMV8_M_DEV_NS);
}

uint32_t tfm_plat_test_non_secure_timer_get_reload_value(void)
{
    return systimer_armv8_m_get_autoinc_reload(&SYSTIMER1_ARMV8_M_DEV_NS);
}

void tfm_plat_test_non_secure_timer_nvic_configure(void)
{
    NVIC_SetPriority(TFM_TIMER1_IRQ, 1);
    NVIC_EnableIRQ(TFM_TIMER1_IRQ);
}
