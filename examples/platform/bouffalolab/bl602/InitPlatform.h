/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
