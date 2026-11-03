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

#include <lvgl.h>

/**
 * Populates @a parent with the device status information:
 * commissioning state, IP address, active device type, and heap statistics.
 *
 * The caller must hold the LVGL lock.
 */
void ShowDeviceInfo(lv_obj_t * parent);

/**
 * Refreshes status values within @a parent.
 *
 * The caller must hold the LVGL lock.
 */
void UpdateDeviceInfo(lv_obj_t * parent);
