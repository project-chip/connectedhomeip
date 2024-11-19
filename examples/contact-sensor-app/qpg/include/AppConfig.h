/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define APP_TASK_NAME "APP"

// Mapping of Functional buttons
#define DOORWINDOW_SENSOR_BUTTON BTN_SW2 // GP_PB_4
#define APP_FUNCTION_BUTTON BTN_SW5      // GP_PB_5

// Mapping of LEDs
#define SYSTEM_STATE_LED LED_GREEN
#define DOORWINDOW_SENSOR_LED LED_WHITE

// Enable/Disable DoorWindow Status LED
#define CONFIG_ENABLE_DOORWINDOW_SENSOR_STATUS_LED 1

#endif // APP_CONFIG_H
