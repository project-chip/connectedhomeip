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
/*******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// #include "board_features.h"
// #include "hal-config-board.h"
#include <blog.h>

void InitPlatform(void);
void Button_Configure_FactoryResetEventHandler(void (*callback)(void));
void Button_Configure_LightingActionEventHandler(void (*callback)(void));
void BL602_LightState_Update(uint8_t red, uint8_t green, uint8_t blue);

#ifdef __cplusplus
}
#endif
