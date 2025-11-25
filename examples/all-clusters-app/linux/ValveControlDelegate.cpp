/**
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

#include "ValveControlDelegate.h"
#include <app/clusters/valve-configuration-and-control-server/CodegenIntegration.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-cluster.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ValveConfigurationAndControl;
using namespace chip::app::Clusters::TimeSynchronization;

static chip::Percent sLevel                = 1;
static uint32_t sLastOpenDuration          = 0;
static constexpr EndpointId kValveEndpoint = 1;

DataModel::Nullable<chip::Percent> ValveControlDelegate::HandleOpenValve(DataModel::Nullable<chip::Percent> level)
{
    chip::Percent currentLevel = sLevel;
    sLevel                     = level.IsNull() ? 100 : level.Value();
    sLastOpenDuration          = 0;
    ChipLogProgress(NotSpecified, "Valve opening from level: %d to %d", currentLevel, sLevel);

    // In this demo application, the transition is considered instant,
    // so current level is set to the requested level and current state is set to kOpen.
    currentLevel = sLevel;

    return DataModel::Nullable<chip::Percent>(currentLevel);
}

CHIP_ERROR ValveControlDelegate::HandleCloseValve()
{
    sLastOpenDuration = 0;
    sLevel            = 0;
    auto valveCluster = ValveConfigurationAndControl::FindClusterOnEndpoint(kValveEndpoint);
    VerifyOrReturnError(valveCluster != nullptr, CHIP_ERROR_UNINITIALIZED);
    ReturnErrorOnFailure(valveCluster->UpdateCurrentLevel(sLevel));
    ChipLogProgress(NotSpecified, "Valve closed");
    return CHIP_NO_ERROR;
}

void ValveControlDelegate::HandleRemainingDurationTick(uint32_t duration)
{
    ChipLogProgress(NotSpecified, "Valve remaining duration ticking: %dsec level: %d duration %d", duration, sLevel,
                    sLastOpenDuration);
    auto valveCluster = ValveConfigurationAndControl::FindClusterOnEndpoint(kValveEndpoint);
    VerifyOrReturn(valveCluster != nullptr);
    if (sLastOpenDuration == 0)
    {
        VerifyOrReturn(CHIP_NO_ERROR == valveCluster->UpdateCurrentLevel(sLevel),
                       ChipLogError(NotSpecified, "Updating current level failed"));
        VerifyOrReturn(CHIP_NO_ERROR == valveCluster->UpdateCurrentState(ValveConfigurationAndControl::ValveStateEnum::kOpen),
                       ChipLogError(NotSpecified, "Updating current state failed"));
    }
    sLastOpenDuration = duration;
}

void ExtendedTimeSyncDelegate::UTCTimeAvailabilityChanged(uint64_t time)
{
    auto valveCluster = ValveConfigurationAndControl::FindClusterOnEndpoint(kValveEndpoint);
    VerifyOrReturn(valveCluster != nullptr);
    valveCluster->UpdateAutoCloseTime(time);
}
