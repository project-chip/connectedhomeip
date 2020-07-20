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

// ---- Lock Example App Config ----

#define LOCK_BUTTON DK_BTN2
#define LOCK_BUTTON_MASK DK_BTN2_MSK
#define FUNCTION_BUTTON DK_BTN1
#define FUNCTION_BUTTON_MASK DK_BTN1_MSK

#define SYSTEM_STATE_LED DK_LED1
#define LOCK_STATE_LED DK_LED2

// Time it takes in ms for the simulated actuator to move from one state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 2000

#endif // APP_CONFIG_H
