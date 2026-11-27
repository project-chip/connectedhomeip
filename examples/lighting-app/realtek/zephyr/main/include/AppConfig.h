/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

// ---- Lighting Example App Config ----

// Lighting LED config
#define USE_RGB_PWM 0

#define APP_TOGGLE_BUTTON 0   // SW1: toggle the light on/off
#define APP_FUNCTION_BUTTON 1 // SW2: reset -> BLE advertising -> factory reset
#define APP_LEVEL_BUTTON 2    // SW3: dimming, unused while PWM is disabled
#define APP_BLE_ADV_BUTTON 3  // SW4: toggle BLE advertising
