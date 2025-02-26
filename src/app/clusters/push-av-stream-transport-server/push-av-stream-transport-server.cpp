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

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

PushAvStreamTransportServer::PushAvStreamTransportServer(EndpointId aEndpointId, PushAvStreamTransportDelegate & aDelegate,
                                                         const BitFlags<Feature> aFeature,
                                                         PersistentStorageDelegate & aPersistentStorage) :
    CommandHandlerInterface(MakeOptional(aEndpointId), CameraAvStreamManagement::Id),
    AttributeAccessInterface(MakeOptional(aEndpointId), CameraAvStreamManagement::Id), mDelegate(aDelegate),
    mEndpointId(aEndpointId), mFeature(aFeature)
{
    mDelegate.SetPushAvStreamTransportServer(this);
}

PushAvStreamTransportServer::~PushAvStreamTransportServer()
{
    // Explicitly set the PushAvStreamTransportServer pointer in the Delegate to null.

    mDelegate.SetPushAvStreamTransportServer(nullptr);

    // Unregister command handler and attribute access interfaces
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR PushAvStreamTransportServer::Init()
{
    LoadPersistentAttributes();

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

bool PushAvStreamTransportServer::HasFeature(Feature feature) const
{
    return mFeature.Has(feature);
}

CHIP_ERROR PushAvStreamTransportServer::ReadAndEncodeCurrentConnections(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & currentConnections : mCurrentConnections)
    {
        ReturnErrorOnFailure(encoder.Encode(currentConnections));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PushAvStreamTransportServer::AddStreamTransportConnection(const uint16_t transportConnectionId)
{
    mCurrentConnections.push_back(transportConnectionId);
    auto path = ConcreteAttributePath(mEndpointId, PushAvStreamTransport::Id, Attributes::CurrentConnections::Id);
    mDelegate.OnAttributeChanged(Attributes::CurrentConnections::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PushAvStreamTransportServer::RemoveStreamTransportConnection(const uint16_t transportConnectionId)
{
    mCurrentConnections.erase(std::remove_if(mCurrentConnections.begin(), mCurrentConnections.end(),
                                             [&](const uint16_t connectionID) { return connectionID == transportConnectionId; }),
                              mCurrentConnections.end());
    auto path = ConcreteAttributePath(mEndpointId, PushAvStreamTransport::Id, Attributes::CurrentConnections::Id);
    mDelegate.OnAttributeChanged(Attributes::CurrentConnections::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PushAvStreamTransportServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == PushAvStreamTransport::Id);
    ChipLogError(Zcl, "Push AV Stream Transport: Reading");

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeature));
        break;

    case SupportedContainerFormats::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mSupportedContainerFormats));
        break;

    case SupportedIngestMethods::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mSupportedIngestMethods));
        break;

    case CurrentConnections::Id:
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeCurrentConnections(encoder); }));
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PushAvStreamTransportServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == PushAvStreamTransport::Id);

    return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
}

void PushAvStreamTransportServer::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Load currentConnections
    mDelegate.LoadCurrentConnections(mCurrentConnections);

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
    }
}

bool PushAvStreamTransportServer::FindStreamTransportConnection(const uint16_t connectionID)
{
    for (auto & id : mCurrentConnections)
    {
        if (id == connectionID)
            return true;
    }
    return false;
}

uint16_t PushAvStreamTransportServer::GenerateConnectionID()
{
    static uint16_t assignedConnectionID = 0;
    uint16_t nextConnectionID;

    if (assignedConnectionID == MAX_PUSH_TRANSPORT_CONNECTION_ID)
        nextConnectionID = 0;
    else
        nextConnectionID = assignedConnectionID + 1;

    while (FindStreamTransportConnection(nextConnectionID) != false)
    {
        if (nextConnectionID == MAX_PUSH_TRANSPORT_CONNECTION_ID)
            nextConnectionID = 0;
        else
            nextConnectionID = nextConnectionID + 1;
    }
    return nextConnectionID;
}

void PushAvStreamTransportServer::HandleAllocatePushTransport(HandlerContext & ctx,
                                                              const Commands::AllocatePushTransport::DecodableType & commandData)
{
    Status status = Status::Success;
    Commands::AllocatePushTransportResponse::Type response;
    auto & transportOptions = commandData.transportOptions;

    uint16_t connectionID                 = GenerateConnectionID();
    TransportStatusEnum outTranportStatus = TransportStatusEnum::kUnknownEnumValue;

    // call the delegate
    status = mDelegate.AllocatePushTransport(connectionID, transportOptions, outTranportStatus);

    if (status == Status::Success)
    {
        response.connectionID     = connectionID;
        response.transportOptions = transportOptions;
        response.transportStatus  = outTranportStatus;

        // add connection to CurrentConnections
        AddStreamTransportConnection(connectionID);

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
    Status status              = Status::Success;
    uint16_t connectionID      = commandData.connectionID;
    auto & outTransportOptions = commandData.transportOptions;

    // Call the delegate
    status = mDelegate.ModifyPushTransport(connectionID, outTransportOptions);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void PushAvStreamTransportServer::HandleSetTransportStatus(HandlerContext & ctx,
                                                           const Commands::SetTransportStatus::DecodableType & commandData)
{
    Status status           = Status::Success;
    uint16_t connectionID   = commandData.connectionID;
    auto & transportOptions = commandData.transportOptions;

    // Call the delegate
    status = mDelegate.SetTransportStatus(connectionID, transportOptions);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void PushAvStreamTransportServer::HandleManuallyTriggerTransport(
    HandlerContext & ctx, const Commands::ManuallyTriggerTransport::DecodableType & commandData)
{
    Status status           = Status::Success;
    uint16_t connectionID   = commandData.connectionID;
    auto & activationReason = commandData.activationReason;
    auto & timeControl      = commandData.timeControl;

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

    DataModel::List<TransportConfigurationStruct> outStreamConfigurations;

    // Call the delegate
    status = mDelegate.FindTransport(connectionID, outStreamConfigurations);
    if (status == Status::Success)
    {
        response.streamConfigurations = outStreamConfigurations;

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
