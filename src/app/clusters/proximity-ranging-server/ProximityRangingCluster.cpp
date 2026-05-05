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
#include <clusters/ProximityRanging/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

using Status = Protocols::InteractionModel::Status;

namespace {
// Maximum number of active sessions supported by ProximityRanging cluster
static constexpr size_t kMaxActiveSessions = 50;
static constexpr uint8_t kInvalidSessionId = 0;
} // namespace

CHIP_ERROR ProximityRangingCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    VerifyOrReturnError(mDriver != nullptr, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR err = mDriver->Init(*this);
    if (err != CHIP_NO_ERROR)
    {
        DefaultServerCluster::Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    return err;
}

void ProximityRangingCluster::Shutdown(ClusterShutdownType shutdownType)
{
    DefaultServerCluster::Shutdown(shutdownType);
    if (mDriver != nullptr)
    {
        mDriver->Shutdown();
    }
}

DataModel::ActionReturnStatus ProximityRangingCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::RangingCapabilities::Id:
        return mDriver->GetRangingCapabilities(encoder);

    case Attributes::WiFiDevIK::Id: {
        uint8_t buf[kDeviceIdentityKeyLen] = { 0 };
        MutableByteSpan span(buf);
        CHIP_ERROR err = mDriver->GetWiFiDevIK(span);
        VerifyOrReturnError(err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, Status::UnsupportedAttribute);
        VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);
        return encoder.Encode(span);
    }

    case Attributes::BLEDeviceID::Id: {
        uint64_t bleDeviceId = 0;
        CHIP_ERROR err       = mDriver->GetBleDeviceId(bleDeviceId);
        VerifyOrReturnError(err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, Status::UnsupportedAttribute);
        VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);
        return encoder.Encode(bleDeviceId);
    }

    case Attributes::BLTDevIK::Id: {
        uint8_t buf[kDeviceIdentityKeyLen] = { 0 };
        MutableByteSpan span(buf);
        CHIP_ERROR err = mDriver->GetBLTDevIK(span);
        VerifyOrReturnError(err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, Status::UnsupportedAttribute);
        VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);
        return encoder.Encode(span);
    }

    case Attributes::BLTCSSecurityLevel::Id: {
        BLTCSSecurityLevelEnum securityLevel;
        CHIP_ERROR err = mDriver->GetBLTCSSecurityLevel(securityLevel);
        VerifyOrReturnError(err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, Status::UnsupportedAttribute);
        VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);
        return encoder.Encode(securityLevel);
    }

    case Attributes::BLTCSModeCapability::Id: {
        BLTCSModeEnum modeCapability = BLTCSModeEnum::kUnknownEnumValue;
        CHIP_ERROR err               = mDriver->GetBLTCSModeCapability(modeCapability);
        VerifyOrReturnError(err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, Status::UnsupportedAttribute);
        VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);
        return encoder.Encode(modeCapability);
    }

    case Attributes::SessionIDList::Id: {
        uint8_t buf[kMaxActiveSessions] = { 0 };
        Span<uint8_t> sessionIds(buf, kMaxActiveSessions);
        CHIP_ERROR err = mDriver->GetActiveSessionIds(sessionIds);
        VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);
        if (sessionIds.empty())
        {
            return encoder.EncodeNull();
        }
        return encoder.EncodeList([&sessionIds](const auto & listEncoder) -> CHIP_ERROR {
            for (size_t i = 0; i < sessionIds.size(); i++)
            {
                ReturnErrorOnFailure(listEncoder.Encode(sessionIds.data()[i]));
            }
            return CHIP_NO_ERROR;
        });
    }

    case Attributes::FeatureMap::Id:
        return encoder.Encode(mDriver->GetFeatureMap());

    case Attributes::ClusterRevision::Id:
        return encoder.Encode(ProximityRanging::kRevision);

    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ProximityRangingCluster::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    static constexpr DataModel::AttributeEntry kOptionalAttributesMeta[] = {
        ProximityRanging::Attributes::WiFiDevIK::kMetadataEntry,
        ProximityRanging::Attributes::BLEDeviceID::kMetadataEntry,
        ProximityRanging::Attributes::BLTDevIK::kMetadataEntry,
        ProximityRanging::Attributes::BLTCSSecurityLevel::kMetadataEntry,
        ProximityRanging::Attributes::BLTCSModeCapability::kMetadataEntry,
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(ProximityRanging::Attributes::kMandatoryMetadata),
                              Span<const DataModel::AttributeEntry>(kOptionalAttributesMeta), mOptionalAttributes);
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
        return HandleStopRangingRequest(request, input_arguments, handler);
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
    ResultCodeEnum resultCode;
    uint8_t sessionId = GenerateSessionId();
    if (sessionId == kInvalidSessionId)
    {
        // Failed to generate session ID without collision
        resultCode = ResultCodeEnum::kBusySessionCapacityReached;
    }
    else
    {
        resultCode          = mDriver->HandleStartRanging(sessionId, commandData);
        response.resultCode = resultCode;
    }

    response.resultCode = resultCode;

    if (resultCode == ResultCodeEnum::kAccepted)
    {
        response.sessionID.SetNonNull(sessionId);
    }
    else
    {
        response.sessionID.SetNull();
    }

    handler->AddResponse(request.path, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
ProximityRangingCluster::HandleStopRangingRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & reader,
                                                  CommandHandler * handler)
{
    Commands::StopRangingRequest::DecodableType commandData;
    VerifyOrReturnValue(commandData.Decode(reader) == CHIP_NO_ERROR, Status::InvalidCommand);

    CHIP_ERROR err = mDriver->HandleStopRanging(commandData.sessionID);
    if (err == CHIP_ERROR_NOT_FOUND)
    {
        return Status::NotFound;
    }
    if (err != CHIP_NO_ERROR)
    {
        return Status::Failure;
    }

    return Status::Success;
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

uint8_t ProximityRangingCluster::GenerateSessionId()
{
    uint8_t buf[kMaxActiveSessions];
    Span<uint8_t> activeSessions(buf);
    if (mDriver->GetActiveSessionIds(activeSessions) != CHIP_NO_ERROR)
    {
        return kInvalidSessionId;
    }

    for (uint8_t attempt = 0; attempt <= kMaxActiveSessions; attempt++)
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
