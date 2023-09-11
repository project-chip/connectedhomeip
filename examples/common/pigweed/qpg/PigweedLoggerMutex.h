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
    void Lock() override { PigweedLogger::lock(); }
    void Unlock() override { PigweedLogger::unlock(); }
};

extern PigweedLoggerMutex logger_mutex;

} // namespace rpc
} // namespace chip
