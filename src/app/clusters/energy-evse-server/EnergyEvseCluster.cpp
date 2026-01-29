/*
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

#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/energy-evse-server/EnergyEvseCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/EnergyEvse/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::EnergyEvse::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

CHIP_ERROR EnergyEvseCluster::Startup(ServerClusterContext & context)
{
    if (mDelegate.GetEndpointId() != mPath.mEndpointId)
    {
        ChipLogError(Zcl, "EVSE: EndpointId mismatch - delegate has %d, cluster has %d", mDelegate.GetEndpointId(),
                     mPath.mEndpointId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return DefaultServerCluster::Startup(context);
}

DataModel::ActionReturnStatus EnergyEvseCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mFeatureFlags);
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case State::Id:
        return encoder.Encode(mDelegate.GetState());
    case SupplyState::Id:
        return encoder.Encode(mDelegate.GetSupplyState());
    case FaultState::Id:
        return encoder.Encode(mDelegate.GetFaultState());
    case ChargingEnabledUntil::Id:
        return encoder.Encode(mDelegate.GetChargingEnabledUntil());
    case DischargingEnabledUntil::Id:
        return encoder.Encode(mDelegate.GetDischargingEnabledUntil());
    case CircuitCapacity::Id:
        return encoder.Encode(mDelegate.GetCircuitCapacity());
    case MinimumChargeCurrent::Id:
        return encoder.Encode(mDelegate.GetMinimumChargeCurrent());
    case MaximumChargeCurrent::Id:
        return encoder.Encode(mDelegate.GetMaximumChargeCurrent());
    case MaximumDischargeCurrent::Id:
        return encoder.Encode(mDelegate.GetMaximumDischargeCurrent());
    case UserMaximumChargeCurrent::Id:
        return encoder.Encode(mDelegate.GetUserMaximumChargeCurrent());
    case RandomizationDelayWindow::Id:
        return encoder.Encode(mDelegate.GetRandomizationDelayWindow());
    case NextChargeStartTime::Id:
        return encoder.Encode(mDelegate.GetNextChargeStartTime());
    case NextChargeTargetTime::Id:
        return encoder.Encode(mDelegate.GetNextChargeTargetTime());
    case NextChargeRequiredEnergy::Id:
        return encoder.Encode(mDelegate.GetNextChargeRequiredEnergy());
    case NextChargeTargetSoC::Id:
        return encoder.Encode(mDelegate.GetNextChargeTargetSoC());
    case ApproximateEVEfficiency::Id:
        return encoder.Encode(mDelegate.GetApproximateEVEfficiency());
    case StateOfCharge::Id:
        return encoder.Encode(mDelegate.GetStateOfCharge());
    case BatteryCapacity::Id:
        return encoder.Encode(mDelegate.GetBatteryCapacity());
    case VehicleID::Id:
        return encoder.Encode(mDelegate.GetVehicleID());
    case SessionID::Id:
        return encoder.Encode(mDelegate.GetSessionID());
    case SessionDuration::Id:
        return encoder.Encode(mDelegate.GetSessionDuration());
    case SessionEnergyCharged::Id:
        return encoder.Encode(mDelegate.GetSessionEnergyCharged());
    case SessionEnergyDischarged::Id:
        return encoder.Encode(mDelegate.GetSessionEnergyDischarged());
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus EnergyEvseCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case UserMaximumChargeCurrent::Id: {
        UserMaximumChargeCurrent::TypeInfo::DecodableType value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return mDelegate.SetUserMaximumChargeCurrent(value);
    }
    case RandomizationDelayWindow::Id: {
        RandomizationDelayWindow::TypeInfo::DecodableType value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return mDelegate.SetRandomizationDelayWindow(value);
    }
    case ApproximateEVEfficiency::Id: {
        ApproximateEVEfficiency::TypeInfo::DecodableType value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return mDelegate.SetApproximateEVEfficiency(value);
    }
    default:
        return Status::UnsupportedAttribute;
    }
}
std::optional<DataModel::ActionReturnStatus> EnergyEvseCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                              TLV::TLVReader & input_arguments,
                                                                              CommandHandler * handler)
{
    using namespace Commands;

    switch (request.path.mCommandId)
    {
    case Disable::Id:
        return HandleDisable(request, input_arguments, handler);
    case EnableCharging::Id:
        return HandleEnableCharging(request, input_arguments, handler);
    case EnableDischarging::Id:
        return HandleEnableDischarging(request, input_arguments, handler);
    case StartDiagnostics::Id:
        return HandleStartDiagnostics(request, input_arguments, handler);
    case SetTargets::Id:
        return HandleSetTargets(request, input_arguments, handler);
    case GetTargets::Id:
        return HandleGetTargets(request, input_arguments, handler);
    case ClearTargets::Id:
        return HandleClearTargets(request, input_arguments, handler);
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR EnergyEvseCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        // V2x feature attributes
        { mFeatureFlags.Has(Feature::kV2x), DischargingEnabledUntil::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kV2x), MaximumDischargeCurrent::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kV2x), SessionEnergyDischarged::kMetadataEntry },
        // ChargingPreferences feature attributes
        { mFeatureFlags.Has(Feature::kChargingPreferences), NextChargeStartTime::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kChargingPreferences), NextChargeTargetTime::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kChargingPreferences), NextChargeRequiredEnergy::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kChargingPreferences), NextChargeTargetSoC::kMetadataEntry },
        { (mFeatureFlags.Has(Feature::kChargingPreferences) &&
           mOptionalAttrs.Has(OptionalAttributes::kSupportsApproximateEvEfficiency)),
          ApproximateEVEfficiency::kMetadataEntry },
        // SoCReporting feature attributes
        { mFeatureFlags.Has(Feature::kSoCReporting), StateOfCharge::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kSoCReporting), BatteryCapacity::kMetadataEntry },
        // PlugAndCharge feature attribute
        { mFeatureFlags.Has(Feature::kPlugAndCharge), VehicleID::kMetadataEntry },
        // Optional attributes (not feature-based)
        { mOptionalAttrs.Has(OptionalAttributes::kSupportsUserMaximumChargingCurrent), UserMaximumChargeCurrent::kMetadataEntry },
        { mOptionalAttrs.Has(OptionalAttributes::kSupportsRandomizationWindow), RandomizationDelayWindow::kMetadataEntry },
    };
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR EnergyEvseCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace Commands;
    ReturnErrorOnFailure(builder.AppendElements({ Disable::kMetadataEntry, EnableCharging::kMetadataEntry }));
    if (mFeatureFlags.Has(Feature::kV2x))
    {
        ReturnErrorOnFailure(builder.AppendElements({ EnableDischarging::kMetadataEntry }));
    }
    if (mOptionalCmds.Has(OptionalCommands::kSupportsStartDiagnostics))
    {
        ReturnErrorOnFailure(builder.AppendElements({ StartDiagnostics::kMetadataEntry }));
    }
    if (mFeatureFlags.Has(Feature::kChargingPreferences))
    {
        ReturnErrorOnFailure(
            builder.AppendElements({ SetTargets::kMetadataEntry, GetTargets::kMetadataEntry, ClearTargets::kMetadataEntry }));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    using namespace Commands;
    if (mFeatureFlags.Has(Feature::kChargingPreferences))
    {
        ReturnErrorOnFailure(builder.AppendElements({ GetTargetsResponse::Id }));
    }
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleDisable(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    return mDelegate.Disable();
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleEnableCharging(const DataModel::InvokeRequest & request,
                                                                      TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    Commands::EnableCharging::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    auto & chargingEnabledUntil = commandData.chargingEnabledUntil;
    auto & minimumChargeCurrent = commandData.minimumChargeCurrent;
    auto & maximumChargeCurrent = commandData.maximumChargeCurrent;

    VerifyOrReturnValue(minimumChargeCurrent >= kMinimumChargeCurrentLimit, Status::ConstraintError);
    VerifyOrReturnValue(maximumChargeCurrent >= kMinimumChargeCurrentLimit, Status::ConstraintError);
    VerifyOrReturnValue(minimumChargeCurrent <= maximumChargeCurrent, Status::ConstraintError);

    return mDelegate.EnableCharging(chargingEnabledUntil, minimumChargeCurrent, maximumChargeCurrent);
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleEnableDischarging(const DataModel::InvokeRequest & request,
                                                                         TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    Commands::EnableDischarging::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    auto & dischargingEnabledUntil = commandData.dischargingEnabledUntil;
    auto & maximumDischargeCurrent = commandData.maximumDischargeCurrent;

    VerifyOrReturnValue(maximumDischargeCurrent >= kMinimumChargeCurrentLimit, Status::ConstraintError);

    return mDelegate.EnableDischarging(dischargingEnabledUntil, maximumDischargeCurrent);
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleStartDiagnostics(const DataModel::InvokeRequest & request,
                                                                        TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    return mDelegate.StartDiagnostics();
}

Status EnergyEvseCluster::ValidateTargets(
    const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules)
{
    uint8_t dayOfWeekBitmap = 0;

    auto iter = chargingTargetSchedules.begin();
    while (iter.Next())
    {
        auto & entry    = iter.GetValue();
        uint8_t bitmask = entry.dayOfWeekForSequence.GetField(static_cast<TargetDayOfWeekBitmap>(kDayOfWeekBitmapMask));
        ChipLogProgress(AppServer, "DayOfWeekForSequence = 0x%02x", bitmask);

        VerifyOrReturnValue((dayOfWeekBitmap & bitmask) == 0, Status::ConstraintError,
                            ChipLogError(AppServer, "DayOfWeekForSequence bit already set"));
        dayOfWeekBitmap |= bitmask;

        auto iterInner   = entry.chargingTargets.begin();
        uint8_t innerIdx = 0;
        while (iterInner.Next())
        {
            auto & targetStruct          = iterInner.GetValue();
            uint16_t minutesPastMidnight = targetStruct.targetTimeMinutesPastMidnight;
            ChipLogProgress(AppServer, "[%d] MinutesPastMidnight : %d", innerIdx,
                            static_cast<short unsigned int>(minutesPastMidnight));

            VerifyOrReturnValue(minutesPastMidnight <= kMaxMinutesPastMidnight, Status::ConstraintError,
                                ChipLogError(AppServer, "MinutesPastMidnight invalid: %d", static_cast<int>(minutesPastMidnight)));

            if (mFeatureFlags.Has(Feature::kSoCReporting))
            {
                VerifyOrReturnValue(targetStruct.targetSoC.HasValue(), Status::InvalidCommand,
                                    ChipLogError(AppServer, "SoCReporting enabled but TargetSoC missing"));
                VerifyOrReturnValue(
                    targetStruct.targetSoC.Value() <= kMaxTargetSoCPercent, Status::ConstraintError,
                    ChipLogError(AppServer, "TargetSoC invalid: %d", static_cast<int>(targetStruct.targetSoC.Value())));
            }
            else
            {
                VerifyOrReturnValue(!targetStruct.targetSoC.HasValue() || targetStruct.targetSoC.Value() == kMaxTargetSoCPercent,
                                    Status::ConstraintError,
                                    ChipLogError(AppServer, "TargetSoC must be 100 if SOC feature not supported"));
            }

            VerifyOrReturnValue(targetStruct.targetSoC.HasValue() || targetStruct.addedEnergy.HasValue(), Status::Failure,
                                ChipLogError(AppServer, "Must have one of AddedEnergy or TargetSoC"));

            VerifyOrReturnValue(
                !targetStruct.addedEnergy.HasValue() || targetStruct.addedEnergy.Value() >= 0, Status::ConstraintError,
                ChipLogError(AppServer, "AddedEnergy invalid: %ld", static_cast<long>(targetStruct.addedEnergy.Value())));
            innerIdx++;
        }

        VerifyOrReturnValue(innerIdx <= kEvseTargetsMaxTargetsPerDay, Status::ResourceExhausted,
                            ChipLogError(AppServer, "Too many targets: %d", innerIdx));
        VerifyOrReturnValue(iterInner.GetStatus() == CHIP_NO_ERROR, Status::InvalidCommand);
    }

    VerifyOrReturnValue(iter.GetStatus() == CHIP_NO_ERROR, Status::InvalidCommand);

    return Status::Success;
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleSetTargets(const DataModel::InvokeRequest & request,
                                                                  TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    Commands::SetTargets::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    auto & chargingTargetSchedules = commandData.chargingTargetSchedules;

    Status status = ValidateTargets(chargingTargetSchedules);
    VerifyOrReturnValue(status == Status::Success, status, ChipLogError(AppServer, "SetTargets validation failed"));

    return mDelegate.SetTargets(chargingTargetSchedules);
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleGetTargets(const DataModel::InvokeRequest & request,
                                                                  TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    Commands::GetTargetsResponse::Type response;
    Status status = mDelegate.GetTargets(response.chargingTargetSchedules);
    VerifyOrReturnValue(status == Status::Success, status);

    handler->AddResponse(request.path, response);
    return Status::Success;
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleClearTargets(const DataModel::InvokeRequest & request,
                                                                    TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    return mDelegate.ClearTargets();
}

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
