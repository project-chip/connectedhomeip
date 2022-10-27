/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>

#include "cmsis.h"
#include "device_definition.h"
#include "ffm/interrupt.h"
#include "load/interrupt_defs.h"
#include "spm_ipc.h"
#include "tfm_hal_interrupt.h"
#include "tfm_peripherals_def.h"
#include "tfm_plat_test.h"

#ifdef TEST_NS_FPU
#define REL_VALUE_NS_THREAD_INTERRUPTED (0xDEADBEEF)
#endif

static struct irq_t timer0_irq = { 0 };

#ifdef TEST_NS_FPU
__attribute__((naked)) void TFM_TIMER0_IRQ_Handler(void)
{
    __asm volatile("   mov    r0, lr                              \n"
                   "   b      TIMER0_IRQHandler_C                 \n");
}
#else
void TFM_TIMER0_IRQ_Handler(void)
{
    spm_handle_interrupt(timer0_irq.p_pt, timer0_irq.p_ildi);
}
#endif

enum tfm_hal_status_t tfm_timer0_irq_init(void * p_pt, struct irq_load_info_t * p_ildi)
{
    timer0_irq.p_ildi = p_ildi;
    timer0_irq.p_pt   = p_pt;

    NVIC_SetPriority(TFM_TIMER0_IRQ, DEFAULT_IRQ_PRIORITY);
    NVIC_ClearTargetState(TFM_TIMER0_IRQ);
    NVIC_DisableIRQ(TFM_TIMER0_IRQ);

    return TFM_HAL_SUCCESS;
}

#ifdef TEST_NS_FPU
/**
 * Change FP registers.
 */
__attribute__((naked)) static void change_fp_regs_in_secure_handler(void)
{
    __asm volatile("mov       r0, #0xE0000000         \n"
                   "vmov      s0, r0                  \n"
                   "mov       r0, #0xE1000000         \n"
                   "vmov      s1, r0                  \n"
                   "mov       r0, #0xE2000000         \n"
                   "vmov      s2, r0                  \n"
                   "mov       r0, #0xE3000000         \n"
                   "vmov      s3, r0                  \n"
                   "mov       r0, #0xE4000000         \n"
                   "vmov      s4, r0                  \n"
                   "mov       r0, #0xE5000000         \n"
                   "vmov      s5, r0                  \n"
                   "mov       r0, #0xE6000000         \n"
                   "vmov      s6, r0                  \n"
                   "mov       r0, #0xE7000000         \n"
                   "vmov      s7, r0                  \n"
                   "mov       r0, #0xE8000000         \n"
                   "vmov      s8, r0                  \n"
                   "mov       r0, #0xE9000000         \n"
                   "vmov      s9, r0                  \n"
                   "mov       r0, #0xEA000000         \n"
                   "vmov      s10, r0                 \n"
                   "mov       r0, #0xEB000000         \n"
                   "vmov      s11, r0                 \n"
                   "mov       r0, #0xEC000000         \n"
                   "vmov      s12, r0                 \n"
                   "mov       r0, #0xED000000         \n"
                   "vmov      s13, r0                 \n"
                   "mov       r0, #0xEE000000         \n"
                   "vmov      s14, r0                 \n"
                   "mov       r0, #0xEF000000         \n"
                   "vmov      s15, r0                 \n"
                   "mov       r0, #0xF0000000         \n"
                   "vmov      s16, r0                 \n"
                   "mov       r0, #0xF1000000         \n"
                   "vmov      s17, r0                 \n"
                   "mov       r0, #0xF2000000         \n"
                   "vmov      s18, r0                 \n"
                   "mov       r0, #0xF3000000         \n"
                   "vmov      s19, r0                 \n"
                   "mov       r0, #0xF4000000         \n"
                   "vmov      s20, r0                 \n"
                   "mov       r0, #0xF5000000         \n"
                   "vmov      s21, r0                 \n"
                   "mov       r0, #0xF6000000         \n"
                   "vmov      s22, r0                 \n"
                   "mov       r0, #0xF7000000         \n"
                   "vmov      s23, r0                 \n"
                   "mov       r0, #0xF8000000         \n"
                   "vmov      s24, r0                 \n"
                   "mov       r0, #0xF9000000         \n"
                   "vmov      s25, r0                 \n"
                   "mov       r0, #0xFA000000         \n"
                   "vmov      s26, r0                 \n"
                   "mov       r0, #0xFB000000         \n"
                   "vmov      s27, r0                 \n"
                   "mov       r0, #0xFC000000         \n"
                   "vmov      s28, r0                 \n"
                   "mov       r0, #0xFD000000         \n"
                   "vmov      s29, r0                 \n"
                   "mov       r0, #0xFE000000         \n"
                   "vmov      s30, r0                 \n"
                   "mov       r0, #0xFF000000         \n"
                   "vmov      s31, r0                 \n"

                   "bx        lr                      \n");
}

/**
 * Subroutine of secure timer0 handler function, to check whether non-secure
 * thread are interrupted by secure exception by LR register.
 */
void TIMER0_IRQHandler_C(uint32_t lr)
{
    /*
     * Check whether no-secure thread is interrupted by this exception.
     * EXC_RETURN bit[6], indicates whether a Secure or Non-secure stack is
     * used to restore stack frame on exception return.
     */
    if ((lr & EXC_RETURN_SECURE_STACK) == 0)
    {
        change_fp_regs_in_secure_handler();
        tfm_plat_test_secure_timer_stop();
        tfm_plat_test_secure_timer_set_reload_value(REL_VALUE_NS_THREAD_INTERRUPTED);
    }
    else
    {
        systimer_armv8_m_clear_autoinc_interrupt(&SYSTIMER0_ARMV8_M_DEV_S);
    }
}
#endif
