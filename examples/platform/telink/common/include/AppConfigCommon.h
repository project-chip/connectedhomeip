/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

// Buttons config
#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
#define BUTTON_FACTORY_RESET GPIO_DT_SPEC_GET(DT_NODELABEL(key_1), gpios)
#define BUTTON_BLE_START GPIO_DT_SPEC_GET(DT_NODELABEL(key_2), gpios)
#define BUTTON_THREAD_START GPIO_DT_SPEC_GET(DT_NODELABEL(key_3), gpios)
#define BUTTON_EXAMPLE_ACTION GPIO_DT_SPEC_GET(DT_NODELABEL(key_4), gpios)
#else
#define BUTTON_COL_1 GPIO_DT_SPEC_GET(DT_NODELABEL(key_matrix_col1), gpios)
#define BUTTON_COL_2 GPIO_DT_SPEC_GET(DT_NODELABEL(key_matrix_col2), gpios)
#define BUTTON_ROW_1 GPIO_DT_SPEC_GET(DT_NODELABEL(key_matrix_row1), gpios)
#define BUTTON_ROW_2 GPIO_DT_SPEC_GET(DT_NODELABEL(key_matrix_row2), gpios)
#endif

// LEDs config
#define LEDS_PORT DEVICE_DT_GET(DT_NODELABEL(gpiob))
#define SYSTEM_STATE_LED 7
#if APP_USE_IDENTIFY_PWM
#define LIGHTING_PWM_SPEC_IDENTIFY_GREEN PWM_DT_SPEC_GET(DT_ALIAS(pwm_led3))
#endif
