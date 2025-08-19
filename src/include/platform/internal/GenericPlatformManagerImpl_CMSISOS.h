/*
 *
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

/**
 *    @file
 *          Provides an generic implementation of PlatformManager features
 *          for platform using CMSISOS 2 OS Abstraction APIs.
 */

#pragma once

#include <platform/CHIPDeviceConfig.h>
#include <platform/internal/GenericPlatformManagerImpl.h>

#include <atomic>
#include <cmsis_os2.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of PlatformManager features for platforms using CMSISOS 2 OS Abstraction APIs
 *
 * This template contains implementations of select features from the PlatformManager abstract
 * interface that are suitable for use on CMSISOS-based platforms.  It is intended to be inherited
 * (directly or indirectly) by the PlatformManagerImpl class, which also appears as the template's
 * ImplClass parameter.
 */
template <class ImplClass>
class GenericPlatformManagerImpl_CMSISOS : public GenericPlatformManagerImpl<ImplClass>
{

protected:
    osMutexId_t mChipStackLock         = nullptr;
    osMessageQueueId_t mChipEventQueue = nullptr;
    osThreadId_t mEventLoopTask        = nullptr;
    bool mChipTimerActive;

#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    osMessageQueueId_t mBackgroundEventQueue = nullptr;
    osThreadId_t mBackgroundEventLoopTask    = nullptr;
#endif

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack();

    void _LockChipStack(void);
    bool _TryLockChipStack(void);
    void _UnlockChipStack(void);

    CHIP_ERROR _PostEvent(const ChipDeviceEvent * event);
    void _RunEventLoop(void);
    CHIP_ERROR _StartEventLoopTask(void);
    CHIP_ERROR _StopEventLoopTask();
    CHIP_ERROR _StartChipTimer(System::Clock::Timeout duration);
    void _Shutdown(void);

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    bool _IsChipStackLockedByCurrentThread() const;
#endif

    CHIP_ERROR _PostBackgroundEvent(const ChipDeviceEvent * event);
    void _RunBackgroundEventLoop(void);
    CHIP_ERROR _StartBackgroundEventLoopTask(void);
    CHIP_ERROR _StopBackgroundEventLoopTask();

    // ===== Methods available to the implementation subclass.

private:
    // ===== Private members for use by this class only.

    inline ImplClass * Impl() { return static_cast<ImplClass *>(this); }

    static void EventLoopTaskMain(void * pvParameter);
    uint32_t SyncNextChipTimerHandling();
    uint32_t mNextTimerBaseTime      = 0;
    uint32_t mNextTimerDurationTicks = 0;
    std::atomic<bool> mShouldRunEventLoop;

#if defined(CHIP_CONFIG_CMSISOS_USE_STATIC_QUEUE) && CHIP_CONFIG_CMSISOS_USE_STATIC_QUEUE
    uint8_t mEventQueueBuffer[CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE * sizeof(ChipDeviceEvent)];
    osMessageQueue_t mEventQueueStruct;
    const osMessageQueueAttr_t mEventQueueAttr = { .cb_mem  = &mEventQueueStruct,
                                                   .cb_size = osMessageQueueCbSize,
                                                   .mq_mem  = mEventQueueBuffer,
                                                   .mq_size = sizeof(mEventQueueBuffer) };

    const osMessageQueueAttr_t * mEventQueueAttrPtr = &mEventQueueAttr;
#else
    // Nothing to configure for queues, Just use this to avoid #ifdef in the class implementation
    const osMessageQueueAttr_t * mEventQueueAttrPtr = nullptr;
#endif // CHIP_CONFIG_CMSISOS_USE_STATIC_QUEUE

#if defined(CHIP_CONFIG_CMSISOS_USE_STATIC_TASK) && CHIP_CONFIG_CMSISOS_USE_STATIC_TASK
    uint8_t mEventLoopStack[CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE];
    osThread_t mEventLoopTaskControlBlock;
#endif // CHIP_CONFIG_CMSISOS_USE_STATIC_TASK

    const osThreadAttr_t mEventLoopTaskAttr = {
        .name      = CHIP_DEVICE_CONFIG_CHIP_TASK_NAME,
        .attr_bits = osThreadDetached,
#if defined(CHIP_CONFIG_CMSISOS_USE_STATIC_TASK) && CHIP_CONFIG_CMSISOS_USE_STATIC_TASK
        .cb_mem    = &mEventLoopTaskControlBlock,
        .cb_size   = osThreadCbSize,
        .stack_mem = mEventLoopStack,
#endif // CHIP_CONFIG_CMSISOS_USE_STATIC_TASK
        .stack_size = CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE,
        .priority   = CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY
    };

#if defined(CHIP_CONFIG_CMSISOS_USE_STATIC_MUTEX) && CHIP_CONFIG_CMSISOS_USE_STATIC_MUTEX
    osMutexCbSize uint8_t mMutexControlBlock[osMutexCbSize];
#endif // CHIP_CONFIG_CMSISOS_USE_STATIC_MUTEX
    const osMutexAttr_t mChipStackMutexAttr = {
        .name = "",
#if defined(CHIP_CONFIG_CMSISOS_USE_STATIC_MUTEX) && CHIP_CONFIG_CMSISOS_USE_STATIC_MUTEX
        .cb_mem  = &mMutexControlBlock,
        .cb_size = osMutexCbSize,
#endif // CHIP_CONFIG_CMSISOS_USE_STATIC_MUTEX
        .attr_bits = osMutexRecursive | osMutexPrioInherit,
    };

#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    static void BackgroundEventLoopTaskMain(void * pvParameter);
    std::atomic<bool> mShouldRunBackgroundEventLoop;

#if defined(CHIP_CONFIG_CMSISOS_USE_STATIC_QUEUE) && CHIP_CONFIG_CMSISOS_USE_STATIC_QUEUE
    uint8_t mBackgroundQueueBuffer[CHIP_DEVICE_CONFIG_BG_MAX_EVENT_QUEUE_SIZE * sizeof(ChipDeviceEvent)];
    osMessageQueue_t mBackgroundQueueStruct;
    const osMessageQueueAttr_t mBgQueueAttr = { .cb_mem  = &mBackgroundQueueStruct,
                                                .cb_size = osMessageQueueCbSize,
                                                .mq_mem  = mBackgroundQueueBuffer,
                                                .mq_size = sizeof(mBackgroundQueueBuffer) };

    const osMessageQueueAttr_t * mBgQueueAttrPtr = &mBgQueueAttr;
#else
    // Nothing to configure for queues, Just use this to avoid #ifdef in the class implementation
    const osMessageQueueAttr_t * mBgQueueAttrPtr = nullptr;
#endif // CHIP_CONFIG_CMSISOS_USE_STATIC_QUEUE

#if defined(CHIP_CONFIG_CMSISOS_USE_STATIC_TASK) && CHIP_CONFIG_CMSISOS_USE_STATIC_TASK
    uint8_t mBackgroundEventLoopStack[CHIP_DEVICE_CONFIG_BG_TASK_STACK_SIZE];
    osThread_t mBackgroundEventLoopTaskControlBlock;
#endif // CHIP_CONFIG_CMSISOS_USE_STATIC_TASK

    const osThreadAttr_t mBgEventLoopTaskAttr = {
        .name      = CHIP_DEVICE_CONFIG_BG_TASK_NAME,
        .attr_bits = osThreadDetached,
#if defined(CHIP_CONFIG_CMSISOS_USE_STATIC_TASK) && CHIP_CONFIG_CMSISOS_USE_STATIC_TASK
        .cb_mem    = &mBackgroundEventLoopTaskControlBlock,
        .cb_size   = osThreadCbSize,
        .stack_mem = mBackgroundEventLoopStack,
#endif // CHIP_CONFIG_CMSISOS_USE_STATIC_TASK
        .stack_size = CHIP_DEVICE_CONFIG_BG_TASK_STACK_SIZE,
        .priority   = CHIP_DEVICE_CONFIG_BG_TASK_PRIORITY
    };
#endif // CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericPlatformManagerImpl_CMSISOS<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
