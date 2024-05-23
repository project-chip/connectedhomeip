/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
