/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __MMIO_DEFS_H__
#define __MMIO_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tfm_peripherals_def.h"
#include <stdint.h>

/* Boundary handle binding macros. */
#define HANDLE_ATTR_PRIV_POS 1U
#define HANDLE_ATTR_PRIV_MASK (0x1UL << HANDLE_ATTR_PRIV_POS)
#define HANDLE_ATTR_NS_POS 0U
#define HANDLE_ATTR_NS_MASK (0x1UL << HANDLE_ATTR_NS_POS)

/* Allowed named MMIO of this platform */
const uintptr_t partition_named_mmio_list[] = { (uintptr_t) TFM_PERIPHERAL_GPIO0,
                                                (uintptr_t) TFM_PERIPHERAL_GPIO1,
                                                (uintptr_t) TFM_PERIPHERAL_GPIO2,
                                                (uintptr_t) TFM_PERIPHERAL_GPIO3,
                                                (uintptr_t) TFM_PERIPHERAL_FMC_CMSDK_GPIO0,
                                                (uintptr_t) TFM_PERIPHERAL_FMC_CMSDK_GPIO1,
                                                (uintptr_t) TFM_PERIPHERAL_FMC_CMSDK_GPIO2,
                                                (uintptr_t) TFM_PERIPHERAL_FMC_CMSDK_GPIO3,
                                                (uintptr_t) TFM_PERIPHERAL_ETHERNET,
                                                (uintptr_t) TFM_PERIPHERAL_USB,
                                                (uintptr_t) TFM_PERIPHERAL_TIMER0,
                                                (uintptr_t) TFM_PERIPHERAL_TIMER1,
                                                (uintptr_t) TFM_PERIPHERAL_TIMER2,
                                                (uintptr_t) TFM_PERIPHERAL_TIMER3,
                                                (uintptr_t) TFM_PERIPHERAL_SLOWCLK,
                                                (uintptr_t) TFM_PERIPHERAL_TOUCH_I2C,
                                                (uintptr_t) TFM_PERIPHERAL_AUDIO_I2C,
                                                (uintptr_t) TFM_PERIPHERAL_ADC_SPI,
                                                (uintptr_t) TFM_PERIPHERAL_SHIELD0_SPI,
                                                (uintptr_t) TFM_PERIPHERAL_SHIELD1_SPI,
                                                (uintptr_t) TFM_PERIPHERAL_SHIELD0_I2C,
                                                (uintptr_t) TFM_PERIPHERAL_SHIELD1_I2C,
                                                (uintptr_t) TFM_PERIPHERAL_DDR4_EEPROM_I2C,
                                                (uintptr_t) TFM_PERIPHERAL_FPGA_SCC,
                                                (uintptr_t) TFM_PERIPHERAL_FPGA_I2S,
                                                (uintptr_t) TFM_PERIPHERAL_FPGA_IO,
                                                (uintptr_t) TFM_PERIPHERAL_STD_UART,
                                                (uintptr_t) TFM_PERIPHERAL_UART1,
                                                (uintptr_t) TFM_PERIPHERAL_UART2,
                                                (uintptr_t) TFM_PERIPHERAL_UART3,
                                                (uintptr_t) TFM_PERIPHERAL_UART4,
                                                (uintptr_t) TFM_PERIPHERAL_UART5,
                                                (uintptr_t) TFM_PERIPHERAL_CLCD,
                                                (uintptr_t) TFM_PERIPHERAL_RTC };

/*
 * Platform AN552 only has named MMIO.
 * If the platform has numbered MMIO, define them in another list.
 */

#ifdef __cplusplus
}
#endif

#endif /* __MMIO_DEFS_H__ */
