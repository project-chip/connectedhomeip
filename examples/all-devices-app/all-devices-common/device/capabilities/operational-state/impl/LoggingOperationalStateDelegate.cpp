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
    static const GenericOperationalState kSupportedStates[] = {
        GenericOperationalState(to_underlying(OperationalStateEnum::kStopped), MakeOptional("Stopped"_span)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kRunning), MakeOptional("Running"_span)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kPaused), MakeOptional("Paused"_span)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kError), MakeOptional("Error"_span)),
    };

    if (index >= MATTER_ARRAY_SIZE(kSupportedStates))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    operationalState = kSupportedStates[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingOperationalStateDelegate::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    static constexpr CharSpan kSupportedPhases[] = { "Starting"_span, "Operating"_span, "Finishing"_span };

    if (index >= MATTER_ARRAY_SIZE(kSupportedPhases))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    return CopyCharSpanToMutableCharSpan(kSupportedPhases[index], operationalPhase);
}

void LoggingOperationalStateDelegate::HandlePauseStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingOperationalStateDelegate: Pause command received.");
    if (mCluster)
    {
        LogErrorOnFailure(mCluster->SetOperationalState(OperationalStateEnum::kPaused));
    }
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

void LoggingOperationalStateDelegate::HandleResumeStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingOperationalStateDelegate: Resume command received.");
    if (mCluster)
    {
        LogErrorOnFailure(mCluster->SetOperationalState(OperationalStateEnum::kRunning));
    }
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

void LoggingOperationalStateDelegate::HandleStartStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingOperationalStateDelegate: Start command received.");
    if (mCluster)
    {
        LogErrorOnFailure(mCluster->SetOperationalState(OperationalStateEnum::kRunning));
    }
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

void LoggingOperationalStateDelegate::HandleStopStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingOperationalStateDelegate: Stop command received.");
    if (mCluster)
    {
        LogErrorOnFailure(mCluster->SetOperationalState(OperationalStateEnum::kStopped));
    }
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

} // namespace chip::app::Clusters::OperationalState
