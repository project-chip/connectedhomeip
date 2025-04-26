/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <protocols/interaction_model/StatusCode.h>

#define MAX_PUSH_TRANSPORT_CONNECTION_ID 65535

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;
using namespace chip::app::Clusters::PushAvStreamTransport::Structs;
using namespace chip::app::Clusters::PushAvStreamTransport::Attributes;
using namespace Protocols::InteractionModel;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

PushAvStreamTransportServer::PushAvStreamTransportServer(PushAvStreamTransportDelegate & aDelegate, EndpointId aEndpointId) :
    AttributeAccessInterface(MakeOptional(aEndpointId), PushAvStreamTransport::Id),
    CommandHandlerInterface(MakeOptional(aEndpointId), PushAvStreamTransport::Id), mDelegate(aDelegate)
{}

PushAvStreamTransportServer::~PushAvStreamTransportServer()
{
    Shutdown();
}

CHIP_ERROR PushAvStreamTransportServer::Init()
{
    LoadPersistentAttributes();

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

void PushAvStreamTransportServer::Shutdown()
{ // Unregister command handler and attribute access interfaces
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR PushAvStreamTransportServer::ReadAndEncodeCurrentConnections(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & currentConnections : mCurrentConnections)
    {
        ReturnErrorOnFailure(encoder.Encode(currentConnections));
    }

    return CHIP_NO_ERROR;
}

PushAvStreamTransportServer::UpsertResultEnum
PushAvStreamTransportServer::UpsertStreamTransportConnection(const TransportConfigurationStruct & transportConfiguration)
{
    UpsertResultEnum result;
    auto it =
        std::find_if(mCurrentConnections.begin(), mCurrentConnections.end(),
                     [id = transportConfiguration.connectionID](const auto & existing) { return existing.connectionID == id; });

    if (it != mCurrentConnections.end())
    {
        *it    = transportConfiguration;
        result = UpsertResultEnum::kUpdated;
    }
    else
    {
        mCurrentConnections.push_back(transportConfiguration);
        result = UpsertResultEnum::kInserted;
    }

    MatterReportingAttributeChangeCallback(AttributeAccessInterface::GetEndpointId().Value(), PushAvStreamTransport::Id,
                                           PushAvStreamTransport::Attributes::CurrentConnections::Id);

    return result;
}

void PushAvStreamTransportServer::RemoveStreamTransportConnection(const uint16_t transportConnectionId)
{
    size_t originalSize = mCurrentConnections.size();

    // Erase-Remove idiom
    mCurrentConnections.erase(std::remove_if(mCurrentConnections.begin(), mCurrentConnections.end(),
                                             [transportConnectionId](const TransportConfigurationStruct & s) {
                                                 return s.connectionID == transportConnectionId;
                                             }),
                              mCurrentConnections.end());

    // If a connection was removed, the size will be smaller.
    if (mCurrentConnections.size() < originalSize)
    {
        // Notify the stack that the CurrentConnections attribute has changed.
        MatterReportingAttributeChangeCallback(AttributeAccessInterface::GetEndpointId().Value(), PushAvStreamTransport::Id,
                                               PushAvStreamTransport::Attributes::CurrentConnections::Id);
    }
}

CHIP_ERROR PushAvStreamTransportServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == PushAvStreamTransport::Id);
    ChipLogError(Zcl, "Push AV Stream Transport[ep=%d]: Reading", AttributeAccessInterface::GetEndpointId().Value());

    if (aPath.mClusterId == PushAvStreamTransport::Id && aPath.mAttributeId == Attributes::CurrentConnections::Id)
    {

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeCurrentConnections(encoder); }));
    }

    return CHIP_NO_ERROR;
}

void PushAvStreamTransportServer::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Load currentConnections
    err = mDelegate.LoadCurrentConnections(mCurrentConnections);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "PushAVStreamTransport: Unable to load allocated connections from the KVS.");
    }

    // Signal delegate that all persistent configuration attributes have been loaded.
    mDelegate.PersistentAttributesLoadedCallback();
}

// CommandHandlerInterface
void PushAvStreamTransportServer::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "PushAV: InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::AllocatePushTransport::Id:
        ChipLogDetail(Zcl, "PushAVStreamTransport: Allocating Push Transport");

        HandleCommand<Commands::AllocatePushTransport::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleAllocatePushTransport(ctx, commandData); });

        break;

    case Commands::DeallocatePushTransport::Id:
        ChipLogDetail(Zcl, "PushAVStreamTransport: Deallocating Push Transport");

        HandleCommand<Commands::DeallocatePushTransport::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleDeallocatePushTransport(ctx, commandData); });

        break;

    case Commands::ModifyPushTransport::Id:
        ChipLogDetail(Zcl, "PushAVStreamTransport: Modifying Push Transport");

        HandleCommand<Commands::ModifyPushTransport::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleModifyPushTransport(ctx, commandData); });

        break;

    case Commands::SetTransportStatus::Id:
        ChipLogDetail(Zcl, "PushAVStreamTransport: Setting Push Transport Status");

        HandleCommand<Commands::SetTransportStatus::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleSetTransportStatus(ctx, commandData); });

        break;

    case Commands::ManuallyTriggerTransport::Id:
        ChipLogDetail(Zcl, "PushAVStreamTransport: Manually Triggered Push Transport");

        HandleCommand<Commands::ManuallyTriggerTransport::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleManuallyTriggerTransport(ctx, commandData); });

        break;

    case Commands::FindTransport::Id:
        ChipLogDetail(Zcl, "PushAVStreamTransport: Finding Push Transport");

        HandleCommand<Commands::FindTransport::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleFindTransport(ctx, commandData); });

        break;
    default:
        // Mark unrecognized command as UnsupportedCommand
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        break;
    }
}

TransportConfigurationStruct * PushAvStreamTransportServer::FindStreamTransportConnection(const uint16_t connectionID)
{
    for (auto & transportConnection : mCurrentConnections)
    {
        if (transportConnection.connectionID == connectionID)
            return &transportConnection;
    }
    return nullptr;
}

uint16_t PushAvStreamTransportServer::GenerateConnectionID()
{
    static uint16_t lastAssignedConnectionID = 0;
    do
    {
        uint16_t nextConnectionID;
        if (lastAssignedConnectionID == MAX_PUSH_TRANSPORT_CONNECTION_ID)
        {
            nextConnectionID = 0;
        }
        else
        {
            nextConnectionID = static_cast<uint16_t>(lastAssignedConnectionID + 1);
        }
        lastAssignedConnectionID = nextConnectionID;
        if (FindStreamTransportConnection(nextConnectionID) == nullptr)
        {
            return nextConnectionID;
        }
    } while (true);
}

void PushAvStreamTransportServer::HandleAllocatePushTransport(HandlerContext & ctx,
                                                              const Commands::AllocatePushTransport::DecodableType & commandData)
{
    Commands::AllocatePushTransportResponse::Type response;
    auto & transportOptions = commandData.transportOptions;

    uint16_t ep = emberAfGetClusterServerEndpointIndex(transportOptions.endpointID, TlsCertificateManagement::Id,
                                                       MATTER_DM_TLS_CERTIFICATE_MANAGEMENT_CLUSTER_CLIENT_ENDPOINT_COUNT);

    if (ep == kEmberInvalidEndpointIndex)
    {
        auto status = static_cast<uint8_t>(StatusCodeEnum::kInvalidTLSEndpoint);
        ChipLogError(Zcl, "HandleAllocatePushTransport: Valid TLSEndpointId not found");
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, status);
        return;
    }

    if (transportOptions.ingestMethod == IngestMethodsEnum::kUnknownEnumValue)
    {
        auto status = static_cast<uint8_t>(StatusCodeEnum::kUnsupportedIngestMethod);
        ChipLogError(Zcl, "HandleAllocatePushTransport: Ingest method not supported");
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, status);
        return;
    }

    if (transportOptions.containerFormat == ContainerFormatEnum::kUnknownEnumValue)
    {
        auto status = static_cast<uint8_t>(StatusCodeEnum::kUnsupportedContainerFormat);
        ChipLogError(Zcl, "HandleAllocatePushTransport: Container format not supported");
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, status);
        return;
    }

    if (transportOptions.triggerOptions.triggerType == TransportTriggerTypeEnum::kUnknownEnumValue)
    {
        auto status = static_cast<uint8_t>(StatusCodeEnum::kInvalidTriggerType);
        ChipLogError(Zcl, "HandleAllocatePushTransport: Invalid Trigger type");
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, status);
        return;
    }

    // Todo: Validate MotionZones list in the TransportTriggerOptionsStruct field

    // Validate the StreamUsageEnum as per resource management and stream priorities.
    CHIP_ERROR err =
        mDelegate.ValidateStreamUsage(transportOptions.streamUsage, transportOptions.videoStreamID, transportOptions.audioStreamID);
    if (err != CHIP_NO_ERROR)
    {
        auto status = static_cast<uint8_t>(StatusCodeEnum::kInvalidStream);
        ChipLogError(Zcl, "HandleAllocatePushTransport: Invalid Stream");
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, status);
        return;
    }

    uint16_t connectionID = GenerateConnectionID();

    TransportConfigurationStruct outTransportConfiguration;
    outTransportConfiguration.connectionID    = connectionID;
    outTransportConfiguration.transportStatus = TransportStatusEnum::kInactive;

    /**
     * delegate should set the TransportOptions fields to the new values.
     * Persistently store the resulting TransportConfigurationStruct and map it to the ConnectionID
     */
    Status status = mDelegate.AllocatePushTransport(transportOptions, outTransportConfiguration);

    if (status == Status::Success)
    {
        // add connection to CurrentConnections
        UpsertStreamTransportConnection(outTransportConfiguration);
        response.transportConfiguration = outTransportConfiguration;

        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    }
}

void PushAvStreamTransportServer::HandleDeallocatePushTransport(
    HandlerContext & ctx, const Commands::DeallocatePushTransport::DecodableType & commandData)
{
    Status status         = Status::Success;
    uint16_t connectionID = commandData.connectionID;

    if (FindStreamTransportConnection(connectionID) == nullptr)
    {
        ChipLogError(Zcl, "HandleDeallocatePushTransport: ConnectionID Not Found.");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Call the delegate
    status = mDelegate.DeallocatePushTransport(connectionID);

    if (status == Status::Success)
        // Remove connection form CurrentConnections
        RemoveStreamTransportConnection(connectionID);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void PushAvStreamTransportServer::HandleModifyPushTransport(HandlerContext & ctx,
                                                            const Commands::ModifyPushTransport::DecodableType & commandData)
{
    Status status           = Status::Success;
    uint16_t connectionID   = commandData.connectionID;
    auto & transportOptions = commandData.transportOptions;

    if (FindStreamTransportConnection(connectionID) == nullptr)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport: ConnectionID Not Found.");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Call the delegate
    status = mDelegate.ModifyPushTransport(connectionID, transportOptions);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void PushAvStreamTransportServer::HandleSetTransportStatus(HandlerContext & ctx,
                                                           const Commands::SetTransportStatus::DecodableType & commandData)
{
    Status status                              = Status::Success;
    DataModel::Nullable<uint16_t> connectionID = commandData.connectionID;
    auto & transportStatus                     = commandData.transportStatus;
    std::vector<uint16_t> connectionIDList;

    if (connectionID.IsNull())
    {
        for (auto & transportConnection : mCurrentConnections)
        {
            connectionIDList.push_back(transportConnection.connectionID);
        }
    }
    else if (FindStreamTransportConnection(connectionID.Value()) == nullptr)
    {
        ChipLogError(Zcl, "HandleSetTransportStatus: ConnectionID Not Found.");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }
    else
    {
        connectionIDList.push_back(connectionID.Value());
    }

    // Call the delegate
    status = mDelegate.SetTransportStatus(connectionIDList, transportStatus);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void PushAvStreamTransportServer::HandleManuallyTriggerTransport(
    HandlerContext & ctx, const Commands::ManuallyTriggerTransport::DecodableType & commandData)
{
    Status status           = Status::Success;
    uint16_t connectionID   = commandData.connectionID;
    auto & activationReason = commandData.activationReason;
    auto & timeControl      = commandData.timeControl;

    TransportConfigurationStruct * transportConfiguration = FindStreamTransportConnection(connectionID);

    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport: ConnectionID Not Found.");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (transportConfiguration->transportStatus == TransportStatusEnum::kInactive)
    {
        auto clusterStatus = static_cast<uint8_t>(StatusCodeEnum::kInvalidTransportStatus);
        ChipLogError(Zcl, "HandleManuallyTriggerTransport: Invalid Transport status");
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, clusterStatus);
        return;
    }
    if (transportConfiguration->transportOptions.HasValue())
    {
        if (transportConfiguration->transportOptions.Value().triggerOptions.triggerType == TransportTriggerTypeEnum::kContinuous)
        {
            {
                auto clusterStatus = static_cast<uint8_t>(StatusCodeEnum::kInvalidTriggerType);
                ChipLogError(Zcl, "HandleManuallyTriggerTransport: Invalid Trigger type");
                ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, clusterStatus);
                return;
            }
        }
        else if (transportConfiguration->transportOptions.Value().triggerOptions.triggerType ==
                     TransportTriggerTypeEnum::kCommand &&
                 !timeControl.HasValue())
        {
            {
                ChipLogError(Zcl, "HandleManuallyTriggerTransport: Time control field not present");
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
                return;
            }
        }
    }

    // Call the delegate
    status = mDelegate.ManuallyTriggerTransport(connectionID, activationReason, timeControl);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void PushAvStreamTransportServer::HandleFindTransport(HandlerContext & ctx,
                                                      const Commands::FindTransport::DecodableType & commandData)
{
    Status status = Status::Success;
    Commands::FindTransportResponse::Type response;

    Optional<DataModel::Nullable<uint16_t>> connectionID = commandData.connectionID;

    DataModel::List<const TransportConfigurationStruct> outTransportConfigurations;

    if (connectionID.Value().IsNull())
    {
        if (mCurrentConnections.size() == 0)
        {
            ChipLogError(Zcl, "HandleFindTransport: ConnectionID not found");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }
    }
    else if (!FindStreamTransportConnection(connectionID.Value().Value()))
    {
        ChipLogError(Zcl, "HandleFindTransport: ConnectionID not found");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Call the delegate
    status = mDelegate.FindTransport(connectionID, outTransportConfigurations);
    if (status == Status::Success)
    {
        response.transportConfigurations = outTransportConfigurations;

        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    }
}

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip

/** @brief Push AV Stream Transport Cluster Server Init
 *
 * Server Init
 *
 */
void MatterPushAvStreamTransportPluginServerInitCallback() {}
