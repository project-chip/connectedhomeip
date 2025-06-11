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

using Protocols::InteractionModel::Status;

namespace {
constexpr ElapsedS kDefaultCountdownTime = 30;
} // namespace

Status ClosureControlDelegate::HandleCalibrateCommand()
{
    ChipLogProgress(AppServer, "HandleCalibrateCommand");
    return ClosureManager::GetInstance().OnCalibrateCommand();
}

Status ClosureControlDelegate::HandleMoveToCommand(const Optional<TargetPositionEnum> & position, const Optional<bool> & latch,
                                              const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleMoveToCommand");
    return ClosureManager::GetInstance().OnMoveToCommand(position, latch, speed);
}

Status ClosureControlDelegate::HandleStopCommand()
{
    ChipLogProgress(AppServer, "HandleStopCommand");
    return ClosureManager::GetInstance().OnStopCommand();
}

CHIP_ERROR ClosureControlDelegate::GetCurrentErrorAtIndex(size_t index, ClosureErrorEnum & closureError)
{
    // This function should return the current error at the specified index.
    // For now, we dont have a ErrorList implemented, so will return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED.
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
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

void ClosureControlEndpoint::OnClosureActionComplete(uint8_t action) 
{
    ChipLogError(AppServer, "#######In OnActionComplete############");
    ClosureManager::Action_t closureAction = static_cast<ClosureManager::Action_t>(action);

    switch (closureAction)
    {
    case ClosureManager::Action_t::STOP_MOTION_ACTION:
        OnStopMotionActionComplete();
        break;
    case ClosureManager::Action_t::STOP_CALIBRATE_ACTION:
        OnStopCalibrateActionComplete();
        break;
    case ClosureManager::Action_t::CALIBRATE_ACTION:
        OnCalibrateActionComplete();
        break;
    case ClosureManager::Action_t::MOVE_TO_ACTION:
        OnMoveToActionComplete();
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in OnActionComplete");
    }
}

void ClosureControlEndpoint::OnStopCalibrateActionComplete()
{
    // This function should handle closure control state updation after stopping of calibration Action.
}

void ClosureControlEndpoint::OnStopMotionActionComplete()
{
    // This function should handle closure control state updation after stopping of Motion Action.
}

void ClosureControlEndpoint::OnCalibrateActionComplete()
{
    ChipLogError(AppServer, "#######In CALIBRATE_ACTION ############");

    DataModel::Nullable<GenericOverallState> overallState(
    GenericOverallState(MakeOptional(DataModel::MakeNullable(PositioningEnum::kFullyClosed)),
                        MakeOptional(DataModel::MakeNullable(true)),
                        MakeOptional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kAuto)),
                        MakeOptional(DataModel::MakeNullable(true))));
    DataModel::Nullable<GenericOverallTarget> overallTarget = DataModel::NullNullable;

    mLogic.SetMainState(MainStateEnum::kStopped);
    mLogic.SetOverallState(overallState);
    mLogic.SetOverallTarget(overallTarget);
    mLogic.SetCountdownTimeFromDelegate(0);
    mLogic.GenerateMovementCompletedEvent();

    ChipLogError(AppServer, "####### CALIBRATE_ACTION done ############");
}

void ClosureControlEndpoint::OnMoveToActionComplete()
{
    // This function should handle closure control state updation after scompletion of Motion Action.   
}
