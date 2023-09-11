/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "PigweedLogger.h"
#include "pigweed/RpcService.h"
#include "rtos/Mutex.h"

namespace chip {
namespace rpc {
class PigweedLoggerMutex : public chip::rpc::Mutex
{

public:
    void Lock() override
    {
        rtos::Mutex * mutex = PigweedLogger::GetSemaphore();
        if (mutex)
        {
            mutex->lock();
        }
    }
    void Unlock() override
    {
        rtos::Mutex * mutex = PigweedLogger::GetSemaphore();
        if (mutex)
        {
            mutex->unlock();
        }
    }
};

extern PigweedLoggerMutex logger_mutex;

} // namespace rpc
} // namespace chip
