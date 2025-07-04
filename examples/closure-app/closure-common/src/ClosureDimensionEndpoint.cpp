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

#include <ClosureDimensionEndpoint.h>
#include <ClosureManager.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app::Clusters::ClosureDimension;

using Protocols::InteractionModel::Status;

namespace {
constexpr Percent100ths kFullClosedTargetPosition = 10000; // Default target position in 100ths of a percent
} // namespace

Status ClosureDimensionDelegate::HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<bool> & latch,
                                                 const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleSetTarget");
    // Add the SetTarget handling logic here
    return ClosureManager::GetInstance().OnSetTargetCommand(pos, latch, speed, GetEndpoint());
}

Status ClosureDimensionDelegate::HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                                            const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleStep");
    SetStepCommandTargetDirection(direction);
    return ClosureManager::GetInstance().OnStepCommand(direction, numberOfSteps, speed, GetEndpoint());
}

CHIP_ERROR ClosureDimensionEndpoint::Init()
{
    ClusterConformance conformance;
    conformance.FeatureMap()
        .Set(Feature::kPositioning)
        .Set(Feature::kMotionLatching)
        .Set(Feature::kUnit)
        .Set(Feature::kLimitation)
        .Set(Feature::kSpeed)
        .Set(Feature::kRotation);
    conformance.OptionalAttributes().Set(OptionalAttributeEnum::kOverflow);

    ClusterInitParameters clusterInitParameters;
    clusterInitParameters.translationDirection = TranslationDirectionEnum::kDownward;
    clusterInitParameters.rotationAxis         = RotationAxisEnum::kCenteredVertical;
    clusterInitParameters.modulationType       = ModulationTypeEnum::kVentilation;

    ReturnErrorOnFailure(mLogic.Init(conformance, clusterInitParameters));
    ReturnErrorOnFailure(mInterface.Init());
    return CHIP_NO_ERROR;
}

void ClosureDimensionEndpoint::OnStopMotionActionComplete()
{
    // Set the Position, latch in OverallTargetState to Null and speed to Auto as the motion has been stopped.
    GenericDimensionStateStruct targetState =
        GenericDimensionStateStruct(NullOptional, NullOptional, MakeOptional(Globals::ThreeLevelAutoEnum::kAuto));
    VerifyOrReturn(mLogic.SetTargetState(DataModel::MakeNullable(targetState)) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to set target in OnStopMotionActionComplete"));
}

void ClosureDimensionEndpoint::OnStopCalibrateActionComplete()
{
    // Current state and target are set to null after calibration is stopped to indicate an unknown state.
    VerifyOrReturn(mLogic.SetCurrentState(DataModel::NullNullable) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to set current state to null in OnStopCalibrateActionComplete"));
    VerifyOrReturn(mLogic.SetTargetState(DataModel::NullNullable) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to set target to null in OnStopCalibrateActionComplete"));
}

void ClosureDimensionEndpoint::OnCalibrateActionComplete()
{
    DataModel::Nullable<GenericDimensionStateStruct> currentState(
        GenericDimensionStateStruct(MakeOptional(DataModel::MakeNullable(kFullClosedTargetPosition)),
                                    MakeOptional(DataModel::MakeNullable(true)), MakeOptional(Globals::ThreeLevelAutoEnum::kAuto)));
    DataModel::Nullable<GenericDimensionStateStruct> targetState{ DataModel::NullNullable };
    mLogic.SetCurrentState(currentState);
    mLogic.SetTargetState(targetState);
}

void ClosureDimensionEndpoint::OnMoveToActionComplete()
{
    UpdateCurrentStateFromTargetState();
}

void ClosureDimensionEndpoint::UpdateCurrentStateFromTargetState()
{
    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    DataModel::Nullable<GenericDimensionStateStruct> targetState;

    VerifyOrReturn(mLogic.GetCurrentState(currentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state, Updating CurrentState From TargetState Failed"));
    VerifyOrReturn(mLogic.GetTargetState(targetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state, Updating CurrentState From TargetState Failed"));

    VerifyOrReturn(!targetState.IsNull(),
                   ChipLogError(AppServer, "Target state is null, Updating CurrentState From TargetState Failed"));
    VerifyOrReturn(!currentState.IsNull(),
                   ChipLogError(AppServer, "Current state is null, Updating CurrentState From TargetState Failed"));

    if (targetState.Value().position.HasValue() && !targetState.Value().position.Value().IsNull())
    {
        currentState.Value().position.SetValue(DataModel::MakeNullable(targetState.Value().position.Value().Value()));
    }

    if (targetState.Value().latch.HasValue() && !targetState.Value().latch.Value().IsNull())
    {
        currentState.Value().latch.SetValue(DataModel::MakeNullable(targetState.Value().latch.Value().Value()));
    }

    if (targetState.Value().speed.HasValue())
    {
        currentState.Value().speed.SetValue(targetState.Value().speed.Value());
    }

    mLogic.SetCurrentState(currentState);
}

void ClosureDimensionEndpoint::OnPanelMotionActionComplete()
{
    UpdateCurrentStateFromTargetState();
}
