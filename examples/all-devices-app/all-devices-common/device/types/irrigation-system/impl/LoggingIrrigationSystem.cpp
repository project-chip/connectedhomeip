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

#include "LoggingIrrigationSystem.h"

#include <device/types/irrigation-system/IrrigationSystem.h>
#include <lib/support/TypeTraits.h>

#include <algorithm>
#include <cstddef>
#include <iterator>

using namespace chip::app::Clusters::OperationalState;

namespace chip {
namespace app {

DataModel::Nullable<uint32_t> LoggingIrrigationSystem::GetCountdownTime()
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigationSystem::GetCountdownTime()");
    return DataModel::NullNullable;
}

CHIP_ERROR LoggingIrrigationSystem::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    static constexpr OperationalStateEnum kStates[] = { OperationalStateEnum::kStopped, OperationalStateEnum::kRunning,
                                                        OperationalStateEnum::kPaused };
    VerifyOrReturnError(index < std::size(kStates), CHIP_ERROR_NOT_FOUND);
    operationalState = GenericOperationalState(to_underlying(kStates[index]));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingIrrigationSystem::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    // No phases are supported.
    return CHIP_ERROR_NOT_FOUND;
}

// OperationalState delegate callbacks: only invoked when the OperationalState cluster is registered.
void LoggingIrrigationSystem::HandlePauseStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigationSystem::HandlePauseStateCallback()");
    // Set Paused first so the valve close notifications do not move the system to Stopped.
    CHIP_ERROR error = OperationalStateCluster().SetOperationalState(OperationalStateEnum::kPaused);
    if (error != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }

    for (size_t i = 0; i < mWaterValves.size(); ++i)
    {
        // Save before closing: closing clears the valve's open level and remaining duration.
        auto openLevel = mWaterValves[i]->OpenLevel();
        if (!openLevel.has_value())
        {
            continue;
        }
        // A valve opened without an OpenDuration has no remaining duration; keep it null rather than calling value().
        auto remaining = mWaterValves[i]->RemainingDuration();
        mPausedZones[i] =
            PausedZone{ *openLevel, remaining.has_value() ? DataModel::MakeNullable(*remaining) : DataModel::NullNullable };
        if (mWaterValves[i]->CloseValve() != CHIP_NO_ERROR)
        {
            mPausedZones[i].reset();
            err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }
    }
}

void LoggingIrrigationSystem::HandleResumeStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigationSystem::HandleResumeStateCallback()");
    for (size_t i = 0; i < mWaterValves.size(); ++i)
    {
        auto & zone = mPausedZones[i];
        if (!zone.has_value())
        {
            continue;
        }
        CHIP_ERROR error = mWaterValves[i]->OpenValve(DataModel::MakeNullable(zone->level), zone->remainingDuration);
        if (error != CHIP_NO_ERROR)
        {
            err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }
        zone.reset();
    }
    CHIP_ERROR error = OperationalStateCluster().SetOperationalState(OperationalStateEnum::kRunning);
    if (error != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void LoggingIrrigationSystem::HandleStartStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigationSystem::HandleStartStateCallback()");
    // Watering is started per zone via the Water Valve endpoints; the system state follows the valves.
    err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
}

void LoggingIrrigationSystem::HandleStopStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigationSystem::HandleStopStateCallback()");

    // Needed when stopping from Paused: the valves are already closed, so no close notification sets Stopped.
    if (OperationalStateCluster().SetOperationalState(OperationalStateEnum::kStopped) != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }

    for (size_t i = 0; i < mWaterValves.size(); ++i)
    {
        if (!mWaterValves[i]->IsOpen())
        {
            continue;
        }

        if (mWaterValves[i]->CloseValve() != CHIP_NO_ERROR)
        {
            err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }
    }
    // Forget paused zones so a Resume after Stop does not reopen them.
    for (auto & zone : mPausedZones)
    {
        zone.reset();
    }
}

CHIP_ERROR LoggingIrrigationSystem::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
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

    mPausedZones.resize(mWaterValves.size());
    return CHIP_NO_ERROR;
}
void LoggingIrrigationSystem::UnregisterParts(CodeDrivenDataModelProvider & provider)
{
    for (const auto & valve : mWaterValves)
    {
        if (valve->GetEndpointId() != kInvalidEndpointId)
        {
            valve->Unregister(provider);
        }
    }
    mWaterValves.clear();
    mPausedZones.clear();
}

void LoggingIrrigationSystem::OnValveOpened()
{
    // The master valve must be open before any zone can water.
    if (mMasterValve.has_value() && !mMasterValve->IsOpen())
    {
        LogErrorOnFailure(mMasterValve->Open());
    }
    if (HasOperationalState())
    {
        // Any open zone means water is flowing, even if it was opened directly while Paused.
        LogErrorOnFailure(OperationalStateCluster().SetOperationalState(OperationalStateEnum::kRunning));
    }
}

void LoggingIrrigationSystem::OnValveClosed()
{
    bool anyOpen = std::any_of(mWaterValves.begin(), mWaterValves.end(), [](const auto & v) { return v->IsOpen(); });
    if (anyOpen)
    {
        // Other zones are still watering.
        return;
    }

    // Last zone closed: close the master valve, also while Paused.
    if (mMasterValve.has_value() && mMasterValve->IsOpen())
    {
        LogErrorOnFailure(mMasterValve->Close());
    }
    // A close never means Running; keep Paused so pausing several zones does not end in Stopped.
    if (HasOperationalState() &&
        OperationalStateCluster().GetCurrentOperationalState() != to_underlying(OperationalStateEnum::kPaused))
    {
        LogErrorOnFailure(OperationalStateCluster().SetOperationalState(OperationalStateEnum::kStopped));
    }
}

} // namespace app
} // namespace chip
