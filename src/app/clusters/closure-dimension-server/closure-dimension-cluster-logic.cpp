/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
/**
 * @file Cross-platform API to handle cluster-specific logic for the closure dimension cluster on a single endpoint.
 */

#include "closure-dimension-cluster-logic.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

using namespace Protocols::InteractionModel;

namespace {

static constexpr Percent100ths PERCENT100THS_MAX_VALUE = 10000;

} // namespace

CHIP_ERROR ClusterLogic::Init(const ClusterConformance & conformance)
{
    // TODO: Add logic to inital State form device or set it to fallback values
    VerifyOrReturnError(!mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(conformance.Valid(), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
    mConformance = conformance;
    mInitialized = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetCurrentState(const GenericCurrentStateStruct & acurrentState)
{
    // TODO : Q reporting for this attribute
    // TODO: To implement the impact of the MoveTo command from the Closure Control cluster

    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    GenericCurrentStateStruct currentState = acurrentState;

    if (currentState.position.HasValue())
    {
        // If Positioning feature is supported,CurrentState position SHALL be available else position SHALL NOT be present.
        if (mConformance.HasFeature(Feature::kPositioning))
        {
            VerifyOrReturnError(currentState.position.Value() <= PERCENT100THS_MAX_VALUE, CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            ChipLogDetail(Zcl,
                          "Disregarding the position value in the current state since the Positioning feature is not enabled.");
            currentState.position.ClearValue();
        }
    }

    if (currentState.latching.HasValue())
    {
        // If MotionLatching feature is supported,CurrentState latching SHALL be available else latching SHALL NOT be present
        if (mConformance.HasFeature(Feature::kMotionLatching))
        {
            VerifyOrReturnError(EnsureKnownEnumValue(currentState.latching.Value()) != LatchingEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            ChipLogDetail(Zcl,
                          "Disregarding the latch value in the current state since the MotionLatching feature is not enabled.");
            currentState.latching.ClearValue();
        }
    }

    if (currentState.speed.HasValue())
    {
        // If Speed feature is supported,CurrentState speed SHALL be available else speed SHALL NOT be present
        if (mConformance.HasFeature(Feature::kSpeed))
        {
            VerifyOrReturnError(EnsureKnownEnumValue(currentState.speed.Value()) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            ChipLogDetail(Zcl, "Disregarding the speed value in the current state since the Speed feature is not enabled.");
            currentState.speed.ClearValue();
        }
    }

    // TODO: currentState.Position value SHALL follow the scaling from "Resolution Attribute".

    if (currentState != mState.currentState)
    {
        mState.currentState = currentState;
        mMatterContext.MarkDirty(Attributes::Current::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetTarget(const GenericTargetStruct & atarget)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    GenericTargetStruct target = atarget;

    if (target.position.HasValue())
    {
        // If Positioning feature is supported,Target position SHALL be available else position SHALL NOT be present
        if (mConformance.HasFeature(Feature::kPositioning))
        {
            VerifyOrReturnError(target.position.Value() <= PERCENT100THS_MAX_VALUE, CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            ChipLogDetail(Zcl, "Disregarding the position value in the target since the Positioning feature is not enabled.");
            target.position.ClearValue();
        }
    }

    // If MotionLatching feature is supported,Target latch SHALL be available else latch SHALL NOT be present
    if (target.latch.HasValue())
    {
        if (mConformance.HasFeature(Feature::kMotionLatching))
        {
            VerifyOrReturnError(EnsureKnownEnumValue(target.latch.Value()) != TargetLatchEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            ChipLogDetail(Zcl, "Disregarding the latch value in the target since the MotionLatching feature is not enabled.");
            target.latch.ClearValue();
        }
    }

    if (target.speed.HasValue())
    {
        // If Speed feature is supported,CurrentState speed SHALL be available else speed SHALL NOT be present
        if (mConformance.HasFeature(Feature::kSpeed))
        {
            VerifyOrReturnError(EnsureKnownEnumValue(target.speed.Value()) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            ChipLogDetail(Zcl, "Disregarding the speed value in the target since the Speed feature is not enabled.");
            target.speed.ClearValue();
        }
    }

    // TODO: Target.Position value SHALL follow the scaling from "Resolution Attribute".
    if (target != mState.target)
    {
        mState.target = target;
        mMatterContext.MarkDirty(Attributes::Target::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetResolution(const Percent100ths resolution)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(resolution <= PERCENT100THS_MAX_VALUE, CHIP_ERROR_INVALID_ARGUMENT);

    if (resolution != mState.resolution)
    {
        mState.resolution = resolution;
        mMatterContext.MarkDirty(Attributes::Resolution::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetStepValue(const Percent100ths stepValue)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(stepValue <= PERCENT100THS_MAX_VALUE, CHIP_ERROR_INVALID_ARGUMENT);

    // TODO: The value of this attribute SHALL be equal to an integer multiple of the Resolution attribute.
    // TODO: The value of this attribute should be large enough to cause a visible change on the closure when the Step command is
    // received.
    if (stepValue != mState.stepValue)
    {
        mState.stepValue = stepValue;
        mMatterContext.MarkDirty(Attributes::StepValue::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetUnit(const ClosureUnitEnum unit)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kUnit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(EnsureKnownEnumValue(unit) != ClosureUnitEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);

    if (unit != mState.unit)
    {
        mState.unit = unit;
        mMatterContext.MarkDirty(Attributes::Unit::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetUnitRange(const DataModel::Nullable<Structs::UnitRangeStruct::Type> & unitRange)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kUnit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (unitRange.IsNull())
    {
        mState.unitRange.SetNull();
        mMatterContext.MarkDirty(Attributes::UnitRange::Id);
        return CHIP_NO_ERROR;
    }

    // If the unit range is invalid, we need to return an error
    VerifyOrReturnError(unitRange.Value().min <= unitRange.Value().max, CHIP_ERROR_INVALID_ARGUMENT);

    // If the unit range is null, we need to set it to the new value
    if (mState.unitRange.IsNull())
    {
        mState.unitRange.SetNonNull(unitRange.Value());
        mMatterContext.MarkDirty(Attributes::UnitRange::Id);
        return CHIP_NO_ERROR;
    }

    // If the unit range is not null, we need to check if the values are different
    if ((unitRange.Value().min != mState.unitRange.Value().min) || (unitRange.Value().max != mState.unitRange.Value().max))
    {
        mState.unitRange.Value().min = unitRange.Value().min;
        mState.unitRange.Value().max = unitRange.Value().max;
        mMatterContext.MarkDirty(Attributes::UnitRange::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetLimitRange(const Structs::RangePercent100thsStruct::Type & limitRange)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kLimitation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    // If the limit range is invalid, we need to return an error
    VerifyOrReturnError(limitRange.min <= limitRange.max, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(limitRange.min <= PERCENT100THS_MAX_VALUE, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(limitRange.max <= PERCENT100THS_MAX_VALUE, CHIP_ERROR_INVALID_ARGUMENT);

    if ((limitRange.min != mState.limitRange.min) || (limitRange.max != mState.limitRange.max))
    {
        mState.limitRange.min = limitRange.min;
        mState.limitRange.max = limitRange.max;
        mMatterContext.MarkDirty(Attributes::LimitRange::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetTranslationDirection(const TranslationDirectionEnum translationDirection)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kTranslation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(EnsureKnownEnumValue(translationDirection) != TranslationDirectionEnum::kUnknownEnumValue,
                        CHIP_ERROR_INVALID_ARGUMENT);
    // TODO:  This attribute is not supposed to change once the installation is finalized.

    if (translationDirection != mState.translationDirection)
    {
        mState.translationDirection = translationDirection;
        mMatterContext.MarkDirty(Attributes::TranslationDirection::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetRotationAxis(const RotationAxisEnum rotationAxis)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kRotation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(EnsureKnownEnumValue(rotationAxis) != RotationAxisEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);

    // TODO:  This attribute is not supposed to change once the installation is finalized.
    if (rotationAxis != mState.rotationAxis)
    {
        mState.rotationAxis = rotationAxis;
        mMatterContext.MarkDirty(Attributes::RotationAxis::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetOverflow(const OverflowEnum overflow)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError((mConformance.HasFeature(Feature::kRotation) || mConformance.HasFeature(Feature::kMotionLatching)),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(mConformance.supportsOverflow, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(EnsureKnownEnumValue(overflow) != OverflowEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);

    // TODO:  If the axis is centered, one part goes Outside and the other part goes Inside. In this case, this attribute SHALL use
    // Top/Bottom/Left/Right Inside or Top/Bottom/Left/Right Outside enumerated value.

    if (overflow != mState.overflow)
    {
        mState.overflow = overflow;
        mMatterContext.MarkDirty(Attributes::Overflow::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetModulationType(const ModulationTypeEnum modulationType)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kModulation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(EnsureKnownEnumValue(modulationType) != ModulationTypeEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);

    // TODO:  This attribute is not supposed to change once the installation is finalized.

    if (modulationType != mState.modulationType)
    {
        mState.modulationType = modulationType;
        mMatterContext.MarkDirty(Attributes::ModulationType::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetCurrentState(GenericCurrentStateStruct & currentState)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    currentState = mState.currentState;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetTarget(GenericTargetStruct & target)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    target = mState.target;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetResolution(Percent100ths & resolution)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    resolution = mState.resolution;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetStepValue(Percent100ths & stepValue)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    stepValue = mState.stepValue;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetUnit(ClosureUnitEnum & unit)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kUnit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    unit = mState.unit;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetUnitRange(DataModel::Nullable<Structs::UnitRangeStruct::Type> & unitRange)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kUnit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    unitRange = mState.unitRange;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetLimitRange(Structs::RangePercent100thsStruct::Type & limitRange)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kLimitation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    limitRange = mState.limitRange;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetTranslationDirection(TranslationDirectionEnum & translationDirection)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kTranslation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    translationDirection = mState.translationDirection;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetRotationAxis(RotationAxisEnum & rotationAxis)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kRotation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    rotationAxis = mState.rotationAxis;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetOverflow(OverflowEnum & overflow)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError((mConformance.HasFeature(Feature::kRotation) || mConformance.HasFeature(Feature::kMotionLatching)),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(mConformance.supportsOverflow, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    overflow = mState.overflow;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetModulationType(ModulationTypeEnum & modulationType)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kModulation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    modulationType = mState.modulationType;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetFeatureMap(Attributes::FeatureMap::TypeInfo::Type & featureMap)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    featureMap = mConformance.featureMap;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetClusterRevision(Attributes::ClusterRevision::TypeInfo::Type & clusterRevision)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    clusterRevision = kClusterRevision;
    return CHIP_NO_ERROR;
}

Status ClusterLogic::HandleSetTargetCommand(Optional<Percent100ths> position, Optional<TargetLatchEnum> latch,
                                            Optional<Globals::ThreeLevelAutoEnum> speed)
{
    VerifyOrDieWithMsg(mInitialized, NotSpecified, "Unexpected command recieved when device is yet to be initialized");
    // TODO: If this command is sent while the device is in a non-compatible internal-state, a status code of INVALID_IN_STATE SHALL
    // be returned.

    GenericCurrentStateStruct currentState;
    VerifyOrReturnError(GetCurrentState(currentState) == CHIP_NO_ERROR, Status::Failure);
    // Check if the current position is valid or else return InvalidInState
    VerifyOrReturnValue(currentState.position.HasValue(), Status::InvalidInState);

    GenericTargetStruct target;
    VerifyOrReturnError(GetTarget(target) == CHIP_NO_ERROR, Status::Failure);

    VerifyOrReturnError(position.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    if (position.HasValue())
    {
        VerifyOrReturnError((position.Value() <= PERCENT100THS_MAX_VALUE), Status::ConstraintError);
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), Status::Success);
        // TODO: not an integer multiple of the Resolution attribute, then Target.Position is updated to the closest integer
        // multiple of the Resolution attribute according to the server, and a status code of SUCCESS is returned.
        // TODO: supported then the device (depending of its current state) start or change the course of its motion procedure and
        // update Target.Position.
        target.position = position;
    }

    if (latch.HasValue())
    {
        VerifyOrReturnError(latch.Value() != TargetLatchEnum::kUnknownEnumValue, Status::ConstraintError);
        VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), Status::Success);
        // TODO: If the device supports the Latching(LT) feature, the device dimension SHALL either fulfill the latch order and
        // update Target.Latch or, if manual intervention is required to latch, respond with INVALID_ACTION and remain in its
        // current state.
        target.latch = latch;
    }

    if (speed.HasValue())
    {
        VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), Status::Success);
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
        // TODO: If Speed(SP) feature is enabled then Target.Speed is updated with the Speed Field.
        // TODO: In case a SetTarget command is sent with only the Speed field then if: It SHOULD affect the current dimension
        // motion speed of the device (this change speed on the fly).
        target.speed = speed;
    }

    VerifyOrReturnError(SetTarget(target) == CHIP_NO_ERROR, Status::Failure);
    // TODO: CHIP_ERROR err = mClusterDriver.HandleSetTarget(target.position, target.latch, target.speed);
    return Status::Success;
}

Status ClusterLogic::HandleStepCommand(StepDirectionEnum direction, uint16_t numberOfSteps,
                                       Optional<Globals::ThreeLevelAutoEnum> speed)
{
    VerifyOrDieWithMsg(mInitialized, NotSpecified, "Unexpected command recieved when device is yet to be initialized");

    // Return UnsupportedCommand if Positioning feature is not supported.
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), Status::UnsupportedCommand);

    // Return ConstraintError if command parameters are out of bounds
    VerifyOrReturnError(direction != StepDirectionEnum::kUnknownEnumValue, Status::ConstraintError);
    VerifyOrReturnError(numberOfSteps > 0, Status::ConstraintError);
    if (speed.HasValue())
    {
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
    }

    // TODO: If the server is in a state where it cannot support the command, the server SHALL respond with an INVALID_IN_STATE
    // response and the Target attribute value SHALL remain unchanged.
    // TODO: , this SHALL update the Target.Position attribute value e.g. by sending multiple commands with short step by step or a
    // single command with multiple steps.

    GenericCurrentStateStruct currentState;
    VerifyOrReturnError(GetCurrentState(currentState) == CHIP_NO_ERROR, Status::Failure);
    // Check if the current position is valid or else return InvalidInState
    VerifyOrReturnValue(currentState.position.HasValue(), Status::InvalidInState);

    GenericTargetStruct stepTarget;
    VerifyOrReturnError(GetTarget(stepTarget) == CHIP_NO_ERROR, Status::Failure);

    Percent100ths stepValue;
    VerifyOrReturnError(GetStepValue(stepValue) == CHIP_NO_ERROR, Status::Failure);

    // Derive Target Position from StepValue and NumberOfSteps.
    int32_t currentPosition = static_cast<uint32_t>(currentState.position.Value());

    // Convert step to position delta
    int32_t delta       = numberOfSteps * stepValue;
    int32_t newPosition = 0;

    // check if device supports Limitation feature, if yes fetch the LimitRange values
    bool limitSupported = mConformance.HasFeature(Feature::kLimitation) ? true : false;

    Structs::RangePercent100thsStruct::Type limitRange;
    if (limitSupported)
    {
        VerifyOrReturnError(GetLimitRange(limitRange) == CHIP_NO_ERROR, Status::Failure);
    }

    // Position = Position - NumberOfSteps * StepValue
    switch (direction)
    {

    case StepDirectionEnum::kDecrease:
        newPosition = currentPosition - delta;
        // Position value SHALL be clamped to 0.00% if the LM feature is not supported or LimitRange.Min if the LM feature is
        // supported.
        newPosition =
            limitSupported ? std::max(newPosition, static_cast<int32_t>(limitRange.min)) : std::max(newPosition, (int32_t) 0);
        break;

    case StepDirectionEnum::kIncrease:
        newPosition = currentPosition + delta;
        // Position value SHALL be clamped to 0.00% if the LM feature is not supported or LimitRange.Max if the LM feature is
        // supported.
        newPosition =
            limitSupported ? std::min(newPosition, static_cast<int32_t>(limitRange.max)) : std::min(newPosition, (int32_t) 10000);
        break;

    default:
        // Should never reach here due to earlier VerifyOrReturnError check
        ChipLogError(AppServer, "Unhandled StepDirectionEnum value");
        return Status::ConstraintError;
    }

    // set the target position
    stepTarget.position.SetValue(static_cast<Percent100ths>(newPosition));

    if (speed.HasValue())
    {
        stepTarget.speed = speed;
    }

    VerifyOrReturnError(SetTarget(stepTarget) == CHIP_NO_ERROR, Status::Failure);
    // TODO: CHIP_ERROR err = mClusterDriver.HandleStep(direction,numberOfSteps,speed);
    return Status::Success;
}
} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
