/*
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

#include <app/clusters/device-energy-management-server/DeviceEnergyManagementCluster.h>

#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/DeviceEnergyManagement/Attributes.h>
#include <clusters/DeviceEnergyManagement/Commands.h>
#include <clusters/DeviceEnergyManagement/Metadata.h>
#include <clusters/DeviceEnergyManagement/Structs.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>

using chip::Protocols::InteractionModel::Status;
namespace chip {
namespace app {
namespace Clusters {

namespace {

bool IsWithinRange(const int64_t power, const uint32_t duration,
                   const DeviceEnergyManagement::Structs::PowerAdjustCapabilityStruct::Type & powerAdjustmentCapability)
{
    if (powerAdjustmentCapability.powerAdjustCapability.IsNull())
    {
        return false;
    }

    for (const auto & pas : powerAdjustmentCapability.powerAdjustCapability.Value())
    {
        if ((power >= pas.minPower) && (duration >= pas.minDuration) && (power <= pas.maxPower) && (duration <= pas.maxDuration))
        {
            return true;
        }
    }
    return false;
}

// Helper function to validate that the ESA is in the expected state before processing a command
DataModel::ActionReturnStatus ValidateESAState(const DataModel::InvokeRequest & request, CommandHandler * handler,
                                               DeviceEnergyManagement::Delegate & delegate,
                                               DeviceEnergyManagement::ESAStateEnum expectedState)
{
    DeviceEnergyManagement::ESAStateEnum currentState = delegate.GetESAState();
    if (currentState != expectedState)
    {
        ChipLogError(Zcl, "DEM: ESAState mismatch - expected %d, got %d", static_cast<int>(expectedState),
                     static_cast<int>(currentState));
        return Status::InvalidInState;
    }
    return Status::Success;
}
} // namespace

using namespace DeviceEnergyManagement;
using namespace DeviceEnergyManagement::Attributes;

DataModel::ActionReturnStatus DeviceEnergyManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mFeatureFlags);

    case ClusterRevision::Id:
        return encoder.Encode(kRevision);

    case ESAType::Id:
        return encoder.Encode(mDelegate.GetESAType());

    case ESACanGenerate::Id:
        return encoder.Encode(mDelegate.GetESACanGenerate());

    case ESAState::Id:
        return encoder.Encode(mDelegate.GetESAState());

    case AbsMinPower::Id:
        return encoder.Encode(mDelegate.GetAbsMinPower());

    case AbsMaxPower::Id:
        return encoder.Encode(mDelegate.GetAbsMaxPower());

    case PowerAdjustmentCapability::Id:
        return encoder.Encode(mDelegate.GetPowerAdjustmentCapability());

    case Forecast::Id:
        return encoder.Encode(mDelegate.GetForecast());

    case OptOutState::Id:
        return encoder.Encode(mDelegate.GetOptOutState());

    default:
        return Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> DeviceEnergyManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                          TLV::TLVReader & input_arguments,
                                                                                          CommandHandler * handler)
{
    using namespace Commands;

    switch (request.path.mCommandId)
    {
    case PowerAdjustRequest::Id:
        return HandlePowerAdjustRequest(request, input_arguments, handler);

    case CancelPowerAdjustRequest::Id:
        return HandleCancelPowerAdjustRequest(request, input_arguments, handler);

    case StartTimeAdjustRequest::Id:
        return HandleStartTimeAdjustRequest(request, input_arguments, handler);

    case PauseRequest::Id:
        return HandlePauseRequest(request, input_arguments, handler);

    case ResumeRequest::Id:
        return HandleResumeRequest(request, input_arguments, handler);

    case ModifyForecastRequest::Id:
        return HandleModifyForecastRequest(request, input_arguments, handler);

    case RequestConstraintBasedForecast::Id:
        return HandleRequestConstraintBasedForecast(request, input_arguments, handler);

    case CancelRequest::Id:
        return HandleCancelRequest(request, input_arguments, handler);

    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR DeviceEnergyManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    DataModel::AttributeEntry optionalAttributes[] = {
        PowerAdjustmentCapability::kMetadataEntry,
        Forecast::kMetadataEntry,
        OptOutState::kMetadataEntry,
    };

    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mEnabledOptionalAttributes);
}

CHIP_ERROR DeviceEnergyManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                           ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace Commands;

    if (mFeatureFlags.Has(Feature::kPowerAdjustment))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            PowerAdjustRequest::kMetadataEntry,
            CancelPowerAdjustRequest::kMetadataEntry,
        }));
    }

    if (mFeatureFlags.Has(Feature::kStartTimeAdjustment))
    {
        ReturnErrorOnFailure(builder.AppendElements({ StartTimeAdjustRequest::kMetadataEntry }));
    }

    if (mFeatureFlags.Has(Feature::kPausable))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            PauseRequest::kMetadataEntry,
            ResumeRequest::kMetadataEntry,
        }));
    }

    if (mFeatureFlags.Has(Feature::kForecastAdjustment))
    {
        ReturnErrorOnFailure(builder.AppendElements({ ModifyForecastRequest::kMetadataEntry }));
    }

    if (mFeatureFlags.Has(Feature::kConstraintBasedAdjustment))
    {
        ReturnErrorOnFailure(builder.AppendElements({ RequestConstraintBasedForecast::kMetadataEntry }));
    }

    if (mFeatureFlags.HasAny(Feature::kStartTimeAdjustment, Feature::kForecastAdjustment, Feature::kConstraintBasedAdjustment))
    {
        ReturnErrorOnFailure(builder.AppendElements({ CancelRequest::kMetadataEntry }));
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus
DeviceEnergyManagementCluster::CheckOptOutAllowsRequest(DeviceEnergyManagement::AdjustmentCauseEnum adjustmentCause)
{
    OptOutStateEnum optOutState = mDelegate.GetOptOutState();

    if (adjustmentCause == AdjustmentCauseEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "DEM: adjustment cause is invalid (%d)", static_cast<int>(adjustmentCause));
        return Status::InvalidValue;
    }

    switch (optOutState)
    {
    case OptOutStateEnum::kNoOptOut: /* User has NOT opted out so allow it */
        ChipLogProgress(Zcl, "DEM: OptOutState = kNoOptOut");
        return DataModel::ActionReturnStatus(Status::Success);

    case OptOutStateEnum::kLocalOptOut: /* User has opted out from Local only*/
        ChipLogProgress(Zcl, "DEM: OptOutState = kLocalOptOut");
        switch (adjustmentCause)
        {
        case AdjustmentCauseEnum::kGridOptimization:
            return DataModel::ActionReturnStatus(Status::Success);
        case AdjustmentCauseEnum::kLocalOptimization:
        default:
            return DataModel::ActionReturnStatus(Status::ConstraintError);
        }

    case OptOutStateEnum::kGridOptOut: /* User has opted out from Grid only */
        ChipLogProgress(Zcl, "DEM: OptOutState = kGridOptOut");
        switch (adjustmentCause)
        {
        case AdjustmentCauseEnum::kLocalOptimization:
            return DataModel::ActionReturnStatus(Status::Success);
        case AdjustmentCauseEnum::kGridOptimization:
        default:
            return DataModel::ActionReturnStatus(Status::ConstraintError);
        }

    case OptOutStateEnum::kOptOut: /* User has opted out from both local and grid */
        ChipLogProgress(Zcl, "DEM: OptOutState = kOptOut");
        return DataModel::ActionReturnStatus(Status::ConstraintError);

    default:
        ChipLogError(Zcl, "DEM: invalid optOutState %d", static_cast<int>(optOutState));
        return DataModel::ActionReturnStatus(Status::InvalidValue);
    }
}

DataModel::ActionReturnStatus DeviceEnergyManagementCluster::HandlePowerAdjustRequest(const DataModel::InvokeRequest & request,
                                                                                      TLV::TLVReader & input_arguments,
                                                                                      CommandHandler * handler)
{
    using namespace Commands;

    PowerAdjustRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    ReturnErrorOnFailure(CheckOptOutAllowsRequest(commandData.cause).GetUnderlyingError());

    DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> powerAdjustmentCapability =
        mDelegate.GetPowerAdjustmentCapability();
    if (powerAdjustmentCapability.IsNull())
    {
        ChipLogError(Zcl, "DEM: PowerAdjustmentCapability is Null");
        return Status::ConstraintError;
    }

    if (!IsWithinRange(commandData.power, commandData.duration, powerAdjustmentCapability.Value()))
    {
        ChipLogError(Zcl, "DEM: Power is not within range- power: '%" PRIdLEAST64 "', duration: '%" PRIdLEAST32 "'",
                     commandData.power, commandData.duration);
        return Status::ConstraintError;
    }

    ESAStateEnum ESAState = mDelegate.GetESAState();
    if (ESAState != ESAStateEnum::kOnline && ESAState != ESAStateEnum::kPowerAdjustActive)
    {
        return Status::InvalidInState;
    }

    // Call on delegate to start the power adjustment if the ESAState PowerAdjustActive, the delegate might refuse the adjustment
    // The delegate is responsible of updating its PowerAdjustmentCapability's cause to the new cause if the adjustment is accepted
    ReturnErrorOnFailure(
        DataModel::ActionReturnStatus(mDelegate.PowerAdjustRequest(commandData.power, commandData.duration, commandData.cause))
            .GetUnderlyingError());

    // Verify the delegate's PowerAdjustmentCapability's cause was updated to the new cause if the adjustment is accepted
    powerAdjustmentCapability = mDelegate.GetPowerAdjustmentCapability();
    if (powerAdjustmentCapability.IsNull())
    {
        ChipLogError(Zcl, "DEM: PowerAdjustmentCapability is Null");
        return Status::ConstraintError;
    }
    if (powerAdjustmentCapability.Value().cause != static_cast<DeviceEnergyManagement::PowerAdjustReasonEnum>(commandData.cause))
    {
        ChipLogError(Zcl,
                     "DEM: PowerAdjustmentCapability's cause was not updated to the new cause- expected: '0x%" PRIx32
                     "', got: '0x%" PRIx32 "'",
                     static_cast<uint32_t>(commandData.cause), static_cast<uint32_t>(powerAdjustmentCapability.Value().cause));
        return Status::ConstraintError;
    }

    return Status::Success;
}

DataModel::ActionReturnStatus
DeviceEnergyManagementCluster::HandleCancelPowerAdjustRequest(const DataModel::InvokeRequest & request,
                                                              TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    using namespace Commands;

    CancelPowerAdjustRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    ReturnErrorOnFailure(ValidateESAState(request, handler, mDelegate, ESAStateEnum::kPowerAdjustActive).GetUnderlyingError());

    ReturnErrorOnFailure(DataModel::ActionReturnStatus(mDelegate.CancelPowerAdjustRequest()).GetUnderlyingError());

    // Verify the delegate's PowerAdjustmentCapability's cause was updated to the new cause if the adjustment is accepted
    DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> powerAdjustmentCapability =
        mDelegate.GetPowerAdjustmentCapability();
    if (powerAdjustmentCapability.IsNull())
    {
        ChipLogError(Zcl, "DEM: PowerAdjustmentCapability is Null");
        return Status::ConstraintError;
    }

    if (powerAdjustmentCapability.Value().cause != PowerAdjustReasonEnum::kNoAdjustment)
    {
        ChipLogError(Zcl,
                     "DEM: PowerAdjustmentCapability's cause was not updated to the new cause- expected: '0x%" PRIx32
                     "', got: '0x%" PRIx32 "'",
                     static_cast<uint32_t>(PowerAdjustReasonEnum::kNoAdjustment),
                     static_cast<uint32_t>(powerAdjustmentCapability.Value().cause));
        return Status::ConstraintError;
    }

    return Status::Success;
}

DataModel::ActionReturnStatus DeviceEnergyManagementCluster::HandleStartTimeAdjustRequest(const DataModel::InvokeRequest & request,
                                                                                          TLV::TLVReader & input_arguments,
                                                                                          CommandHandler * handler)
{
    using namespace Commands;

    StartTimeAdjustRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    ReturnErrorOnFailure(CheckOptOutAllowsRequest(commandData.cause).GetUnderlyingError());

    DataModel::Nullable<Structs::ForecastStruct::Type> forecastNullable = mDelegate.GetForecast();
    if (forecastNullable.IsNull())
    {
        ChipLogError(Zcl, "DEM: Forecast is Null");
        return Status::Failure;
    }

    auto & forecast = forecastNullable.Value();

    // earliestStartTime is optional based on the StartTimeAdjust (STA) feature AND is nullable
    if (!forecast.earliestStartTime.HasValue() || !forecast.latestEndTime.HasValue())
    {
        ChipLogError(Zcl, "DEM: EarliestStartTime / LatestEndTime do not have values");
        return Status::Failure;
    }

    DataModel::Nullable<uint32_t> & earliestStartTimeNullable = forecast.earliestStartTime.Value();
    uint32_t latestEndTimeEpoch                               = forecast.latestEndTime.Value();
    uint32_t earliestStartTimeEpoch;

    if (earliestStartTimeNullable.IsNull())
    {
        uint32_t matterEpoch = 0;
        CHIP_ERROR err       = System::Clock::GetClock_MatterEpochS(matterEpoch);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "DEM: Unable to get current time - err:%" CHIP_ERROR_FORMAT, err.Format());
            return Status::Failure;
        }
        earliestStartTimeEpoch = matterEpoch; // Null means we can start immediately (NOW)
    }
    else
    {
        earliestStartTimeEpoch = earliestStartTimeNullable.Value();
    }

    uint32_t duration = forecast.endTime - forecast.startTime;
    if (commandData.requestedStartTime < earliestStartTimeEpoch)
    {
        ChipLogError(Zcl, "DEM: Bad requestedStartTime %" PRIu32 ", earlier than earliestStartTime %" PRIu32 ".",
                     commandData.requestedStartTime, earliestStartTimeEpoch);
        return Status::ConstraintError;
    }

    if ((commandData.requestedStartTime + duration) > latestEndTimeEpoch)
    {
        ChipLogError(Zcl, "DEM: Bad requestedStartTimeEpoch + duration %" PRIu32 ", later than latestEndTime %" PRIu32 ".",
                     commandData.requestedStartTime + duration, latestEndTimeEpoch);
        return Status::ConstraintError;
    }

    // Store original forecastID to verify it was incremented
    uint32_t originalForecastID = forecast.forecastID;

    ReturnErrorOnFailure(
        DataModel::ActionReturnStatus(mDelegate.StartTimeAdjustRequest(commandData.requestedStartTime, commandData.cause))
            .GetUnderlyingError());

    // Verify the delegate updated the Forecast attribute as required
    DataModel::Nullable<Structs::ForecastStruct::Type> updatedForecastNullable = mDelegate.GetForecast();
    if (updatedForecastNullable.IsNull())
    {
        ChipLogError(Zcl, "DEM: Forecast is Null after StartTimeAdjustRequest");
        return Status::Failure;
    }

    auto & updatedForecast = updatedForecastNullable.Value();

    // Verify Forecast.startTime was updated to requestedStartTime
    if (updatedForecast.startTime != commandData.requestedStartTime)
    {
        ChipLogError(Zcl, "DEM: Forecast.startTime was not updated to requestedStartTime - expected: %" PRIu32 ", got: %" PRIu32,
                     commandData.requestedStartTime, updatedForecast.startTime);
        return Status::ConstraintError;
    }

    // Verify Forecast.forecastID was incremented (new ForecastID)
    if (updatedForecast.forecastID <= originalForecastID)
    {
        ChipLogError(Zcl, "DEM: Forecast.forecastID was not incremented - original: %" PRIu32 ", got: %" PRIu32, originalForecastID,
                     updatedForecast.forecastID);
        return Status::ConstraintError;
    }

    // Verify Forecast.endTime was updated to requestedStartTime + duration
    uint32_t expectedEndTime = commandData.requestedStartTime + duration;
    if (updatedForecast.endTime != expectedEndTime)
    {
        ChipLogError(Zcl, "DEM: Forecast.endTime was not updated correctly - expected: %" PRIu32 ", got: %" PRIu32, expectedEndTime,
                     updatedForecast.endTime);
        return Status::ConstraintError;
    }

    return Status::Success;
}

DataModel::ActionReturnStatus DeviceEnergyManagementCluster::HandlePauseRequest(const DataModel::InvokeRequest & request,
                                                                                TLV::TLVReader & input_arguments,
                                                                                CommandHandler * handler)
{
    using namespace Commands;

    PauseRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    ReturnErrorOnFailure(CheckOptOutAllowsRequest(commandData.cause).GetUnderlyingError());

    DataModel::Nullable<Structs::ForecastStruct::Type> forecast = mDelegate.GetForecast();
    if (forecast.IsNull())
    {
        ChipLogError(Zcl, "DEM: Forecast is Null");
        return Status::Failure;
    }

    // Value SHALL be between the MinPauseDuration and MaxPauseDuration indicated in the
    // ActiveSlotNumber index in the Slots list in the Forecast
    if (forecast.Value().activeSlotNumber.IsNull())
    {
        ChipLogError(Zcl, "DEM: activeSlotNumber Is Null");
        return Status::Failure;
    }

    uint16_t activeSlotNumber = forecast.Value().activeSlotNumber.Value();
    if (activeSlotNumber >= forecast.Value().slots.size())
    {
        ChipLogError(Zcl, "DEM: Bad activeSlotNumber %d, size()=%d.", activeSlotNumber,
                     static_cast<int>(forecast.Value().slots.size()));
        return Status::Failure;
    }

    const auto & activeSlot = forecast.Value().slots[activeSlotNumber];

    if (!activeSlot.slotIsPausable.HasValue())
    {
        ChipLogError(Zcl, "DEM: activeSlotNumber %d does not include slotIsPausable.", activeSlotNumber);
        return Status::Failure;
    }

    if (!activeSlot.minPauseDuration.HasValue())
    {
        ChipLogError(Zcl, "DEM: activeSlotNumber %d does not include minPauseDuration.", activeSlotNumber);
        return Status::Failure;
    }

    if (!activeSlot.maxPauseDuration.HasValue())
    {
        ChipLogError(Zcl, "DEM: activeSlotNumber %d does not include maxPauseDuration.", activeSlotNumber);
        return Status::Failure;
    }

    if (!activeSlot.slotIsPausable.Value())
    {
        ChipLogError(Zcl, "DEM: activeSlotNumber %d is NOT pausable.", activeSlotNumber);
        return Status::Failure;
    }

    if ((commandData.duration < activeSlot.minPauseDuration.Value()) ||
        (commandData.duration > activeSlot.maxPauseDuration.Value()))
    {
        ChipLogError(Zcl, "DEM: out of range pause duration %" PRIu32, commandData.duration);
        return Status::ConstraintError;
    }

    ReturnErrorOnFailure(
        DataModel::ActionReturnStatus(mDelegate.PauseRequest(commandData.duration, commandData.cause)).GetUnderlyingError());

    return Status::Success;
}

DataModel::ActionReturnStatus DeviceEnergyManagementCluster::HandleResumeRequest(const DataModel::InvokeRequest & request,
                                                                                 TLV::TLVReader & input_arguments,
                                                                                 CommandHandler * handler)
{
    using namespace Commands;

    ResumeRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    ReturnErrorOnFailure(ValidateESAState(request, handler, mDelegate, ESAStateEnum::kPaused).GetUnderlyingError());

    ReturnErrorOnFailure(DataModel::ActionReturnStatus(mDelegate.ResumeRequest()).GetUnderlyingError());

    // Verify the Delegate updated its state and ForecastUpdateReason
    VerifyOrReturnError(mDelegate.GetESAState() != ESAStateEnum::kPaused, Status::InvalidInState);
    DataModel::Nullable<Structs::ForecastStruct::Type> forecast = mDelegate.GetForecast();
    VerifyOrReturnError(!forecast.IsNull(), Status::Failure);
    VerifyOrReturnError(forecast.Value().forecastUpdateReason == ForecastUpdateReasonEnum::kInternalOptimization,
                        Status::InvalidInState);

    return Status::Success;
}

DataModel::ActionReturnStatus DeviceEnergyManagementCluster::HandleModifyForecastRequest(const DataModel::InvokeRequest & request,
                                                                                         TLV::TLVReader & input_arguments,
                                                                                         CommandHandler * handler)
{
    using namespace Commands;

    ModifyForecastRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    ReturnErrorOnFailure(CheckOptOutAllowsRequest(commandData.cause).GetUnderlyingError());

    DataModel::Nullable<Structs::ForecastStruct::Type> forecast = mDelegate.GetForecast();
    if (forecast.IsNull())
    {
        ChipLogError(Zcl, "DEM: Forecast is Null");
        return Status::Failure;
    }

    // Check the various values in the slot structures
    auto iterator = commandData.slotAdjustments.begin();
    while (iterator.Next())
    {
        const Structs::SlotAdjustmentStruct::Type & slotAdjustment = iterator.GetValue();

        // Check for an invalid slotIndex
        if (slotAdjustment.slotIndex >= forecast.Value().slots.size())
        {
            ChipLogError(Zcl, "DEM: Bad slot index %d", slotAdjustment.slotIndex);
            return Status::Failure;
        }

        // Check to see if trying to modify a slot which has already been run
        if (!forecast.Value().activeSlotNumber.IsNull() && slotAdjustment.slotIndex < forecast.Value().activeSlotNumber.Value())
        {
            ChipLogError(Zcl, "DEM: Modifying already run slot index %d", slotAdjustment.slotIndex);
            return Status::ConstraintError;
        }

        const Structs::SlotStruct::Type & slot = forecast.Value().slots[slotAdjustment.slotIndex];

        // NominalPower is only relevant if PFR is supported
        if (mFeatureFlags.Has(Feature::kPowerForecastReporting))
        {
            if (!slotAdjustment.nominalPower.HasValue() || !slot.minPowerAdjustment.HasValue() ||
                !slot.maxPowerAdjustment.HasValue() || slotAdjustment.nominalPower.Value() < slot.minPowerAdjustment.Value() ||
                slotAdjustment.nominalPower.Value() > slot.maxPowerAdjustment.Value())
            {
                ChipLogError(Zcl, "DEM: Bad nominalPower");
                return Status::ConstraintError;
            }
        }

        if (!slot.minDurationAdjustment.HasValue() || !slot.maxDurationAdjustment.HasValue() ||
            slotAdjustment.duration < slot.minDurationAdjustment.Value() ||
            slotAdjustment.duration > slot.maxDurationAdjustment.Value())
        {
            ChipLogError(Zcl, "DEM: Bad min/max duration");
            return Status::ConstraintError;
        }
    }

    if (iterator.GetStatus() != CHIP_NO_ERROR)
    {
        return Status::InvalidCommand;
    }

    ReturnErrorOnFailure(DataModel::ActionReturnStatus(mDelegate.ModifyForecastRequest(
                                                           commandData.forecastID, commandData.slotAdjustments, commandData.cause))
                             .GetUnderlyingError());

    return Status::Success;
}

DataModel::ActionReturnStatus
DeviceEnergyManagementCluster::HandleRequestConstraintBasedForecast(const DataModel::InvokeRequest & request,
                                                                    TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    using namespace Commands;

    RequestConstraintBasedForecast::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    ReturnErrorOnFailure(CheckOptOutAllowsRequest(commandData.cause).GetUnderlyingError());

    uint32_t currentUtcTime = 0;
    CHIP_ERROR err          = System::Clock::GetClock_MatterEpochS(currentUtcTime);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "DEM: Failed to get UTC time");
        return Status::Failure;
    }

    // Check for invalid power levels and whether the constraint time/duration is in the past
    {
        auto iterator = commandData.constraints.begin();
        if (iterator.Next())
        {
            const Structs::ConstraintsStruct::DecodableType & constraint = iterator.GetValue();

            // Check to see if this constraint is in the past
            if (constraint.startTime < currentUtcTime)
            {
                return Status::ConstraintError;
            }

            if (mFeatureFlags.Has(Feature::kPowerForecastReporting))
            {
                if (!constraint.nominalPower.HasValue())
                {
                    ChipLogError(Zcl, "DEM: RequestConstraintBasedForecast no nominalPower");
                    return Status::InvalidCommand;
                }

                if (constraint.nominalPower.Value() < mDelegate.GetAbsMinPower() ||
                    constraint.nominalPower.Value() > mDelegate.GetAbsMaxPower())
                {
                    ChipLogError(Zcl,
                                 "DEM: RequestConstraintBasedForecast nominalPower " ChipLogFormatX64
                                 " out of range [" ChipLogFormatX64 ", " ChipLogFormatX64 "]",
                                 ChipLogValueX64(constraint.nominalPower.Value()), ChipLogValueX64(mDelegate.GetAbsMinPower()),
                                 ChipLogValueX64(mDelegate.GetAbsMaxPower()));
                    return Status::ConstraintError;
                }

                if (!constraint.maximumEnergy.HasValue())
                {
                    ChipLogError(Zcl, "DEM: RequestConstraintBasedForecast no value for maximumEnergy");
                    return Status::InvalidCommand;
                }
            }

            if (mFeatureFlags.Has(Feature::kStateForecastReporting))
            {
                if (!constraint.loadControl.HasValue())
                {
                    ChipLogError(Zcl, "DEM: RequestConstraintBasedForecast no loadControl");
                    return Status::InvalidCommand;
                }

                if (constraint.loadControl.Value() < -100 || constraint.loadControl.Value() > 100)
                {
                    ChipLogError(Zcl, "DEM: RequestConstraintBasedForecast bad loadControl %d", constraint.loadControl.Value());
                    return Status::ConstraintError;
                }
            }
        }

        if (iterator.GetStatus() != CHIP_NO_ERROR)
        {
            return Status::InvalidCommand;
        }
    }

    // Check for overlapping elements
    {
        auto iterator = commandData.constraints.begin();
        if (iterator.Next())
        {
            // Get the first constraint
            Structs::ConstraintsStruct::DecodableType prevConstraint = iterator.GetValue();

            // Start comparing next vs prev constraints
            while (iterator.Next())
            {
                const Structs::ConstraintsStruct::DecodableType & constraint = iterator.GetValue();
                if (constraint.startTime < prevConstraint.startTime ||
                    prevConstraint.startTime + prevConstraint.duration >= constraint.startTime)
                {
                    ChipLogError(Zcl, "DEM: RequestConstraintBasedForecast overlapping constraint times");
                    return Status::ConstraintError;
                }

                prevConstraint = constraint;
            }
        }

        if (iterator.GetStatus() != CHIP_NO_ERROR)
        {
            return Status::InvalidCommand;
        }
    }

    ReturnErrorOnFailure(
        DataModel::ActionReturnStatus(mDelegate.RequestConstraintBasedForecast(commandData.constraints, commandData.cause))
            .GetUnderlyingError());

    return Status::Success;
}

DataModel::ActionReturnStatus DeviceEnergyManagementCluster::HandleCancelRequest(const DataModel::InvokeRequest & request,
                                                                                 TLV::TLVReader & input_arguments,
                                                                                 CommandHandler * handler)
{
    using namespace Commands;

    CancelRequest::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    DataModel::Nullable<Structs::ForecastStruct::Type> forecast = mDelegate.GetForecast();

    Status status = Status::Failure;
    if (forecast.IsNull())
    {
        ChipLogDetail(Zcl, "Cancelling on a Null forecast!");
        status = Status::Failure;
    }
    else if (forecast.Value().forecastUpdateReason == ForecastUpdateReasonEnum::kInternalOptimization)
    {
        ChipLogDetail(Zcl, "Bad Cancel when ESA ForecastUpdateReason was already Internal Optimization!");
        status = Status::InvalidInState;
    }
    else
    {
        status = mDelegate.CancelRequest();
        if (status == Status::Success)
        {
            DataModel::Nullable<Structs::ForecastStruct::Type> updatedForecast = mDelegate.GetForecast();
            if (updatedForecast.IsNull() ||
                updatedForecast.Value().forecastUpdateReason != ForecastUpdateReasonEnum::kInternalOptimization)
            {
                return Status::Failure;
            }
        }
    }

    return status;
}

} // namespace Clusters
} // namespace app
} // namespace chip
