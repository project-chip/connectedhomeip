/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cmsis.h"
#include "target_cfg.h"

struct platform_data_t tfm_peripheral_gpio0 = { GPIO0_CMSDK_BASE_S, GPIO0_CMSDK_BASE_S + 0xFFF, PPC_SP_MAIN_EXP0,
                                                GPIO0_MAIN_PPCEXP0_POS_MASK };

struct platform_data_t tfm_peripheral_gpio1 = { GPIO1_CMSDK_BASE_S, GPIO1_CMSDK_BASE_S + 0xFFF, PPC_SP_MAIN_EXP0,
                                                GPIO1_MAIN_PPCEXP0_POS_MASK };

struct platform_data_t tfm_peripheral_gpio2 = { GPIO2_CMSDK_BASE_S, GPIO2_CMSDK_BASE_S + 0xFFF, PPC_SP_MAIN_EXP0,
                                                GPIO2_MAIN_PPCEXP0_POS_MASK };

struct platform_data_t tfm_peripheral_gpio3 = { GPIO3_CMSDK_BASE_S, GPIO3_CMSDK_BASE_S + 0xFFF, PPC_SP_MAIN_EXP0,
                                                GPIO3_MAIN_PPCEXP0_POS_MASK };

struct platform_data_t tfm_peripheral_fmc_cmsdk_gpio0 = { FMC_CMSDK_GPIO_0_BASE_S, FMC_CMSDK_GPIO_0_BASE_S + 0xFFF,
                                                          PPC_SP_MAIN_EXP1, FMC_GPIO0_MAIN_PPCEXP0_POS_MASK };

struct platform_data_t tfm_peripheral_fmc_cmsdk_gpio1 = { FMC_CMSDK_GPIO_1_BASE_S, FMC_CMSDK_GPIO_1_BASE_S + 0xFFF,
                                                          PPC_SP_MAIN_EXP1, FMC_GPIO1_MAIN_PPCEXP0_POS_MASK };

struct platform_data_t tfm_peripheral_fmc_cmsdk_gpio2 = { FMC_CMSDK_GPIO_2_BASE_S, FMC_CMSDK_GPIO_2_BASE_S + 0xFFF,
                                                          PPC_SP_MAIN_EXP1, FMC_GPIO2_MAIN_PPCEXP0_POS_MASK };

struct platform_data_t tfm_peripheral_fmc_cmsdk_gpio3 = { FMC_CMSDK_GPIO_3_BASE_S, FMC_CMSDK_GPIO_3_BASE_S + 0xFFF,
                                                          PPC_SP_MAIN_EXP1, FMC_AHB_USER_MAIN_PPCEXP0_POS_MASK };

struct platform_data_t tfm_peripheral_ethernet = { ETHERNET_BASE_S, ETHERNET_BASE_S + 0xFFFFF, PPC_SP_MAIN_EXP0,
                                                   USB_AND_ETHERNET_MAIN_PPCEXP0_POS_MASK };

struct platform_data_t tfm_peripheral_usb = { USB_BASE_S, USB_BASE_S + 0xFFFFF, PPC_SP_MAIN_EXP0,
                                              USB_AND_ETHERNET_MAIN_PPCEXP0_POS_MASK };

struct platform_data_t tfm_peripheral_timer0 = { SYSTIMER0_ARMV8_M_BASE_S, SYSTIMER0_ARMV8_M_BASE_S + 0xFFF, PPC_SP_PERIPH0,
                                                 SYSTEM_TIMER0_PERIPH_PPC0_POS_MASK };

struct platform_data_t tfm_peripheral_timer1 = { SYSTIMER1_ARMV8_M_BASE_S, SYSTIMER1_ARMV8_M_BASE_S + 0xFFF, PPC_SP_PERIPH0,
                                                 SYSTEM_TIMER1_PERIPH_PPC0_POS_MASK };

struct platform_data_t tfm_peripheral_timer2 = { SYSTIMER2_ARMV8_M_BASE_S, SYSTIMER2_ARMV8_M_BASE_S + 0xFFF, PPC_SP_PERIPH0,
                                                 SYSTEM_TIMER2_PERIPH_PPC0_POS_MASK };

struct platform_data_t tfm_peripheral_timer3 = { SYSTIMER3_ARMV8_M_BASE_S, SYSTIMER3_ARMV8_M_BASE_S + 0xFFF, PPC_SP_PERIPH0,
                                                 SYSTEM_TIMER3_PERIPH_PPC0_POS_MASK };

struct platform_data_t tfm_peripheral_slowclk = { SLOWCLK_TIMER_CMSDK_BASE_S, SLOWCLK_TIMER_CMSDK_BASE_S + 0xFFF, PPC_SP_PERIPH1,
                                                  SLOWCLK_TIMER_PERIPH_PPC1_POS_MASK };

struct platform_data_t tfm_peripheral_touch_i2c = { FPGA_SBCon_I2C_TOUCH_BASE_S, FPGA_SBCon_I2C_TOUCH_BASE_S + 0xFFF,
                                                    PPC_SP_PERIPH_EXP1, FPGA_I2C_TOUCH_PERIPH_PPCEXP1_POS_MASK };

struct platform_data_t tfm_peripheral_audio_i2c = { FPGA_SBCon_I2C_AUDIO_BASE_S, FPGA_SBCon_I2C_AUDIO_BASE_S + 0xFFF,
                                                    PPC_SP_PERIPH_EXP1, FPGA_I2C_AUDIO_PERIPH_PPCEXP1_POS_MASK };

struct platform_data_t tfm_peripheral_adc_spi = { FPGA_SPI_ADC_BASE_S, FPGA_SPI_ADC_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP1,
                                                  FPGA_SPI_ADC_PERIPH_PPCEXP1_POS_MASK };

struct platform_data_t tfm_peripheral_shield0_spi = { FPGA_SPI_SHIELD0_BASE_S, FPGA_SPI_SHIELD0_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP1,
                                                      FPGA_SPI_SHIELD0_PERIPH_PPCEXP1_POS_MASK };

struct platform_data_t tfm_peripheral_shield1_spi = { FPGA_SPI_SHIELD1_BASE_S, FPGA_SPI_SHIELD1_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP1,
                                                      FPGA_SPI_SHIELD1_PERIPH_PPCEXP1_POS_MASK };

struct platform_data_t tfm_peripheral_shield0_i2c = { SBCon_I2C_SHIELD0_BASE_S, SBCon_I2C_SHIELD0_BASE_S + 0xFFF,
                                                      PPC_SP_PERIPH_EXP1, SBCon_I2C_SHIELD0_PERIPH_PPCEXP1_POS_MASK };

struct platform_data_t tfm_peripheral_shield1_i2c = { SBCon_I2C_SHIELD1_BASE_S, SBCon_I2C_SHIELD1_BASE_S + 0xFFF,
                                                      PPC_SP_PERIPH_EXP1, SBCon_I2C_SHIELD1_PERIPH_PPCEXP1_POS_MASK };

struct platform_data_t tfm_peripheral_ddr4_eeprom_i2c = { FPGA_DDR4_EEPROM_BASE_S, FPGA_DDR4_EEPROM_BASE_S + 0xFFF,
                                                          PPC_SP_PERIPH_EXP1, FPGA_SBCon_I2C_PERIPH_PPCEXP1_POS_MASK };

struct platform_data_t tfm_peripheral_fpga_scc = { FPGA_SCC_BASE_S, FPGA_SCC_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP2,
                                                   FPGA_SCC_PERIPH_PPCEXP2_POS_MASK };

struct platform_data_t tfm_peripheral_fpga_i2s = { FPGA_I2S_BASE_S, FPGA_I2S_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP2,
                                                   FPGA_I2S_PERIPH_PPCEXP2_POS_MASK };

struct platform_data_t tfm_peripheral_fpga_io = { FPGA_IO_BASE_S, FPGA_IO_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP2,
                                                  FPGA_IO_PERIPH_PPCEXP2_POS_MASK };

struct platform_data_t tfm_peripheral_std_uart = { UART0_BASE_NS, UART0_BASE_NS + 0xFFF, PPC_SP_DO_NOT_CONFIGURE, -1 };

struct platform_data_t tfm_peripheral_uart1 = { UART1_BASE_S, UART1_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP2,
                                                UART1_PERIPH_PPCEXP2_POS_MASK };

struct platform_data_t tfm_peripheral_uart2 = { UART2_BASE_S, UART2_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP2,
                                                UART2_PERIPH_PPCEXP2_POS_MASK };

struct platform_data_t tfm_peripheral_uart3 = { UART3_BASE_S, UART3_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP2,
                                                UART3_PERIPH_PPCEXP2_POS_MASK };

struct platform_data_t tfm_peripheral_uart4 = { UART4_BASE_S, UART4_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP2,
                                                UART4_PERIPH_PPCEXP2_POS_MASK };

struct platform_data_t tfm_peripheral_uart5 = { UART5_BASE_S, UART5_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP2,
                                                UART5_PERIPH_PPCEXP2_POS_MASK };

struct platform_data_t tfm_peripheral_clcd = { CLCD_Config_Reg_BASE_S, CLCD_Config_Reg_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP2,
                                               CLCD_PERIPH_PPCEXP2_POS_MASK };

struct platform_data_t tfm_peripheral_rtc = { RTC_BASE_S, RTC_BASE_S + 0xFFF, PPC_SP_PERIPH_EXP2, RTC_PERIPH_PPCEXP2_POS_MASK };
