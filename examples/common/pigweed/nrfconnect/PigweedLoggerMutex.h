/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "PigweedLogger.h"
#include "pigweed/RpcService.h"
#include <zephyr/kernel.h>

namespace chip {
namespace rpc {
class PigweedLoggerMutex : public ::chip::rpc::Mutex
{
public:
    PigweedLoggerMutex() {}
    void Lock() override
    {
        k_sem * sem = PigweedLogger::GetSemaphore();
        if (sem)
        {
            k_sem_take(sem, K_FOREVER);
        }
    }
    void Unlock() override
    {
        k_sem * sem = PigweedLogger::GetSemaphore();
        if (sem)
        {
            k_sem_give(sem);
        }
    }
};

extern PigweedLoggerMutex logger_mutex;

} // namespace rpc
} // namespace chip
