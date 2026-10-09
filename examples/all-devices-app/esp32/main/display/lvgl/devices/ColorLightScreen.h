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

#pragma once

#include <device/capabilities/color-light/ColorLight.h>
#include <lvgl.h>

namespace chip::app {

/**
 * Renders on/off, brightness and colour temperature for a Color Temperature Light.
 * Must be called while holding the LVGL lock.
 */
void ShowColorTemperatureLightScreen(lv_obj_t * parent, ColorLight & device);

/**
 * Renders on/off, brightness, hue/saturation and colour temperature for an Extended Color Light.
 * Must be called while holding the LVGL lock.
 */
void ShowExtendedColorLightScreen(lv_obj_t * parent, ColorLight & device);

} // namespace chip::app
