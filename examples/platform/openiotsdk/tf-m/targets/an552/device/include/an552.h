/*
 * Copyright (c) 2021 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __AN552_H__
#define __AN552_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ======================  Start of section using anonymous unions  ============== */
#if defined(__CC_ARM)
#pragma push
#pragma anon_unions
#elif defined(__ICCARM__)
#pragma language = extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc11-extensions"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined(__GNUC__)
/* anonymous unions are enabled by default */
#elif defined(__TMS470__)
/* anonymous unions are enabled by default */
#elif defined(__TASKING__)
#pragma warning 586
#elif defined(__CSMC__)
/* anonymous unions are enabled by default */
#else
#warning Not supported compiler type
#endif

/* ========  Configuration of Core Peripherals  ================================== */
#define __CM55_REV 0x0001U        /* Core revision r0p1 */
#define __SAUREGION_PRESENT 1U    /* SAU regions present */
#define __MPU_PRESENT 1U          /* MPU present */
#define __VTOR_PRESENT 1U         /* VTOR present */
#define __NVIC_PRIO_BITS 3U       /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig 0U /* Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT 1U          /* FPU present */
#define __FPU_DP 1U               /* double precision FPU */
#define __DSP_PRESENT 1U          /* DSP extension present */
#define __PMU_PRESENT 1U          /* PMU present */
#define __PMU_NUM_EVENTCNT 8U     /* Number of PMU event counters */
#define __ICACHE_PRESENT 1U       /* Instruction Cache present */
#define __DCACHE_PRESENT 1U       /* Data Cache present */

#include "platform_irq.h"

#include "core_cm55.h" /* Processor and core peripherals */
#include "platform_base_address.h"
#include "platform_pins.h"
#include "platform_regs.h"
#include "system_core_init.h"

/* =====================  End of section using anonymous unions  ================ */
#if defined(__CC_ARM)
#pragma pop
#elif defined(__ICCARM__)
/* leave anonymous unions enabled */
#elif (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
/* anonymous unions are enabled by default */
#elif defined(__TMS470__)
/* anonymous unions are enabled by default */
#elif defined(__TASKING__)
#pragma warning restore
#elif defined(__CSMC__)
/* anonymous unions are enabled by default */
#else
#warning Not supported compiler type
#endif

#ifdef __cplusplus
}
#endif

#endif /* __AN552_H__ */
