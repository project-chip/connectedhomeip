/*
 *    Copyright 2024-2025 Project CHIP Authors
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

/* ---- App Config ---- */
#define APP_DEVICE_TYPE_ENDPOINT CONFIG_CHIP_APP_DEVICE_TYPE_ENDPOINT

/* ---- LED Manager Config ---- */
#define LED_MANAGER_STATUS_LED_INDEX CONFIG_CHIP_APP_LED_MANAGER_STATUS_LED_INDEX
#define LED_MANAGER_LIGHT_LED_INDEX CONFIG_CHIP_APP_LED_MANAGER_LIGHT_LED_INDEX

#ifdef CONFIG_CHIP_APP_LED_MANAGER_STATUS_LED
#define LED_MANAGER_ENABLE_STATUS_LED CONFIG_CHIP_APP_LED_MANAGER_STATUS_LED
#endif // CONFIG_CHIP_APP_LED_MANAGER_STATUS_LED

#ifdef CONFIG_CHIP_APP_QUEUE_TICKS_TO_WAIT
#define APP_QUEUE_TICKS_TO_WAIT CONFIG_CHIP_APP_QUEUE_TICKS_TO_WAIT
#endif // CONFIG_CHIP_APP_QUEUE_TICKS_TO_WAIT
