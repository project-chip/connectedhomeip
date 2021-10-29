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

/**
 *    @file
 *          Stub platform manager for fake platform.
 */

#pragma once

#include <platform/PlatformManager.h>

#include <queue>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for Linux platforms.
 */
class PlatformManagerImpl final : public PlatformManager
{
public:
    // ===== Platform-specific members that may be accessed directly by the application.

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR InitChipStackInner() override { return CHIP_NO_ERROR; }
    CHIP_ERROR ShutdownInner() override { return CHIP_NO_ERROR; }

    CHIP_ERROR AddEventHandler(EventHandlerFunct handler, intptr_t arg = 0) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    void RemoveEventHandler(EventHandlerFunct handler, intptr_t arg = 0) override {}
    void ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg = 0) override {}

    void RunEventLoop() override
    {
        do
        {
            ProcessDeviceEvents();
        } while (mShouldRunEventLoop);
    }

    void ProcessDeviceEvents()
    {
        while (!mQueue.empty())
        {
            const ChipDeviceEvent & event = mQueue.front();
            DispatchEvent(&event);
            mQueue.pop();
        }
    }

    CHIP_ERROR StartEventLoopTask() override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    CHIP_ERROR StopEventLoopTask() override
    {
        mShouldRunEventLoop = false;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR PostEvent(const ChipDeviceEvent * event) override
    {
        mQueue.emplace(*event);
        return CHIP_NO_ERROR;
    }

    void DispatchEvent(const ChipDeviceEvent * event) override
    {
        switch (event->Type)
        {
        case DeviceEventType::kChipLambdaEvent:
            event->LambdaEvent();
            break;

        default:
            break;
        }
    }

    CHIP_ERROR StartChipTimer(System::Clock::Timeout duration) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    void LockChipStack() override {}
    bool TryLockChipStack() override { return true; }
    void UnlockChipStack() override {}

    CHIP_ERROR GetCurrentHeapFree(uint64_t & currentHeapFree) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetCurrentHeapUsed(uint64_t & currentHeapUsed) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark) override { return CHIP_NO_ERROR; }

    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetUpTime(uint64_t & upTime) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetBootReasons(uint8_t & bootReasons) override { return CHIP_NO_ERROR; }

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    bool IsChipStackLockedByCurrentThread() const override { return true; };
#endif
    // ===== Members for internal use by the following friends.

    bool mShouldRunEventLoop = true;
    std::queue<ChipDeviceEvent> mQueue;
};

} // namespace DeviceLayer
} // namespace chip
