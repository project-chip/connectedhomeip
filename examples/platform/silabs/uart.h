/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void uartConsoleInit(void);
int16_t uartConsoleWrite(const char * Buf, uint16_t BufLength);
int16_t uartLogWrite(const char * log, uint16_t length);
int16_t uartConsoleRead(char * Buf, uint16_t NbBytesToRead);

void uartMainLoop(void * args);

// Implemented by in openthread code
#ifndef PW_RPC_ENABLED
extern void otPlatUartReceived(const uint8_t * aBuf, uint16_t aBufLength);
extern void otPlatUartSendDone(void);
extern void otSysEventSignalPending(void);
#endif

#ifdef __cplusplus
} // extern "C"
#endif
