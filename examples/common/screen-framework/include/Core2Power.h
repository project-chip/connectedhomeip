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
 * @file Core2Power.h
 *
 * M5Stack Core2 board wiring on top of the generic AXP192.h driver: which
 * rail powers the LCD backlight and logic, and which GPIO drives the LCD's
 * RESET line. Unlike the original M5Stack Basic/Gray, none of these are
 * plain ESP32 GPIOs -- they're all switched by the AXP192 PMIC over I2C.
 */

#pragma once

#include "esp_system.h"

#if CONFIG_DEVICE_TYPE_M5STACK_CORE2

#include "driver/i2c.h"
#include "esp_err.h"

namespace Core2Power {

// The AXP192, FT6336U touch controller and BM8563 RTC all share the Core2's
// internal I2C bus (SDA=GPIO21, SCL=GPIO22). Init() installs the I2C driver
// on this port; other drivers on the same bus (e.g. CapacitiveTouchButtons)
// reuse it rather than calling i2c_driver_install() a second time.
constexpr i2c_port_t kInternalI2CPort = I2C_NUM_1;

/**
 * @brief
 *  Initialize the internal I2C bus shared with the AXP192 (and the FT6336U
 *  touch controller), then power on and reset the LCD via the AXP192.
 *
 *  Must be called before InitDisplay()/TFT_PinsInit() so the panel is
 *  powered by the time the SPI init sequence talks to it, and before
 *  CapacitiveTouchButtons::Init() since that reuses the same I2C bus.
 *
 * @return ESP_OK on success
 */
esp_err_t Init();

/**
 * @brief
 *  Set the LCD backlight on/off by enabling or disabling the AXP192 rail
 *  that powers it (DCDC3). There is no direct GPIO to drive here, so this
 *  replaces the LEDC PWM based brightness control used on M5Stack Basic/Gray.
 *
 * @param on    true to enable the backlight rail, false to disable it
 */
void SetBacklight(bool on);

} // namespace Core2Power

#endif // CONFIG_DEVICE_TYPE_M5STACK_CORE2
