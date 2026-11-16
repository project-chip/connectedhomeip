/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

// POSIX implementation of the JFA response-signal shim (see JFAResponseSignal.h).

#include "JFAResponseSignal.h"

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace joint_fabric_service {

namespace {

std::mutex gResponseMutex;
std::condition_variable gResponseCv;
bool gSignaled = false;

} // namespace

void JFASignalResponse()
{
    {
        std::lock_guard<std::mutex> lock(gResponseMutex);
        gSignaled = true;
    }
    gResponseCv.notify_one();
}

bool JFAWaitResponse(uint32_t timeoutMs)
{
    std::unique_lock<std::mutex> lock(gResponseMutex);
    return gResponseCv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [] { return gSignaled; });
}

void JFAResetResponseSignal()
{
    std::lock_guard<std::mutex> lock(gResponseMutex);
    gSignaled = false;
}

} // namespace joint_fabric_service
