/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef HEAP_MONITORING
#include "FreeRTOS.h"

#define MONITORING_STACK_SIZE_byte 1280

class MemMonitoring
{
public:
    static void startHeapMonitoring();

private:
    static void HeapMonitoring(void * pvParameter);
};

#endif
