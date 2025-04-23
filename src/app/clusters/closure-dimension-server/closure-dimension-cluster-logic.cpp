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
#include <platform/LockTracker.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

using namespace Protocols::InteractionModel;
using namespace chip::app::Clusters::ClosureDimension::Attributes;

namespace {

static constexpr Percent100ths kPercents100thsMaxValue = 10000;

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

    //TODO: SpecIssue: All the attributes like resolution, unit etc which are constant for closure should be made to be set only during Initialization.

    mInitialized = true;
    return CHIP_NO_ERROR;
}

// TODO: CurrentState should be QuietReporting.
CHIP_ERROR ClusterLogic::SetCurrentState(const DataModel::Nullable<GenericCurrentStateStruct> & incomingCurrentState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnValue(mState.currentState != incomingCurrentState, CHIP_NO_ERROR);

    DataModel::Nullable<GenericCurrentStateStruct> & clusterCurrentState = mState.currentState;

    bool validateClusterCurrentStateMembers  = false;
    bool validateClusterCurrentStatePosition = false;
    bool validateClusterCurrentStateLatch    = false;
    bool validateClusterCurrentStateSpeed    = false;

    bool validateIncomingCurrentStateMembers  = false;
    bool validateIncomingCurrentStatePosition = false;
    bool validateIncomingCurrentStateLatch    = false;
    bool validateIncomingCurrentStateSpeed    = false;

    bool requirePositioningUpdate = false;
    bool requireLatchUpdate       = false;
    bool requireSpeedUpdate       = false;

    /*
        Determine checks that need to be executed based on current and provided values
    */

    // Determine CurrentState member values, if a value is present, if it is null or if it has a valid value, to determine what
    // checks need to be executed.
    if (!clusterCurrentState.IsNull())
    {

        // If the CurrentState has a value, we need to check internal members to validate if we need to mark the attribute as dirty.
        validateClusterCurrentStateMembers = true;

        validateClusterCurrentStatePosition = clusterCurrentState.Value().position.HasValue();
        validateClusterCurrentStateLatch    = clusterCurrentState.Value().latch.HasValue();
        validateClusterCurrentStateSpeed    = clusterCurrentState.Value().speed.HasValue();
    }

    // Determine CurrentState member values, if a value is present, if it is null or if it has a valid value, to determine what
    // checks need to be executed.
    if (!incomingCurrentState.IsNull())
    {
        // If the provided CurrentState has a value, we need to check its members to validate
        validateIncomingCurrentStateMembers = true;

        validateIncomingCurrentStatePosition = incomingCurrentState.Value().position.HasValue();
        validateIncomingCurrentStateLatch    = incomingCurrentState.Value().latch.HasValue();
        validateIncomingCurrentStateSpeed    = incomingCurrentState.Value().speed.HasValue();
    }

    /*
        Validate incoming values and FeatureMap Conformance
    */

    // Validate the incoming Position value has valid input parameters and FeatureMap conformance.
    if (validateIncomingCurrentStatePosition)
    {
        //  If the position member is present in the incoming CurrentState, we need to check if the Positioning
        //  feature is supported by the device. If the Positioning feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        // We don't need to check if values are present since the check was done above.
        const Percent100ths & position = incomingCurrentState.Value().position.Value();
        VerifyOrReturnError(position <= kPercents100thsMaxValue, CHIP_ERROR_INVALID_ARGUMENT);

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requirePositioningUpdate = !validateClusterCurrentStatePosition || clusterCurrentState.Value().position.Value() != position;
    }

    // Validate the incoming latch value has valid input parameters and FeatureMap conformance.
    if (validateIncomingCurrentStateLatch)
    {
        //  If the latching member is present in the incoming CurrentState, we need to check if the MotionLatching
        //  feature is supported by the device. If the MotionLatching feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        // We don't need to check is values are present since the check was done above.
        const bool & latch = incomingCurrentState.Value().latch.Value();

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requireLatchUpdate = !validateClusterCurrentStateLatch || clusterCurrentState.Value().latch.Value() != latch;
    }

    // Validate the incoming Speed value has valid input parameters and FeatureMap conformance.
    if (validateIncomingCurrentStateSpeed)
    {
        //  If the speed member is present in the incoming CurrentState, we need to check if the Speed feature is
        //  supported by the device. If the Speed feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        const Globals::ThreeLevelAutoEnum & speed = incomingCurrentState.Value().speed.Value();

        VerifyOrReturnError(EnsureKnownEnumValue(speed) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT);

        // We need to update the cluster position value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requireSpeedUpdate = !validateClusterCurrentStateSpeed || clusterCurrentState.Value().speed.Value() != speed;
    }

    /*
        Update the cluster state and mark it as dirty if necessary
    */

    // Cluster CurrentState going from non-null to null
    if (validateClusterCurrentStateMembers && !validateIncomingCurrentStateMembers)
    {
        clusterCurrentState.SetNull();
        mMatterContext.MarkDirty(Attributes::CurrentState::Id);
    }
    else if (requirePositioningUpdate || requireLatchUpdate || requireSpeedUpdate)
    {
        clusterCurrentState.SetNonNull(incomingCurrentState.Value());
        mMatterContext.MarkDirty(Attributes::CurrentState::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetTarget(const DataModel::Nullable<GenericTargetStruct> & incomingTarget)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnValue(mState.target != incomingTarget, CHIP_NO_ERROR);

    DataModel::Nullable<GenericTargetStruct> & clusterTarget = mState.target;

    bool validateClusterTargetMembers  = false;
    bool validateClusterTargetPosition = false;
    bool validateClusterTargetLatch    = false;
    bool validateClusterTargetSpeed    = false;

    bool validateIncomingTargetMembers  = false;
    bool validateIncomingTargetPosition = false;
    bool validateIncomingTargetLatch    = false;
    bool validateIncomingTargetSpeed    = false;

    bool requirePositioningUpdate = false;
    bool requireLatchUpdate       = false;
    bool requireSpeedUpdate       = false;

    /*
        Determine checks that need to be executed based on current and provided values
    */

    // Determine Target member values, if a value is present, if it is null or if it has a valid value, to determine what
    // checks need to be executed.
    if (!clusterTarget.IsNull())
    {

        // If the Target has a value, we need to check internal members to validate if we need to mark the attribute as dirty.
        validateClusterTargetMembers = true;

        validateClusterTargetPosition = clusterTarget.Value().position.HasValue();
        validateClusterTargetLatch    = clusterTarget.Value().latch.HasValue();
        validateClusterTargetSpeed    = clusterTarget.Value().speed.HasValue();
    }

    // Determine Target member values, if a value is present, if it is null or if it has a valid value, to determine what
    // checks need to be executed.
    if (!incomingTarget.IsNull())
    {
        // If the provided Target has a value, we need to check its members to validate
        validateIncomingTargetMembers = true;

        validateIncomingTargetPosition = incomingTarget.Value().position.HasValue();
        validateIncomingTargetLatch    = incomingTarget.Value().latch.HasValue();
        validateIncomingTargetSpeed    = incomingTarget.Value().speed.HasValue();
    }

    /*
        Validate incoming values and FeatureMap Conformance
    */

    // Validate the incoming Position value has valid input parameters and FeatureMap conformance.
    if (validateIncomingTargetPosition)
    {
        //  If the position member is present in the incoming Target, we need to check if the Positioning
        //  feature is supported by the device. If the Positioning feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        // We don't need to check if values are present since the check was done above.
        const Percent100ths & position = incomingTarget.Value().position.Value();
        VerifyOrReturnError(position <= kPercents100thsMaxValue, CHIP_ERROR_INVALID_ARGUMENT);

        // Incoming Target Position value SHALL follow the scaling from Resolution Attribute.
        Percent100ths resolution;
        ReturnErrorOnFailure(GetResolution(resolution));
        VerifyOrReturnError(
            incomingTarget.Value().position.Value() % resolution == 0, CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(NotSpecified, "Target Position value SHALL follow the scaling from Resolution Attribute"));

        // We need to update the cluster position value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requirePositioningUpdate = !validateClusterTargetPosition || clusterTarget.Value().position.Value() != position;
    }

    // Validate the incoming latch value has valid input parameters and FeatureMap conformance.
    if (validateIncomingTargetLatch)
    {
        //  If the latching member is present in the incoming Target, we need to check if the MotionLatching
        //  feature is supported by the device. If the MotionLatching feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        // We don't need to check is values are present since the check was done above.
        const bool & latch = incomingTarget.Value().latch.Value();

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requireLatchUpdate = !validateClusterTargetLatch || clusterTarget.Value().latch.Value() != latch;
    }

    // Validate the incoming Speed value has valid input parameters and FeatureMap conformance.
    if (validateIncomingTargetSpeed)
    {
        //  If the speed member is present in the incoming Target, we need to check if the Speed feature is
        //  supported by the device. If the Speed feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        const Globals::ThreeLevelAutoEnum & speed = incomingTarget.Value().speed.Value();

        VerifyOrReturnError(EnsureKnownEnumValue(speed) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT);

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requireSpeedUpdate = !validateClusterTargetSpeed || clusterTarget.Value().speed.Value() != speed;
    }

    /*
        Update the cluster state and mark it as dirty if necessary
    */

    // Cluster Target going from non-null to null
    if (validateClusterTargetMembers && !validateIncomingTargetMembers)
    {
        clusterTarget.SetNull();
        mMatterContext.MarkDirty(Attributes::Target::Id);
    }
    else if (requirePositioningUpdate || requireLatchUpdate || requireSpeedUpdate)
    {
        clusterTarget.SetNonNull(incomingTarget.Value());
        mMatterContext.MarkDirty(Attributes::Target::Id);
    }

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

    // If Unit is Degrees , 0° value orientation is the value corresponding to the perpendicular axis to the Closure panel.
    // Range of values SHALL contain -90 to 90 only
    //TODO: Spec issue - No explicit mention of constrains for degrees.
    if (unit == ClosureUnitEnum::kDegree)
    {
        VerifyOrReturnError(unitRange.Value().min >= -90 && unitRange.Value().min <= 90, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(unitRange.Value().max >= -90 && unitRange.Value().max <= 90, CHIP_ERROR_INVALID_ARGUMENT);
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
        ChipLogError(NotSpecified,
                     "LimitRange.Min and LimitRange.Max SHALL be equal to an integer multiple of the Resolution attribute."));
    VerifyOrReturnError(
        limitRange.max % resolution == 0, CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(NotSpecified,
                     "LimitRange.Min and LimitRange.Max SHALL be equal to an integer multiple of the Resolution attribute."));

    if ((limitRange.min != mState.limitRange.min) || (limitRange.max != mState.limitRange.max))
    {
        mState.limitRange.min = limitRange.min;
        mState.limitRange.max = limitRange.max;
        mMatterContext.MarkDirty(Attributes::LimitRange::Id);
    }

    return CHIP_NO_ERROR;
}

// This attribute is not supposed to change once the installation is finalized.
// so SetTranslationDirection should only be called from Init()
CHIP_ERROR ClusterLogic::SetTranslationDirection(const TranslationDirectionEnum translationDirection)
{
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

// This attribute is not supposed to change once the installation is finalized.
// so SetRotationAxis should only be called from Init().
CHIP_ERROR ClusterLogic::SetRotationAxis(const RotationAxisEnum rotationAxis)
{
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

    // No need to check for feature conformance, as feature conformance is validated during Initilization.
    VerifyOrReturnError(mConformance.OptionalAttributes().Has(OptionalAttributeEnum::kOverflow),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(EnsureKnownEnumValue(overflow) != OverflowEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);

    RotationAxisEnum rotationAxis;
    ReturnErrorOnFailure(GetRotationAxis(rotationAxis));

    // If the axis is centered, one part goes Outside and the other part goes Inside.
    // In this case, this attribute SHALL use Top/Bottom/Left/Right Inside or Top/Bottom/Left/Right Outside enumerated value.
    if (rotationAxis == RotationAxisEnum::kCenteredHorizontal || rotationAxis == RotationAxisEnum::kCenteredVertical)
    {
        VerifyOrReturnError(overflow != OverflowEnum::kNoOverflow && overflow != OverflowEnum::kInside &&
                            overflow != OverflowEnum::kOutside, CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (overflow != mState.overflow)
    {
        mState.overflow = overflow;
        mMatterContext.MarkDirty(Attributes::Overflow::Id);
    }

    return CHIP_NO_ERROR;
}

// This attribute is not supposed to change once the installation is finalized.
// so SetModulationType should only be called from Init().
CHIP_ERROR ClusterLogic::SetModulationType(const ModulationTypeEnum modulationType)
{
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

CHIP_ERROR ClusterLogic::GetCurrentState(DataModel::Nullable<GenericCurrentStateStruct> & currentState)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    currentState = mState.currentState;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetTarget(DataModel::Nullable<GenericTargetStruct> & target)
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
    VerifyOrReturnError((mConformance.OptionalAttributes().Has(OptionalAttributeEnum::kOverflow)),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
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
    featureMap = mConformance.FeatureMap();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetClusterRevision(Attributes::ClusterRevision::TypeInfo::Type & clusterRevision)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    clusterRevision = kClusterRevision;
    return CHIP_NO_ERROR;
}

Status ClusterLogic::HandleSetTargetCommand(Optional<Percent100ths> position, Optional<bool> latch,
                                            Optional<Globals::ThreeLevelAutoEnum> speed)
{
    VerifyOrDieWithMsg(mInitialized, AppServer, "Unexpected command received when device is yet to be initialized");

    Status status = Status::Success;

    //TODO: SpecIssue No Explicit error code mentioned in case all 3 fields are absent.
    // If all command parameters don't have a value, return InvalidCommand
    VerifyOrReturnValue(position.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    // TODO: SpecIssue If this command is sent while the device is in a non-compatible internal-state, a status code of
    // INVALID_IN_STATE SHALL be returned.

    GenericTargetStruct target{};



    // If position field is present and Positioning(PS) feature is not supported, we should not set target.position value.
    if (position.HasValue() && mConformance.HasFeature(Feature::kPositioning))
    {
        VerifyOrReturnError((position.Value() <= kPercents100thsMaxValue), Status::ConstraintError);

        //TODO: Specissue: Previously spec says if target position in not multiple of resolution, we should set to nearest integer multiple.
        //But in latest spec the behaviour is not defined

        // If the Limitation Feature is active, the device will automatically offset the Target.Position value to fit within LimitRange.Min and LimitRange.Max.
        Structs::RangePercent100thsStruct::Type limitRange;

        if (mConformance.HasFeature(Feature::kLimitation))
        {
            VerifyOrReturnError(GetLimitRange(limitRange) == CHIP_NO_ERROR, Status::Failure);

            if(position.Value() > limitRange.max) {
                position.Value() = limitRange.max;
            }

            if(position.Value() < limitRange.min) {
                position.Value() = limitRange.min;
            }
        }

        target.position.SetValue(position.Value());
    }

    // If latch field is present and MotionLatching feature is not supported, we should not set target.latch value.
    if (latch.HasValue() && mConformance.HasFeature(Feature::kMotionLatching))
    {
        if (mDelegate.IsManualLatchingNeeded())
        {
            return Status::InvalidAction;
        }

        target.latch = latch;
    }

    // If speed field is present and Speed feature is not supported, we should not set target.speed value.
    if (speed.HasValue() && mConformance.HasFeature(Feature::kSpeed))
    {
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
        target.speed = speed;
    }

    // Check if the current position is valid or else return InvalidInState
    DataModel::Nullable<GenericCurrentStateStruct> currentState;
    VerifyOrReturnError(GetCurrentState(currentState) == CHIP_NO_ERROR, Status::Failure);
    VerifyOrReturnValue(!currentState.IsNull(), Status::InvalidInState);
    VerifyOrReturnValue(currentState.Value().position.HasValue(), Status::InvalidInState);

    // Target should only be set when delegate function returns status as Success. Return failure otherwise
    VerifyOrReturnValue(mDelegate.HandleSetTarget(position, latch, speed) == Status::Success, Status::Failure);

    DataModel::Nullable<GenericTargetStruct> nullableTarget;
    nullableTarget.SetNonNull(target);
    VerifyOrReturnValue(SetTarget(nullableTarget) == CHIP_NO_ERROR, Status::Failure);

    return status;
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

    Status status = Status::Success;

    GenericTargetStruct stepTarget{};

    // If speed field is present and Speed feature is not supported, we should not set stepTarget.speed value.
    if (speed.HasValue() && mConformance.HasFeature(Feature::kSpeed))
    {
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
        stepTarget.speed = speed;
    }

    // TODO: SpecIssue: If the server is in a state where it cannot support the command, the server SHALL respond with an
    // INVALID_IN_STATE response and the Target attribute value SHALL remain unchanged.

    // Check if the current position is valid or else return InvalidInState
    DataModel::Nullable<GenericCurrentStateStruct> currentState;
    VerifyOrReturnError(GetCurrentState(currentState) == CHIP_NO_ERROR, Status::Failure);
    VerifyOrReturnValue(!currentState.IsNull(), Status::InvalidInState);
    VerifyOrReturnValue(currentState.Value().position.HasValue(), Status::InvalidInState);

    // Derive Target Position from StepValue and NumberOfSteps.
    Percent100ths stepValue;
    VerifyOrReturnError(GetStepValue(stepValue) == CHIP_NO_ERROR, Status::Failure);

    // Convert step to position delta.
    // As StepValue can only take maxvalue of kPercents100thsMaxValue(which is 10000). Below product will be within limits of int32_t
    uint32_t delta       = numberOfSteps * stepValue;
    uint32_t newPosition = 0;

    // check if device supports Limitation feature, if yes fetch the LimitRange values
    bool limitSupported = mConformance.HasFeature(Feature::kLimitation) ? true : false;

    Structs::RangePercent100thsStruct::Type limitRange;

    if (limitSupported)
    {
        VerifyOrReturnError(GetLimitRange(limitRange) == CHIP_NO_ERROR, Status::Failure);
    }

    // Position = Position - NumberOfSteps * StepValue
    uint32_t currentPosition = static_cast<uint32_t>(currentState.Value().position.Value());

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

    // Target should only be set when delegate function returns status as Success. Return failure otherwise
    VerifyOrReturnValue(mDelegate.HandleStep(direction, numberOfSteps, speed) == Status::Success, Status::Failure);

    stepTarget.position.SetValue(static_cast<Percent100ths>(newPosition));
    DataModel::Nullable<GenericTargetStruct> nullableTarget;
    nullableTarget.SetNonNull(stepTarget);
    VerifyOrReturnError(SetTarget(nullableTarget) == CHIP_NO_ERROR, Status::Failure);

    return status;
}

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
