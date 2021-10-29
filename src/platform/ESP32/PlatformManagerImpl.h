/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides an implementation of the PlatformManager object
 *          for the ESP32 platform.
 */

#pragma once

#include <platform/FreeRTOS/GenericPlatformManagerImpl_FreeRTOS.h>
#include <system/SystemClock.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for the ESP32 platform.
 */
class PlatformManagerImpl final : public Internal::GenericPlatformManagerImpl_FreeRTOS
{
public:
    // ===== Platform-specific members that may be accessed directly by the application.

    static void HandleESPSystemEvent(void * arg, esp_event_base_t eventBase, int32_t eventId, void * eventData);

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR InitChipStackInner(void) override;
    CHIP_ERROR ShutdownInner() override;
    CHIP_ERROR GetCurrentHeapFree(uint64_t & currentHeapFree) override;
    CHIP_ERROR GetCurrentHeapUsed(uint64_t & currentHeapUsed) override;
    CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark) override;

    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override;
    CHIP_ERROR GetUpTime(uint64_t & upTime) override;
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override;
    CHIP_ERROR GetBootReasons(uint8_t & bootReasons) override;
    // ===== Members for internal use by the following friends.

    chip::System::Clock::Timestamp mStartTime = System::Clock::kZero;
};

} // namespace DeviceLayer
} // namespace chip
