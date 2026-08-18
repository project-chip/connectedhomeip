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

#include <device/types/laundry-washer/impl/EmulatedLaundryWasher.h>

#include <clusters/LaundryWasherMode/Enums.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

EmulatedLaundryWasher::EmulatedLaundryWasher(const Context & context) :
    LaundryWasher(LaundryWasher::Config{
        .operationalStateDelegate = static_cast<Clusters::OperationalState::Delegate &>(*this),
        .controlsDelegate         = static_cast<Clusters::LaundryWasherControls::Delegate &>(*this),
        .modeDelegate             = static_cast<Clusters::ModeBase::AppDelegate &>(*this),
        .diagnosticDataProvider   = context.diagnosticDataProvider,
    }),
    mTimerDelegate(context.timerDelegate)
{}

EmulatedLaundryWasher::~EmulatedLaundryWasher()
{
    CancelTimer();
}

void EmulatedLaundryWasher::Unregister(CodeDrivenDataModelProvider & provider)
{
    CancelTimer();
    mCountdownTime.SetNull();
    LaundryWasher::Unregister(provider);
}

void EmulatedLaundryWasher::CancelTimer()
{
    mTimerDelegate.CancelTimer(this);
}

void EmulatedLaundryWasher::StartEmulatedOperationTimer()
{
    CancelTimer();
    mCountdownTime = DataModel::MakeNullable<uint32_t>(kEmulatedOperationDurationSec);
    SuccessOrDie(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kEmulatedOperationDurationSec)));
}

void EmulatedLaundryWasher::TimerFired()
{
    ChipLogProgress(Zcl, "EmulatedLaundryWasher: Emulated operation timer finished. Reverting state to Stopped.");
    mCountdownTime.SetNull();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kStopped));
}

CHIP_ERROR EmulatedLaundryWasher::GetOperationalStateAtIndex(size_t index, Clusters::OperationalState::GenericOperationalState & operationalState)
{
    static const Clusters::OperationalState::GenericOperationalState kSupportedStates[] = {
        Clusters::OperationalState::GenericOperationalState(
            to_underlying(Clusters::OperationalState::OperationalStateEnum::kStopped), MakeOptional("Stopped"_span)),
        Clusters::OperationalState::GenericOperationalState(
            to_underlying(Clusters::OperationalState::OperationalStateEnum::kRunning), MakeOptional("Running"_span)),
        Clusters::OperationalState::GenericOperationalState(
            to_underlying(Clusters::OperationalState::OperationalStateEnum::kPaused), MakeOptional("Paused"_span)),
        Clusters::OperationalState::GenericOperationalState(
            to_underlying(Clusters::OperationalState::OperationalStateEnum::kError), MakeOptional("Error"_span)),
    };

    if (index >= MATTER_ARRAY_SIZE(kSupportedStates))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    operationalState = kSupportedStates[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmulatedLaundryWasher::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    static constexpr CharSpan kSupportedPhases[] = { "Starting"_span, "Operating"_span, "Finishing"_span };

    if (index >= MATTER_ARRAY_SIZE(kSupportedPhases))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    return CopyCharSpanToMutableCharSpan(kSupportedPhases[index], operationalPhase);
}

void EmulatedLaundryWasher::HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedLaundryWasher: Pause command received.");
    CancelTimer();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kPaused;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kPaused));
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedLaundryWasher::HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedLaundryWasher: Resume command received.");
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kRunning;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kRunning));
    StartEmulatedOperationTimer();
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedLaundryWasher::HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedLaundryWasher: Start command received.");
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kRunning;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kRunning));
    StartEmulatedOperationTimer();
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedLaundryWasher::HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedLaundryWasher: Stop command received.");
    CancelTimer();
    mCountdownTime.SetNull();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kStopped));
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

CHIP_ERROR EmulatedLaundryWasher::GetSpinSpeedAtIndex(size_t index, MutableCharSpan & spinSpeed)
{
    VerifyOrReturnError(index < MATTER_ARRAY_SIZE(kSpinSpeeds), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kSpinSpeeds[index], spinSpeed);
}

CHIP_ERROR EmulatedLaundryWasher::GetSupportedRinseAtIndex(size_t index, Clusters::LaundryWasherControls::NumberOfRinsesEnum & supportedRinse)
{
    VerifyOrReturnError(index < MATTER_ARRAY_SIZE(kRinses), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    supportedRinse = kRinses[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmulatedLaundryWasher::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeLabels), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kModeLabels[modeIndex], label);
}

CHIP_ERROR EmulatedLaundryWasher::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeLabels), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = modeIndex;
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmulatedLaundryWasher::GetModeTagsByIndex(uint8_t modeIndex,
                                                      DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeTagValues), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    VerifyOrReturnError(modeTags.size() >= 1, CHIP_ERROR_INVALID_ARGUMENT);

    modeTags[0].mfgCode.ClearValue();
    modeTags[0].value = kModeTagValues[modeIndex];
    modeTags.reduce_size(1);
    return CHIP_NO_ERROR;
}

void EmulatedLaundryWasher::HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    response.status = to_underlying(Clusters::ModeBase::StatusCode::kSuccess);
}

} // namespace chip::app
