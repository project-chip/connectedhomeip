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

#include "LoggingRvcOperationalStateDelegate.h"
#include <clusters/RvcOperationalState/Enums.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::Clusters::OperationalState {

CHIP_ERROR LoggingRvcOperationalStateDelegate::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    static const GenericOperationalState kSupportedStates[] = {
        GenericOperationalState(to_underlying(OperationalStateEnum::kStopped), MakeOptional("Stopped"_span)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kRunning), MakeOptional("Running"_span)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kPaused), MakeOptional("Paused"_span)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kError), MakeOptional("Error"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger),
                                MakeOptional("SeekingCharger"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging),
                                MakeOptional("Charging"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked), MakeOptional("Docked"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kEmptyingDustBin),
                                MakeOptional("EmptyingDustBin"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCleaningMop),
                                MakeOptional("CleaningMop"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kFillingWaterTank),
                                MakeOptional("FillingWaterTank"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kUpdatingMaps),
                                MakeOptional("UpdatingMaps"_span)),
    };

    if (index >= MATTER_ARRAY_SIZE(kSupportedStates))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    operationalState = kSupportedStates[index];
    return CHIP_NO_ERROR;
}

void LoggingRvcOperationalStateDelegate::HandleGoHomeCommandCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingRvcOperationalStateDelegate: Go Home command received.");
    if (mCluster)
    {
        LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kStopped)));
    }
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

} // namespace chip::app::Clusters::OperationalState
