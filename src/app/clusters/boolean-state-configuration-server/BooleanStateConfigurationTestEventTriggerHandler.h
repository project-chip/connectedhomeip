/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/TestEventTriggerDelegate.h>

namespace chip {

enum class BooleanStateConfigurationTrigger : uint64_t
{
    kSensorTrigger   = 0x0080000000000000,
    kSensorUntrigger = 0x0080000000000001,
};

class BooleanStateConfigurationTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;
};

} // namespace chip

/**
 * @brief User handler for handling the test event trigger
 *
 * @note If TestEventTrigger is enabled, it needs to be implemented in the app
 *
 * @param eventTrigger Event trigger to handle
 *
 * @retval true on success
 * @retval false if error happened
 */
bool HandleBooleanStateConfigurationTestEventTrigger(uint64_t eventTrigger);
