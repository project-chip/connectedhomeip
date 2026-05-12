/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 *
 */

#include "ClosureDimensionCluster.h"
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ClosureDimension/Attributes.h>
#include <clusters/ClosureDimension/Commands.h>
#include <clusters/ClosureDimension/Metadata.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureDimension;
using namespace chip::app::Clusters::ClosureDimension::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

using namespace Protocols::InteractionModel;

namespace {

constexpr Percent100ths kPercents100thsMaxValue    = 10000;
constexpr uint64_t kPositionQuietReportingInterval = 5000;

} // namespace

ClosureDimensionCluster::ClosureDimensionCluster(const Config & config) :
    DefaultServerCluster({ config.mEndpointId, ClosureDimension::Id }), mDelegate(config.mDelegate),
    mResolution(config.mResolution), mStepValue(config.mStepValue), mUnit(config.mUnit), mUnitRange(config.mUnitRange),
    mLimitRange(config.mLimitRange), mTranslationDirection(config.mTranslationDirection), mRotationAxis(config.mRotationAxis),
    mOverflow(config.mOverflow), mModulationType(config.mModulationType), mLatchControlModes(config.mLatchControlModes),
    mFeatureMap(config.mFeatureMap)
{
    // Positioning or MotionLatching must be enabled
    VerifyOrDieWithMsg(mFeatureMap.Has(Feature::kPositioning) || mFeatureMap.Has(Feature::kMotionLatching), AppServer,
                       "Validation failed: Neither Positioning nor MotionLatching is enabled.");

    // If Unit, Limitation or speed is enabled, Positioning must be enabled
    if (mFeatureMap.Has(Feature::kUnit) || mFeatureMap.Has(Feature::kLimitation) || mFeatureMap.Has(Feature::kSpeed))
    {
        VerifyOrDieWithMsg(mFeatureMap.Has(Feature::kPositioning), AppServer,
                           "Validation failed: Unit, Limitation, and speed requires the Positioning feature.");
    }

    // If Translation, Rotation or Modulation is enabled, Positioning must be enabled.
    if (mFeatureMap.Has(Feature::kTranslation) || mFeatureMap.Has(Feature::kRotation) || mFeatureMap.Has(Feature::kModulation))
    {
        VerifyOrDieWithMsg(mFeatureMap.Has(Feature::kPositioning), AppServer,
                           "Validation failed: Translation, Rotation or Modulation requires Positioning enabled.");
    }

    // Only one of Translation, Rotation or Modulation features must be enabled. Return false otherwise.
    if ((mFeatureMap.Has(Feature::kTranslation) && mFeatureMap.Has(Feature::kRotation)) ||
        (mFeatureMap.Has(Feature::kRotation) && mFeatureMap.Has(Feature::kModulation)) ||
        (mFeatureMap.Has(Feature::kModulation) && mFeatureMap.Has(Feature::kTranslation)))
    {
        VerifyOrDieWithMsg(false, AppServer,
                           "Validation failed: Only one of Translation, Rotation or Modulation feature can be enabled.");
    }
}

ClosureDimensionCluster::~ClosureDimensionCluster() {}

CHIP_ERROR ClosureDimensionCluster::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using OptionalEntry                = AttributeListBuilder::OptionalAttributeEntry;
    OptionalEntry optionalAttributes[] = {
        { mFeatureMap.Has(Feature::kPositioning), Attributes::Resolution::kMetadataEntry },
        { mFeatureMap.Has(Feature::kPositioning), Attributes::StepValue::kMetadataEntry },
        { mFeatureMap.Has(Feature::kUnit), Attributes::Unit::kMetadataEntry },
        { mFeatureMap.Has(Feature::kUnit), Attributes::UnitRange::kMetadataEntry },
        { mFeatureMap.Has(Feature::kLimitation), Attributes::LimitRange::kMetadataEntry },
        { mFeatureMap.Has(Feature::kTranslation), Attributes::TranslationDirection::kMetadataEntry },
        { mFeatureMap.Has(Feature::kRotation), Attributes::RotationAxis::kMetadataEntry },
        { mFeatureMap.Has(Feature::kRotation), Attributes::Overflow::kMetadataEntry },
        { mFeatureMap.Has(Feature::kModulation), Attributes::ModulationType::kMetadataEntry },
        { mFeatureMap.Has(Feature::kMotionLatching), Attributes::LatchControlModes::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR ClosureDimensionCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kMandatoryCommands[] = {
        Commands::SetTarget::kMetadataEntry,
    };

    static constexpr DataModel::AcceptedCommandEntry kStepCommand[] = {
        Commands::Step::kMetadataEntry,
    };

    ReturnErrorOnFailure(builder.ReferenceExisting(kMandatoryCommands));
    if (mFeatureMap.Has(Feature::kPositioning))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kStepCommand));
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus ClosureDimensionCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::CurrentState::Id:
        return encoder.Encode(GetCurrentState());
    case Attributes::TargetState::Id:
        return encoder.Encode(GetTargetState());
    case Attributes::Resolution::Id:
        return encoder.Encode(GetResolution());
    case Attributes::StepValue::Id:
        return encoder.Encode(GetStepValue());
    case Attributes::Unit::Id:
        return encoder.Encode(GetUnit());
    case Attributes::UnitRange::Id:
        return encoder.Encode(GetUnitRange());
    case Attributes::LimitRange::Id:
        return encoder.Encode(GetLimitRange());
    case Attributes::TranslationDirection::Id:
        return encoder.Encode(GetTranslationDirection());
    case Attributes::RotationAxis::Id:
        return encoder.Encode(GetRotationAxis());
    case Attributes::Overflow::Id:
        return encoder.Encode(GetOverflow());
    case Attributes::ModulationType::Id:
        return encoder.Encode(GetModulationType());
    case Attributes::LatchControlModes::Id:
        return encoder.Encode(GetLatchControlModes());
    case Attributes::FeatureMap::Id:
        return encoder.Encode(GetFeatureMap());
    case Attributes::ClusterRevision::Id:
        return encoder.Encode(kRevision);
    default:
        return Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> ClosureDimensionCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                    chip::TLV::TLVReader & input_arguments,
                                                                                    CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::SetTarget::Id: {
        Commands::SetTarget::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleSetTargetCommand(commandData.position, commandData.latch, commandData.speed);
    }
    case Commands::Step::Id: {
        Commands::Step::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleStepCommand(commandData.direction, commandData.numberOfSteps, commandData.speed);
    }
    default:
        return Status::UnsupportedCommand;
    }
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
//  i.e Speed and latch changes are not considered when calculating the at most 5 seconds quiet reportable changes for Position.
CHIP_ERROR ClosureDimensionCluster::SetCurrentState(const DataModel::Nullable<GenericDimensionStateStruct> & incomingCurrentState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mCurrentState != incomingCurrentState, CHIP_NO_ERROR);

    bool markDirty = false;

    if (!incomingCurrentState.IsNull())
    {
        // Validate the incoming Position value has valid input parameters and FeatureMap conformance.
        if (incomingCurrentState.Value().position.HasValue())
        {
            //  If the position member is present in the incoming CurrentState, we need to check if the Positioning
            //  feature is supported by the closure. If the Positioning feature is not supported, return an error.
            VerifyOrReturnError(mFeatureMap.Has(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            if (!incomingCurrentState.Value().position.Value().IsNull())
            {

                VerifyOrReturnError(incomingCurrentState.Value().position.Value().Value() <= kPercents100thsMaxValue,
                                    CHIP_ERROR_INVALID_ARGUMENT);
            }

            bool targetPositionReached = false;
            auto now                   = System::SystemClock().GetMonotonicTimestamp();

            // Logic to determine if target position is reached.
            // If the target position is reached, current state attribute will be marked dirty and reported.
            if (!mTargetState.IsNull() && mTargetState.Value().position.HasValue() &&
                !mTargetState.Value().position.Value().IsNull() &&
                mTargetState.Value().position == incomingCurrentState.Value().position)
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
            VerifyOrReturnError(mFeatureMap.Has(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        }

        // Changes to this attribute SHALL only be marked as reportable when latch changes.
        if (!mCurrentState.IsNull() && mCurrentState.Value().latch != incomingCurrentState.Value().latch)
        {
            markDirty = true;
        }

        // Validate the incoming Speed value has valid input parameters and FeatureMap conformance.
        if (incomingCurrentState.Value().speed.HasValue())
        {
            //  If the speed member is present in the incoming CurrentState, we need to check if the Speed feature is
            //  supported by the closure. If the Speed feature is not supported, return an error.
            VerifyOrReturnError(mFeatureMap.Has(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            VerifyOrReturnError(EnsureKnownEnumValue(incomingCurrentState.Value().speed.Value()) !=
                                    Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }

        // Changes to this attribute SHALL be marked as reportable when speed changes.
        if (!mCurrentState.IsNull() && mCurrentState.Value().speed != incomingCurrentState.Value().speed)
        {
            markDirty = true;
        }
    }

    // If the current state is null and the incoming current state is not null and vice versa, we need to mark dirty.
    if (mCurrentState.IsNull() != incomingCurrentState.IsNull())
    {
        markDirty = true;
    }

    mCurrentState = incomingCurrentState;

    NotifyAttributeChanged(Attributes::CurrentState::Id,
                           markDirty ? DataModel::AttributeChangeType::kReportable : DataModel::AttributeChangeType::kQuiet);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureDimensionCluster::SetTargetState(const DataModel::Nullable<GenericDimensionStateStruct> & incomingTargetState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mTargetState != incomingTargetState, CHIP_NO_ERROR);

    if (!incomingTargetState.IsNull())
    {
        // Validate the incoming Position value has valid input parameters and FeatureMap conformance.
        if (incomingTargetState.Value().position.HasValue())
        {
            //  If the position member is present in the incoming TargetState, we need to check if the Positioning
            //  feature is supported by the closure. If the Positioning feature is not supported, return an error.
            VerifyOrReturnError(mFeatureMap.Has(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            if (!incomingTargetState.Value().position.Value().IsNull())
            {
                VerifyOrReturnError(incomingTargetState.Value().position.Value().Value() <= kPercents100thsMaxValue,
                                    CHIP_ERROR_INVALID_ARGUMENT);

                // Incoming TargetState Position value SHALL follow the scaling from Resolution Attribute.
                Percent100ths resolution = GetResolution();
                VerifyOrReturnError(
                    incomingTargetState.Value().position.Value().Value() % resolution == 0, CHIP_ERROR_INVALID_ARGUMENT,
                    ChipLogError(AppServer, "TargetState Position value SHALL follow the scaling from Resolution Attribute"));
            }
        }

        // Validate the incoming latch value has valid FeatureMap conformance.
        if (incomingTargetState.Value().latch.HasValue())
        {
            //  If the latching member is present in the incoming TargetState, we need to check if the MotionLatching
            //  feature is supported by the closure. If the MotionLatching feature is not supported, return an error.
            VerifyOrReturnError(mFeatureMap.Has(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        }

        // Validate the incoming Speed value has valid input parameters and FeatureMap conformance.
        if (incomingTargetState.Value().speed.HasValue())
        {
            //  If the speed member is present in the incoming TargetState, we need to check if the Speed feature is
            //  supported by the closure. If the Speed feature is not supported, return an error.
            VerifyOrReturnError(mFeatureMap.Has(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            VerifyOrReturnError(EnsureKnownEnumValue(incomingTargetState.Value().speed.Value()) !=
                                    Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    SetAttributeValue(mTargetState, incomingTargetState, Attributes::TargetState::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureDimensionCluster::SetUnitRange(const DataModel::Nullable<Structs::UnitRangeStruct::Type> & unitRange)
{
    VerifyOrReturnError(mFeatureMap.Has(Feature::kUnit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (unitRange.IsNull())
    {
        SetAttributeValue(mUnitRange, DataModel::NullNullable, Attributes::UnitRange::Id);
        return CHIP_NO_ERROR;
    }

    // Return error if unitRange is invalid
    VerifyOrReturnError(unitRange.Value().min <= unitRange.Value().max, CHIP_ERROR_INVALID_ARGUMENT);

    ClosureUnitEnum unit = GetUnit();

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

    // If the current unitRange is null, we need to set it to the new value
    if (mUnitRange.IsNull())
    {
        mUnitRange.SetNonNull(unitRange.Value());
        NotifyAttributeChanged(Attributes::UnitRange::Id);
        return CHIP_NO_ERROR;
    }

    // If both the current unitRange and incoming unitRange are not null, we need to update if the values are different
    if ((unitRange.Value().min != mUnitRange.Value().min) || (unitRange.Value().max != mUnitRange.Value().max))
    {
        mUnitRange.Value().min = unitRange.Value().min;
        mUnitRange.Value().max = unitRange.Value().max;
        NotifyAttributeChanged(Attributes::UnitRange::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureDimensionCluster::SetLimitRange(const Structs::RangePercent100thsStruct::Type & limitRange)
{
    VerifyOrReturnError(mFeatureMap.Has(Feature::kLimitation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    // If the limit range is invalid, we need to return an error
    VerifyOrReturnError(limitRange.min <= limitRange.max, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(limitRange.min <= kPercents100thsMaxValue, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(limitRange.max <= kPercents100thsMaxValue, CHIP_ERROR_INVALID_ARGUMENT);

    // LimitRange.Min and LimitRange.Max SHALL be equal to an integer multiple of the Resolution attribute.
    Percent100ths resolution = GetResolution();
    VerifyOrReturnError(
        limitRange.min % resolution == 0, CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(AppServer, "LimitRange.Min SHALL be equal to an integer multiple of the Resolution attribute."));
    VerifyOrReturnError(
        limitRange.max % resolution == 0, CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(AppServer, "LimitRange.Max SHALL be equal to an integer multiple of the Resolution attribute."));

    if ((limitRange.min != mLimitRange.min) || (limitRange.max != mLimitRange.max))
    {
        mLimitRange.min = limitRange.min;
        mLimitRange.max = limitRange.max;
        NotifyAttributeChanged(Attributes::LimitRange::Id);
    }

    return CHIP_NO_ERROR;
}

Status ClosureDimensionCluster::HandleSetTargetCommand(Optional<Percent100ths> position, Optional<bool> latch,
                                                       Optional<Globals::ThreeLevelAutoEnum> speed)
{
    //  If all command parameters don't have a value, return InvalidCommand
    VerifyOrReturnError(position.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    // TODO: If this command is sent while the closure is in a non-compatible internal-state, a status code of
    // INVALID_IN_STATE SHALL be returned.

    DataModel::Nullable<GenericDimensionStateStruct> targetState = GetTargetState();

    // If targetState is null, we need to initialize to default value.
    // This is to ensure that we can set the position, latch, and speed values in the targetState.
    if (targetState.IsNull())
    {
        targetState.SetNonNull(GenericDimensionStateStruct{});
    }

    // If position field is present and Positioning(PS) feature is not supported, we should not set targetState.position value.
    if (position.HasValue() && mFeatureMap.Has(Feature::kPositioning))
    {
        VerifyOrReturnError((position.Value() <= kPercents100thsMaxValue), Status::ConstraintError);

        // If the Limitation Feature is active, the closure will automatically offset the TargetState.Position value to fit within
        // LimitRange.Min and LimitRange.Max.
        if (mFeatureMap.Has(Feature::kLimitation))
        {
            Structs::RangePercent100thsStruct::Type limitRange = GetLimitRange();
            if (position.Value() > limitRange.max)
            {
                position.Value() = limitRange.max;
            }

            else if (position.Value() < limitRange.min)
            {
                position.Value() = limitRange.min;
            }
        }

        Percent100ths resolution = GetResolution();
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
    if (latch.HasValue() && mFeatureMap.Has(Feature::kMotionLatching))
    {
        // If latch value is true and the Remote Latching feature is not supported, or
        // if latch value is false and the Remote Unlatching feature is not supported, return InvalidInState.
        if ((latch.Value() && !mLatchControlModes.Has(LatchControlModesBitmap::kRemoteLatching)) ||
            (!latch.Value() && !mLatchControlModes.Has(LatchControlModesBitmap::kRemoteUnlatching)))
        {
            return Status::InvalidInState;
        }

        targetState.Value().latch.SetValue(DataModel::MakeNullable(latch.Value()));
    }

    // If speed field is present and Speed feature is not supported, we should not set targetState.speed value.
    if (speed.HasValue() && mFeatureMap.Has(Feature::kSpeed))
    {
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
        targetState.Value().speed.SetValue(speed.Value());
    }

    // Check if the current position is valid or else return InvalidInState
    DataModel::Nullable<GenericDimensionStateStruct> currentState = GetCurrentState();
    VerifyOrReturnError(!currentState.IsNull(), Status::InvalidInState);
    if (mFeatureMap.Has(Feature::kPositioning))
    {
        VerifyOrReturnError(currentState.Value().position.HasValue() && !currentState.Value().position.Value().IsNull(),
                            Status::InvalidInState);
    }

    // If this command requests a position change while the Latch field of the CurrentState is True (Latched), and the Latch field
    // of this command is not set to False (Unlatched), a status code of INVALID_IN_STATE SHALL be returned.
    if (mFeatureMap.Has(Feature::kMotionLatching))
    {
        if (position.HasValue() && currentState.Value().latch.HasValue() && !currentState.Value().latch.Value().IsNull() &&
            currentState.Value().latch.Value().Value())
        {
            VerifyOrReturnError(latch.HasValue() && !latch.Value(), Status::InvalidInState,
                                ChipLogError(AppServer,
                                             "Latch is True in State, but SetTarget command does not set latch to False"
                                             "when position change is requested on endpoint : %d",
                                             GetEndpointId()));
        }
    }

    // Target should only be set when delegate function returns status as Success. Return failure otherwise
    VerifyOrReturnError(mDelegate.HandleSetTarget(position, latch, speed) == Status::Success, Status::Failure);

    VerifyOrReturnError(SetTargetState(targetState) == CHIP_NO_ERROR, Status::Failure);

    return Status::Success;
}

Status ClosureDimensionCluster::HandleStepCommand(StepDirectionEnum direction, uint16_t numberOfSteps,
                                                  Optional<Globals::ThreeLevelAutoEnum> speed)
{
    // Return ConstraintError if command parameters are out of bounds
    VerifyOrReturnError(direction != StepDirectionEnum::kUnknownEnumValue, Status::ConstraintError);
    VerifyOrReturnError(numberOfSteps > 0, Status::ConstraintError);

    DataModel::Nullable<GenericDimensionStateStruct> stepTarget = GetTargetState();

    if (stepTarget.IsNull())
    {
        // If stepTarget is null, we need to initialize to default value.
        // This is to ensure that we can set the position, latch, and speed values in the stepTarget.
        stepTarget.SetNonNull(GenericDimensionStateStruct{});
    }

    // If speed field is present and Speed feature is not supported, we should not set stepTarget.speed value.
    if (speed.HasValue() && mFeatureMap.Has(Feature::kSpeed))
    {
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
        stepTarget.Value().speed.SetValue(speed.Value());
    }

    // TODO: If the server is in a state where it cannot support the command, the server SHALL respond with an
    // INVALID_IN_STATE response and the TargetState attribute value SHALL remain unchanged.

    // Check if the current position is valid or else return InvalidInState
    DataModel::Nullable<GenericDimensionStateStruct> currentState = GetCurrentState();
    VerifyOrReturnError(!currentState.IsNull(), Status::InvalidInState);
    VerifyOrReturnError(currentState.Value().position.HasValue() && !currentState.Value().position.Value().IsNull(),
                        Status::InvalidInState);

    if (mFeatureMap.Has(Feature::kMotionLatching))
    {
        if (currentState.Value().latch.HasValue() && !currentState.Value().latch.Value().IsNull())
        {
            VerifyOrReturnError(!currentState.Value().latch.Value().Value(), Status::InvalidInState,
                                ChipLogError(AppServer,
                                             "Step command cannot be processed when current latch is True"
                                             "on endpoint : %d",
                                             GetEndpointId()));
        }
        // Return InvalidInState if currentState is latched
    }

    // Derive TargetState Position from StepValue and NumberOfSteps.
    Percent100ths stepValue = GetStepValue();

    // Convert step to position delta.
    // As StepValue can only take maxvalue of kPercents100thsMaxValue(which is 10000). Below product will be within limits of
    // int32_t
    uint32_t delta       = numberOfSteps * stepValue;
    uint32_t newPosition = 0;

    // check if closure supports Limitation feature, if yes fetch the LimitRange values
    bool limitSupported = mFeatureMap.Has(Feature::kLimitation) ? true : false;

    Structs::RangePercent100thsStruct::Type limitRange;

    if (limitSupported)
    {
        limitRange = GetLimitRange();
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
