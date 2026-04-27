/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#pragma once

#include <app/TestEventTriggerDelegate.h>

namespace chip {
namespace DeviceLayer {

/**
 * @brief nRF Connect platform-specific test event trigger handler for factory reset operations
 *
 * This handler provides a test event trigger that can be used to initiate
 * a factory reset operation via the TestEventTrigger command in the
 * General Diagnostics cluster on nRF Connect SDK platforms.
 */
class FactoryResetTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    /**
     * @brief Test event trigger value for factory reset
     *
     * This trigger value follows the standard pattern for test event triggers.
     * The format is 0x[UniqueID]0000[SubTrigger] where bits 32-47 are always 0x0000
     * to ensure clearEndpointInEventTrigger() doesn't modify the value.
     * Format: 0xFFFF000000000001
     */
    static constexpr uint64_t kFactoryResetTrigger = 0xFFFF'0000'0000'0001;

    /**
     * @brief Handle the test event trigger
     *
     * @param eventTrigger The event trigger value to handle
     * @return CHIP_NO_ERROR if the trigger was handled successfully
     * @return CHIP_ERROR_INVALID_ARGUMENT if the trigger is not recognized
     */
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;
};

} // namespace DeviceLayer
} // namespace chip
