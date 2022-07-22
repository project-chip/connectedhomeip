/*******************************************************************************
 * File Name: cycfg_routing.h
 *
 * Description:
 * Establishes all necessary connections between hardware elements.
 * This file was automatically generated and should not be modified.
 * Tools Package 2.3.0.4276
 * 30739A0 CSP
 * personalities 1.0.0.31
 * udd 3.0.0.1636
 *
 ********************************************************************************
 * Copyright 2022 Cypress Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ********************************************************************************/

#if !defined(CYCFG_ROUTING_H)
#define CYCFG_ROUTING_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "cycfg_notices.h"
static inline void init_cycfg_routing(void) {}
#define init_cycfg_connectivity() init_cycfg_routing()
#if CHIP_PACKAGE_WLCSP
#define ioss_0_pin_4_AUX UNKNOWN
#define ioss_0_pin_6_AUX UNKNOWN
#define ioss_0_pin_10_AUX UNKNOWN
#define ioss_0_pin_17_AUX UNKNOWN
#define ioss_0_pin_26_AUX UNKNOWN
#else // !CHIP_PACKAGE_WLCSP
#define ioss_0_pin_4_AUX UNKNOWN
#define ioss_0_pin_6_AUX UNKNOWN
#define ioss_0_pin_10_AUX UNKNOWN
#define ioss_0_pin_16_AUX UNKNOWN
#define ioss_0_pin_17_AUX UNKNOWN
#define ioss_0_pin_25_AUX UNKNOWN
#define ioss_0_pin_26_AUX UNKNOWN
#define ioss_0_pin_28_AUX UNKNOWN
#define ioss_0_pin_29_AUX UNKNOWN
#endif

#define amplifiers_0_rx_pu_0_TRIGGER_IN WICED_RX_PU
#define amplifiers_0_tx_pu_0_TRIGGER_IN WICED_TX_PU
#define pwm_0_pwm_0_TRIGGER_IN WICED_PWM0
#define spi_1_clk_0_TRIGGER_IN WICED_SPI_2_CLK
#define spi_1_cs_0_TRIGGER_IN WICED_SPI_2_CS
#define spi_1_miso_0_TRIGGER_IN WICED_SPI_2_MISO
#define spi_1_mosi_0_TRIGGER_IN WICED_SPI_2_MOSI
#define uart_1_rxd_0_TRIGGER_IN WICED_UART_2_RXD
#define uart_1_txd_0_TRIGGER_IN WICED_UART_2_TXD

#if defined(__cplusplus)
}
#endif

#endif /* CYCFG_ROUTING_H */
