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

#include "LoggingIrrigation.h"

#include <algorithm>
#include <iterator>

using namespace chip::app::Clusters::OperationalState;

namespace chip {
namespace app {

DataModel::Nullable<uint32_t> LoggingIrrigation::GetCountdownTime()
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigation::GetCountdownTime()");
    return DataModel::NullNullable;
}

CHIP_ERROR LoggingIrrigation::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    static constexpr OperationalStateEnum kStates[] = { OperationalStateEnum::kStopped, OperationalStateEnum::kRunning,
                                                        OperationalStateEnum::kPaused };
    VerifyOrReturnError(index < std::size(kStates), CHIP_ERROR_NOT_FOUND);
    operationalState = GenericOperationalState(to_underlying(kStates[index]));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingIrrigation::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    // No phases are supported.
    return CHIP_ERROR_NOT_FOUND;
}

void LoggingIrrigation::HandlePauseStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigation::HandlePauseStateCallback()");
    // Set Paused first so the valve close notifications do not move the system to Stopped.
    CHIP_ERROR error = OperationalStateCluster().SetOperationalState(OperationalStateEnum::kPaused);
    if (error != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }

    for (const auto & valve : mWaterValves)
    {
        if (!valve->IsOpen())
        {
            continue;
        }
        if (valve->Pause() != CHIP_NO_ERROR)
        {
            err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }
    }
}

void LoggingIrrigation::HandleResumeStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigation::HandleResumeStateCallback()");
    for (const auto & valve : mWaterValves)
    {
        if (!valve->IsPaused())
        {
            continue;
        }
        if (valve->Resume() != CHIP_NO_ERROR)
        {
            err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }
    }
    CHIP_ERROR error = OperationalStateCluster().SetOperationalState(OperationalStateEnum::kRunning);
    if (error != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void LoggingIrrigation::HandleStartStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigation::HandleStartStateCallback()");
    // Watering is started per zone via the Water Valve endpoints; the system state follows the valves.
    err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
}

void LoggingIrrigation::HandleStopStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigation::HandleStopStateCallback()");

    for (const auto & valve : mWaterValves)
    {
        valve->ClearPause();
        if (valve->IsOpen() && valve->CloseValve() != CHIP_NO_ERROR)
        {
            err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }
    }

    // Needed when stopping from Paused: the valves are already closed, so no close notification sets Stopped.
    if (OperationalStateCluster().SetOperationalState(OperationalStateEnum::kStopped) != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

CHIP_ERROR LoggingIrrigation::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                              EndpointComposition composition)
{
    VerifyOrReturnError(!mValveContext.empty(), CHIP_ERROR_INCORRECT_STATE);

    for (auto & context : mValveContext)
    {
        auto valve = std::make_unique<WaterValve>(mTimerDelegate, context.startupConfiguration, context.valveContext, this);
        EndpointComposition valveEndpointComposition(GetEndpointId(), composition.pattern, context.tags);
        ReturnErrorOnFailure(valve->Register(allocator.Allocate(), provider, valveEndpointComposition));
        mWaterValves.push_back(std::move(valve));
    }

    return CHIP_NO_ERROR;
}
void LoggingIrrigation::UnregisterParts(CodeDrivenDataModelProvider & provider)
{
    for (const auto & valve : mWaterValves)
    {
        if (valve->GetEndpointId() != kInvalidEndpointId)
        {
            valve->Unregister(provider);
        }
    }
    mWaterValves.clear();
}

void LoggingIrrigation::OnValveStateChanged()
{
    bool anyOpen = std::any_of(mWaterValves.begin(), mWaterValves.end(), [](const auto & v) { return v->IsOpen(); });
    if (anyOpen)
    {
        LogErrorOnFailure(OperationalStateCluster().SetOperationalState(OperationalStateEnum::kRunning));
    }
    else if (OperationalStateCluster().GetCurrentOperationalState() != to_underlying(OperationalStateEnum::kPaused))
    {
        LogErrorOnFailure(OperationalStateCluster().SetOperationalState(OperationalStateEnum::kStopped));
    }
}

} // namespace app
} // namespace chip
