/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

// ---- Light Sample App Config ----
#define SW1_BUTTON 0
#define SW2_BUTTON 1
#define SW3_BUTTON 2
#define SW4_BUTTON 3

#define D1_LED 0
#define D2_LED 1
#define D3_LED 2

#define APP_TASK_NAME "APP"
#define APP_TOGGLE_BUTTON SW1_BUTTON
#define APP_FUNCTION_BUTTON SW2_BUTTON
#define APP_GENERIC_SWITCH_BUTTON SW3_BUTTON

#define APP_LED D1_LED
#define IDENTIFY_STATE_LED D2_LED
#define SYSTEM_STATE_LED D3_LED

#endif // APP_CONFIG_H
