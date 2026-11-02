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

#include <cstddef>
#include <iterator>

using namespace chip::app::Clusters::OperationalState;

namespace chip {
namespace app {

DataModel::Nullable<uint32_t> LoggingIrrigationSystem::GetCountdownTime()
{
    // Longest remaining duration of the open valves; valves opened without a duration have none to count.
    DataModel::Nullable<uint32_t> countdownTime;
    for (const auto & valve : mWaterValves)
    {
        if (!valve->IsOpen())
        {
            continue;
        }
        DataModel::Nullable<uint32_t> remaining = valve->ValveConfigurationAndControlCluster().GetRemainingDuration();
        if (!remaining.IsNull() && (countdownTime.IsNull() || remaining.Value() > countdownTime.Value()))
        {
            countdownTime = remaining;
        }
    }
    return countdownTime;
}

CHIP_ERROR LoggingIrrigationSystem::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    static constexpr OperationalStateEnum kStates[] = { OperationalStateEnum::kStopped, OperationalStateEnum::kRunning,
                                                        OperationalStateEnum::kPaused, OperationalStateEnum::kError };
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

    for (size_t i = 0; i < mWaterValves.size(); ++i)
    {
        if (!mWaterValves[i]->IsOpen())
        {
            mPausedZones[i].reset();
            continue;
        }
        auto & valveCluster = mWaterValves[i]->ValveConfigurationAndControlCluster();
        // Save before closing: closing clears the valve's open level and remaining duration.
        mPausedZones[i] = PausedZone{ valveCluster.GetCurrentLevel(), valveCluster.GetRemainingDuration() };
        if (valveCluster.CloseValve() != CHIP_NO_ERROR)
        {
            mPausedZones[i].reset();
            err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }
    }
    // Set Paused last: closing the last valve above sets Stopped; this overrides it.
    CHIP_ERROR error = OperationalStateCluster().SetOperationalState(OperationalStateEnum::kPaused);
    if (error != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
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
        CHIP_ERROR error = mWaterValves[i]->ValveConfigurationAndControlCluster().OpenValve(zone->level, zone->remainingDuration);
        if (error != CHIP_NO_ERROR)
        {
            err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }
        zone.reset();
    }
}

void LoggingIrrigationSystem::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigationSystem::OnIdentifyStart()");
}

void LoggingIrrigationSystem::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigationSystem::OnIdentifyStop()");
}

void LoggingIrrigationSystem::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "LoggingIrrigationSystem::OnTriggerEffect()");
}

CHIP_ERROR LoggingIrrigationSystem::RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate).WithDelegate(this));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mOperationalStateCluster.Create(
        endpoint, *this,
        Clusters::OperationalState::OperationalStateCluster::Config{
            .optionalAttributes =
                Clusters::OperationalState::OperationalStateCluster::OptionalAttributeSet().Set<Attributes::CountdownTime::Id>() });
    ReturnErrorOnFailure(provider.AddCluster(mOperationalStateCluster.Registration()));

    mFlowMeasurementCluster.Create(endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mFlowMeasurementCluster.Registration()));

    return CHIP_NO_ERROR;
}

void LoggingIrrigationSystem::UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
    if (mFlowMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mFlowMeasurementCluster.Cluster()));
        mFlowMeasurementCluster.Destroy();
    }
    if (mOperationalStateCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOperationalStateCluster.Cluster()));
        mOperationalStateCluster.Destroy();
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

    for (size_t i = 0; i < mWaterValves.size(); ++i)
    {
        if (!mWaterValves[i]->IsOpen())
        {
            continue;
        }

        if (mWaterValves[i]->ValveConfigurationAndControlCluster().CloseValve() != CHIP_NO_ERROR)
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

    // Set Stopped last so a failed close above does not report Stopped while a zone is still open.
    // Still needed when stopping from Paused: the valves are already closed, so no valve close sets Stopped.
    if (OperationalStateCluster().SetOperationalState(OperationalStateEnum::kStopped) != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

CHIP_ERROR LoggingIrrigationSystem::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                                  EndpointComposition composition)
{
    VerifyOrReturnError(!mValveContext.empty(), CHIP_ERROR_INCORRECT_STATE);

    for (auto & context : mValveContext)
    {
        auto valve = std::make_unique<IrrigationWaterValve>(mTimerDelegate, context.config, mOperationalStateCluster, mMasterValve,
                                                            mOpenValves);
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
    mOpenValves = 0;
}

DataModel::Nullable<Percent> IrrigationWaterValve::HandleOpenValve(DataModel::Nullable<Percent> level)
{
    if (!mOpen)
    {
        mOpen = true;
        // The master valve must be open before any valve can water.
        if (mOpenValves++ == 0 && mMasterValve.has_value())
        {
            LogErrorOnFailure(mMasterValve->Open());
        }
        // Any open valve means water is flowing, even if it was opened directly while Paused.
        if (mOperationalState.IsConstructed())
        {
            LogErrorOnFailure(mOperationalState.Cluster().SetOperationalState(OperationalStateEnum::kRunning));
        }
    }
    return LoggingWaterValve::HandleOpenValve(level);
}

CHIP_ERROR IrrigationWaterValve::HandleCloseValve()
{
    ReturnErrorOnFailure(LoggingWaterValve::HandleCloseValve());

    if (mOpen && --mOpenValves == 0)
    {
        // Last valve closed: close the master valve, also while Paused.
        if (mMasterValve.has_value())
        {
            LogErrorOnFailure(mMasterValve->Close());
        }
        // A close never means Running; keep Paused so pausing several valves does not end in Stopped.
        if (mOperationalState.IsConstructed() &&
            mOperationalState.Cluster().GetCurrentOperationalState() != to_underlying(OperationalStateEnum::kPaused))
        {
            LogErrorOnFailure(mOperationalState.Cluster().SetOperationalState(OperationalStateEnum::kStopped));
        }
    }
    mOpen = false;
    UpdateCountdownTime();
    return CHIP_NO_ERROR;
}

void IrrigationWaterValve::HandleRemainingDurationTick(uint32_t duration)
{
    LoggingWaterValve::HandleRemainingDurationTick(duration);
    UpdateCountdownTime();
}

void IrrigationWaterValve::UpdateCountdownTime()
{
    if (mOperationalState.IsConstructed())
    {
        mOperationalState.Cluster().UpdateCountdownTimeFromDelegate();
    }
}

} // namespace app
} // namespace chip
