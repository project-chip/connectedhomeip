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
#include <app/util/endpoint-config-api.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace chef {
namespace OvenCavityOperationalState {

constexpr size_t kOvenCavityOperationalStateTableSize = MATTER_DM_OPERATIONAL_STATE_OVEN_CLUSTER_SERVER_ENDPOINT_COUNT;
static_assert(kOvenCavityOperationalStateTableSize <= kEmberInvalidEndpointIndex, "OvenCavityOperationalState table size error");

std::unique_ptr<Delegate> gDelegateTable[kOvenCavityOperationalStateTableSize];
std::unique_ptr<Clusters::OvenCavityOperationalState::Instance> gInstanceTable[kOvenCavityOperationalStateTableSize];

static void onOperationalStateTimerTick(System::Layer * systemLayer, void * data)
{
    Delegate * delegate = reinterpret_cast<Delegate *>(data);

    uint8_t opState = delegate->GetCurrentOperationalState();
    if (opState != to_underlying(Clusters::OvenCavityOperationalState::OperationalStateEnum::kRunning) &&
        opState != to_underlying(Clusters::OvenCavityOperationalState::OperationalStateEnum::kPaused))
    {
        ChipLogError(DeviceLayer, "onOperationalStateTimerTick: Operational cycle can not be active in state %d", opState);
        (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, delegate);
        return;
    }

    if (!delegate->CheckCycleActive())
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, delegate);
        return;
    }

    delegate->CycleSecondTick();

    if (delegate->CheckCycleComplete())
    {
        GenericOperationalError err;
        delegate->HandleStopStateCallback(err);
    }
    else
    {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, delegate);
    }
}

DataModel::Nullable<uint32_t> Delegate::GetCountdownTime()
{
    if (mRunningTime.IsNull() || CheckCycleComplete())
        return DataModel::NullNullable;

    return DataModel::MakeNullable(kCycleSeconds - mRunningTime.Value());
}

bool Delegate::StartCycle()
{
    if (!mRunningTime.IsNull())
    {
        ChipLogError(DeviceLayer, "StartCycle: Cycle is in progress. Can not start new cycle.");
        return false;
    }
    mRunningTime.SetNonNull(static_cast<uint32_t>(0));
    mPausedTime.SetNonNull(static_cast<uint32_t>(0));
    app::DataModel::Nullable<uint8_t> phase = GetRunningPhase();
    if (phase.IsNull())
    {
        ChipLogError(DeviceLayer, "StartCycle: Phase did't update to non-NULL.");
        return false;
    }
    if (phase.Value() != 0)
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

void Delegate::CycleSecondTick()
{
    if (mRunningTime.IsNull() || CheckCycleComplete())
        return;
    uint8_t opState                       = GetCurrentOperationalState();
    DataModel::Nullable<uint8_t> oldPhase = GetRunningPhase();
    if (opState == to_underlying(Clusters::OvenCavityOperationalState::OperationalStateEnum::kRunning))
    {
        mRunningTime.SetNonNull(mRunningTime.Value() + 1);
    }
    else if (opState == to_underlying(Clusters::OvenCavityOperationalState::OperationalStateEnum::kPaused))
    {
        if (mPausedTime.IsNull())
            mPausedTime.SetNonNull(static_cast<uint32_t>(0));
        mPausedTime.SetNonNull(mPausedTime.Value() + 1);
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

    ChipLogDetail(DeviceLayer, "CycleSecondTick: Running time: %d. Paused time: %d", mRunningTime.Value(), mPausedTime.Value());
    if (newPhase.IsNull())
    {
        ChipLogDetail(DeviceLayer, "CycleSecondTick: Phase: NULL");
    }
    else
    {
        ChipLogDetail(DeviceLayer, "CycleSecondTick: Phase: %d", newPhase.Value());
    }
}

DataModel::Nullable<uint8_t> Delegate::GetRunningPhase()
{
    if (mRunningTime.IsNull() || CheckCycleComplete())
        return DataModel::NullNullable;

    if (mRunningTime.Value() >= kPreHeatingSeconds + kPreHeatedSeconds)
        return 2; // cooling down
    else if (mRunningTime.Value() >= kPreHeatingSeconds)
        return 1; // pre-heated
    else
        return 0; // pre-heating
}

bool Delegate::CheckCycleComplete()
{
    return !mRunningTime.IsNull() && mRunningTime.Value() >= kCycleSeconds;
}

void Delegate::EndCycle()
{
    mRunningTime.SetNull();
    mPausedTime.SetNull();
    CHIP_ERROR err = GetInstance()->SetCurrentPhase(DataModel::NullNullable);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "EndCycle: Error setting current phase %" CHIP_ERROR_FORMAT, err.Format());
    }
}

bool Delegate::CheckCycleActive()
{
    return !mRunningTime.IsNull();
}

uint8_t Delegate::GetCurrentOperationalState()
{
    return GetInstance()->GetCurrentOperationalState();
}

void Delegate::HandleStartStateCallback(OperationalState::GenericOperationalState & err)
{
    OperationalState::GenericOperationalError current_err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    GetInstance()->GetCurrentOperationalError(current_err);
    uint8_t opState = GetCurrentOperationalState();

    if (current_err.errorStateID != to_underlying(OperationalState::ErrorStateEnum::kNoError) ||
        opState == Clusters::OvenCavityOperationalState::OperationalStateEnum::kError)
    {
        err.Set(to_underlying(Clusters::OvenCavityOperationalState::ErrorStateEnum::kUnableToStartOrResume));
        return;
    }

    if (opState == to_underlying(Clusters::OvenCavityOperationalState::OperationalStateEnum::kRunning) ||
        opState == to_underlying(Clusters::OvenCavityOperationalState::OperationalStateEnum::kPaused))
    {
        err.Set(to_underlying(Clusters::OvenCavityOperationalState::ErrorStateEnum::kNoError));
        return;
    }

    if (CheckCycleActive())
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);
        EndCycle();
    }

    auto error =
        GetInstance()->SetOperationalState(to_underlying(Clusters::OvenCavityOperationalState::OperationalStateEnum::kRunning));
    if (error != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(Clusters::OvenCavityOperationalState::ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }
    if (!StartCycle())
    {
        err.Set(to_underlying(Clusters::OvenCavityOperationalState::ErrorStateEnum::kUnableToStartOrResume));
        return;
    }
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, this);
    err.Set(to_underlying(Clusters::OvenCavityOperationalState::ErrorStateEnum::kNoError));
}

void Delegate::HandleStopStateCallback(OperationalState::GenericOperationalError & err)
{
    uint32_t RunningTime = mRunningTime.IsNull() ? 0 : mRunningTime.Value();
    uint32_t PausedTime  = mPausedTime.IsNull() ? 0 : mPausedTime.Value();
    uint8_t opState      = GetCurrentOperationalState();
    if (CheckCycleActive())
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);
        EndCycle();
    }

    if (opState != Clusters::OvenCavityOperationalState::OperationalStateEnum::kRunning &&
        opState != Clusters::OvenCavityOperationalState::OperationalStateEnum::kPaused)
    {
        ChipLogDetail(DeviceLayer, "HandleStopStateCallback: Cycle not started. Current state = %d. Returning.",
                      to_underlying(current_state));
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
        return;
    }

    auto error =
        GetInstance()->SetOperationalState(to_underlying(Clusters::OvenCavityOperationalState::OperationalStateEnum::kStopped));
    if (error != CHIP_NO_ERROR)
    {
        err.Set(to_underlying(Clusters::OvenCavityOperationalState::ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }

    OperationalState::GenericOperationalError current_err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    GetInstance()->GetCurrentOperationalError(current_err);

    Optional<DataModel::Nullable<uint32_t>> totalTime((DataModel::Nullable<uint32_t>(RunningTime + PausedTime)));
    Optional<DataModel::Nullable<uint32_t>> pausedTime((DataModel::Nullable<uint32_t>(PausedTime)));

    GetInstance()->OnOperationCompletionDetected(static_cast<uint8_t>(current_err.errorStateID), totalTime, pausedTime);
    err.Set(to_underlying(Clusters::OvenCavityOperationalState::ErrorStateEnum::kNoError));
}

void Delegate::HandlePauseStateCallback(OperationalState::GenericOperationalError & err)
{
    err.Set(to_underlying(Clusters::OvenCavityOperationalState::ErrorStateEnum::kUnableToCompleteOperation)); // Not supported.
}

void Delegate::HandleResumeStateCallback(OperationalState::GenericOperationalError & err)
{
    err.Set(to_underlying(Clusters::OvenCavityOperationalState::ErrorStateEnum::kUnableToCompleteOperation)); // Not supported.
}

CHIP_ERROR Delegate::GenericOperationalStateDelegateImpl::GetOperationalStateAtIndex(
    size_t index, OperationalState::GenericOperationalState & operationalState)
{
    if (index >= mOperationalStateList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalState = OperationalState::GenericOperationalState(to_underlying(mOperationalStateList[index]));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Delegate::GenericOperationalStateDelegateImpl::GetOperationalPhaseAtIndex(size_t index,
                                                                                     MutableCharSpan & operationalPhase)
{
    if (index >= mOperationalPhaseList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return CopyCharSpanToMutableCharSpan(mOperationalPhaseList[index], operationalPhase);
}

/**
 * Initializes OvenCavityOperationalState cluster for the app (all endpoints).
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
        uint16_t epIndex = emberAfGetClusterServerEndpointIndex(endpointId, Clusters::OvenCavityOperationalState::Id,
                                                                MATTER_DM_OPERATIONAL_STATE_OVEN_CLUSTER_SERVER_ENDPOINT_COUNT);
        if (epIndex >= kOvenCavityOperationalStateTableSize)
            continue;

        gDelegateTable[epIndex] = std::make_unique<Delegate>();

        gInstanceTable[epIndex] =
            std::make_unique<Clusters::OvenCavityOperationalState::Instance>(gDelegateTable[epIndex].get(), endpointId);
        gInstanceTable[epIndex]->Init();

        ChipLogProgress(DeviceLayer, "Endpoint %d OvenCavityOperationalState Initialized.", endpointId);
    }
}

} // namespace OvenCavityOperationalState
} // namespace chef
