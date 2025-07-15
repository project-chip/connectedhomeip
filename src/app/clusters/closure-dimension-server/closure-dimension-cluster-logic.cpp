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
#include <clusters/ClosureDimension/Metadata.h>
#include <platform/LockTracker.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

using namespace Protocols::InteractionModel;
using namespace chip::app::Clusters::ClosureDimension::Attributes;

namespace {

constexpr Percent100ths kPercents100thsMaxValue    = 10000;
constexpr uint64_t kPositionQuietReportingInterval = 5000;

} // namespace

CHIP_ERROR ClusterLogic::Init(const ClusterConformance & conformance, const ClusterInitParameters & clusterInitParameters)
{
    VerifyOrReturnError(!mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(conformance.Valid(), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
    mConformance = conformance;

    // Need to set TranslationDirection, RotationAxis, ModulationType before Initilization of closure, as they should not be changed
    // after Initalization.
    if (conformance.HasFeature(Feature::kTranslation))
    {
        ReturnErrorOnFailure(SetTranslationDirection(clusterInitParameters.translationDirection));
    }

    if (conformance.HasFeature(Feature::kRotation))
    {
        ReturnErrorOnFailure(SetRotationAxis(clusterInitParameters.rotationAxis));
    }

    if (conformance.HasFeature(Feature::kModulation))
    {
        ReturnErrorOnFailure(SetModulationType(clusterInitParameters.modulationType));
    }

    mInitialized = true;
    return CHIP_NO_ERROR;
}

// Specification rules for CurrentState quiet reporting:
// Changes to this attribute SHALL only be marked as reportable in the following cases:
// When the Position changes from null to any other value and vice versa, or
// At most once every 5 seconds when the Position changes from one non-null value to another non-null value, or
// When Target.Position is reached, or
// When CurrentState.Speed changes, or
// When CurrentState.Latch changes.

// At present, QuieterReportingAttribute class does not support Structs.
//  so each field of current state struct has to be handled independently.
//  At present, we are using QuieterReportingAttribute class for Position only.
//  Latch and Speed changes are directly handled by the cluster logic seperately.
//  i.e Speed and latch changes are not considered when calucalting the at most 5 seconds quiet reportable changes for Position.
CHIP_ERROR ClusterLogic::SetCurrentState(const DataModel::Nullable<GenericDimensionStateStruct> & incomingCurrentState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mState.currentState != incomingCurrentState, CHIP_NO_ERROR);

    bool markDirty = false;

    if (!incomingCurrentState.IsNull())
    {
        // Validate the incoming Position value has valid input parameters and FeatureMap conformance.
        if (incomingCurrentState.Value().position.HasValue())
        {
            //  If the position member is present in the incoming CurrentState, we need to check if the Positioning
            //  feature is supported by the closure. If the Positioning feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            if (!incomingCurrentState.Value().position.Value().IsNull())
            {

                VerifyOrReturnError(incomingCurrentState.Value().position.Value().Value() <= kPercents100thsMaxValue,
                                    CHIP_ERROR_INVALID_ARGUMENT);
            }

            bool targetPositionReached = false;
            auto now                   = System::SystemClock().GetMonotonicTimestamp();

            // Logic to determine if target position is reached.
            // If the target position is reached, current state attribute will be marked dirty and reported.
            if (!mState.targetState.IsNull() && mState.targetState.Value().position.HasValue() &&
                !mState.targetState.Value().position.Value().IsNull() &&
                mState.targetState.Value().position == incomingCurrentState.Value().position)
            {
                targetPositionReached = true;
            }

            if (targetPositionReached)
            {
                auto predicate =
                    [](const decltype(quietReportableCurrentStatePosition)::SufficientChangePredicateCandidate &) -> bool {
                    return true;
                };
                markDirty |= (quietReportableCurrentStatePosition.SetValue(incomingCurrentState.Value().position.Value(), now,
                                                                           predicate) == AttributeDirtyState::kMustReport);
            }
            else
            {
                // Predicate to report at most once every 5 seconds when the Position changes from one non-null value to another
                // non-null value, or when the Position changes from null to any other value and vice versa
                System::Clock::Milliseconds64 reportInterval = System::Clock::Milliseconds64(kPositionQuietReportingInterval);
                auto predicate = quietReportableCurrentStatePosition.GetPredicateForSufficientTimeSinceLastDirty(reportInterval);
                markDirty |= (quietReportableCurrentStatePosition.SetValue(incomingCurrentState.Value().position.Value(), now,
                                                                           predicate) == AttributeDirtyState::kMustReport);
            }
        }

        // Validate the incoming latch value has valid FeatureMap conformance.
        if (incomingCurrentState.Value().latch.HasValue())
        {
            //  If the latching member is present in the incoming CurrentState, we need to check if the MotionLatching
            //  feature is supported by the closure. If the MotionLatching feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        }

        // Changes to this attribute SHALL only be marked as reportable when latch changes.
        if (!mState.currentState.IsNull() && mState.currentState.Value().latch != incomingCurrentState.Value().latch)
        {
            markDirty = true;
        }

        // Validate the incoming Speed value has valid input parameters and FeatureMap conformance.
        if (incomingCurrentState.Value().speed.HasValue())
        {
            //  If the speed member is present in the incoming CurrentState, we need to check if the Speed feature is
            //  supported by the closure. If the Speed feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            VerifyOrReturnError(EnsureKnownEnumValue(incomingCurrentState.Value().speed.Value()) !=
                                    Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }

        // Changes to this attribute SHALL be marked as reportable when speed changes.
        if (!mState.currentState.IsNull() && mState.currentState.Value().speed != incomingCurrentState.Value().speed)
        {
            markDirty = true;
        }
    }

    // If the current state is null and the incoming current state is not null and vice versa, we need to mark dirty.
    if (mState.currentState.IsNull() != incomingCurrentState.IsNull())
    {
        markDirty = true;
    }

    mState.currentState = incomingCurrentState;

    if (markDirty)
    {
        mMatterContext.MarkDirty(Attributes::CurrentState::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetTargetState(const DataModel::Nullable<GenericDimensionStateStruct> & incomingTargetState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mState.targetState != incomingTargetState, CHIP_NO_ERROR);

    if (!incomingTargetState.IsNull())
    {
        // Validate the incoming Position value has valid input parameters and FeatureMap conformance.
        if (incomingTargetState.Value().position.HasValue())
        {
            //  If the position member is present in the incoming TargetState, we need to check if the Positioning
            //  feature is supported by the closure. If the Positioning feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            if (!incomingTargetState.Value().position.Value().IsNull())
            {
                VerifyOrReturnError(incomingTargetState.Value().position.Value().Value() <= kPercents100thsMaxValue,
                                    CHIP_ERROR_INVALID_ARGUMENT);

                // Incoming TargetState Position value SHALL follow the scaling from Resolution Attribute.
                Percent100ths resolution;
                ReturnErrorOnFailure(GetResolution(resolution));
                VerifyOrReturnError(
                    incomingTargetState.Value().position.Value().Value() % resolution == 0, CHIP_ERROR_INVALID_ARGUMENT,
                    ChipLogError(NotSpecified, "TargetState Position value SHALL follow the scaling from Resolution Attribute"));
            }
        }

        // Validate the incoming latch value has valid FeatureMap conformance.
        if (incomingTargetState.Value().latch.HasValue())
        {
            //  If the latching member is present in the incoming TargetState, we need to check if the MotionLatching
            //  feature is supported by the closure. If the MotionLatching feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        }

        // Validate the incoming Speed value has valid input parameters and FeatureMap conformance.
        if (incomingTargetState.Value().speed.HasValue())
        {
            //  If the speed member is present in the incoming TargetState, we need to check if the Speed feature is
            //  supported by the closure. If the Speed feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            VerifyOrReturnError(EnsureKnownEnumValue(incomingTargetState.Value().speed.Value()) !=
                                    Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    mState.targetState = incomingTargetState;
    mMatterContext.MarkDirty(Attributes::TargetState::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetResolution(const Percent100ths resolution)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(0 < resolution && resolution <= kPercents100thsMaxValue, CHIP_ERROR_INVALID_ARGUMENT);

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
    VerifyOrReturnError(stepValue <= kPercents100thsMaxValue, CHIP_ERROR_INVALID_ARGUMENT);

    // StepValue SHALL be equal to an integer multiple of the Resolution attribute , if not return Invalid Argument.
    Percent100ths resolution;
    ReturnErrorOnFailure(GetResolution(resolution));
    VerifyOrReturnError(stepValue % resolution == 0, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(NotSpecified, "StepValue SHALL be equal to an integer multiple of the Resolution attribute"));

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
        // Mark UnitRange attribute as dirty only if value changes.
        if (!mState.unitRange.IsNull())
        {
            mState.unitRange.SetNull();
            mMatterContext.MarkDirty(Attributes::UnitRange::Id);
        }
        return CHIP_NO_ERROR;
    }

    // Return error if unitRange is invalid
    VerifyOrReturnError(unitRange.Value().min <= unitRange.Value().max, CHIP_ERROR_INVALID_ARGUMENT);

    ClosureUnitEnum unit;
    ReturnErrorOnFailure(GetUnit(unit));

    // If Unit is Millimeter , Range values SHALL contain unsigned values from 0 to 32767 only
    if (unit == ClosureUnitEnum::kMillimeter)
    {
        VerifyOrReturnError(unitRange.Value().min >= 0 && unitRange.Value().min <= 32767, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(unitRange.Value().max >= 0 && unitRange.Value().max <= 32767, CHIP_ERROR_INVALID_ARGUMENT);
    }

    // If Unit is Degrees the maximum span range is 360 degrees.
    if (unit == ClosureUnitEnum::kDegree)
    {
        VerifyOrReturnError(unitRange.Value().min >= -360 && unitRange.Value().min <= 360, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(unitRange.Value().max >= -360 && unitRange.Value().max <= 360, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError((unitRange.Value().max - unitRange.Value().min) <= 360, CHIP_ERROR_INVALID_ARGUMENT);
    }

    // If the mState unitRange is null, we need to set it to the new value
    if (mState.unitRange.IsNull())
    {
        mState.unitRange.SetNonNull(unitRange.Value());
        mMatterContext.MarkDirty(Attributes::UnitRange::Id);
        return CHIP_NO_ERROR;
    }

    // If both the mState unitRange and unitRange are not null, we need to update mState unitRange if the values are different
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
    VerifyOrReturnError(limitRange.min <= kPercents100thsMaxValue, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(limitRange.max <= kPercents100thsMaxValue, CHIP_ERROR_INVALID_ARGUMENT);

    // LimitRange.Min and LimitRange.Max SHALL be equal to an integer multiple of the Resolution attribute.
    Percent100ths resolution;
    ReturnErrorOnFailure(GetResolution(resolution));
    VerifyOrReturnError(
        limitRange.min % resolution == 0, CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(NotSpecified, "LimitRange.Min SHALL be equal to an integer multiple of the Resolution attribute."));
    VerifyOrReturnError(
        limitRange.max % resolution == 0, CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(NotSpecified, "LimitRange.Max SHALL be equal to an integer multiple of the Resolution attribute."));

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
    // This attribute is not supposed to change once the initialization is completed.
    VerifyOrReturnError(!mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kTranslation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(EnsureKnownEnumValue(translationDirection) != TranslationDirectionEnum::kUnknownEnumValue,
                        CHIP_ERROR_INVALID_ARGUMENT);

    if (translationDirection != mState.translationDirection)
    {
        mState.translationDirection = translationDirection;
        mMatterContext.MarkDirty(Attributes::TranslationDirection::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetRotationAxis(const RotationAxisEnum rotationAxis)
{
    // This attribute is not supposed to change once the initialization is completed
    VerifyOrReturnError(!mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kRotation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(EnsureKnownEnumValue(rotationAxis) != RotationAxisEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);

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

    VerifyOrReturnError(mConformance.HasFeature(Feature::kRotation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(EnsureKnownEnumValue(overflow) != OverflowEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);

    RotationAxisEnum rotationAxis;
    ReturnErrorOnFailure(GetRotationAxis(rotationAxis));

    // If the axis is centered, one part goes Outside and the other part goes Inside.
    // In this case, this attribute SHALL use Top/Bottom/Left/Right Inside or Top/Bottom/Left/Right Outside enumerated value.
    if (rotationAxis == RotationAxisEnum::kCenteredHorizontal || rotationAxis == RotationAxisEnum::kCenteredVertical)
    {
        VerifyOrReturnError(overflow != OverflowEnum::kNoOverflow && overflow != OverflowEnum::kInside &&
                                overflow != OverflowEnum::kOutside,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (overflow != mState.overflow)
    {
        mState.overflow = overflow;
        mMatterContext.MarkDirty(Attributes::Overflow::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetModulationType(const ModulationTypeEnum modulationType)
{
    // This attribute is not supposed to change once the initialization is completed
    VerifyOrReturnError(!mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kModulation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(EnsureKnownEnumValue(modulationType) != ModulationTypeEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);

    if (modulationType != mState.modulationType)
    {
        mState.modulationType = modulationType;
        mMatterContext.MarkDirty(Attributes::ModulationType::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetLatchControlModes(const BitFlags<LatchControlModesBitmap> & latchControlModes)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (mState.latchControlModes != latchControlModes)
    {
        mState.latchControlModes = latchControlModes;
        mMatterContext.MarkDirty(Attributes::LatchControlModes::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetCurrentState(DataModel::Nullable<GenericDimensionStateStruct> & currentState)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    currentState = mState.currentState;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetTargetState(DataModel::Nullable<GenericDimensionStateStruct> & targetState)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    targetState = mState.targetState;
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
    VerifyOrReturnError((mConformance.HasFeature(Feature::kRotation)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
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

CHIP_ERROR ClusterLogic::GetLatchControlModes(BitFlags<LatchControlModesBitmap> & latchControlModes)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    latchControlModes = mState.latchControlModes;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetFeatureMap(BitFlags<Feature> & featureMap)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    featureMap = mConformance.FeatureMap();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetClusterRevision(Attributes::ClusterRevision::TypeInfo::Type & clusterRevision)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    clusterRevision = ClosureDimension::kRevision;
    return CHIP_NO_ERROR;
}

Status ClusterLogic::HandleSetTargetCommand(Optional<Percent100ths> position, Optional<bool> latch,
                                            Optional<Globals::ThreeLevelAutoEnum> speed)
{
    VerifyOrDieWithMsg(mInitialized, AppServer, "Unexpected command received when closure is yet to be initialized");

    //  If all command parameters don't have a value, return InvalidCommand
    VerifyOrReturnError(position.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    // TODO: If this command is sent while the closure is in a non-compatible internal-state, a status code of
    // INVALID_IN_STATE SHALL be returned.

    DataModel::Nullable<GenericDimensionStateStruct> targetState;
    VerifyOrReturnError(GetTargetState(targetState) == CHIP_NO_ERROR, Status::Failure);

    // If targetState is null, we need to initialize to default value.
    // This is to ensure that we can set the position, latch, and speed values in the targetState.
    if (targetState.IsNull())
    {
        targetState.SetNonNull(GenericDimensionStateStruct{});
    }

    // If position field is present and Positioning(PS) feature is not supported, we should not set targetState.position value.
    if (position.HasValue() && mConformance.HasFeature(Feature::kPositioning))
    {
        VerifyOrReturnError((position.Value() <= kPercents100thsMaxValue), Status::ConstraintError);

        // If the Limitation Feature is active, the closure will automatically offset the TargetState.Position value to fit within
        // LimitRange.Min and LimitRange.Max.
        if (mConformance.HasFeature(Feature::kLimitation))
        {
            Structs::RangePercent100thsStruct::Type limitRange;

            VerifyOrReturnError(GetLimitRange(limitRange) == CHIP_NO_ERROR, Status::Failure);

            if (position.Value() > limitRange.max)
            {
                position.Value() = limitRange.max;
            }

            else if (position.Value() < limitRange.min)
            {
                position.Value() = limitRange.min;
            }
        }

        Percent100ths resolution;
        VerifyOrReturnError(GetResolution(resolution) == CHIP_NO_ERROR, Status::Failure,
                            ChipLogError(AppServer, "Unable to get Resolution value while handling SetTarget command"));
        // Check if position.Value() is an integer multiple of resolution, else round to nearest valid value
        if (position.Value() % resolution != 0)
        {
            Percent100ths roundedPosition =
                static_cast<Percent100ths>(((position.Value() + resolution / 2) / resolution) * resolution);
            ChipLogProgress(AppServer, "Rounding position from %u to nearest valid value %u based on resolution %u",
                            position.Value(), roundedPosition, resolution);
            position.SetValue(roundedPosition);
        }
        targetState.Value().position.SetValue(DataModel::MakeNullable(position.Value()));
    }

    // If latch field is present and MotionLatching feature is not supported, we should not set targetState.latch value.
    if (latch.HasValue() && mConformance.HasFeature(Feature::kMotionLatching))
    {
        // If latch value is true and the Remote Latching feature is not supported, or
        // if latch value is false and the Remote Unlatching feature is not supported, return InvalidInState.
        if ((latch.Value() && !mState.latchControlModes.Has(LatchControlModesBitmap::kRemoteLatching)) ||
            (!latch.Value() && !mState.latchControlModes.Has(LatchControlModesBitmap::kRemoteUnlatching)))
        {
            return Status::InvalidInState;
        }

        targetState.Value().latch.SetValue(DataModel::MakeNullable(latch.Value()));
    }

    // If speed field is present and Speed feature is not supported, we should not set targetState.speed value.
    if (speed.HasValue() && mConformance.HasFeature(Feature::kSpeed))
    {
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
        targetState.Value().speed.SetValue(speed.Value());
    }

    // Check if the current position is valid or else return InvalidInState
    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    VerifyOrReturnError(GetCurrentState(currentState) == CHIP_NO_ERROR, Status::Failure);
    VerifyOrReturnError(!currentState.IsNull(), Status::InvalidInState);
    VerifyOrReturnError(currentState.Value().position.HasValue() && !currentState.Value().position.Value().IsNull(),
                        Status::InvalidInState);

    // If this command requests a position change while the Latch field of the CurrentState is True (Latched), and the Latch field
    // of this command is not set to False (Unlatched), a status code of INVALID_IN_STATE SHALL be returned.
    if (mConformance.HasFeature(Feature::kMotionLatching))
    {
        if (position.HasValue() && currentState.Value().latch.HasValue() && !currentState.Value().latch.Value().IsNull() &&
            currentState.Value().latch.Value().Value())
        {
            VerifyOrReturnError(latch.HasValue() && !latch.Value(), Status::InvalidInState,
                                ChipLogError(AppServer,
                                             "Latch is True in State, but SetTarget command does not set latch to False"
                                             "when position change is requested on endpoint : %d",
                                             mMatterContext.GetEndpointId()));
        }
    }

    // Target should only be set when delegate function returns status as Success. Return failure otherwise
    VerifyOrReturnError(mDelegate.HandleSetTarget(position, latch, speed) == Status::Success, Status::Failure);

    VerifyOrReturnError(SetTargetState(targetState) == CHIP_NO_ERROR, Status::Failure);

    return Status::Success;
}

Status ClusterLogic::HandleStepCommand(StepDirectionEnum direction, uint16_t numberOfSteps,
                                       Optional<Globals::ThreeLevelAutoEnum> speed)
{
    VerifyOrDieWithMsg(mInitialized, NotSpecified, "Unexpected command recieved when closure is yet to be initialized");

    // Return UnsupportedCommand if Positioning feature is not supported.
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), Status::UnsupportedCommand);

    // Return ConstraintError if command parameters are out of bounds
    VerifyOrReturnError(direction != StepDirectionEnum::kUnknownEnumValue, Status::ConstraintError);
    VerifyOrReturnError(numberOfSteps > 0, Status::ConstraintError);

    DataModel::Nullable<GenericDimensionStateStruct> stepTarget;
    VerifyOrReturnError(GetTargetState(stepTarget) == CHIP_NO_ERROR, Status::Failure);

    if (stepTarget.IsNull())
    {
        // If stepTarget is null, we need to initialize to default value.
        // This is to ensure that we can set the position, latch, and speed values in the stepTarget.
        stepTarget.SetNonNull(GenericDimensionStateStruct{});
    }

    // If speed field is present and Speed feature is not supported, we should not set stepTarget.speed value.
    if (speed.HasValue() && mConformance.HasFeature(Feature::kSpeed))
    {
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
        stepTarget.Value().speed.SetValue(speed.Value());
    }

    // TODO: If the server is in a state where it cannot support the command, the server SHALL respond with an
    // INVALID_IN_STATE response and the TargetState attribute value SHALL remain unchanged.

    // Check if the current position is valid or else return InvalidInState
    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    VerifyOrReturnError(GetCurrentState(currentState) == CHIP_NO_ERROR, Status::Failure);
    VerifyOrReturnError(!currentState.IsNull(), Status::InvalidInState);
    VerifyOrReturnError(currentState.Value().position.HasValue() && !currentState.Value().position.Value().IsNull(),
                        Status::InvalidInState);

    if (mConformance.HasFeature(Feature::kMotionLatching))
    {
        if (currentState.Value().latch.HasValue() && !currentState.Value().latch.Value().IsNull())
        {
            VerifyOrReturnError(!currentState.Value().latch.Value().Value(), Status::InvalidInState,
                                ChipLogError(AppServer,
                                             "Step command cannot be processed when current latch is True"
                                             "on endpoint : %d",
                                             mMatterContext.GetEndpointId()));
        }
        // Return InvalidInState if currentState is latched
    }

    // Derive TargetState Position from StepValue and NumberOfSteps.
    Percent100ths stepValue;
    VerifyOrReturnError(GetStepValue(stepValue) == CHIP_NO_ERROR, Status::Failure);

    // Convert step to position delta.
    // As StepValue can only take maxvalue of kPercents100thsMaxValue(which is 10000). Below product will be within limits of
    // int32_t
    uint32_t delta       = numberOfSteps * stepValue;
    uint32_t newPosition = 0;

    // check if closure supports Limitation feature, if yes fetch the LimitRange values
    bool limitSupported = mConformance.HasFeature(Feature::kLimitation) ? true : false;

    Structs::RangePercent100thsStruct::Type limitRange;

    if (limitSupported)
    {
        VerifyOrReturnError(GetLimitRange(limitRange) == CHIP_NO_ERROR, Status::Failure);
    }

    // Position = Position - NumberOfSteps * StepValue
    uint32_t currentPosition = static_cast<uint32_t>(currentState.Value().position.Value().Value());

    switch (direction)
    {

    case StepDirectionEnum::kDecrease:
        // To avoid underflow, newPosition will be set to 0 if currentPosition is less than or equal to delta
        newPosition = (currentPosition > delta) ? currentPosition - delta : 0;
        // Position value SHALL be clamped to 0.00% if the LM feature is not supported or LimitRange.Min if the LM feature is
        // supported.
        newPosition = limitSupported ? std::max(newPosition, static_cast<uint32_t>(limitRange.min)) : newPosition;
        break;

    case StepDirectionEnum::kIncrease:
        // To avoid overflow, newPosition will be set to UINT32_MAX if sum of currentPosition and delta is greater than UINT32_MAX
        newPosition = (currentPosition > UINT32_MAX - delta) ? UINT32_MAX : currentPosition + delta;
        // Position value SHALL be clamped to 0.00% if the LM feature is not supported or LimitRange.Max if the LM feature is
        // supported.
        newPosition = limitSupported ? std::min(newPosition, static_cast<uint32_t>(limitRange.max))
                                     : std::min(newPosition, static_cast<uint32_t>(kPercents100thsMaxValue));
        break;

    default:
        // Should never reach here due to earlier VerifyOrReturnError check
        ChipLogError(AppServer, "Unhandled StepDirectionEnum value");
        return Status::ConstraintError;
    }

    // TargetState should only be set when delegate function returns status as Success. Return failure otherwise
    VerifyOrReturnError(mDelegate.HandleStep(direction, numberOfSteps, speed) == Status::Success, Status::Failure);

    stepTarget.Value().position.SetValue(DataModel::MakeNullable(static_cast<Percent100ths>(newPosition)));
    VerifyOrReturnError(SetTargetState(stepTarget) == CHIP_NO_ERROR, Status::Failure);

    return Status::Success;
}

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
