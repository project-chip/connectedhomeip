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

#include "LoggingOperationalStateDelegate.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::Clusters::OperationalState {

CHIP_ERROR LoggingOperationalStateDelegate::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    static constexpr uint8_t kSupportedStates[] = {
        to_underlying(OperationalStateEnum::kStopped),
        to_underlying(OperationalStateEnum::kRunning),
        to_underlying(OperationalStateEnum::kPaused),
        to_underlying(OperationalStateEnum::kError),
    };

    if (index >= MATTER_ARRAY_SIZE(kSupportedStates))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    operationalState.operationalStateID = kSupportedStates[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingOperationalStateDelegate::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    // Provide a basic set of phases to satisfy the reviewer's comment about phase lists.
    static const char * kSupportedPhases[] = { "Starting", "Operating", "Finishing" };

    if (index >= MATTER_ARRAY_SIZE(kSupportedPhases))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    return CopyCharSpanToMutableCharSpan(CharSpan::fromCharString(kSupportedPhases[index]), operationalPhase);
}

void LoggingOperationalStateDelegate::HandlePauseStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingOperationalStateDelegate: Pause command received.");
    if (mCluster)
    {
        LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kPaused)));
    }
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

void LoggingOperationalStateDelegate::HandleResumeStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingOperationalStateDelegate: Resume command received.");
    if (mCluster)
    {
        LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kRunning)));
    }
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

void LoggingOperationalStateDelegate::HandleStartStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingOperationalStateDelegate: Start command received.");
    if (mCluster)
    {
        LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kRunning)));
    }
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

void LoggingOperationalStateDelegate::HandleStopStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingOperationalStateDelegate: Stop command received.");
    if (mCluster)
    {
        LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kStopped)));
    }
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

} // namespace chip::app::Clusters::OperationalState
