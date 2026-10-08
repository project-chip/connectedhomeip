/*
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
 */

#include <app/clusters/water-heater-management-server/WaterHeaterManagementCluster.h>

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/WaterHeaterManagement/Events.h>
#include <clusters/WaterHeaterManagement/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WaterHeaterManagement;
using namespace chip::app::Clusters::WaterHeaterManagement::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

/***************************************************************************
 *
 * The Cluster implementation
 *
 ***************************************************************************/

WaterHeaterManagementCluster::WaterHeaterManagementCluster(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature) :
    DefaultServerCluster({ aEndpointId, WaterHeaterManagement::Id }), mDelegate(aDelegate), mFeature(aFeature)
{
    /* set the base class delegates endpointId */
    mDelegate.SetEndpointId(aEndpointId);
    mDelegate.SetCluster(this);
}

WaterHeaterManagementCluster::~WaterHeaterManagementCluster()
{
    mDelegate.SetCluster(nullptr);
}

bool WaterHeaterManagementCluster::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

CHIP_ERROR WaterHeaterManagementCluster::GenerateBoostStartedEvent(uint32_t durationSecs, Optional<bool> oneShot,
                                                                   Optional<bool> emergencyBoost,
                                                                   Optional<int16_t> temporarySetpoint,
                                                                   Optional<Percent> targetPercentage,
                                                                   Optional<Percent> targetReheat)
{
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Events::BoostStarted::Type event;
    event.boostInfo.duration          = durationSecs;
    event.boostInfo.oneShot           = oneShot;
    event.boostInfo.emergencyBoost    = emergencyBoost;
    event.boostInfo.temporarySetpoint = temporarySetpoint;
    event.boostInfo.targetPercentage  = targetPercentage;
    event.boostInfo.targetReheat      = targetReheat;

    VerifyOrReturnError(mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId).has_value(),
                        CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WaterHeaterManagementCluster::GenerateBoostEndedEvent()
{
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Events::BoostEnded::Type event;
    VerifyOrReturnError(mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId).has_value(),
                        CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

// ServerClusterInterface implementation
DataModel::ActionReturnStatus WaterHeaterManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                          AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case HeaterTypes::Id:
        return encoder.Encode(mDelegate.GetHeaterTypes());
    case HeatDemand::Id:
        return encoder.Encode(mDelegate.GetHeatDemand());
    case TankVolume::Id:
        return encoder.Encode(mDelegate.GetTankVolume());
    case EstimatedHeatRequired::Id:
        return encoder.Encode(mDelegate.GetEstimatedHeatRequired());
    case TankPercentage::Id:
        return encoder.Encode(mDelegate.GetTankPercentage());
    case BoostState::Id:
        return encoder.Encode(mDelegate.GetBoostState());
    case FeatureMap::Id:
        return encoder.Encode(mFeature);
    case ClusterRevision::Id:
        return encoder.Encode(WaterHeaterManagement::kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> WaterHeaterManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                         chip::TLV::TLVReader & input_arguments,
                                                                                         CommandHandler * handler)
{
    using namespace Commands;

    switch (request.path.mCommandId)
    {
    case Boost::Id: {
        Commands::Boost::DecodableType boostReq;
        ReturnErrorOnFailure(boostReq.Decode(input_arguments));
        return HandleBoost(boostReq);
    }
    case CancelBoost::Id: {
        Commands::CancelBoost::DecodableType cancelBoostReq;
        ReturnErrorOnFailure(cancelBoostReq.Decode(input_arguments));
        return HandleCancelBoost(cancelBoostReq);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

Status WaterHeaterManagementCluster::HandleBoost(const Commands::Boost::DecodableType & commandData)
{
    uint32_t duration                   = commandData.boostInfo.duration;
    Optional<bool> oneShot              = commandData.boostInfo.oneShot;
    Optional<bool> emergencyBoost       = commandData.boostInfo.emergencyBoost;
    Optional<int16_t> temporarySetpoint = commandData.boostInfo.temporarySetpoint;
    Optional<Percent> targetPercentage  = commandData.boostInfo.targetPercentage;
    Optional<Percent> targetReheat      = commandData.boostInfo.targetReheat;

    //  Notify the appliance if the appliance hardware cannot be adjusted, then return Failure
    if (HasFeature(WaterHeaterManagement::Feature::kTankPercent))
    {
        if (targetPercentage.HasValue())
        {
            if (targetPercentage.Value() > 100)
            {
                ChipLogError(Zcl, "Bad targetPercentage %u", targetPercentage.Value());
                return Status::InvalidCommand;
            }
        }

        if (targetReheat.HasValue())
        {
            if (targetReheat.Value() > 100)
            {
                ChipLogError(Zcl, "Bad targetReheat %u", targetReheat.Value());
                return Status::InvalidCommand;
            }

            if (!targetPercentage.HasValue())
            {
                ChipLogError(Zcl, "targetPercentage must be specified if targetReheat specified");
                return Status::InvalidCommand;
            }

            if (oneShot.HasValue())
            {
                ChipLogError(Zcl, "Cannot specify targetReheat with targetPercentage and oneShot. oneShot must be excluded");
                return Status::InvalidCommand;
            }
        }
    }
    else if (targetPercentage.HasValue() || targetReheat.HasValue())
    {
        ChipLogError(Zcl, "Cannot specify targetPercentage or targetReheat if the feature TankPercent is not supported");
        return Status::InvalidCommand;
    }

    Status status = mDelegate.HandleBoost(duration, oneShot, emergencyBoost, temporarySetpoint, targetPercentage, targetReheat);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "WHM: Boost command failed. status " ChipLogFormatIMStatus, ChipLogValueIMStatus(status));
    }
    return status;
}

Status WaterHeaterManagementCluster::HandleCancelBoost(const Commands::CancelBoost::DecodableType & commandData)
{
    Status status = mDelegate.HandleCancelBoost();
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "WHM: CancelBoost command failed. status " ChipLogFormatIMStatus, ChipLogValueIMStatus(status));
    }
    return status;
}

CHIP_ERROR WaterHeaterManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                    ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // This cluster only has Mandatory attributes
    AttributeListBuilder listBuilder(builder);

    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFeature.Has(Feature::kEnergyManagement), TankVolume::kMetadataEntry },
        { mFeature.Has(Feature::kEnergyManagement), EstimatedHeatRequired::kMetadataEntry },
        { mFeature.Has(Feature::kTankPercent), TankPercentage::kMetadataEntry },
    };

    return listBuilder.Append(Span(WaterHeaterManagement::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR WaterHeaterManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        Commands::Boost::kMetadataEntry,
        Commands::CancelBoost::kMetadataEntry,
    };
    return builder.ReferenceExisting(Span(kCommands));
}

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
