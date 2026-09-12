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
 * @file CapacitiveTouchButtons.h
 *
 * Boards like the M5Stack Core2 (and CoreS3, which uses the same touch
 * silicon family) have no physical navigation buttons: touch input comes
 * from an FT6206/FT6236/FT6336/FT5x06-family capacitive touch controller on
 * I2C instead. This emulates the M5Stack Basic/Gray's 3-button bar
 * (Button.h) by watching for touches in three zones along the bottom of the
 * screen and forwarding them to ScreenManager::ButtonPressed() with the same
 * button IDs (1, 2, 3) that Button.cpp uses, so screens don't need to know
 * which input scheme is in use.
 *
 * This driver only depends on the touch controller's I2C bus/address, not on
 * any particular board's power sequencing, so the same code should work
 * as-is for any board in this touch controller family, given the I2C port
 * its controller sits on.
 */

#pragma once

#include "driver/i2c.h"
#include "esp_err.h"

namespace CapacitiveTouchButtons {

/**
 * @brief
 *  Start the periodic timer that polls the touch controller over I2C and
 *  turns touches in the bottom button bar into ScreenManager::ButtonPressed()
 *  calls.
 *
 * @param i2cPort   The already-installed I2C port the touch controller is
 *                  on (e.g. Core2Power::kInternalI2CPort on the Core2).
 * @return ESP_OK on success
 */
esp_err_t Init(i2c_port_t i2cPort);

} // namespace CapacitiveTouchButtons
