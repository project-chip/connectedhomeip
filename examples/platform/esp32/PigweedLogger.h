/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

namespace PigweedLogger {

/* Initialise a mutex variable to avoid the conflict for ESP_LOG* on UART0 and pw_sys_io */
void init();

/*Wraps ESP_LOG* into HDLC frame format*/
int putString(const char * buffer, size_t size);

SemaphoreHandle_t * getSemaphore();

} // namespace PigweedLogger
