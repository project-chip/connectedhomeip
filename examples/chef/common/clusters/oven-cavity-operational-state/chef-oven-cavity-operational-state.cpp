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

#include "chef-oven-cavity-operational-state.h"
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#ifdef MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER
namespace chip {
namespace app {
namespace Clusters {
namespace OvenCavityOperationalState {

constexpr size_t kOvenCavityOperationalStateTableSize = MATTER_DM_OPERATIONAL_STATE_OVEN_CLUSTER_SERVER_ENDPOINT_COUNT;
static_assert(kOvenCavityOperationalStateTableSize < kEmberInvalidEndpointIndex, "OvenCavityOperationalState table size error");

std::unique_ptr<ChefDelegate> gDelegateTable[kOvenCavityOperationalStateTableSize];
std::unique_ptr<Instance> gInstanceTable[kOvenCavityOperationalStateTableSize];

/**
 * Timer mechanism -
 *   1. Call StartTimer with input of one second and function onOvenCavityOperationalStateTimerTick to start the timer.
 *      This will call onOvenCavityOperationalStateTimerTick after one second.
 *   2. onOvenCavityOperationalStateTimerTick is the timer tick function. It decides whether to run another one second iteration
 *      (if there are seconds left to run) or just return and end the timer (when state is invalid or cycle is complete).
 * Timer usage -
 *   * Timer is started on receiving start command.
 *   * Timer continues to run until running time has surpassed cycle time (cycle is complete) OR operational state is non-running.
 *   * Stop command handler ends an ongoing timer.
 */
static void onOvenCavityOperationalStateTimerTick(System::Layer * systemLayer, void * data)
{
    ChefDelegate * delegate = reinterpret_cast<ChefDelegate *>(data);

    uint8_t opState = delegate->GetCurrentOperationalState();
    if (opState != to_underlying(OperationalStateEnum::kRunning))
    {
        ChipLogError(DeviceLayer, "onOperationalStateTimerTick: Operational cycle can not be active in state %d", opState);
        return;
    }

    if (!delegate->CheckCycleActive())
        return;

    delegate->CycleSecondTick();

    if (delegate->CheckCycleComplete())
    {
        OperationalState::GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
        delegate->HandleStopStateCallback(err);
    }
    else
    {
        CHIP_ERROR err =
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOvenCavityOperationalStateTimerTick, delegate);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "onOvenCavityOperationalStateTimerTick: Failed to start timer. %" CHIP_ERROR_FORMAT,
                         err.Format());
        }
    }
}

DataModel::Nullable<uint32_t> ChefDelegate::GetCountdownTime()
{
    if (mRunningTime.IsNull() || CheckCycleComplete())
        return DataModel::NullNullable;

    return DataModel::MakeNullable(kCycleSeconds - mRunningTime.Value());
}

bool ChefDelegate::StartCycle()
{
    if (!mRunningTime.IsNull())
    {
        ChipLogError(DeviceLayer, "StartCycle: Cycle is in progress. Can not start new cycle.");
        return false;
    }
    mRunningTime.SetNonNull(static_cast<uint32_t>(0));
    app::DataModel::Nullable<uint8_t> phase = GetRunningPhase();
    if (phase.IsNull())
    {
        ChipLogError(DeviceLayer, "StartCycle: Phase did't update to non-NULL.");
        return false;
    }
    if (phase.Value() != kPreHeatingIndex)
    {
        ChipLogError(DeviceLayer, "StartCycle: Unexpected start phase: %d.", phase.Value());
        return false;
    }
    CHIP_ERROR err = GetInstance()->SetCurrentPhase(phase);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "StartCycle: Error setting current phase %" CHIP_ERROR_FORMAT, err.Format());
    }
    ChipLogDetail(DeviceLayer, "Oven Cycle Started. Current Phase: %d. Cycle time: %d", phase.Value(), kCycleSeconds);
    return true;
}

void ChefDelegate::CycleSecondTick()
{
    if (mRunningTime.IsNull() || CheckCycleComplete())
        return;
    uint8_t opState                       = GetCurrentOperationalState();
    DataModel::Nullable<uint8_t> oldPhase = GetRunningPhase();
    if (opState == to_underlying(OperationalStateEnum::kRunning))
    {
        mRunningTime.SetNonNull(mRunningTime.ValueOr(0) + 1);
    }
    else
    {
        ChipLogError(DeviceLayer, "CycleSecondTick: Invalid state %d", opState);
        return;
    }
    DataModel::Nullable<uint8_t> newPhase = GetRunningPhase();
    if (newPhase != oldPhase)
    {
        CHIP_ERROR err = GetInstance()->SetCurrentPhase(newPhase);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "CycleSecondTick: Error setting current phase %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
    GetInstance()->UpdateCountdownTimeFromDelegate();

    ChipLogDetail(DeviceLayer, "CycleSecondTick: Running time: %d.", mRunningTime.ValueOr(0));
    if (newPhase.IsNull())
    {
        ChipLogDetail(DeviceLayer, "CycleSecondTick: Phase: NULL");
    }
    else
    {
        ChipLogDetail(DeviceLayer, "CycleSecondTick: Phase: %d", newPhase.Value());
    }
}

DataModel::Nullable<uint8_t> ChefDelegate::GetRunningPhase()
{
    if (mRunningTime.IsNull() || CheckCycleComplete())
        return DataModel::NullNullable;

    if (mRunningTime.Value() >= kPreHeatingSeconds + kPreHeatedSeconds)
        return kCoolingDownIndex;
    else if (mRunningTime.Value() >= kPreHeatingSeconds)
        return kPreHeatedIndex;
    else
        return kPreHeatingIndex;
}

bool ChefDelegate::CheckCycleComplete()
{
    return !mRunningTime.IsNull() && mRunningTime.Value() >= kCycleSeconds;
}

void ChefDelegate::EndCycle()
{
    mRunningTime.SetNull();
    CHIP_ERROR err = GetInstance()->SetCurrentPhase(DataModel::NullNullable);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "EndCycle: Error setting current phase %" CHIP_ERROR_FORMAT, err.Format());
    }
}

bool ChefDelegate::CheckCycleActive()
{
    return !mRunningTime.IsNull();
}

uint8_t ChefDelegate::GetCurrentOperationalState()
{
    return GetInstance()->GetCurrentOperationalState();
}

void ChefDelegate::HandleStartStateCallback(OperationalState::GenericOperationalError & err)
{
    OperationalState::GenericOperationalError current_err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    GetInstance()->GetCurrentOperationalError(current_err);
    uint8_t opState = GetCurrentOperationalState();

    if (current_err.errorStateID != to_underlying(OperationalState::ErrorStateEnum::kNoError) ||
        opState == to_underlying(OperationalStateEnum::kError))
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume));
        return;
    }

    if (opState == to_underlying(OperationalStateEnum::kRunning))
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
        return;
    }

    if (CheckCycleActive())
        EndCycle();

    CHIP_ERROR error = GetInstance()->SetOperationalState(to_underlying(OperationalStateEnum::kRunning));
    if (error != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }
    if (!StartCycle())
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume));
        return;
    }
    error = DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOvenCavityOperationalStateTimerTick, this);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleStartStateCallback: Failed to start timer. %" CHIP_ERROR_FORMAT, error.Format());
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

void ChefDelegate::HandleStopStateCallback(OperationalState::GenericOperationalError & err)
{
    uint32_t RunningTime = mRunningTime.ValueOr(0);
    uint8_t opState      = GetCurrentOperationalState();
    if (CheckCycleActive())
    {
        DeviceLayer::SystemLayer().CancelTimer(onOvenCavityOperationalStateTimerTick, this);
        EndCycle();
    }

    if (opState != to_underlying(OperationalStateEnum::kRunning))
    {
        ChipLogDetail(DeviceLayer, "HandleStopStateCallback: Cycle not started. Current state = %d. Returning.", opState);
        err.Set(to_underlying(ErrorStateEnum::kNoError));
        return;
    }

    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalStateEnum::kStopped));
    if (error != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }

    OperationalState::GenericOperationalError current_err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    GetInstance()->GetCurrentOperationalError(current_err);

    Optional<DataModel::Nullable<uint32_t>> totalTime((DataModel::Nullable<uint32_t>(RunningTime)));

    GetInstance()->OnOperationCompletionDetected(static_cast<uint8_t>(current_err.errorStateID), totalTime, NullOptionalType());
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

void ChefDelegate::HandlePauseStateCallback(OperationalState::GenericOperationalError & err)
{
    err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation)); // Not supported.
}

void ChefDelegate::HandleResumeStateCallback(OperationalState::GenericOperationalError & err)
{
    err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation)); // Not supported.
}

CHIP_ERROR ChefDelegate::GetOperationalStateAtIndex(size_t index, OperationalState::GenericOperationalState & operationalState)
{
    if (index >= mOperationalStateList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalState = OperationalState::GenericOperationalState(to_underlying(mOperationalStateList[index]));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChefDelegate::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    if (index >= mOperationalPhaseList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return CopyCharSpanToMutableCharSpan(mOperationalPhaseList[index], operationalPhase);
}

/**
 * Initializes OvenCavityOperationalState cluster for the app (all endpoints where cluster is enabled).
 */
void InitChefOvenCavityOperationalStateCluster()
{
    const uint16_t endpointCount = emberAfEndpointCount();

    for (uint16_t endpointIndex = 0; endpointIndex < endpointCount; endpointIndex++)
    {
        EndpointId endpointId = emberAfEndpointFromIndex(endpointIndex);
        if (endpointId == kInvalidEndpointId)
        {
            continue;
        }

        // Check if endpoint has OvenCavityOperationalState cluster enabled
        uint16_t epIndex =
            emberAfGetClusterServerEndpointIndex(endpointId, Id, MATTER_DM_OPERATIONAL_STATE_OVEN_CLUSTER_SERVER_ENDPOINT_COUNT);
        if (epIndex >= kOvenCavityOperationalStateTableSize)
            continue;

        gDelegateTable[epIndex] = std::make_unique<ChefDelegate>();

        gInstanceTable[epIndex] = std::make_unique<Instance>(gDelegateTable[epIndex].get(), endpointId);
        gInstanceTable[epIndex]->Init();

        ChipLogProgress(DeviceLayer, "Endpoint %d OvenCavityOperationalState Initialized.", endpointId);
    }
}

} // namespace OvenCavityOperationalState

} // namespace Clusters
} // namespace app
} // namespace chip

#endif // MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER
