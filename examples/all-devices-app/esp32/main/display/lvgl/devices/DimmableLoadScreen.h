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

#include <device/capabilities/dimmable-load/DimmableLoad.h>
#include <lvgl.h>

namespace chip::app {

/**
 * Renders the device control screen for any DimmableLoad-based device (light, plug-in unit, mounted control).
 * Must be called while holding the LVGL lock.
 */
void ShowDimmableLoadScreen(lv_obj_t * parent, const char * title, DimmableLoad & device);

} // namespace chip::app
