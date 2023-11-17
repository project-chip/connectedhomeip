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

/**
 * \file device_definition.c
 * \brief This file defines exports the structures based on the peripheral
 * definitions from device_cfg.h.
 * This file is meant to be used as a helper for baremetal
 * applications and/or as an example of how to configure the generic
 * driver structures.
 */

#include "device_definition.h"
#include "platform/include/tfm_plat_defs.h"
#include "platform_base_address.h"

/* UART CMSDK driver structures */
#ifdef UART0_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART0_CMSDK_DEV_CFG_S = { .base             = UART0_BASE_S,
                                                                   .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART0_CMSDK_DEV_DATA_S     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART0_CMSDK_DEV_S                      = { &(UART0_CMSDK_DEV_CFG_S), &(UART0_CMSDK_DEV_DATA_S) };
#endif
#ifdef UART0_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART0_CMSDK_DEV_CFG_NS = { .base             = UART0_BASE_NS,
                                                                    .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART0_CMSDK_DEV_DATA_NS     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART0_CMSDK_DEV_NS                      = { &(UART0_CMSDK_DEV_CFG_NS), &(UART0_CMSDK_DEV_DATA_NS) };
#endif

#ifdef UART1_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART1_CMSDK_DEV_CFG_S = { .base             = UART1_BASE_S,
                                                                   .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART1_CMSDK_DEV_DATA_S     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART1_CMSDK_DEV_S                      = { &(UART1_CMSDK_DEV_CFG_S), &(UART1_CMSDK_DEV_DATA_S) };
#endif
#ifdef UART1_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART1_CMSDK_DEV_CFG_NS = { .base             = UART1_BASE_NS,
                                                                    .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART1_CMSDK_DEV_DATA_NS     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART1_CMSDK_DEV_NS                      = { &(UART1_CMSDK_DEV_CFG_NS), &(UART1_CMSDK_DEV_DATA_NS) };
#endif

#ifdef UART2_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART2_CMSDK_DEV_CFG_S = { .base             = UART2_BASE_S,
                                                                   .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART2_CMSDK_DEV_DATA_S     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART2_CMSDK_DEV_S                      = { &(UART2_CMSDK_DEV_CFG_S), &(UART2_CMSDK_DEV_DATA_S) };
#endif
#ifdef UART2_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART2_CMSDK_DEV_CFG_NS = { .base             = UART2_BASE_NS,
                                                                    .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART2_CMSDK_DEV_DATA_NS     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART2_CMSDK_DEV_NS                      = { &(UART2_CMSDK_DEV_CFG_NS), &(UART2_CMSDK_DEV_DATA_NS) };
#endif

#ifdef UART3_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART3_CMSDK_DEV_CFG_S = { .base             = UART3_BASE_S,
                                                                   .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART3_CMSDK_DEV_DATA_S     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART3_CMSDK_DEV_S                      = { &(UART3_CMSDK_DEV_CFG_S), &(UART3_CMSDK_DEV_DATA_S) };
#endif
#ifdef UART3_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART3_CMSDK_DEV_CFG_NS = { .base             = UART3_BASE_NS,
                                                                    .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART3_CMSDK_DEV_DATA_NS     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART3_CMSDK_DEV_NS                      = { &(UART3_CMSDK_DEV_CFG_NS), &(UART3_CMSDK_DEV_DATA_NS) };
#endif

#ifdef UART4_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART4_CMSDK_DEV_CFG_S = { .base             = UART4_BASE_S,
                                                                   .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART4_CMSDK_DEV_DATA_S     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART4_CMSDK_DEV_S                      = { &(UART4_CMSDK_DEV_CFG_S), &(UART4_CMSDK_DEV_DATA_S) };
#endif
#ifdef UART4_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART4_CMSDK_DEV_CFG_NS = { .base             = UART4_BASE_NS,
                                                                    .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART4_CMSDK_DEV_DATA_NS     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART4_CMSDK_DEV_NS                      = { &(UART4_CMSDK_DEV_CFG_NS), &(UART4_CMSDK_DEV_DATA_NS) };
#endif

#ifdef UART5_CMSDK_S
static const struct uart_cmsdk_dev_cfg_t UART5_CMSDK_DEV_CFG_S = { .base             = UART5_BASE_S,
                                                                   .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART5_CMSDK_DEV_DATA_S     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART5_CMSDK_DEV_S                      = { &(UART5_CMSDK_DEV_CFG_S), &(UART5_CMSDK_DEV_DATA_S) };
#endif
#ifdef UART5_CMSDK_NS
static const struct uart_cmsdk_dev_cfg_t UART5_CMSDK_DEV_CFG_NS = { .base             = UART5_BASE_NS,
                                                                    .default_baudrate = DEFAULT_UART_BAUDRATE };
static struct uart_cmsdk_dev_data_t UART5_CMSDK_DEV_DATA_NS     = { .state = 0, .system_clk = 0, .baudrate = 0 };
struct uart_cmsdk_dev_t UART5_CMSDK_DEV_NS                      = { &(UART5_CMSDK_DEV_CFG_NS), &(UART5_CMSDK_DEV_DATA_NS) };
#endif

/* SSE-300 PPC driver structures */
#ifdef PPC_SSE300_MAIN0_S
static struct ppc_sse300_dev_cfg_t PPC_SSE300_MAIN0_CFG_S   = { .sacfg_base  = SSE300_SACFG_BASE_S,
                                                                .nsacfg_base = SSE300_NSACFG_BASE_NS,
                                                                .ppc_name    = PPC_SSE300_MAIN0 };
static struct ppc_sse300_dev_data_t PPC_SSE300_MAIN0_DATA_S = {
    .sacfg_ns_ppc = 0, .sacfg_sp_ppc = 0, .nsacfg_nsp_ppc = 0, .int_bit_mask = 0, .is_initialized = false
};
struct ppc_sse300_dev_t PPC_SSE300_MAIN0_DEV_S = { &PPC_SSE300_MAIN0_CFG_S, &PPC_SSE300_MAIN0_DATA_S };
#endif

#ifdef PPC_SSE300_MAIN_EXP0_S
static struct ppc_sse300_dev_cfg_t PPC_SSE300_MAIN_EXP0_CFG_S   = { .sacfg_base  = SSE300_SACFG_BASE_S,
                                                                    .nsacfg_base = SSE300_NSACFG_BASE_NS,
                                                                    .ppc_name    = PPC_SSE300_MAIN_EXP0 };
static struct ppc_sse300_dev_data_t PPC_SSE300_MAIN_EXP0_DATA_S = {
    .sacfg_ns_ppc = 0, .sacfg_sp_ppc = 0, .nsacfg_nsp_ppc = 0, .int_bit_mask = 0, .is_initialized = false
};
struct ppc_sse300_dev_t PPC_SSE300_MAIN_EXP0_DEV_S = { &PPC_SSE300_MAIN_EXP0_CFG_S, &PPC_SSE300_MAIN_EXP0_DATA_S };
#endif

#ifdef PPC_SSE300_MAIN_EXP1_S
static struct ppc_sse300_dev_cfg_t PPC_SSE300_MAIN_EXP1_CFG_S   = { .sacfg_base  = SSE300_SACFG_BASE_S,
                                                                    .nsacfg_base = SSE300_NSACFG_BASE_NS,
                                                                    .ppc_name    = PPC_SSE300_MAIN_EXP1 };
static struct ppc_sse300_dev_data_t PPC_SSE300_MAIN_EXP1_DATA_S = {
    .sacfg_ns_ppc = 0, .sacfg_sp_ppc = 0, .nsacfg_nsp_ppc = 0, .int_bit_mask = 0, .is_initialized = false
};
struct ppc_sse300_dev_t PPC_SSE300_MAIN_EXP1_DEV_S = { &PPC_SSE300_MAIN_EXP1_CFG_S, &PPC_SSE300_MAIN_EXP1_DATA_S };
#endif

#ifdef PPC_SSE300_MAIN_EXP2_S
static struct ppc_sse300_dev_cfg_t PPC_SSE300_MAIN_EXP2_CFG_S   = { .sacfg_base  = SSE300_SACFG_BASE_S,
                                                                    .nsacfg_base = SSE300_NSACFG_BASE_NS,
                                                                    .ppc_name    = PPC_SSE300_MAIN_EXP2 };
static struct ppc_sse300_dev_data_t PPC_SSE300_MAIN_EXP2_DATA_S = {
    .sacfg_ns_ppc = 0, .sacfg_sp_ppc = 0, .nsacfg_nsp_ppc = 0, .int_bit_mask = 0, .is_initialized = false
};
struct ppc_sse300_dev_t PPC_SSE300_MAIN_EXP2_DEV_S = { &PPC_SSE300_MAIN_EXP2_CFG_S, &PPC_SSE300_MAIN_EXP2_DATA_S };
#endif

#ifdef PPC_SSE300_MAIN_EXP3_S
static struct ppc_sse300_dev_cfg_t PPC_SSE300_MAIN_EXP3_CFG_S   = { .sacfg_base  = SSE300_SACFG_BASE_S,
                                                                    .nsacfg_base = SSE300_NSACFG_BASE_NS,
                                                                    .ppc_name    = PPC_SSE300_MAIN_EXP3 };
static struct ppc_sse300_dev_data_t PPC_SSE300_MAIN_EXP3_DATA_S = {
    .sacfg_ns_ppc = 0, .sacfg_sp_ppc = 0, .nsacfg_nsp_ppc = 0, .int_bit_mask = 0, .is_initialized = false
};
struct ppc_sse300_dev_t PPC_SSE300_MAIN_EXP3_DEV_S = { &PPC_SSE300_MAIN_EXP3_CFG_S, &PPC_SSE300_MAIN_EXP3_DATA_S };
#endif

#ifdef PPC_SSE300_PERIPH0_S
static struct ppc_sse300_dev_cfg_t PPC_SSE300_PERIPH0_CFG_S   = { .sacfg_base  = SSE300_SACFG_BASE_S,
                                                                  .nsacfg_base = SSE300_NSACFG_BASE_NS,
                                                                  .ppc_name    = PPC_SSE300_PERIPH0 };
static struct ppc_sse300_dev_data_t PPC_SSE300_PERIPH0_DATA_S = {
    .sacfg_ns_ppc = 0, .sacfg_sp_ppc = 0, .nsacfg_nsp_ppc = 0, .int_bit_mask = 0, .is_initialized = false
};
struct ppc_sse300_dev_t PPC_SSE300_PERIPH0_DEV_S = { &PPC_SSE300_PERIPH0_CFG_S, &PPC_SSE300_PERIPH0_DATA_S };
#endif

#ifdef PPC_SSE300_PERIPH1_S
static struct ppc_sse300_dev_cfg_t PPC_SSE300_PERIPH1_CFG_S   = { .sacfg_base  = SSE300_SACFG_BASE_S,
                                                                  .nsacfg_base = SSE300_NSACFG_BASE_NS,
                                                                  .ppc_name    = PPC_SSE300_PERIPH1 };
static struct ppc_sse300_dev_data_t PPC_SSE300_PERIPH1_DATA_S = {
    .sacfg_ns_ppc = 0, .sacfg_sp_ppc = 0, .nsacfg_nsp_ppc = 0, .int_bit_mask = 0, .is_initialized = false
};
struct ppc_sse300_dev_t PPC_SSE300_PERIPH1_DEV_S = { &PPC_SSE300_PERIPH1_CFG_S, &PPC_SSE300_PERIPH1_DATA_S };
#endif

#ifdef PPC_SSE300_PERIPH_EXP0_S
static struct ppc_sse300_dev_cfg_t PPC_SSE300_PERIPH_EXP0_CFG_S   = { .sacfg_base  = SSE300_SACFG_BASE_S,
                                                                      .nsacfg_base = SSE300_NSACFG_BASE_NS,
                                                                      .ppc_name    = PPC_SSE300_PERIPH_EXP0 };
static struct ppc_sse300_dev_data_t PPC_SSE300_PERIPH_EXP0_DATA_S = {
    .sacfg_ns_ppc = 0, .sacfg_sp_ppc = 0, .nsacfg_nsp_ppc = 0, .int_bit_mask = 0, .is_initialized = false
};
struct ppc_sse300_dev_t PPC_SSE300_PERIPH_EXP0_DEV_S = { &PPC_SSE300_PERIPH_EXP0_CFG_S, &PPC_SSE300_PERIPH_EXP0_DATA_S };
#endif

#ifdef PPC_SSE300_PERIPH_EXP1_S
static struct ppc_sse300_dev_cfg_t PPC_SSE300_PERIPH_EXP1_CFG_S   = { .sacfg_base  = SSE300_SACFG_BASE_S,
                                                                      .nsacfg_base = SSE300_NSACFG_BASE_NS,
                                                                      .ppc_name    = PPC_SSE300_PERIPH_EXP1 };
static struct ppc_sse300_dev_data_t PPC_SSE300_PERIPH_EXP1_DATA_S = {
    .sacfg_ns_ppc = 0, .sacfg_sp_ppc = 0, .nsacfg_nsp_ppc = 0, .int_bit_mask = 0, .is_initialized = false
};
struct ppc_sse300_dev_t PPC_SSE300_PERIPH_EXP1_DEV_S = { &PPC_SSE300_PERIPH_EXP1_CFG_S, &PPC_SSE300_PERIPH_EXP1_DATA_S };
#endif

#ifdef PPC_SSE300_PERIPH_EXP2_S
static struct ppc_sse300_dev_cfg_t PPC_SSE300_PERIPH_EXP2_CFG_S   = { .sacfg_base  = SSE300_SACFG_BASE_S,
                                                                      .nsacfg_base = SSE300_NSACFG_BASE_NS,
                                                                      .ppc_name    = PPC_SSE300_PERIPH_EXP2 };
static struct ppc_sse300_dev_data_t PPC_SSE300_PERIPH_EXP2_DATA_S = {
    .sacfg_ns_ppc = 0, .sacfg_sp_ppc = 0, .nsacfg_nsp_ppc = 0, .int_bit_mask = 0, .is_initialized = false
};
struct ppc_sse300_dev_t PPC_SSE300_PERIPH_EXP2_DEV_S = { &PPC_SSE300_PERIPH_EXP2_CFG_S, &PPC_SSE300_PERIPH_EXP2_DATA_S };
#endif

#ifdef PPC_SSE300_PERIPH_EXP3_S
static struct ppc_sse300_dev_cfg_t PPC_SSE300_PERIPH_EXP3_CFG_S   = { .sacfg_base  = SSE300_SACFG_BASE_S,
                                                                      .nsacfg_base = SSE300_NSACFG_BASE_NS,
                                                                      .ppc_name    = PPC_SSE300_PERIPH_EXP3 };
static struct ppc_sse300_dev_data_t PPC_SSE300_PERIPH_EXP3_DATA_S = {
    .sacfg_ns_ppc = 0, .sacfg_sp_ppc = 0, .nsacfg_nsp_ppc = 0, .int_bit_mask = 0, .is_initialized = false
};
struct ppc_sse300_dev_t PPC_SSE300_PERIPH_EXP3_DEV_S = { &PPC_SSE300_PERIPH_EXP3_CFG_S, &PPC_SSE300_PERIPH_EXP3_DATA_S };
#endif

/* System counters */
#ifdef SYSCOUNTER_CNTRL_ARMV8_M_S

#if SYSCOUNTER_ARMV8_M_DEFAULT_SCALE0_INT > SYSCOUNTER_ARMV8_M_SCALE_VAL_INT_MAX
#error SYSCOUNTER_ARMV8_M_DEFAULT_SCALE0_INT is invalid.
#endif
#if SYSCOUNTER_ARMV8_M_DEFAULT_SCALE0_FRACT > SYSCOUNTER_ARMV8_M_SCALE_VAL_FRACT_MAX
#error SYSCOUNTER_ARMV8_M_DEFAULT_SCALE0_FRACT is invalid.
#endif
#if SYSCOUNTER_ARMV8_M_DEFAULT_SCALE1_INT > SYSCOUNTER_ARMV8_M_SCALE_VAL_INT_MAX
#error SYSCOUNTER_ARMV8_M_DEFAULT_SCALE1_INT is invalid.
#endif
#if SYSCOUNTER_ARMV8_M_DEFAULT_SCALE1_FRACT > SYSCOUNTER_ARMV8_M_SCALE_VAL_FRACT_MAX
#error SYSCOUNTER_ARMV8_M_DEFAULT_SCALE1_FRACT is invalid.
#endif

static const struct syscounter_armv8_m_cntrl_dev_cfg_t SYSCOUNTER_CNTRL_ARMV8_M_DEV_CFG_S = {
    .base                        = SYSCNTR_CNTRL_BASE_S,
    .scale0.integer              = SYSCOUNTER_ARMV8_M_DEFAULT_SCALE0_INT,
    .scale0.fixed_point_fraction = SYSCOUNTER_ARMV8_M_DEFAULT_SCALE0_FRACT,
    .scale1.integer              = SYSCOUNTER_ARMV8_M_DEFAULT_SCALE1_INT,
    .scale1.fixed_point_fraction = SYSCOUNTER_ARMV8_M_DEFAULT_SCALE1_FRACT
};
static struct syscounter_armv8_m_cntrl_dev_data_t SYSCOUNTER_CNTRL_ARMV8_M_DEV_DATA_S = { .is_initialized = false };
struct syscounter_armv8_m_cntrl_dev_t SYSCOUNTER_CNTRL_ARMV8_M_DEV_S                  = { &(SYSCOUNTER_CNTRL_ARMV8_M_DEV_CFG_S),
                                                                                          &(SYSCOUNTER_CNTRL_ARMV8_M_DEV_DATA_S) };
#endif

#ifdef SYSCOUNTER_READ_ARMV8_M_S
static const struct syscounter_armv8_m_read_dev_cfg_t SYSCOUNTER_READ_ARMV8_M_DEV_CFG_S = {
    .base = SYSCNTR_READ_BASE_S,
};
struct syscounter_armv8_m_read_dev_t SYSCOUNTER_READ_ARMV8_M_DEV_S = {
    &(SYSCOUNTER_READ_ARMV8_M_DEV_CFG_S),
};
#endif
#ifdef SYSCOUNTER_READ_ARMV8_M_NS
static const struct syscounter_armv8_m_read_dev_cfg_t SYSCOUNTER_READ_ARMV8_M_DEV_CFG_NS = {
    .base = SYSCNTR_READ_BASE_NS,
};
struct syscounter_armv8_m_read_dev_t SYSCOUNTER_READ_ARMV8_M_DEV_NS = {
    &(SYSCOUNTER_CNTRL_ARMV8_M_DEV_CFG_NS),
};
#endif

/* System timers */
#ifdef SYSTIMER0_ARMV8_M_S
static const struct systimer_armv8_m_dev_cfg_t SYSTIMER0_ARMV8_M_DEV_CFG_S
#ifdef TEST_NS_SLIH_IRQ
    TFM_LINK_SET_RO_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#elif defined(TEST_NS_FLIH_IRQ)
    TFM_LINK_SET_RO_IN_PARTITION_SECTION("TFM_SP_FLIH_TEST", "APP-ROT")
#endif
    = { .base = SYSTIMER0_ARMV8_M_BASE_S, .default_freq_hz = SYSTIMER0_ARMV8M_DEFAULT_FREQ_HZ };
static struct systimer_armv8_m_dev_data_t SYSTIMER0_ARMV8_M_DEV_DATA_S
#ifdef TEST_NS_SLIH_IRQ
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#elif defined(TEST_NS_FLIH_IRQ)
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_FLIH_TEST", "APP-ROT")
#endif
    = { .is_initialized = false };
struct systimer_armv8_m_dev_t SYSTIMER0_ARMV8_M_DEV_S
#ifdef TEST_NS_SLIH_IRQ
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_SLIH_TEST", "APP-ROT")
#elif defined(TEST_NS_FLIH_IRQ)
    TFM_LINK_SET_RW_IN_PARTITION_SECTION("TFM_SP_FLIH_TEST", "APP-ROT")
#endif
    = { &(SYSTIMER0_ARMV8_M_DEV_CFG_S), &(SYSTIMER0_ARMV8_M_DEV_DATA_S) };
#endif

#ifdef SYSTIMER0_ARMV8_M_NS
static const struct systimer_armv8_m_dev_cfg_t SYSTIMER0_ARMV8_M_DEV_CFG_NS = { .base = SYSTIMER0_ARMV8_M_BASE_NS,
                                                                                .default_freq_hz =
                                                                                    SYSTIMER0_ARMV8M_DEFAULT_FREQ_HZ };
static struct systimer_armv8_m_dev_data_t SYSTIMER0_ARMV8_M_DEV_DATA_NS     = { .is_initialized = false };
struct systimer_armv8_m_dev_t SYSTIMER0_ARMV8_M_DEV_NS = { &(SYSTIMER0_ARMV8_M_DEV_CFG_NS), &(SYSTIMER0_ARMV8_M_DEV_DATA_NS) };
#endif

#ifdef SYSTIMER1_ARMV8_M_S
static const struct systimer_armv8_m_dev_cfg_t SYSTIMER1_ARMV8_M_DEV_CFG_S = { .base = SYSTIMER1_ARMV8_M_BASE_S,
                                                                               .default_freq_hz =
                                                                                   SYSTIMER1_ARMV8M_DEFAULT_FREQ_HZ };
static struct systimer_armv8_m_dev_data_t SYSTIMER1_ARMV8_M_DEV_DATA_S     = { .is_initialized = false };
struct systimer_armv8_m_dev_t SYSTIMER1_ARMV8_M_DEV_S = { &(SYSTIMER1_ARMV8_M_DEV_CFG_S), &(SYSTIMER1_ARMV8_M_DEV_DATA_S) };
#endif

#ifdef SYSTIMER1_ARMV8_M_NS
static const struct systimer_armv8_m_dev_cfg_t SYSTIMER1_ARMV8_M_DEV_CFG_NS = { .base = SYSTIMER1_ARMV8_M_BASE_NS,
                                                                                .default_freq_hz =
                                                                                    SYSTIMER1_ARMV8M_DEFAULT_FREQ_HZ };
static struct systimer_armv8_m_dev_data_t SYSTIMER1_ARMV8_M_DEV_DATA_NS     = { .is_initialized = false };
struct systimer_armv8_m_dev_t SYSTIMER1_ARMV8_M_DEV_NS = { &(SYSTIMER1_ARMV8_M_DEV_CFG_NS), &(SYSTIMER1_ARMV8_M_DEV_DATA_NS) };
#endif

#ifdef SYSTIMER2_ARMV8_M_S
static const struct systimer_armv8_m_dev_cfg_t SYSTIMER2_ARMV8_M_DEV_CFG_S = { .base = SYSTIMER2_ARMV8_M_BASE_S,
                                                                               .default_freq_hz =
                                                                                   SYSTIMER2_ARMV8M_DEFAULT_FREQ_HZ };
static struct systimer_armv8_m_dev_data_t SYSTIMER2_ARMV8_M_DEV_DATA_S     = { .is_initialized = false };
struct systimer_armv8_m_dev_t SYSTIMER2_ARMV8_M_DEV_S = { &(SYSTIMER2_ARMV8_M_DEV_CFG_S), &(SYSTIMER2_ARMV8_M_DEV_DATA_S) };
#endif

#ifdef SYSTIMER2_ARMV8_M_NS
static const struct systimer_armv8_m_dev_cfg_t SYSTIMER2_ARMV8_M_DEV_CFG_NS = { .base = SYSTIMER2_ARMV8_M_BASE_NS,
                                                                                .default_freq_hz =
                                                                                    SYSTIMER2_ARMV8M_DEFAULT_FREQ_HZ };
static struct systimer_armv8_m_dev_data_t SYSTIMER2_ARMV8_M_DEV_DATA_NS     = { .is_initialized = false };
struct systimer_armv8_m_dev_t SYSTIMER2_ARMV8_M_DEV_NS = { &(SYSTIMER2_ARMV8_M_DEV_CFG_NS), &(SYSTIMER2_ARMV8_M_DEV_DATA_NS) };
#endif

#ifdef SYSTIMER3_ARMV8_M_S
static const struct systimer_armv8_m_dev_cfg_t SYSTIMER3_ARMV8_M_DEV_CFG_S = { .base = SYSTIMER3_ARMV8_M_BASE_S,
                                                                               .default_freq_hz =
                                                                                   SYSTIMER3_ARMV8M_DEFAULT_FREQ_HZ };
static struct systimer_armv8_m_dev_data_t SYSTIMER3_ARMV8_M_DEV_DATA_S     = { .is_initialized = false };
struct systimer_armv8_m_dev_t SYSTIMER3_ARMV8_M_DEV_S = { &(SYSTIMER3_ARMV8_M_DEV_CFG_S), &(SYSTIMER3_ARMV8_M_DEV_DATA_S) };
#endif

#ifdef SYSTIMER3_ARMV8_M_NS
static const struct systimer_armv8_m_dev_cfg_t SYSTIMER3_ARMV8_M_DEV_CFG_NS = { .base = SYSTIMER3_ARMV8_M_BASE_NS,
                                                                                .default_freq_hz =
                                                                                    SYSTIMER3_ARMV8M_DEFAULT_FREQ_HZ };
static struct systimer_armv8_m_dev_data_t SYSTIMER3_ARMV8_M_DEV_DATA_NS     = { .is_initialized = false };
struct systimer_armv8_m_dev_t SYSTIMER3_ARMV8_M_DEV_NS = { &(SYSTIMER3_ARMV8_M_DEV_CFG_NS), &(SYSTIMER3_ARMV8_M_DEV_DATA_NS) };
#endif

/* System Watchdogs */
#ifdef SYSWDOG_ARMV8_M_S
static const struct syswdog_armv8_m_dev_cfg_t SYSWDOG_ARMV8_M_DEV_CFG_S = { .base = SYSWDOG_ARMV8_M_CNTRL_BASE_S };
struct syswdog_armv8_m_dev_t SYSWDOG_ARMV8_M_DEV_S                      = { &(SYSWDOG_ARMV8_M_DEV_CFG_S) };
#endif

#ifdef SYSWDOG_ARMV8_M_NS
static const struct syswdog_armv8_m_dev_cfg_t SYSWDOG_ARMV8_M_DEV_CFG_NS = { .base = SYSWDOG_ARMV8_M_CNTRL_BASE_NS };
struct syswdog_armv8_m_dev_t SYSWDOG_ARMV8_M_DEV_NS                      = { &(SYSWDOG_ARMV8_M_DEV_CFG_NS) };
#endif

/* ARM MPC SSE 300 driver structures */
#ifdef MPC_SRAM_S
static const struct mpc_sie_dev_cfg_t MPC_SRAM_DEV_CFG_S = { .base = MPC_SRAM_BASE_S };
static struct mpc_sie_dev_data_t MPC_SRAM_DEV_DATA_S     = { .range_list = 0, .nbr_of_ranges = 0, .is_initialized = false };
struct mpc_sie_dev_t MPC_SRAM_DEV_S                      = { &(MPC_SRAM_DEV_CFG_S), &(MPC_SRAM_DEV_DATA_S) };
#endif

#ifdef MPC_QSPI_S
static const struct mpc_sie_dev_cfg_t MPC_QSPI_DEV_CFG_S = { .base = MPC_QSPI_BASE_S };
static struct mpc_sie_dev_data_t MPC_QSPI_DEV_DATA_S     = { .range_list = 0, .nbr_of_ranges = 0, .is_initialized = false };
struct mpc_sie_dev_t MPC_QSPI_DEV_S                      = { &(MPC_QSPI_DEV_CFG_S), &(MPC_QSPI_DEV_DATA_S) };
#endif

#ifdef MPC_DDR4_S
static const struct mpc_sie_dev_cfg_t MPC_DDR4_DEV_CFG_S = { .base = MPC_DDR4_BASE_S };
static struct mpc_sie_dev_data_t MPC_DDR4_DEV_DATA_S     = { .range_list = 0, .nbr_of_ranges = 0, .is_initialized = false };
struct mpc_sie_dev_t MPC_DDR4_DEV_S                      = { &(MPC_DDR4_DEV_CFG_S), &(MPC_DDR4_DEV_DATA_S) };
#endif

#ifdef MPC_ISRAM0_S
static const struct mpc_sie_dev_cfg_t MPC_ISRAM0_DEV_CFG_S = { .base = MPC_ISRAM0_BASE_S };
static struct mpc_sie_dev_data_t MPC_ISRAM0_DEV_DATA_S     = { .range_list = 0, .nbr_of_ranges = 0, .is_initialized = false };
struct mpc_sie_dev_t MPC_ISRAM0_DEV_S                      = { &(MPC_ISRAM0_DEV_CFG_S), &(MPC_ISRAM0_DEV_DATA_S) };
#endif

#ifdef MPC_ISRAM1_S
static const struct mpc_sie_dev_cfg_t MPC_ISRAM1_DEV_CFG_S = { .base = MPC_ISRAM1_BASE_S };
static struct mpc_sie_dev_data_t MPC_ISRAM1_DEV_DATA_S     = { .range_list = 0, .nbr_of_ranges = 0, .is_initialized = false };
struct mpc_sie_dev_t MPC_ISRAM1_DEV_S                      = { &(MPC_ISRAM1_DEV_CFG_S), &(MPC_ISRAM1_DEV_DATA_S) };
#endif

#ifdef MPS3_IO_S
static struct arm_mps3_io_dev_cfg_t MPS3_IO_DEV_CFG_S = { .base = FPGA_IO_BASE_S };
struct arm_mps3_io_dev_t MPS3_IO_DEV_S                = { .cfg = &(MPS3_IO_DEV_CFG_S) };
#endif

#ifdef MPS3_IO_NS
static struct arm_mps3_io_dev_cfg_t MPS3_IO_DEV_CFG_NS = { .base = FPGA_IO_BASE_NS };
struct arm_mps3_io_dev_t MPS3_IO_DEV_NS                = { .cfg = &(MPS3_IO_DEV_CFG_NS) };
#endif
