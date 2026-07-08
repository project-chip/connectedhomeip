/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/proximity-ranging-server/ProximityRangingCluster.h>

#include <algorithm>
#include <app/EventLogging.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ProximityRanging/Events.h>
#include <clusters/ProximityRanging/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

using Status = Protocols::InteractionModel::Status;

namespace {
static constexpr uint8_t kInvalidSessionId = 0;
} // namespace

CHIP_ERROR ProximityRangingCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    CHIP_ERROR err = mDriver.Init(*this);
    if (err != CHIP_NO_ERROR)
    {
        DefaultServerCluster::Shutdown(ClusterShutdownType::kClusterShutdown);
        return err;
    }
    mDriverInitialized = true;
    return CHIP_NO_ERROR;
}

void ProximityRangingCluster::Shutdown(ClusterShutdownType shutdownType)
{
    DefaultServerCluster::Shutdown(shutdownType);
    if (mDriverInitialized)
    {
        mDriver.Shutdown();
        mDriverInitialized = false;
    }
}

DataModel::ActionReturnStatus ProximityRangingCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::RangingCapabilities::Id:
        return mDriver.GetRangingCapabilities(encoder);

    case Attributes::BLEDeviceID::Id: {
        auto config = mDriver.GetBleRbcConfig();
        VerifyOrReturnError(config.has_value(), Status::UnsupportedAttribute);
        return encoder.Encode(config->deviceId);
    }

    case Attributes::WiFiDevIK::Id: {
        auto config = mDriver.GetWiFiUsdConfig();
        VerifyOrReturnError(config.has_value(), Status::UnsupportedAttribute);
        return encoder.Encode(ByteSpan(config->deviceIdentityKey));
    }

    case Attributes::BLTDevIK::Id: {
        auto config = mDriver.GetBltcsConfig();
        VerifyOrReturnError(config.has_value(), Status::UnsupportedAttribute);
        return encoder.Encode(ByteSpan(config->deviceIdentityKey));
    }

    case Attributes::BLTCSSecurityLevel::Id: {
        auto config = mDriver.GetBltcsConfig();
        VerifyOrReturnError(config.has_value(), Status::UnsupportedAttribute);
        return encoder.Encode(config->securityLevel);
    }

    case Attributes::BLTCSModeCapability::Id: {
        auto config = mDriver.GetBltcsConfig();
        VerifyOrReturnError(config.has_value(), Status::UnsupportedAttribute);
        return encoder.Encode(config->modeCapability);
    }

    case Attributes::SessionIDList::Id: {
        const size_t numSessions = mDriver.GetNumActiveSessionIds();
        if (numSessions == 0)
        {
            return encoder.EncodeEmptyList();
        }
        Platform::ScopedMemoryBuffer<uint8_t> buf;
        VerifyOrReturnError(buf.Calloc(numSessions), Status::ResourceExhausted);
        Span<uint8_t> sessionIds(buf.Get(), numSessions);
        ReturnErrorOnFailure(mDriver.GetActiveSessionIds(sessionIds));
        return encoder.EncodeList([&sessionIds](const auto & listEncoder) -> CHIP_ERROR {
            for (size_t i = 0; i < sessionIds.size(); i++)
            {
                ReturnErrorOnFailure(listEncoder.Encode(sessionIds.data()[i]));
            }
            return CHIP_NO_ERROR;
        });
    }

    case Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatureMap);

    case Attributes::ClusterRevision::Id:
        return encoder.Encode(ProximityRanging::kRevision);

    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ProximityRangingCluster::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using OptionalEntry                     = AttributeListBuilder::OptionalAttributeEntry;
    const OptionalEntry featureAttributes[] = {
        { mFeatureMap.Has(Feature::kWiFiUsdProximityDetection), Attributes::WiFiDevIK::kMetadataEntry },
        { mFeatureMap.Has(Feature::kBleBeaconRssi), Attributes::BLEDeviceID::kMetadataEntry },
        { mFeatureMap.Has(Feature::kBluetoothChannelSounding), Attributes::BLTDevIK::kMetadataEntry },
        { mFeatureMap.Has(Feature::kBluetoothChannelSounding), Attributes::BLTCSSecurityLevel::kMetadataEntry },
        { mFeatureMap.Has(Feature::kBluetoothChannelSounding), Attributes::BLTCSModeCapability::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(ProximityRanging::Attributes::kMandatoryMetadata), Span(featureAttributes));
}

CHIP_ERROR ProximityRangingCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        Commands::StartRangingRequest::kMetadataEntry,
        Commands::StopRangingRequest::kMetadataEntry,
    };
    return builder.ReferenceExisting(kCommands);
}

CHIP_ERROR ProximityRangingCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kGenerated[] = { Commands::StartRangingResponse::Id };
    return builder.ReferenceExisting(kGenerated);
}

std::optional<DataModel::ActionReturnStatus> ProximityRangingCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                    TLV::TLVReader & input_arguments,
                                                                                    CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::StartRangingRequest::Id:
        return HandleStartRangingRequest(request, input_arguments, handler);
    case Commands::StopRangingRequest::Id:
        return HandleStopRangingRequest(request, input_arguments);
    default:
        return Status::UnsupportedCommand;
    }
}

std::optional<DataModel::ActionReturnStatus>
ProximityRangingCluster::HandleStartRangingRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & reader,
                                                   CommandHandler * handler)
{
    Commands::StartRangingRequest::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(reader));

    Commands::StartRangingResponse::Type response;
    response.resultCode = ValidateStartRangingRequest(commandData);
    response.sessionID.SetNull();
    if (response.resultCode == ResultCodeEnum::kAccepted)
    {
        uint8_t sessionId = GenerateSessionId();
        if (sessionId == kInvalidSessionId)
        {
            response.resultCode = ResultCodeEnum::kBusySessionCapacityReached;
        }
        else
        {
            response.resultCode = mDriver.HandleStartRanging(sessionId, commandData);
            if (response.resultCode == ResultCodeEnum::kAccepted)
            {
                response.sessionID.SetNonNull(sessionId);
            }
        }
    }

    handler->AddResponse(request.path, response);
    return std::nullopt;
}

DataModel::ActionReturnStatus ProximityRangingCluster::HandleStopRangingRequest(const DataModel::InvokeRequest & request,
                                                                                TLV::TLVReader & reader)
{
    Commands::StopRangingRequest::DecodableType commandData;
    VerifyOrReturnValue(commandData.Decode(reader) == CHIP_NO_ERROR, Status::InvalidCommand);

    CHIP_ERROR err = mDriver.HandleStopRanging(commandData.sessionID);
    if (err == CHIP_ERROR_NOT_FOUND)
    {
        // If SessionID does not match any active ranging session, the Server SHALL response with the status code INVALID_IN_STATE
        return Status::InvalidInState;
    }
    return err;
}

void ProximityRangingCluster::OnMeasurementData(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement)
{
    VerifyOrReturn(mContext != nullptr);

    Events::RangingResult::Type event;
    event.sessionID         = sessionId;
    event.rangingResultData = measurement;
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void ProximityRangingCluster::OnSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status)
{
    VerifyOrReturn(mContext != nullptr);

    Events::RangingSessionStatus::Type event;
    event.sessionID = sessionId;
    event.status    = status;
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

ResultCodeEnum
ProximityRangingCluster::ValidateStartRangingRequest(const Commands::StartRangingRequest::DecodableType & request) const
{
    const bool hasWiFi = request.wiFiRangingDeviceRoleConfig.HasValue();
    const bool hasBle  = request.BLERangingDeviceRoleConfig.HasValue();
    const bool hasBlt  = request.BLTChannelSoundingDeviceRoleConfig.HasValue();

    switch (request.technology)
    {
    case RangingTechEnum::kBluetoothChannelSounding: {
        if (!mFeatureMap.Has(Feature::kBluetoothChannelSounding) || !hasBlt || hasWiFi || hasBle)
        {
            return ResultCodeEnum::kRejectedInfeasibleRanging;
        }
        auto role = request.BLTChannelSoundingDeviceRoleConfig.Value().role;
        if (role != RangingRoleEnum::kBLTInitiatorRole && role != RangingRoleEnum::kBLTReflectorRole)
        {
            return ResultCodeEnum::kRejectedInfeasibleRanging;
        }
        break;
    }
    case RangingTechEnum::kWiFiRoundTripTimeRanging:
    case RangingTechEnum::kWiFiNextGenerationRanging: {
        if (!mFeatureMap.Has(Feature::kWiFiUsdProximityDetection) || !hasWiFi || hasBle || hasBlt)
        {
            return ResultCodeEnum::kRejectedInfeasibleRanging;
        }
        auto role = request.wiFiRangingDeviceRoleConfig.Value().role;
        if (role != RangingRoleEnum::kWiFiSubscriberRole && role != RangingRoleEnum::kWiFiPublisherRole)
        {
            return ResultCodeEnum::kRejectedInfeasibleRanging;
        }
        break;
    }
    case RangingTechEnum::kBLEBeaconRSSIRanging: {
        if (!mFeatureMap.Has(Feature::kBleBeaconRssi) || !hasBle || hasWiFi || hasBlt)
        {
            return ResultCodeEnum::kRejectedInfeasibleRanging;
        }
        auto role = request.BLERangingDeviceRoleConfig.Value().role;
        if (role != RangingRoleEnum::kBLEScanningRole && role != RangingRoleEnum::kBLEBeaconRole)
        {
            return ResultCodeEnum::kRejectedInfeasibleRanging;
        }
        break;
    }
    default:
        return ResultCodeEnum::kRejectedInfeasibleRanging;
    }

    if (request.trigger.endTime <= request.trigger.startTime)
    {
        return ResultCodeEnum::kRejectedInfeasibleRangingTriggers;
    }
    if (request.trigger.rangingInstanceInterval.HasValue() && request.trigger.rangingInstanceInterval.Value() == 0)
    {
        return ResultCodeEnum::kRejectedInfeasibleRangingTriggers;
    }

    if (request.reportingCondition.HasValue())
    {
        const auto & rc = request.reportingCondition.Value();
        if (rc.minDistanceCondition.HasValue() && rc.minDistanceCondition.Value() == 0)
        {
            return ResultCodeEnum::kRejectedInfeasibleRanging;
        }
        if (rc.maxDistanceCondition.HasValue() && rc.maxDistanceCondition.Value() == 0)
        {
            return ResultCodeEnum::kRejectedInfeasibleRanging;
        }
        if (rc.minDistanceCondition.HasValue() && rc.maxDistanceCondition.HasValue() &&
            rc.minDistanceCondition.Value() > rc.maxDistanceCondition.Value())
        {
            return ResultCodeEnum::kRejectedInfeasibleRanging;
        }
    }

    return ResultCodeEnum::kAccepted;
}

uint8_t ProximityRangingCluster::GenerateSessionId()
{
    const size_t numSessions = mDriver.GetNumActiveSessionIds();
    if (numSessions == 0)
    {
        uint8_t candidate = mNextSessionId++;
        if (candidate == kInvalidSessionId)
        {
            candidate = mNextSessionId++;
        }
        return candidate;
    }
    Platform::ScopedMemoryBuffer<uint8_t> buf;
    VerifyOrReturnValue(buf.Calloc(numSessions), kInvalidSessionId);
    Span<uint8_t> activeSessions(buf.Get(), numSessions);
    if (mDriver.GetActiveSessionIds(activeSessions) != CHIP_NO_ERROR)
    {
        return kInvalidSessionId;
    }

    for (size_t attempt = 0; attempt <= numSessions; attempt++)
    {
        uint8_t candidate = mNextSessionId++;
        if (candidate == kInvalidSessionId)
        {
            candidate = mNextSessionId++;
        }
        if (std::none_of(activeSessions.data(), activeSessions.data() + activeSessions.size(),
                         [candidate](uint8_t value) { return value == candidate; }))
        {
            return candidate;
        }
    }
    return kInvalidSessionId;
}

void ProximityRangingCluster::OnAttributeChanged(AttributeId attributeId)
{
    NotifyAttributeChanged(attributeId);
}

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
