/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file AXP192.h
 *
 * Minimal driver for the X-Powers AXP192 power-management IC used by the
 * M5Stack Core2 (and other AXP192-based M5Stack boards). This only exposes
 * the handful of registers verified against M5Stack's own published Core2
 * driver (https://github.com/m5stack/M5Core2, MIT licensed): the two rails
 * and one GPIO needed to power and reset the Core2's LCD. It intentionally
 * does not model charging, VBUS or ADC configuration, and does not assume
 * any particular board's rail/GPIO wiring -- that's the job of a per-board
 * wrapper (e.g. Core2Power) built on top of these primitives.
 */

#pragma once

#include "esp_system.h"

#if CONFIG_DEVICE_TYPE_M5STACK_CORE2

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include <cstdint>

namespace AXP192 {

/**
 * @brief
 *  Install the I2C driver on `port` and probe the AXP192 at its fixed
 *  address (0x34). Must be called once before any other function here.
 */
esp_err_t Init(i2c_port_t port, gpio_num_t sda, gpio_num_t scl);

/// Sets the DCDC3 rail's output voltage (700-3500mV, 25mV steps).
esp_err_t SetDCDC3Voltage(uint16_t millivolts);
/// Enables or disables the DCDC3 rail.
esp_err_t SetDCDC3Enable(bool enable);

/// Sets an LDO's (2 or 3) output voltage (1800-3300mV, 100mV steps).
esp_err_t SetLDOVoltage(uint8_t ldoNumber, uint16_t millivolts);
/// Enables or disables an LDO (2 or 3).
esp_err_t SetLDOEnable(uint8_t ldoNumber, bool enable);

/// Configures GPIO4 as a push-pull output.
esp_err_t SetGPIO4PushPullOutput();
/// Drives GPIO4 high or low.
esp_err_t SetGPIO4Level(bool high);

} // namespace AXP192

#endif // CONFIG_DEVICE_TYPE_M5STACK_CORE2
