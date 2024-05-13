/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 * Copyright (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TFM_PERIPHERALS_DEF_H__
#define __TFM_PERIPHERALS_DEF_H__

#include "platform_irq.h"
#include "target_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Quantized default IRQ priority, the value is:
 * (Number of configurable priority) / 4: (1UL << __NVIC_PRIO_BITS) / 4
 */
#define DEFAULT_IRQ_PRIORITY (1UL << (__NVIC_PRIO_BITS - 2))

#define TFM_TIMER0_IRQ (TIMER0_IRQn)
#define TFM_TIMER1_IRQ (TIMER1_IRQn)

#define TFM_FPU_S_TEST_IRQ (GPIO0_0_IRQn)
#define TFM_FPU_NS_TEST_IRQ (GPIO0_1_IRQn)

extern struct platform_data_t tfm_peripheral_gpio0;
extern struct platform_data_t tfm_peripheral_gpio1;
extern struct platform_data_t tfm_peripheral_gpio2;
extern struct platform_data_t tfm_peripheral_gpio3;
extern struct platform_data_t tfm_peripheral_fmc_cmsdk_gpio0;
extern struct platform_data_t tfm_peripheral_fmc_cmsdk_gpio1;
extern struct platform_data_t tfm_peripheral_fmc_cmsdk_gpio2;
extern struct platform_data_t tfm_peripheral_fmc_cmsdk_gpio3;
extern struct platform_data_t tfm_peripheral_ethernet;
extern struct platform_data_t tfm_peripheral_usb;
extern struct platform_data_t tfm_peripheral_timer0;
extern struct platform_data_t tfm_peripheral_timer1;
extern struct platform_data_t tfm_peripheral_timer2;
extern struct platform_data_t tfm_peripheral_timer3;
extern struct platform_data_t tfm_peripheral_slowclk;
extern struct platform_data_t tfm_peripheral_touch_i2c;
extern struct platform_data_t tfm_peripheral_audio_i2c;
extern struct platform_data_t tfm_peripheral_adc_spi;
extern struct platform_data_t tfm_peripheral_shield0_spi;
extern struct platform_data_t tfm_peripheral_shield1_spi;
extern struct platform_data_t tfm_peripheral_shield0_i2c;
extern struct platform_data_t tfm_peripheral_shield1_i2c;
extern struct platform_data_t tfm_peripheral_ddr4_eeprom_i2c;
extern struct platform_data_t tfm_peripheral_fpga_scc;
extern struct platform_data_t tfm_peripheral_fpga_i2s;
extern struct platform_data_t tfm_peripheral_fpga_io;
extern struct platform_data_t tfm_peripheral_std_uart;
extern struct platform_data_t tfm_peripheral_uart1;
extern struct platform_data_t tfm_peripheral_uart2;
extern struct platform_data_t tfm_peripheral_uart3;
extern struct platform_data_t tfm_peripheral_uart4;
extern struct platform_data_t tfm_peripheral_uart5;
extern struct platform_data_t tfm_peripheral_clcd;
extern struct platform_data_t tfm_peripheral_rtc;

#define TFM_PERIPHERAL_GPIO0 (&tfm_peripheral_gpio0)
#define TFM_PERIPHERAL_GPIO1 (&tfm_peripheral_gpio1)
#define TFM_PERIPHERAL_GPIO2 (&tfm_peripheral_gpio2)
#define TFM_PERIPHERAL_GPIO3 (&tfm_peripheral_gpio3)
#define TFM_PERIPHERAL_FMC_CMSDK_GPIO0 (&tfm_peripheral_fmc_cmsdk_gpio0)
#define TFM_PERIPHERAL_FMC_CMSDK_GPIO1 (&tfm_peripheral_fmc_cmsdk_gpio1)
#define TFM_PERIPHERAL_FMC_CMSDK_GPIO2 (&tfm_peripheral_fmc_cmsdk_gpio2)
#define TFM_PERIPHERAL_FMC_CMSDK_GPIO3 (&tfm_peripheral_fmc_cmsdk_gpio3)
#define TFM_PERIPHERAL_ETHERNET (&tfm_peripheral_ethernet)
#define TFM_PERIPHERAL_USB (&tfm_peripheral_usb)
#define TFM_PERIPHERAL_TIMER0 (&tfm_peripheral_timer0)
#define TFM_PERIPHERAL_TIMER1 (&tfm_peripheral_timer1)
#define TFM_PERIPHERAL_TIMER2 (&tfm_peripheral_timer2)
#define TFM_PERIPHERAL_TIMER3 (&tfm_peripheral_timer3)
#define TFM_PERIPHERAL_SLOWCLK (&tfm_peripheral_slowclk)
#define TFM_PERIPHERAL_TOUCH_I2C (&tfm_peripheral_touch_i2c)
#define TFM_PERIPHERAL_AUDIO_I2C (&tfm_peripheral_audio_i2c)
#define TFM_PERIPHERAL_ADC_SPI (&tfm_peripheral_adc_spi)
#define TFM_PERIPHERAL_SHIELD0_SPI (&tfm_peripheral_shield0_spi)
#define TFM_PERIPHERAL_SHIELD1_SPI (&tfm_peripheral_shield1_spi)
#define TFM_PERIPHERAL_SHIELD0_I2C (&tfm_peripheral_shield0_i2c)
#define TFM_PERIPHERAL_SHIELD1_I2C (&tfm_peripheral_shield1_i2c)
#define TFM_PERIPHERAL_DDR4_EEPROM_I2C (&tfm_peripheral_ddr4_eeprom_i2c)
#define TFM_PERIPHERAL_FPGA_SCC (&tfm_peripheral_fpga_scc)
#define TFM_PERIPHERAL_FPGA_I2S (&tfm_peripheral_fpga_i2s)
#define TFM_PERIPHERAL_FPGA_IO (&tfm_peripheral_fpga_io)
#define TFM_PERIPHERAL_STD_UART (&tfm_peripheral_std_uart)
#define TFM_PERIPHERAL_UART1 (&tfm_peripheral_uart1)
#define TFM_PERIPHERAL_UART2 (&tfm_peripheral_uart2)
#define TFM_PERIPHERAL_UART3 (&tfm_peripheral_uart3)
#define TFM_PERIPHERAL_UART4 (&tfm_peripheral_uart4)
#define TFM_PERIPHERAL_UART5 (&tfm_peripheral_uart5)
#define TFM_PERIPHERAL_CLCD (&tfm_peripheral_clcd)
#define TFM_PERIPHERAL_RTC (&tfm_peripheral_rtc)

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PERIPHERALS_DEF_H__ */
