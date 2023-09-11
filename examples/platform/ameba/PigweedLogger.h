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

void init();
int putString(const char * buffer, size_t size);
SemaphoreHandle_t * getSemaphore();

} // namespace PigweedLogger
