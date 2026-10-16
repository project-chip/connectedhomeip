/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/data-model/Nullable.h>
#include <device/types/water-valve/impl/LoggingWaterValve.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/logging/CHIPLogging.h>
namespace chip {
namespace app {

DataModel::Nullable<Percent> LoggingWaterValve::HandleOpenValve(DataModel::Nullable<Percent> level)
{
    if (mValveCluster.IsConstructed() &&
        !mValveCluster.Cluster().GetFeatureMap().Has(Clusters::ValveConfigurationAndControl::Feature::kLevel))
    {
        ChipLogProgress(AppServer, "WaterValve: Opening valve");
        mValveCluster.Cluster().UpdateCurrentState(Clusters::ValveConfigurationAndControl::ValveStateEnum::kOpen);
        return DataModel::NullNullable;
    }

    Percent targetLevel = level.ValueOr(100);
    ChipLogProgress(AppServer, "WaterValve: Opening valve to level %u", targetLevel);

    return DataModel::MakeNullable(targetLevel);
}

CHIP_ERROR LoggingWaterValve::HandleCloseValve()
{
    ChipLogProgress(AppServer, "WaterValve: Closing valve");
    if (mValveCluster.IsConstructed())
    {
        mValveCluster.Cluster().UpdateCurrentLevel(0);
        mValveCluster.Cluster().UpdateCurrentState(Clusters::ValveConfigurationAndControl::ValveStateEnum::kClosed);
    }

    return CHIP_NO_ERROR;
}

void LoggingWaterValve::HandleRemainingDurationTick(uint32_t duration)
{
    ChipLogProgress(AppServer, "WaterValve: Remaining duration tick: %lu", static_cast<unsigned long>(duration));
}
} // namespace app
} // namespace chip
