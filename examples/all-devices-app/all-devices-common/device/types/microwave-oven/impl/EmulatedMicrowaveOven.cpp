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

#include <device/types/microwave-oven/impl/EmulatedMicrowaveOven.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

EmulatedMicrowaveOven::EmulatedMicrowaveOven(const Context & context) :
    MicrowaveOven(MicrowaveOven::Config{
        .operationalStateDelegate   = *this,
        .controlIntegrationDelegate = *this,
        .controlAppDelegate         = *this,
        .modeDelegate               = *this,
        .diagnosticDataProvider     = context.diagnosticDataProvider,
    }),
    mTimerDelegate(context.timerDelegate)
{}

EmulatedMicrowaveOven::~EmulatedMicrowaveOven()
{
    CancelTimer();
}

void EmulatedMicrowaveOven::Unregister(CodeDrivenDataModelProvider & provider)
{
    CancelTimer();
    mCountdownTime.SetNull();
    MicrowaveOven::Unregister(provider);
}

DataModel::Nullable<uint32_t> EmulatedMicrowaveOven::GetCountdownTime()
{
    if (mOperationalState != Clusters::OperationalState::OperationalStateEnum::kRunning)
    {
        return mCountdownTime;
    }

    // Per Matter Operational State Specification Section 1.14.5.1 (CountdownTime Attribute):
    // Standard 1-second countdown progress at 1 unit/sec SHALL NOT trigger attribute reporting to prevent
    // network flooding. Therefore, CountdownTime is dynamically computed on read from the target completion timestamp,
    // avoiding 1-second timers or reporting spam.
    auto now = System::SystemClock().GetMonotonicTimestamp();
    if (now >= mOperationEndTime)
    {
        return DataModel::MakeNullable<uint32_t>(0);
    }

    auto remainingSec = std::chrono::duration_cast<System::Clock::Seconds32>(mOperationEndTime - now).count();
    return DataModel::MakeNullable<uint32_t>(static_cast<uint32_t>(remainingSec));
}

void EmulatedMicrowaveOven::CancelTimer()
{
    mTimerDelegate.CancelTimer(this);
}

void EmulatedMicrowaveOven::StartEmulatedOperationTimer(uint32_t durationSec)
{
    CancelTimer();
    mOperationEndTime = System::SystemClock().GetMonotonicTimestamp() + System::Clock::Seconds32(durationSec);
    mCountdownTime    = DataModel::MakeNullable<uint32_t>(durationSec);
    SuccessOrDie(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(durationSec)));
}

void EmulatedMicrowaveOven::TimerFired()
{
    ChipLogProgress(Zcl, "EmulatedMicrowaveOven: Emulated operation timer finished. Reverting state to Stopped.");
    mCountdownTime.SetNull();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kStopped));
}

CHIP_ERROR EmulatedMicrowaveOven::GetOperationalStateAtIndex(size_t index,
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

CHIP_ERROR EmulatedMicrowaveOven::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    static constexpr CharSpan kSupportedPhases[] = { "Starting"_span, "Operating"_span, "Finishing"_span };

    if (index >= MATTER_ARRAY_SIZE(kSupportedPhases))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    return CopyCharSpanToMutableCharSpan(kSupportedPhases[index], operationalPhase);
}

void EmulatedMicrowaveOven::HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedMicrowaveOven: Pause command received.");
    mCountdownTime = GetCountdownTime();
    CancelTimer();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kPaused;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kPaused));
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedMicrowaveOven::HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedMicrowaveOven: Resume command received.");
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kRunning;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kRunning));
    uint32_t remainingSec = mCountdownTime.IsNull() ? kEmulatedOperationDurationSec : mCountdownTime.Value();
    StartEmulatedOperationTimer(remainingSec);
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedMicrowaveOven::HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedMicrowaveOven: Start command received.");
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kRunning;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kRunning));
    StartEmulatedOperationTimer();
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedMicrowaveOven::HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedMicrowaveOven: Stop command received.");
    CancelTimer();
    mCountdownTime.SetNull();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kStopped));
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

Protocols::InteractionModel::Status
EmulatedMicrowaveOven::HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTimeSec, bool startAfterSetting,
                                                          Optional<uint8_t> powerSettingNum, Optional<uint8_t> wattSettingIndex)
{
    if (startAfterSetting)
    {
        Clusters::OperationalState::GenericOperationalError err(
            to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
        HandleStartStateCallback(err);
    }
    return Protocols::InteractionModel::Status::Success;
}

CHIP_ERROR EmulatedMicrowaveOven::GetWattSettingByIndex(uint8_t index, uint16_t & wattSetting)
{
    if (index == 0)
    {
        wattSetting = kWattRating;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR EmulatedMicrowaveOven::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeLabels), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kModeLabels[modeIndex], label);
}

CHIP_ERROR EmulatedMicrowaveOven::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeLabels), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = modeIndex;
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmulatedMicrowaveOven::GetModeTagsByIndex(uint8_t modeIndex,
                                                     DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeTagValues), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    VerifyOrReturnError(modeTags.size() >= 1, CHIP_ERROR_INVALID_ARGUMENT);

    modeTags[0].mfgCode.ClearValue();
    modeTags[0].value = kModeTagValues[modeIndex];
    modeTags.reduce_size(1);
    return CHIP_NO_ERROR;
}

void EmulatedMicrowaveOven::HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    response.status = to_underlying(Clusters::ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR EmulatedMicrowaveOven::GetNormalOperatingMode(uint8_t & mode) const
{
    mode = 0;
    return CHIP_NO_ERROR;
}

bool EmulatedMicrowaveOven::IsSupportedMode(uint8_t mode) const
{
    return mode == 0 || mode == 1;
}

bool EmulatedMicrowaveOven::IsSupportedOperationalStateCommand(EndpointId endpointId, CommandId commandId) const
{
    return commandId == Clusters::OperationalState::Commands::Start::Id;
}

} // namespace chip::app
