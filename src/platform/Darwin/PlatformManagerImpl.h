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
 *          Provides an implementation of the PlatformManager object.
 */

#pragma once

#include <dispatch/dispatch.h>
#include <platform/GenericPlatformManagerImpl.h>

static constexpr const char * const CHIP_CONTROLLER_QUEUE = "com.zigbee.chip.framework.controller.workqueue";

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for Darwin platforms.
 */
class PlatformManagerImpl final : public Internal::GenericPlatformManagerImpl
{
public:
    // ===== Platform-specific members that may be accessed directly by the application.

    dispatch_queue_t GetWorkQueue()
    {
        if (mWorkQueue == nullptr)
        {
            mWorkQueue = dispatch_queue_create(CHIP_CONTROLLER_QUEUE, DISPATCH_QUEUE_SERIAL);
            dispatch_suspend(mWorkQueue);
        }
        return mWorkQueue;
    }

private:
    // ===== Methods that implement the PlatformManager abstract interface.
    CHIP_ERROR InitChipStackInner() override;

    CHIP_ERROR StartChipTimer(System::Clock::Timeout delay) override { return CHIP_ERROR_NOT_IMPLEMENTED; };
    CHIP_ERROR StartEventLoopTask() override;
    CHIP_ERROR StopEventLoopTask() override;
    void RunEventLoop() override;
    void ProcessDeviceEvents() override;
    void LockChipStack() override{};
    bool TryLockChipStack() override { return false; };
    void UnlockChipStack() override{};
    CHIP_ERROR PostEvent(const ChipDeviceEvent * event) override;

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    bool _IsChipStackLockedByCurrentThread() const { return false; };
#endif

    // ===== Members for internal use by the following friends.

    friend class Internal::BLEManagerImpl;

    dispatch_queue_t mWorkQueue = nullptr;
    // Semaphore used to implement blocking behavior in _RunEventLoop.
    dispatch_semaphore_t mRunLoopSem;

    bool mIsWorkQueueRunning = false;
};

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * chip applications can use this to gain access to features of the PlatformManager
 * that are specific to the ESP32 platform.
 */
PlatformManagerImpl & PlatformMgrImpl();

} // namespace DeviceLayer
} // namespace chip
