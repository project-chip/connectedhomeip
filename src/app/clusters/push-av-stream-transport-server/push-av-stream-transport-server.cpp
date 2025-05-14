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

static constexpr uint16_t kMaxConnectionId = 65535; // This is also invalid connectionID

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

PushAvStreamTransportServer::PushAvStreamTransportServer(PushAvStreamTransportDelegate & aDelegate, EndpointId aEndpointId,
                                                         const BitFlags<Feature> aFeatures) :
    AttributeAccessInterface(MakeOptional(aEndpointId), PushAvStreamTransport::Id),
    CommandHandlerInterface(MakeOptional(aEndpointId), PushAvStreamTransport::Id), mDelegate(aDelegate), mFeatures(aFeatures),
    mSupportedFormats{ SupportedFormatStruct{ ContainerFormatEnum::kCmaf, IngestMethodsEnum::kCMAFIngest } }
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

bool PushAvStreamTransportServer::HasFeature(Feature feature) const
{
    return mFeatures.Has(feature);
}

CHIP_ERROR PushAvStreamTransportServer::ReadAndEncodeSupportedFormats(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & supportsFormat : mSupportedFormats)
    {
        ReturnErrorOnFailure(encoder.Encode(supportsFormat));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PushAvStreamTransportServer::ReadAndEncodeCurrentConnections(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & currentConnections : mCurrentConnections)
    {
        ReturnErrorOnFailure(encoder.Encode(currentConnections.transportConfiguration));
    }

    return CHIP_NO_ERROR;
}

PushAvStreamTransportServer::UpsertResultEnum PushAvStreamTransportServer::UpsertStreamTransportConnection(
    const TransportConfigurationStructWithFabricIndex & transportConfiguration)
{
    UpsertResultEnum result;
    auto it = std::find_if(mCurrentConnections.begin(), mCurrentConnections.end(),
                           [id = transportConfiguration.transportConfiguration.connectionID](const auto & existing) {
                               return existing.transportConfiguration.connectionID == id;
                           });

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
                                             [transportConnectionId](const TransportConfigurationStructWithFabricIndex & s) {
                                                 return s.transportConfiguration.connectionID == transportConnectionId;
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
    ChipLogProgress(Zcl, "Push AV Stream Transport[ep=%d]: Reading", AttributeAccessInterface::GetEndpointId().Value());

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeatures));
        break;

    case SupportedFormats::Id:
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeSupportedFormats(encoder); }));
        break;

    case CurrentConnections::Id:
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeCurrentConnections(encoder); }));
        break;
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

TransportConfigurationStructWithFabricIndex *
PushAvStreamTransportServer::FindStreamTransportConnection(const uint16_t connectionID)
{
    for (auto & transportConnection : mCurrentConnections)
    {
        if (transportConnection.transportConfiguration.connectionID == connectionID)
            return &transportConnection;
    }
    return nullptr;
}

uint16_t PushAvStreamTransportServer::GenerateConnectionID()
{
    static uint16_t lastID = 0;

    for (uint16_t i = 0; i < kMaxConnectionId; ++i)
    {
        uint16_t candidateID = static_cast<uint16_t>((lastID + i + 1) % kMaxConnectionId); // Wrap from 0 to 65534
        if (FindStreamTransportConnection(candidateID) == nullptr)
        {
            lastID = candidateID;
            return candidateID;
        }
    }

    return kMaxConnectionId; // All 0 to 65534 IDs are in use
}

void PushAvStreamTransportServer::HandleAllocatePushTransport(HandlerContext & ctx,
                                                              const Commands::AllocatePushTransport::DecodableType & commandData)
{
    Commands::AllocatePushTransportResponse::Type response;
    auto & transportOptions = commandData.transportOptions;

    // Todo: TLSEndpointID Validation

    bool isFormatSupported                  = false;
    IngestMethodsEnum ingestMethod          = commandData.transportOptions.ingestMethod;
    ContainerOptionsStruct containerOptions = commandData.transportOptions.containerOptions;

    for (auto & supportsFormat : mSupportedFormats)
    {
        if ((supportsFormat.ingestMethod == ingestMethod) && (supportsFormat.containerFormat == containerOptions.containerType))
        {
            isFormatSupported = true;
        }
    }

    if (isFormatSupported == false)
    {
        auto status = static_cast<uint8_t>(StatusCodeEnum::kInvalidCombination);
        ChipLogError(Zcl, "HandleAllocatePushTransport: Invalid Format Combination");
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, status);
        return;
    }

    bool isValidUrl = mDelegate.validateUrl(std::string(transportOptions.url.data(), transportOptions.url.size()));

    if (isValidUrl == false)
    {
        auto status = static_cast<uint8_t>(StatusCodeEnum::kInvalidURL);
        ChipLogError(Zcl, "HandleAllocatePushTransport: Invalid Url");
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
    // Validate Bandwidth Requirement
    CHIP_ERROR err = mDelegate.ValidateBandwidthLimit(transportOptions.streamUsage, transportOptions.videoStreamID,
                                                      transportOptions.audioStreamID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport: Resource Exhausted");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
        return;
    }

    // Validate the StreamUsageEnum as per resource management and stream priorities.
    err =
        mDelegate.ValidateStreamUsage(transportOptions.streamUsage, transportOptions.videoStreamID, transportOptions.audioStreamID);
    if (err != CHIP_NO_ERROR)
    {
        auto status = static_cast<uint8_t>(StatusCodeEnum::kInvalidStream);
        ChipLogError(Zcl, "HandleAllocatePushTransport: Invalid Stream");
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, status);
        return;
    }

    uint16_t connectionID = GenerateConnectionID();

    if (connectionID == kMaxConnectionId)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport: Max Connections Exhausted");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
        return;
    }

    TransportConfigurationStruct outTransportConfiguration;
    outTransportConfiguration.connectionID    = connectionID;
    outTransportConfiguration.transportStatus = TransportStatusEnum::kInactive;

    TransportOptionsStorage transportOptionArgs(transportOptions);
    Status status = mDelegate.AllocatePushTransport(transportOptionArgs, connectionID);

    if (status == Status::Success)
    {
        // add connection to CurrentConnections
        FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();
        outTransportConfiguration.transportOptions.SetValue(transportOptionArgs);
        TransportConfigurationStructWithFabricIndex transportConfiguration({ outTransportConfiguration, peerFabricIndex });
        UpsertStreamTransportConnection(transportConfiguration);
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
    Status status                                                        = Status::Success;
    uint16_t connectionID                                                = commandData.connectionID;
    TransportConfigurationStructWithFabricIndex * transportConfiguration = FindStreamTransportConnection(connectionID);
    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleDeallocatePushTransport: ConnectionID Not Found.");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (transportConfiguration->fabricIndex != ctx.mCommandHandler.GetAccessingFabricIndex())
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

    TransportConfigurationStructWithFabricIndex * transportConfiguration = FindStreamTransportConnection(connectionID);

    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport: ConnectionID Not Found.");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (transportConfiguration->fabricIndex != ctx.mCommandHandler.GetAccessingFabricIndex())
    {
        ChipLogError(Zcl, "HandleModifyPushTransport: ConnectionID Not Found.");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (mDelegate.GetTransportStatus(connectionID) == PushAvStreamTransportStatusEnum::kBusy)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport: Connection is Busy");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
        return;
    }

    TransportOptionsStruct transportOptionArgs;
    transportOptionArgs.streamUsage                = transportOptions.streamUsage;
    transportOptionArgs.videoStreamID              = transportOptions.videoStreamID;
    transportOptionArgs.audioStreamID              = transportOptions.audioStreamID;
    transportOptionArgs.endpointID                 = transportOptions.endpointID;
    transportOptionArgs.url                        = transportOptions.url;
    transportOptionArgs.triggerOptions.triggerType = transportOptions.triggerOptions.triggerType;
    // Todo: copy motion zones
    transportOptionArgs.triggerOptions.motionSensitivity = transportOptions.triggerOptions.motionSensitivity;
    transportOptionArgs.triggerOptions.motionTimeControl = transportOptions.triggerOptions.motionTimeControl;
    transportOptionArgs.triggerOptions.maxPreRollLen     = transportOptions.triggerOptions.maxPreRollLen;
    transportOptionArgs.ingestMethod                     = transportOptions.ingestMethod;
    transportOptionArgs.containerOptions                 = transportOptions.containerOptions;
    transportOptionArgs.expiryTime                       = transportOptions.expiryTime;

    // Call the delegate
    status = mDelegate.ModifyPushTransport(connectionID, transportOptionArgs);

    if (status == Status::Success)
    {
        if (transportConfiguration->transportConfiguration.transportOptions.HasValue())
        {
            transportConfiguration->transportConfiguration.transportOptions =
                static_cast<Optional<Structs::TransportOptionsStruct::Type>>(transportOptionArgs);
        }
    }

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
            if (transportConnection.fabricIndex == ctx.mCommandHandler.GetAccessingFabricIndex())
            {
                transportConnection.transportConfiguration.transportStatus = transportStatus;
                connectionIDList.push_back(transportConnection.transportConfiguration.connectionID);
            }
        }
    }
    else
    {
        TransportConfigurationStructWithFabricIndex * transportConfiguration = FindStreamTransportConnection(connectionID.Value());
        if (transportConfiguration == nullptr)
        {
            ChipLogError(Zcl, "HandleSetTransportStatus: ConnectionID Not Found.");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }
        if (transportConfiguration->fabricIndex != ctx.mCommandHandler.GetAccessingFabricIndex())
        {
            ChipLogError(Zcl, "HandleSetTransportStatus: ConnectionID Not Found.");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }

        transportConfiguration->transportConfiguration.transportStatus = transportStatus;
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

    TransportConfigurationStructWithFabricIndex * transportConfiguration = FindStreamTransportConnection(connectionID);

    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport: ConnectionID Not Found.");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (transportConfiguration->fabricIndex != ctx.mCommandHandler.GetAccessingFabricIndex())
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport: ConnectionID Not Found.");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (mDelegate.GetTransportStatus(connectionID) == PushAvStreamTransportStatusEnum::kBusy)
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport: Connection is Busy");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
        return;
    }

    if (transportConfiguration->transportConfiguration.transportStatus == TransportStatusEnum::kInactive)
    {
        auto clusterStatus = static_cast<uint8_t>(StatusCodeEnum::kInvalidTransportStatus);
        ChipLogError(Zcl, "HandleManuallyTriggerTransport: Invalid Transport status");
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, clusterStatus);
        return;
    }
    if (transportConfiguration->transportConfiguration.transportOptions.HasValue())
    {
        if (transportConfiguration->transportConfiguration.transportOptions.Value().triggerOptions.triggerType ==
            TransportTriggerTypeEnum::kContinuous)
        {

            auto clusterStatus = static_cast<uint8_t>(StatusCodeEnum::kInvalidTriggerType);
            ChipLogError(Zcl, "HandleManuallyTriggerTransport: Invalid Trigger type");
            ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, clusterStatus);
            return;
        }
        if (transportConfiguration->transportConfiguration.transportOptions.Value().triggerOptions.triggerType ==
                TransportTriggerTypeEnum::kCommand &&
            !timeControl.HasValue())
        {

            ChipLogError(Zcl, "HandleManuallyTriggerTransport: Time control field not present");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
    }

    // Call the delegate
    status = mDelegate.ManuallyTriggerTransport(connectionID, activationReason, timeControl);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void PushAvStreamTransportServer::HandleFindTransport(HandlerContext & ctx,
                                                      const Commands::FindTransport::DecodableType & commandData)
{
    Commands::FindTransportResponse::Type response;

    Optional<DataModel::Nullable<uint16_t>> connectionID = commandData.connectionID;

    size_t count      = 0;
    size_t bufferSize = mCurrentConnections.size();

    Platform::ScopedMemoryBuffer<TransportConfigurationStruct> transportConfigurations;
    if (!transportConfigurations.Calloc(bufferSize))
    {
        ChipLogError(Zcl, "Memory allocation failed for forecast buffer");
        return;
    }

    DataModel::List<const TransportConfigurationStruct> outTransportConfigurations;

    if ((connectionID.HasValue() == false) || connectionID.Value().IsNull())
    {
        if (mCurrentConnections.size() == 0)
        {
            ChipLogError(Zcl, "HandleFindTransport: ConnectionID not found");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }

        for (auto & connection : mCurrentConnections)
        {
            if (connection.fabricIndex == ctx.mCommandHandler.GetAccessingFabricIndex())
            {
                transportConfigurations[count++] = connection.transportConfiguration;
            }
        }
    }
    else
    {
        TransportConfigurationStructWithFabricIndex * transportConfiguration =
            FindStreamTransportConnection(connectionID.Value().Value());
        if (transportConfiguration == nullptr)
        {
            ChipLogError(Zcl, "HandleFindTransport: ConnectionID not found");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }
        if (transportConfiguration->fabricIndex != ctx.mCommandHandler.GetAccessingFabricIndex())
        {
            ChipLogError(Zcl, "HandleFindTransport: ConnectionID Not Found.");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }
        transportConfigurations[count++] = transportConfiguration->transportConfiguration;
    }

    response.transportConfigurations = DataModel::List<const TransportConfigurationStruct>(
        Span<TransportConfigurationStruct>(transportConfigurations.Get(), count));

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
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
