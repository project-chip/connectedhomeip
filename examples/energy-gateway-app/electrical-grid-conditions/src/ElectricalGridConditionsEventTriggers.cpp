/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "ElectricalGridConditionsMain.h"
#include <app/clusters/electrical-grid-conditions-server/ElectricalGridConditionsTestEventTriggerHandler.h>
#include <app/util/af-types.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

using namespace chip::app::Clusters::ElectricalGridConditions;
using namespace chip::app::Clusters::ElectricalGridConditions::Structs;

void SetTestEventTrigger_CurrentConditionsUpdate()
{
    // Change the value of CurrentConditions
    Structs::ElectricalGridConditionsStruct::Type newConditionsStruct;
    DataModel::Nullable<Structs::ElectricalGridConditionsStruct::Type> newConditions;
    uint32_t matterEpoch = 0;

    if (System::Clock::GetClock_MatterEpochS(matterEpoch) != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "SetTestEventTrigger_CurrentConditionsUpdate() could not get time");
    }

    newConditionsStruct.periodStart = matterEpoch;
    newConditionsStruct.periodEnd.SetNonNull(newConditionsStruct.periodStart + 30 * 60);

    newConditionsStruct.gridCarbonIntensity  = 230; // grammes per kWh
    newConditionsStruct.gridCarbonLevel      = ThreeLevelEnum::kMedium;
    newConditionsStruct.localCarbonIntensity = 0; // grammes per kWh (from Solar)
    newConditionsStruct.localCarbonLevel     = ThreeLevelEnum::kLow;

    newConditions.SetNonNull(newConditionsStruct);

    ElectricalGridConditionsInstance * inst = GetElectricalGridConditionsInstance();
    if (inst == nullptr)
    {
        ChipLogError(Support, "ElectricalGridConditionsInstance not available.");
        return;
    }
    inst->SetCurrentConditions(newConditions);
}

void SetTestEventTrigger_ForecastConditionsUpdate()
{
    ElectricalGridConditionsInstance * inst = GetElectricalGridConditionsInstance();
    if (inst == nullptr)
    {
        ChipLogError(Support, "ElectricalGridConditionsInstance not available.");
        return;
    }

    uint32_t matterEpoch = 0;
    if (System::Clock::GetClock_MatterEpochS(matterEpoch) != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "SetTestEventTrigger_ForecastConditionsUpdate() could not get time");
        return;
    }

    constexpr size_t kForecastSize = 48;

    constexpr int16_t kMinLevel         = 0;   // grammes per kWh
    constexpr int16_t kMaxLevel         = 600; // grammes per kWh
    constexpr uint32_t k30MinsInSeconds = 30 * 60;

    static Structs::ElectricalGridConditionsStruct::Type sForecastEntries[kForecastSize];

    uint32_t currentStart = matterEpoch;

    for (size_t i = 0; i < kForecastSize; ++i)
    {
        Structs::ElectricalGridConditionsStruct::Type & newConditionsStruct = sForecastEntries[i];

        int16_t level = static_cast<int16_t>(kMinLevel + rand() % (kMaxLevel - kMinLevel + 1));

        newConditionsStruct.gridCarbonIntensity = level;
        newConditionsStruct.gridCarbonLevel     = ((level < 100)       ? ThreeLevelEnum::kLow
                                                       : (level < 300) ? ThreeLevelEnum::kMedium
                                                                       : ThreeLevelEnum::kHigh);

        newConditionsStruct.localCarbonIntensity = level;
        newConditionsStruct.localCarbonLevel     = ((level < 100)       ? ThreeLevelEnum::kLow
                                                        : (level < 300) ? ThreeLevelEnum::kMedium
                                                                        : ThreeLevelEnum::kHigh);

        newConditionsStruct.periodStart = currentStart;
        newConditionsStruct.periodEnd.SetNonNull(currentStart + k30MinsInSeconds - 1);
        currentStart += k30MinsInSeconds;
    }

    // Create list from the static array
    DataModel::List<Structs::ElectricalGridConditionsStruct::Type> forecastList(
        Span<Structs::ElectricalGridConditionsStruct::Type>(sForecastEntries, kForecastSize));

    inst->SetForecastConditions(forecastList); // Should now be safe: all memory lives long enough
}

bool HandleElectricalGridConditionsTestEventTrigger(uint64_t eventTrigger)
{
    ElectricalGridConditionsTrigger trigger = static_cast<ElectricalGridConditionsTrigger>(eventTrigger);

    switch (trigger)
    {
    case ElectricalGridConditionsTrigger::kCurrentConditionsUpdate:
        ChipLogProgress(Support, "[ElectricalGridConditions-Test-Event] => Current Conditions Update");
        SetTestEventTrigger_CurrentConditionsUpdate();
        break;
    case ElectricalGridConditionsTrigger::kForecastConditionsUpdate:
        ChipLogProgress(Support, "[ElectricalGridConditions-Test-Event] => Forecast Conditions Update");
        SetTestEventTrigger_ForecastConditionsUpdate();
        break;

    default:
        return false;
    }

    return true;
}
