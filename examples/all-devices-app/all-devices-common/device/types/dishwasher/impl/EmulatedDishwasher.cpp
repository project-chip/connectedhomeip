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

#include <device/types/dishwasher/impl/EmulatedDishwasher.h>

#include <clusters/DishwasherMode/Enums.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

EmulatedDishwasher::EmulatedDishwasher(const Context & context) :
    Dishwasher(Dishwasher::Config{
        .operationalStateDelegate = *this,
        .modeDelegate             = *this,
        .diagnosticDataProvider   = context.diagnosticDataProvider,
    }),
    mTimerDelegate(context.timerDelegate)
{}

EmulatedDishwasher::~EmulatedDishwasher()
{
    CancelTimer();
}

void EmulatedDishwasher::Unregister(CodeDrivenDataModelProvider & provider)
{
    CancelTimer();
    mCountdownTime.SetNull();
    Dishwasher::Unregister(provider);
}

void EmulatedDishwasher::CancelTimer()
{
    mTimerDelegate.CancelTimer(this);
}

void EmulatedDishwasher::StartEmulatedOperationTimer()
{
    CancelTimer();
    mCountdownTime = DataModel::MakeNullable<uint32_t>(kEmulatedOperationDurationSec);
    SuccessOrDie(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kEmulatedOperationDurationSec)));
}

void EmulatedDishwasher::TimerFired()
{
    ChipLogProgress(Zcl, "EmulatedDishwasher: Emulated operation timer finished. Reverting state to Stopped.");
    mCountdownTime.SetNull();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kStopped));
}

CHIP_ERROR EmulatedDishwasher::GetOperationalStateAtIndex(size_t index,
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

CHIP_ERROR EmulatedDishwasher::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    static constexpr CharSpan kSupportedPhases[] = { "Starting"_span, "Operating"_span, "Finishing"_span };

    if (index >= MATTER_ARRAY_SIZE(kSupportedPhases))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    return CopyCharSpanToMutableCharSpan(kSupportedPhases[index], operationalPhase);
}

void EmulatedDishwasher::HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedDishwasher: Pause command received.");
    CancelTimer();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kPaused;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kPaused));
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedDishwasher::HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedDishwasher: Resume command received.");
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kRunning;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kRunning));
    StartEmulatedOperationTimer();
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedDishwasher::HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedDishwasher: Start command received.");
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kRunning;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kRunning));
    StartEmulatedOperationTimer();
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void EmulatedDishwasher::HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "EmulatedDishwasher: Stop command received.");
    CancelTimer();
    mCountdownTime.SetNull();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kStopped));
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

CHIP_ERROR EmulatedDishwasher::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kLabels), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kLabels[modeIndex], label);
}

CHIP_ERROR EmulatedDishwasher::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kLabels), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = modeIndex;
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmulatedDishwasher::GetModeTagsByIndex(uint8_t modeIndex,
                                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kLabels), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);

    switch (modeIndex)
    {
    case kModeIndexNormal:
        VerifyOrReturnError(modeTags.size() >= 1, CHIP_ERROR_INVALID_ARGUMENT);
        modeTags[0].mfgCode.ClearValue();
        modeTags[0].value = to_underlying(Clusters::DishwasherMode::ModeTag::kNormal);
        modeTags.reduce_size(1);
        break;
    case kModeIndexHeavy:
        VerifyOrReturnError(modeTags.size() >= 2, CHIP_ERROR_INVALID_ARGUMENT);
        modeTags[0].mfgCode.ClearValue();
        modeTags[0].value = to_underlying(Clusters::DishwasherMode::ModeTag::kHeavy);
        modeTags[1].mfgCode.ClearValue();
        modeTags[1].value = to_underlying(Clusters::DishwasherMode::ModeTag::kMax);
        modeTags.reduce_size(2);
        break;
    case kModeIndexLight:
        VerifyOrReturnError(modeTags.size() >= 1, CHIP_ERROR_INVALID_ARGUMENT);
        modeTags[0].mfgCode.ClearValue();
        modeTags[0].value = to_underlying(Clusters::DishwasherMode::ModeTag::kLight);
        modeTags.reduce_size(1);
        break;
    default:
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return CHIP_NO_ERROR;
}

void EmulatedDishwasher::HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    if (newMode == kModeValueLight)
    {
        response.status = to_underlying(Clusters::ModeBase::StatusCode::kInvalidInMode);
        response.statusText.SetValue("Invalid in current state"_span);
        return;
    }
    response.status = to_underlying(Clusters::ModeBase::StatusCode::kSuccess);
}

} // namespace chip::app
