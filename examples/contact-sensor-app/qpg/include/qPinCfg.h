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

#ifndef _QPINCFG_H_
#define _QPINCFG_H_

#include "qPinCfg_Common.h"

/*****************************************************************************
 *                    Modules Definitions
 * ***************************************************************************
 * Use QPINCFG_LIST to create the following lists to configure App Modules
 *
 * QPINCFG_UNUSED - list of unused GPIOs to be pulled low
 *
 *****************************************************************************/
// --- Unused pins
#define QPINCFG_UNUSED                                                                                                             \
    QPINCFG_GPIO_LIST(PB1_BUTTON_GPIO_PIN, PB2_BUTTON_GPIO_PIN, PB3_BUTTON_GPIO_PIN, RED_LED_GPIO_PIN, GREEN_LED_GPIO_PIN,         \
                      BLUE_LED_GPIO_PIN, ANIO0_GPIO_PIN, SW_BUTTON_GPIO_PIN, EXT_32KXTAL_P, EXT_32KXTAL_N,                         \
                      DEBUG_SWJDP_SWDIO_TMS_GPIO, DEBUG_SWJDP_SWCLK_TCK_GPIO, BOARD_UNUSED_GPIO_PINS)

#endif // _QPINCFG_H_
