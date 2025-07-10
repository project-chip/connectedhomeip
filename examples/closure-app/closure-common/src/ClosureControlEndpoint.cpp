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

#include <ClosureControlEndpoint.h>
#include <ClosureManager.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::DataModel;

using Protocols::InteractionModel::Status;

namespace {

constexpr ElapsedS kDefaultCountdownTime = 30;

enum class ClosureControlTestEventTrigger : uint64_t
{
    // MainState is Error(3) Test Event | Simulate that the device is in error state, add at least one element to the
    // CurrentErrorList attribute
    kMainStateIsError = 0x0104000000000000,

    // MainState is Protected(5) Test Event | Simulate that the device is in protected state
    kMainStateIsProtected = 0x0104000000000001,

    // MainState is Disengaged(6) Test Event | Simulate that the device is in disengaged state
    kMainStateIsDisengaged = 0x0104000000000002,

    // MainState is SetupRequired(7) Test Event | Simulate that the device is in SetupRequired state
    kMainStateIsSetupRequired = 0x0104000000000003,

    // MainState Test clear Event | Returns the device to pre-test status for that test event.
    kClearEvent = 0x0104000000000004,

};

} // namespace

Status ClosureControlDelegate::HandleCalibrateCommand()
{
    return ClosureManager::GetInstance().OnCalibrateCommand();
}

Status ClosureControlDelegate::HandleMoveToCommand(const Optional<TargetPositionEnum> & position, const Optional<bool> & latch,
                                                   const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    return ClosureManager::GetInstance().OnMoveToCommand(position, latch, speed);
}

Status ClosureControlDelegate::HandleStopCommand()
{
    return ClosureManager::GetInstance().OnStopCommand();
}

bool ClosureControlDelegate::IsReadyToMove()
{
    // This function should return true if the closure is ready to move.
    // For now, we will return true.
    return true;
}

bool ClosureControlDelegate::IsManualLatchingNeeded()
{
    // This function should return true if manual latching is needed.
    // For now, we will return false.
    return false;
}

ElapsedS ClosureControlDelegate::GetCalibrationCountdownTime()
{
    // This function should return the calibration countdown time.
    // For now, we will return kDefaultCountdownTime.
    return kDefaultCountdownTime;
}

ElapsedS ClosureControlDelegate::GetMovingCountdownTime()
{
    // This function should return the moving countdown time.
    // For now, we will return kDefaultCountdownTime.
    return kDefaultCountdownTime;
}

ElapsedS ClosureControlDelegate::GetWaitingForMotionCountdownTime()
{
    // This function should return the waiting for motion countdown time.
    // For now, we will return kDefaultCountdownTime.
    return kDefaultCountdownTime;
}

CHIP_ERROR ClosureControlDelegate::HandleEventTrigger(uint64_t eventTrigger)
{
    eventTrigger                           = clearEndpointInEventTrigger(eventTrigger);
    ClosureControlTestEventTrigger trigger = static_cast<ClosureControlTestEventTrigger>(eventTrigger);
    ClusterLogic * logic                   = GetLogic();

    switch (trigger)
    {
    case ClosureControlTestEventTrigger::kMainStateIsSetupRequired:
        ReturnErrorOnFailure(logic->SetMainState(MainStateEnum::kSetupRequired));
        break;
    case ClosureControlTestEventTrigger::kMainStateIsProtected:
        ReturnErrorOnFailure(logic->SetMainState(MainStateEnum::kProtected));
        break;
    case ClosureControlTestEventTrigger::kMainStateIsError:
        ReturnErrorOnFailure(logic->SetMainState(MainStateEnum::kError));
        ReturnErrorOnFailure(logic->AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor));
        break;
    case ClosureControlTestEventTrigger::kMainStateIsDisengaged:
        ReturnErrorOnFailure(logic->SetMainState(MainStateEnum::kDisengaged));
        break;
    case ClosureControlTestEventTrigger::kClearEvent:
        ReturnErrorOnFailure(logic->SetMainState(MainStateEnum::kStopped));
        logic->ClearCurrentErrorList();
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlEndpoint::Init()
{
    ClusterConformance conformance;
    conformance.FeatureMap()
        .Set(Feature::kPositioning)
        .Set(Feature::kMotionLatching)
        .Set(Feature::kSpeed)
        .Set(Feature::kVentilation)
        .Set(Feature::kPedestrian)
        .Set(Feature::kCalibration)
        .Set(Feature::kProtection)
        .Set(Feature::kManuallyOperable);
    conformance.OptionalAttributes().Set(OptionalAttributeEnum::kCountdownTime);

    ClusterInitParameters initParams;

    ReturnErrorOnFailure(mLogic.Init(conformance, initParams));
    ReturnErrorOnFailure(mInterface.Init());

    return CHIP_NO_ERROR;
}

void ClosureControlEndpoint::OnStopCalibrateActionComplete()
{
    VerifyOrReturn(mLogic.SetMainState(MainStateEnum::kStopped) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to set main state in OnStopCalibrateActionComplete"));

    // After stopping calibration, the overall and target state is explicitly nulled to indicate an unknown state,
    VerifyOrReturn(mLogic.SetOverallCurrentState(DataModel::NullNullable) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to set overall state to null in OnStopCalibrateActionComplete"));
    VerifyOrReturn(mLogic.SetOverallTargetState(DataModel::NullNullable) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to set overall target to null in OnStopCalibrateActionComplete"));
    VerifyOrReturn(mLogic.SetCountdownTimeFromDelegate(0) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to set countdown time to 0 in OnStopCalibrateActionComplete"));
    VerifyOrReturn(mLogic.GenerateMovementCompletedEvent() == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to generate movement completed event in OnStopCalibrateActionComplete"));
}

void ClosureControlEndpoint::OnStopMotionActionComplete()
{
    MainStateEnum presentMainState;
    VerifyOrReturn(mLogic.GetMainState(presentMainState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get main state in OnStopMotionActionComplete"));

    // If the current main state is WaitingForMotion, it means the device hasn't started moving yet,
    // so we don't need to update the current state.
    if (presentMainState != MainStateEnum::kWaitingForMotion)
    {
        // Set the OverallState position to PartiallyOpened as motion has been stopped
        // and the closure is not fully closed or fully opened.
        auto position = MakeOptional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened));

        DataModel::Nullable<GenericOverallCurrentState> overallCurrentState;
        VerifyOrReturn(mLogic.GetOverallCurrentState(overallCurrentState) == CHIP_NO_ERROR,
                       ChipLogError(AppServer, "Failed to get overall state in OnStopMotionActionComplete"));

        if (overallCurrentState.IsNull())
        {
            overallCurrentState.SetNonNull(GenericOverallCurrentState(position, NullOptional, NullOptional, NullOptional));
        }
        else
        {
            overallCurrentState.Value().position = position;
        }

        VerifyOrReturn(mLogic.SetOverallCurrentState(overallCurrentState) == CHIP_NO_ERROR,
                       ChipLogError(AppServer, "Failed to set overall state in OnStopMotionActionComplete"));
    }

    VerifyOrReturn(mLogic.SetMainState(MainStateEnum::kStopped) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to set main state in OnStopMotionActionComplete"));

    // Set the Position, latch in OverallTargetState to Null and speed to Auto as the motion has been stopped.
    GenericOverallTargetState overallTargetState(MakeOptional(DataModel::NullNullable), MakeOptional(DataModel::NullNullable),
                                                 MakeOptional(Globals::ThreeLevelAutoEnum::kAuto));
    VerifyOrReturn(mLogic.SetOverallTargetState(DataModel::MakeNullable(overallTargetState)) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to set overall target in OnStopMotionActionComplete"));

    VerifyOrReturn(mLogic.SetCountdownTimeFromDelegate(0) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to set countdown time to 0 in OnStopMotionActionComplete"));
    VerifyOrReturn(mLogic.GenerateMovementCompletedEvent() == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to generate movement completed event in OnStopMotionActionComplete"));
}

void ClosureControlEndpoint::OnCalibrateActionComplete()
{
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(GenericOverallCurrentState(
        MakeOptional(DataModel::MakeNullable(CurrentPositionEnum::kFullyClosed)), MakeOptional(DataModel::MakeNullable(true)),
        MakeOptional(Globals::ThreeLevelAutoEnum::kAuto), MakeOptional(DataModel::MakeNullable(true))));
    DataModel::Nullable<GenericOverallTargetState> overallTargetState = DataModel::NullNullable;

    mLogic.SetMainState(MainStateEnum::kStopped);
    mLogic.SetOverallCurrentState(overallCurrentState);
    mLogic.SetOverallTargetState(overallTargetState);
    mLogic.SetCountdownTimeFromDelegate(0);
    mLogic.GenerateMovementCompletedEvent();
}

void ClosureControlEndpoint::OnMoveToActionComplete()
{
    UpdateCurrentStateFromTargetState();
    mLogic.SetMainState(MainStateEnum::kStopped);
    mLogic.SetCountdownTimeFromDelegate(0);
    mLogic.GenerateMovementCompletedEvent();
}

void ClosureControlEndpoint::UpdateCurrentStateFromTargetState()
{
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState;
    DataModel::Nullable<GenericOverallTargetState> overallTargetState;

    VerifyOrReturn(mLogic.GetOverallCurrentState(overallCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get overall state from closure Endpoint"));
    VerifyOrReturn(mLogic.GetOverallTargetState(overallTargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get overall target from closure Endpoint"));

    VerifyOrReturn(!overallTargetState.IsNull(), ChipLogError(AppServer, "Current overall target is null, Move to action Failed"));
    VerifyOrReturn(!overallCurrentState.IsNull(), ChipLogError(AppServer, "Current overall state is null, Move to action Failed"));

    if (overallTargetState.Value().position.HasValue() && !overallTargetState.Value().position.Value().IsNull())
    {
        // Map the target position to the current positioning enum.
        CurrentPositionEnum currentPositioning =
            MapTargetPositionToCurrentPositioning(overallTargetState.Value().position.Value().Value());
        overallCurrentState.Value().position.SetValue(MakeNullable(currentPositioning));
    }

    if (overallTargetState.Value().latch.HasValue() && !overallTargetState.Value().latch.Value().IsNull())
    {
        overallCurrentState.Value().latch.SetValue(MakeNullable(overallTargetState.Value().latch.Value().Value()));
    }

    if (overallTargetState.Value().speed.HasValue())
    {
        overallCurrentState.Value().speed.SetValue(overallTargetState.Value().speed.Value());
    }

    bool isClosureInSecureState = true;

    // First, check if the closure is fully closed and has positioning feature.
    if (mLogic.GetConformance().FeatureMap().Has(Feature::kPositioning))
    {
        isClosureInSecureState &= overallCurrentState.Value().position.HasValue() &&
            !overallCurrentState.Value().position.Value().IsNull() &&
            overallCurrentState.Value().position.Value().Value() == CurrentPositionEnum::kFullyClosed;
    }

    // Next, check if motion latching is enabled and latch is true.
    if (mLogic.GetConformance().FeatureMap().Has(Feature::kMotionLatching))
    {
        isClosureInSecureState &= overallCurrentState.Value().latch.HasValue() &&
            !overallCurrentState.Value().latch.Value().IsNull() && overallCurrentState.Value().latch.Value().Value() == true;
    }

    overallCurrentState.Value().secureState.SetValue(MakeNullable(isClosureInSecureState));

    mLogic.SetOverallCurrentState(overallCurrentState);
}

CurrentPositionEnum ClosureControlEndpoint::MapTargetPositionToCurrentPositioning(TargetPositionEnum value)
{
    switch (value)
    {
    case TargetPositionEnum::kMoveToFullyClosed:
        return CurrentPositionEnum::kFullyClosed;
    case TargetPositionEnum::kMoveToFullyOpen:
        return CurrentPositionEnum::kFullyOpened;
    case TargetPositionEnum::kMoveToPedestrianPosition:
        return CurrentPositionEnum::kOpenedForPedestrian;
    case TargetPositionEnum::kMoveToVentilationPosition:
        return CurrentPositionEnum::kOpenedForVentilation;
    case TargetPositionEnum::kMoveToSignaturePosition:
        return CurrentPositionEnum::kOpenedAtSignature;
    default:
        return CurrentPositionEnum::kUnknownEnumValue;
    }
}

void ClosureControlEndpoint::OnPanelMotionActionComplete()
{
    mLogic.SetMainState(MainStateEnum::kStopped);

    // Set the OverallState position to PartiallyOpened as motion has been stopped
    auto position = MakeOptional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened));

    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState;
    DataModel::Nullable<GenericOverallTargetState> overallTargetState;

    VerifyOrReturn(mLogic.GetOverallCurrentState(overallCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get OverallCurrentState"));
    VerifyOrReturn(mLogic.GetOverallTargetState(overallTargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get OverallTargetState"));

    if (overallCurrentState.IsNull())
    {
        overallCurrentState.SetNonNull(GenericOverallCurrentState(position, NullOptional, NullOptional, NullOptional));
    }
    else
    {
        overallCurrentState.Value().position = position;
    }

    // Set latch and speed to their target values if they are set in the overall target.
    if (!overallTargetState.IsNull())
    {
        if (overallTargetState.Value().latch.HasValue() && !overallTargetState.Value().latch.Value().IsNull())
        {
            overallCurrentState.Value().latch.SetValue(DataModel::MakeNullable(overallTargetState.Value().latch.Value().Value()));
        }

        if (overallTargetState.Value().speed.HasValue())
        {
            // If the target speed was Auto, we set it to Auto.
            overallCurrentState.Value().speed.SetValue(overallTargetState.Value().speed.Value());
        }
    }
    mLogic.SetOverallCurrentState(overallCurrentState);

    mLogic.SetCountdownTimeFromDelegate(0);
    mLogic.GenerateMovementCompletedEvent();
}
