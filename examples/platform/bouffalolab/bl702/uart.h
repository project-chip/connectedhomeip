/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void uartInit(void);
int16_t uartWrite(const char * Buf, uint16_t BufLength);
int16_t uartRead(char * Buf, uint16_t NbBytesToRead);

#ifdef __cplusplus
} // extern "C"
#endif
