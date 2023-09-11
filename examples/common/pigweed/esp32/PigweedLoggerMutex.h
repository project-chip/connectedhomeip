/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "FreeRTOS.h"
#include "PigweedLogger.h"
#include "RpcService.h"
#include "semphr.h"

namespace chip {
namespace rpc {
class PigweedLoggerMutex : public chip::rpc::Mutex
{

public:
    void Lock() override
    {
        SemaphoreHandle_t * sem = PigweedLogger::getSemaphore();
        if (sem)
        {
            xSemaphoreTake(*sem, portMAX_DELAY);
        }
    }
    void Unlock() override
    {
        SemaphoreHandle_t * sem = PigweedLogger::getSemaphore();
        if (sem)
        {
            xSemaphoreGive(*sem);
        }
    }
};

extern PigweedLoggerMutex logger_mutex;

} // namespace rpc
} // namespace chip
