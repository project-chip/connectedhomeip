/*
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

#include <lib/support/CodeUtils.h>
#include <system/PlatformEventSupport.h>
#include <system/SystemLayer.h>

namespace chip {
namespace System {

CHIP_ERROR Layer::ScheduleLambdaBridge(LambdaBridge && bridge)
{
    CHIP_ERROR lReturn = PlatformEventing::ScheduleLambdaBridge(*this, std::move(bridge));
    if (lReturn != CHIP_NO_ERROR)
    {
        ChipLogError(chipSystemLayer, "Failed to queue CHIP System Layer lambda event: %s", ErrorStr(lReturn));
    }
    return lReturn;
}

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

CHIP_ERROR LayerFreeRTOS::RegisterLock(TaskLocks_e task, SemaphoreHandle_t * handle)
{
    if (handle == nullptr || task >= TASK_UNDEFINED)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (mRegisteredLocks[task] != nullptr)
    {
        // Already registered ??
        return CHIP_ERROR_INTERNAL;
    }

    mRegisteredLocks[task] = handle;
    return CHIP_NO_ERROR;
}
CHIP_ERROR LayerFreeRTOS::DeregisterLock(TaskLocks_e task)
{
    if (task >= TASK_UNDEFINED)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mRegisteredLocks[task] = nullptr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerFreeRTOS::LockTask(TaskLocks_e task)
{
    if (task < TASK_UNDEFINED && mRegisteredLocks[task] != nullptr)
    {
        xSemaphoreTake(*mRegisteredLocks[task], portMAX_DELAY);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}
CHIP_ERROR LayerFreeRTOS::UnlockTask(TaskLocks_e task)
{
    if (task < TASK_UNDEFINED && mRegisteredLocks[task] != nullptr)
    {
        xSemaphoreGive(*mRegisteredLocks[task]);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}
#endif
} // namespace System
} // namespace chip
