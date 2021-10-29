/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          Provides an generic implementation of PlatformManager features
 *          for use on FreeRTOS platforms.
 */

#pragma once

#include <platform/CHIPDeviceConfig.h>
#include <platform/GenericPlatformManagerImpl.h>

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#else
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#endif

#include <atomic>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of PlatformManager features that works on FreeRTOS platforms.
 *
 * This template contains implementations of select features from the PlatformManager abstract
 * interface that are suitable for use on FreeRTOS-based platforms.  It is intended to be inherited
 * (directly or indirectly) by the PlatformManagerImpl class, which also appears as the template's
 * ImplClass parameter.
 */
class GenericPlatformManagerImpl_FreeRTOS : public GenericPlatformManagerImpl
{

protected:
    TimeOut_t mNextTimerBaseTime;
    TickType_t mNextTimerDurationTicks;
    SemaphoreHandle_t mChipStackLock;
    QueueHandle_t mChipEventQueue;
    TaskHandle_t mEventLoopTask;
    bool mChipTimerActive;

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR InitChipStackInner() override;
    void LockChipStack(void) override;
    bool TryLockChipStack(void) override;
    void UnlockChipStack(void) override;
    CHIP_ERROR PostEvent(const ChipDeviceEvent * event) override;
    void RunEventLoop(void) override;
    CHIP_ERROR StartEventLoopTask(void) override;
    CHIP_ERROR StopEventLoopTask() override;
    CHIP_ERROR StartChipTimer(System::Clock::Timeout duration) override;
    CHIP_ERROR ShutdownInner(void) override;

    // = override=== Methods available to the implementation subclass.

    void PostEventFromISR(const ChipDeviceEvent * event, BaseType_t & yieldRequired);

private:
    // ===== Private members for use by this class only.
    static void EventLoopTaskMain(void * arg);

#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE) && CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE
    uint8_t mEventQueueBuffer[CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE * sizeof(ChipDeviceEvent)];
    StaticQueue_t mEventQueueStruct;
#endif

    std::atomic<bool> mShouldRunEventLoop;
#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_TASK) && CHIP_CONFIG_FREERTOS_USE_STATIC_TASK
    StackType_t mEventLoopStack[CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE / sizeof(StackType_t)];
    StaticTask_t mventLoopTaskStruct;
#endif
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
