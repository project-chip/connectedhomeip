/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "AppConfig.h"
#include "board_features.h"

#define MAX_STR_LEN 48

void initLCD(void);
void * LCDContext();
int LCD_clear(void * pContext);
int LCD_drawPixel(void * pContext, int32_t x, int32_t y);
int LCD_update(void);
void LCDWriteQRCode(uint8_t * text);

#ifdef __cplusplus
}
#endif
