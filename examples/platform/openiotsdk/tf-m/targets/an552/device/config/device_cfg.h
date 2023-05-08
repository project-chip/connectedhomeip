/*
 * Copyright (c) 2019-2022 Arm Limited. All rights reserved.
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

#ifndef __DEVICE_CFG_H__
#define __DEVICE_CFG_H__

/**
 * \file device_cfg.h
 * \brief
 * This is the device configuration file with only used peripherals
 * defined and configured via the secure and/or non-secure base address.
 */

/* ARM Memory Protection Controller (MPC) */
#define MPC_ISRAM0_S
#define MPC_ISRAM1_S
#define MPC_SRAM_S
#define MPC_QSPI_S

/* ARM Peripheral Protection Controllers (PPC) */
#define PPC_SSE300_MAIN0_S
#define PPC_SSE300_MAIN_EXP0_S
#define PPC_SSE300_MAIN_EXP1_S
#define PPC_SSE300_MAIN_EXP2_S
#define PPC_SSE300_MAIN_EXP3_S
#define PPC_SSE300_PERIPH0_S
#define PPC_SSE300_PERIPH1_S
#define PPC_SSE300_PERIPH_EXP0_S
#define PPC_SSE300_PERIPH_EXP1_S
#define PPC_SSE300_PERIPH_EXP2_S
#define PPC_SSE300_PERIPH_EXP3_S

/* ARM UART CMSDK */
#define DEFAULT_UART_BAUDRATE 115200
#define UART0_CMSDK_NS

/** System Counter Armv8-M */
#define SYSCOUNTER_CNTRL_ARMV8_M_S
#define SYSCOUNTER_CNTRL_ARMV8_M_DEV SYSCOUNTER_CNTRL_ARMV8_M_DEV_S

/**
 * Arbitrary scaling values for test purposes
 */
#define SYSCOUNTER_ARMV8_M_DEFAULT_SCALE0_INT 1u
#define SYSCOUNTER_ARMV8_M_DEFAULT_SCALE0_FRACT 0u
#define SYSCOUNTER_ARMV8_M_DEFAULT_SCALE1_INT 1u
#define SYSCOUNTER_ARMV8_M_DEFAULT_SCALE1_FRACT 0u

/* System Timer Armv8-M */
#define SYSTIMER0_ARMV8_M_S

#define SYSTIMER1_ARMV8_M_NS

#define SYSTIMER0_ARMV8M_DEFAULT_FREQ_HZ (32000000ul)
#define SYSTIMER1_ARMV8M_DEFAULT_FREQ_HZ (32000000ul)

#endif /* __DEVICE_CFG_H__ */
