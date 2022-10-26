/***************************************************************************//**
 * @file
 * @brief Board Control
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_BOARD_CONTROL_CONFIG_H
#define SL_BOARD_CONTROL_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <q SL_BOARD_ENABLE_SENSOR_RHT> Enable Relative Humidity and Temperature sensor
// <i> Default: 0
#define SL_BOARD_ENABLE_SENSOR_RHT              0

// <q SL_BOARD_ENABLE_SENSOR_HALL> Enable Hall Effect sensor
// <i> Default: 0
#define SL_BOARD_ENABLE_SENSOR_HALL             0

// <q SL_BOARD_ENABLE_SENSOR_PRESSURE> Enable Barometric Pressure sensor
// <i> Default: 0
#define SL_BOARD_ENABLE_SENSOR_PRESSURE         0

// <q SL_BOARD_ENABLE_SENSOR_LIGHT> Enable Light sensor
// <i> Default: 0
#define SL_BOARD_ENABLE_SENSOR_LIGHT            0

// <q SL_BOARD_ENABLE_SENSOR_IMU> Enable Inertial Measurement Unit
// <i> Default: 0
#define SL_BOARD_ENABLE_SENSOR_IMU              0

// <q SL_BOARD_ENABLE_SENSOR_MICROPHONE> Enable Microphone
// <i> Default: 0
#define SL_BOARD_ENABLE_SENSOR_MICROPHONE       0

// <q SL_BOARD_DISABLE_MEMORY_SPI> Disable SPI Flash
// <i> Default: 1
#define SL_BOARD_DISABLE_MEMORY_SPI             1

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>

// <gpio> SL_BOARD_ENABLE_SENSOR_RHT
// $[GPIO_SL_BOARD_ENABLE_SENSOR_RHT]
#define SL_BOARD_ENABLE_SENSOR_RHT_PORT         gpioPortC
#define SL_BOARD_ENABLE_SENSOR_RHT_PIN          9
// [GPIO_SL_BOARD_ENABLE_SENSOR_RHT]$

// <gpio> SL_BOARD_ENABLE_SENSOR_HALL
// $[GPIO_SL_BOARD_ENABLE_SENSOR_HALL]
#define SL_BOARD_ENABLE_SENSOR_HALL_PORT        gpioPortC
#define SL_BOARD_ENABLE_SENSOR_HALL_PIN         9
// [GPIO_SL_BOARD_ENABLE_SENSOR_HALL]$

// <gpio> SL_BOARD_ENABLE_SENSOR_PRESSURE
// $[GPIO_SL_BOARD_ENABLE_SENSOR_PRESSURE]
#define SL_BOARD_ENABLE_SENSOR_PRESSURE_PORT    gpioPortC
#define SL_BOARD_ENABLE_SENSOR_PRESSURE_PIN     9
// [GPIO_SL_BOARD_ENABLE_SENSOR_PRESSURE]$

// <gpio> SL_BOARD_ENABLE_SENSOR_LIGHT
// $[GPIO_SL_BOARD_ENABLE_SENSOR_LIGHT]
#define SL_BOARD_ENABLE_SENSOR_LIGHT_PORT       gpioPortC
#define SL_BOARD_ENABLE_SENSOR_LIGHT_PIN        9
// [GPIO_SL_BOARD_ENABLE_SENSOR_LIGHT]$

// <gpio> SL_BOARD_ENABLE_SENSOR_IMU
// $[GPIO_SL_BOARD_ENABLE_SENSOR_IMU]
#define SL_BOARD_ENABLE_SENSOR_IMU_PORT         gpioPortC
#define SL_BOARD_ENABLE_SENSOR_IMU_PIN          9
// [GPIO_SL_BOARD_ENABLE_SENSOR_IMU]$

// <gpio> SL_BOARD_ENABLE_SENSOR_MICROPHONE
// $[GPIO_SL_BOARD_ENABLE_SENSOR_MICROPHONE]
#define SL_BOARD_ENABLE_SENSOR_MICROPHONE_PORT  gpioPortC
#define SL_BOARD_ENABLE_SENSOR_MICROPHONE_PIN   8
// [GPIO_SL_BOARD_ENABLE_SENSOR_MICROPHONE]$

// <<< sl:end pin_tool >>>

#endif // SL_BOARD_CONTROL_CONFIG_H
