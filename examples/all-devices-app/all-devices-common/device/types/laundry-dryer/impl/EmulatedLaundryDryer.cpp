/*
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

#include <device/types/laundry-dryer/impl/EmulatedLaundryDryer.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

EmulatedLaundryDryer::EmulatedLaundryDryer(TimerDelegate & timerDelegate) :
    LaundryDryer(static_cast<Clusters::OperationalState::Delegate &>(*this)), mTimerDelegate(timerDelegate)
{}

EmulatedLaundryDryer::~EmulatedLaundryDryer()
{
    CancelTimer();
}

void EmulatedLaundryDryer::Unregister(CodeDrivenDataModelProvider & provider)
{
    CancelTimer();
    mCountdownTime.SetNull();
    LaundryDryer::Unregister(provider);
}

void EmulatedLaundryDryer::CancelTimer()
{
    mTimerDelegate.CancelTimer(this);
}

void EmulatedLaundryDryer::StartEmulatedOperationTimer()
{
    CancelTimer();
    mCountdownTime = DataModel::MakeNullable<uint32_t>(kEmulatedOperationDurationSec);
    SuccessOrDie(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kEmulatedOperationDurationSec)));
}

void EmulatedLaundryDryer::TimerFired()
{
    ChipLogProgress(Zcl, "EmulatedLaundryDryer: Emulated operation timer finished. Reverting state to Stopped.");
    mCountdownTime.SetNull();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kStopped));
}

CHIP_ERROR EmulatedLaundryDryer::GetOperationalStateAtIndex(size_t index,
                                                            Clusters::OperationalState::GenericOperationalState & operationalState)
{
    static const Clusters::OperationalState::GenericOperationalState kSupportedStates[] = {
        Clusters::OperationalState::GenericOperationalState(
            to_underlying(Clusters::OperationalState::OperationalStateEnum::kStopped), MakeOptional("Stopped"_span)),
        Clusters::OperationalState::GenericOperationalState(
            to_underlying(Clusters::OperationalState::OperationalStateEnum::kRunning), MakeOptional("Running"_span)),
        Clusters::OperationalState::GenericOperationalState(
            to_underlying(Clusters::OperationalState::OperationalStateEnum::kPaused), MakeOptional("Paused"_span)),
        Clusters::OperationalState::GenericOperationalState(to_underlying(Clusters::OperationalState::OperationalStateEnum::kError),
                                                            MakeOptional("Error"_span)),
    };

    if (index >= MATTER_ARRAY_SIZE(kSupportedStates))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    operationalState = kSupportedStates[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmulatedLaundryDryer::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    static constexpr CharSpan kSupportedPhases[] = { "Starting"_span, "Operating"_span, "Finishing"_span };

    if (index >= MATTER_ARRAY_SIZE(kSupportedPhases))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    return CopyCharSpanToMutableCharSpan(kSupportedPhases[index], operationalPhase);
}

void EmulatedLaundryDryer::HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedLaundryDryer: Pause command received.");
    CancelTimer();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kPaused;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kPaused));
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedLaundryDryer::HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedLaundryDryer: Resume command received.");
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kRunning;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kRunning));
    StartEmulatedOperationTimer();
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedLaundryDryer::HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedLaundryDryer: Start command received.");
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kRunning;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kRunning));
    StartEmulatedOperationTimer();
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedLaundryDryer::HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedLaundryDryer: Stop command received.");
    CancelTimer();
    mCountdownTime.SetNull();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kStopped));
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

} // namespace chip::app
