/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "PigweedLogger.h"
#include "pigweed/RpcService.h"
#include "semphr.h"
#include <FreeRTOS.h>

namespace chip {
namespace rpc {
class PigweedLoggerMutex : public ::chip::rpc::Mutex
{

public:
    PigweedLoggerMutex() {}
    void Lock() override
    {
        SemaphoreHandle_t * sem = PigweedLogger::GetSemaphore();
        if (sem)
        {
            xSemaphoreTake(*sem, portMAX_DELAY);
        }
    }
    void Unlock() override
    {
        SemaphoreHandle_t * sem = PigweedLogger::GetSemaphore();
        if (sem)
        {
            xSemaphoreGive(*sem);
        }
    }
};

extern PigweedLoggerMutex logger_mutex;

} // namespace rpc
} // namespace chip
