/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
bool HandleCommodityTariffTestEventTrigger(uint64_t eventTrigger);

namespace chip {

/*
 * These Test EventTrigger values can be used to produce artificial DEM forecasts
 *
 * They are sent along with the enableKey (manufacturer defined secret)
 * in the General Diagnostic cluster TestEventTrigger command
 */
enum class CommodityTariffTrigger : uint64_t
{
    // Simulate a tariff data update to trigger loading and validation of new data followed by the cluster attributes changes.
    kTariffDataUpdated = 0x0700'0000'0000'0000,
    // Force jump to next day to check of updating for time of day context-dependent attribute values.
    kForcedOneDayForward = 0x0700'0000'0000'0001
};

class CommodityTariffTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    CommodityTariffTestEventTriggerHandler() {}

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        if (HandleCommodityTariffTestEventTrigger(eventTrigger))
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
