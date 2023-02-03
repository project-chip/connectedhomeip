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
 * \file platform_pins.h
 * \brief This file defines all the pins for this platform.
 */

#ifndef __PLATFORM_PINS_H__
#define __PLATFORM_PINS_H__

/* AHB GPIO pin names */
enum arm_gpio_pin_name_t
{
    AHB_GPIO0_0  = 0U,
    AHB_GPIO0_1  = 1U,
    AHB_GPIO0_2  = 2U,
    AHB_GPIO0_3  = 3U,
    AHB_GPIO0_4  = 4U,
    AHB_GPIO0_5  = 5U,
    AHB_GPIO0_6  = 6U,
    AHB_GPIO0_7  = 7U,
    AHB_GPIO0_8  = 8U,
    AHB_GPIO0_9  = 9U,
    AHB_GPIO0_10 = 10U,
    AHB_GPIO0_11 = 11U,
    AHB_GPIO0_12 = 12U,
    AHB_GPIO0_13 = 13U,
    AHB_GPIO0_14 = 14U,
    AHB_GPIO0_15 = 15U,
    AHB_GPIO1_0  = 0U,
    AHB_GPIO1_1  = 1U,
    AHB_GPIO1_2  = 2U,
    AHB_GPIO1_3  = 3U,
    AHB_GPIO1_4  = 4U,
    AHB_GPIO1_5  = 5U,
    AHB_GPIO1_6  = 6U,
    AHB_GPIO1_7  = 7U,
    AHB_GPIO1_8  = 8U,
    AHB_GPIO1_9  = 9U,
    AHB_GPIO1_10 = 10U,
    AHB_GPIO1_11 = 11U,
    AHB_GPIO1_12 = 12U,
    AHB_GPIO1_13 = 13U,
    AHB_GPIO1_14 = 14U,
    AHB_GPIO1_15 = 15U,
    AHB_GPIO2_0  = 0U,
    AHB_GPIO2_1  = 1U,
    AHB_GPIO2_2  = 2U,
    AHB_GPIO2_3  = 3U,
    AHB_GPIO2_4  = 4U,
    AHB_GPIO2_5  = 5U,
    AHB_GPIO2_6  = 6U,
    AHB_GPIO2_7  = 7U,
    AHB_GPIO2_8  = 8U,
    AHB_GPIO2_9  = 9U,
    AHB_GPIO2_10 = 10U,
    AHB_GPIO2_11 = 11U,
    AHB_GPIO2_12 = 12U,
    AHB_GPIO2_13 = 13U,
    AHB_GPIO2_14 = 14U,
    AHB_GPIO2_15 = 15U,
    AHB_GPIO3_0  = 0U,
    AHB_GPIO3_1  = 1U,
    AHB_GPIO3_2  = 2U,
    AHB_GPIO3_3  = 3U,
    AHB_GPIO3_4  = 4U,
    AHB_GPIO3_5  = 5U,
    AHB_GPIO3_6  = 6U,
    AHB_GPIO3_7  = 7U,
    AHB_GPIO3_8  = 8U,
    AHB_GPIO3_9  = 9U,
    AHB_GPIO3_10 = 10U,
    AHB_GPIO3_11 = 11U,
    AHB_GPIO3_12 = 12U,
    AHB_GPIO3_13 = 13U,
    AHB_GPIO3_15 = 15U,
    AHB_GPIO3_14 = 14U,
};

/* GPIO shield 0 definition */
#define SH0_UART_RX AHB_GPIO0_0
#define SH0_UART_TX AHB_GPIO0_1
#define SH0_SPI_SS AHB_GPIO0_10
#define SH0_SPI_MOSI AHB_GPIO0_11
#define SH0_SPI_MISO AHB_GPIO0_12
#define SH0_SPI_SCK AHB_GPIO0_13
#define SH0_I2C_SDA AHB_GPIO0_14
#define SH0_I2C_SCL AHB_GPIO0_15

/* GPIO shield 1 definition */
#define SH1_UART_RX AHB_GPIO1_0
#define SH1_UART_TX AHB_GPIO1_1

#define SH1_SPI_SS AHB_GPIO1_10
#define SH1_SPI_MOSI AHB_GPIO1_11
#define SH1_SPI_MISO AHB_GPIO1_12
#define SH1_SPI_SCK AHB_GPIO1_13
#define SH1_I2C_SDA AHB_GPIO1_14
#define SH1_I2C_SCL AHB_GPIO1_15

#endif /* __PLATFORM_PINS_H__ */
