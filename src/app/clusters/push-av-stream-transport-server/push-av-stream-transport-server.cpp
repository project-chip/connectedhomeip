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
#include <app/EventLogging.h>
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
{
    /* set the base class delegates endpointId */
    mDelegate.SetEndpointId(aEndpointId);
}

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
    const TransportConfigurationStorageWithFabricIndex & transportConfiguration)
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
                                             [transportConnectionId](const TransportConfigurationStorageWithFabricIndex & s) {
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

TransportConfigurationStorageWithFabricIndex *
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

void PushAvStreamTransportServer::PushAVStreamTransportDeallocateCallback(System::Layer *, void * callbackContext)
{
    PushAVStreamTransportDeallocateCallbackContext * transportDeallocateContext =
        static_cast<PushAVStreamTransportDeallocateCallbackContext *>((callbackContext));

    uint16_t connectionID = transportDeallocateContext->connectionID;

    // Call the delegate
    auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(
        transportDeallocateContext->instance->mDelegate.DeallocatePushTransport(connectionID));

    if (delegateStatus.IsSuccess() == true)
    {
        ChipLogProgress(Zcl, "Push AV Stream Transport Deallocate timer expired. %s", "Deallocating");

        // Remove connection from CurrentConnections
        transportDeallocateContext->instance->RemoveStreamTransportConnection(connectionID);
    }
    else
    {
        ChipLogError(Zcl, "Push AV Stream Transport Deallocate timer expired. %s", "Deallocation Failed");
    }

    delete transportDeallocateContext;
}

void PushAvStreamTransportServer::ScheduleTransportDeallocate(uint16_t connectionID, uint32_t timeoutSec)
{
    uint32_t timeoutMs = timeoutSec * MILLISECOND_TICKS_PER_SECOND;

    PushAVStreamTransportDeallocateCallbackContext * transportDeallocateContext =
        new (std::nothrow) PushAVStreamTransportDeallocateCallbackContext{ this, connectionID };

    if (transportDeallocateContext == nullptr)
    {
        ChipLogError(Zcl, "Failed to allocate memory for deallocate context");
        return;
    }

    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(timeoutMs),
                                                           PushAVStreamTransportDeallocateCallback,
                                                           static_cast<void *>(transportDeallocateContext));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to schedule deallocate: timeout=%" PRIu32 ", status=%" CHIP_ERROR_FORMAT, timeoutSec,
                     err.Format());
    }
}

void PushAvStreamTransportServer::HandleAllocatePushTransport(HandlerContext & ctx,
                                                              const Commands::AllocatePushTransport::DecodableType & commandData)
{
    Commands::AllocatePushTransportResponse::Type response;
    auto & transportOptions = commandData.transportOptions;

    // Contraints check on incoming transport Options

    VerifyOrReturn(transportOptions.streamUsage != StreamUsageEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid streamUsage ",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    VerifyOrReturn(transportOptions.videoStreamID.HasValue() || transportOptions.audioStreamID.HasValue(), {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing videoStreamID and audioStreamID",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    VerifyOrReturn(transportOptions.url.size() <= 2000, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing videoStreamID and audioStreamID",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
    });

    auto & triggerOptions = transportOptions.triggerOptions;

    VerifyOrReturn(triggerOptions.triggerType != TransportTriggerTypeEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid triggerType ",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion)
    {
        VerifyOrReturn(triggerOptions.motionZones.HasValue(), {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing motion zones ",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        });

        if (triggerOptions.motionZones.Value().IsNull() == false)
        {
            auto & motionZonesList = triggerOptions.motionZones;
            auto iter              = motionZonesList.Value().Value().begin();

            while (iter.Next())
            {
                auto & transportZoneOption = iter.GetValue();

                if (mFeatures.Has(Feature::kPerZoneSensitivity))
                {
                    VerifyOrReturn(transportZoneOption.sensitivity.HasValue(), {
                        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing Zone Sensitivity ",
                                     AttributeAccessInterface::GetEndpointId().Value());
                        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
                    });

                    VerifyOrReturn(transportZoneOption.sensitivity.Value() >= 1 && transportZoneOption.sensitivity.Value() <= 10, {
                        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Zone Sensitivity Constraint Error",
                                     AttributeAccessInterface::GetEndpointId().Value());
                        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
                    });
                }
            }
        }

        if (mFeatures.Has(Feature::kPerZoneSensitivity) == false)
        {
            VerifyOrReturn(triggerOptions.motionSensitivity.HasValue(), {
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing Motion Sensitivity ",
                             AttributeAccessInterface::GetEndpointId().Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            });
            VerifyOrReturn(
                triggerOptions.motionSensitivity.Value().Value() >= 1 && triggerOptions.motionSensitivity.Value().Value() <= 10, {
                    ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Motion Sensitivity Constraint Error",
                                 AttributeAccessInterface::GetEndpointId().Value());
                    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
                });
        }

        VerifyOrReturn(triggerOptions.motionTimeControl.HasValue(), {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing Motion Time Control ",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        });

        VerifyOrReturn(triggerOptions.motionTimeControl.Value().initialDuration >= 1, {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Motion Time Control (InitialDuration) Constraint Error",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        });

        VerifyOrReturn(triggerOptions.motionTimeControl.Value().maxDuration >= 1, {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Motion Time Control (MaxDuration) Constraint Error",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        });
    }

    if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion ||
        triggerOptions.triggerType == TransportTriggerTypeEnum::kCommand)
    {
        VerifyOrReturn(triggerOptions.maxPreRollLen.HasValue(), {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing Max Pre Roll Len field ",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        });
    }

    VerifyOrReturn(transportOptions.ingestMethod != IngestMethodsEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Ingest Method ",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    // Todo: TLSEndpointID Validation

    IngestMethodsEnum ingestMethod = commandData.transportOptions.ingestMethod;

    VerifyOrReturn(transportOptions.ingestMethod != IngestMethodsEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Ingest Method ",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    ContainerOptionsStruct containerOptions = commandData.transportOptions.containerOptions;

    VerifyOrReturn(containerOptions.containerType != ContainerFormatEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Container Format ",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    if (containerOptions.containerType == ContainerFormatEnum::kCmaf)
    {
        VerifyOrReturn(containerOptions.CMAFContainerOptions.HasValue(), {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing CMAF Container Options ",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        });

        if (containerOptions.CMAFContainerOptions.Value().CENCKey.HasValue())
        {
            VerifyOrReturn(containerOptions.CMAFContainerOptions.Value().CENCKey.Value().size() <= kMaxCENCKeyLength, {
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: CMAF Container Options CENC Key constraint Error",
                             AttributeAccessInterface::GetEndpointId().Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            });
        }

        if (mFeatures.Has(Feature::kMetadata))
        {
            VerifyOrReturn(containerOptions.CMAFContainerOptions.Value().metadataEnabled.HasValue(), {
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing CMAF Container Options MetadataEnabled ",
                             AttributeAccessInterface::GetEndpointId().Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            });
        }

        if (containerOptions.CMAFContainerOptions.Value().CENCKey.HasValue())
        {
            VerifyOrReturn(containerOptions.CMAFContainerOptions.Value().CENCKeyID.HasValue(), {
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing CMAF Container Options CENC Key ID ",
                             AttributeAccessInterface::GetEndpointId().Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            });

            VerifyOrReturn(containerOptions.CMAFContainerOptions.Value().CENCKeyID.Value().size() <= kMaxCENCKeyIDLength, {
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: CMAF Container Options CENC Key ID constraint Error",
                             AttributeAccessInterface::GetEndpointId().Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            });
        }
    }

    bool isFormatSupported = false;

    for (auto & supportsFormat : mSupportedFormats)
    {
        if ((supportsFormat.ingestMethod == ingestMethod) && (supportsFormat.containerFormat == containerOptions.containerType))
        {
            isFormatSupported = true;
        }
    }

    if (isFormatSupported == false)
    {
        auto status = to_underlying(StatusCodeEnum::kInvalidCombination);
        ChipLogError(Zcl,
                     "HandleAllocatePushTransport[ep=%d]: Invalid Ingest Method and Container Format Combination : (Ingest Method: "
                     "%02X and Container Format: %02X)",
                     AttributeAccessInterface::GetEndpointId().Value(), static_cast<uint8_t>(ingestMethod),
                     static_cast<uint8_t>(containerOptions.containerType));
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, status);
        return;
    }

    bool isValidUrl = mDelegate.ValidateUrl(std::string(transportOptions.url.data(), transportOptions.url.size()));

    if (isValidUrl == false)
    {
        auto status = to_underlying(StatusCodeEnum::kInvalidURL);
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Url", AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, status);
        return;
    }

    /*Spec issue for invalid Trigger Type: https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/11701*/
    if (transportOptions.triggerOptions.triggerType == TransportTriggerTypeEnum::kUnknownEnumValue)
    {
        auto status = to_underlying(StatusCodeEnum::kInvalidTriggerType);
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Trigger type",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, status);
        return;
    }

    // Validate ZoneId

    // Validate Bandwidth Requirement
    CHIP_ERROR err = mDelegate.ValidateBandwidthLimit(transportOptions.streamUsage, transportOptions.videoStreamID,
                                                      transportOptions.audioStreamID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Resource Exhausted",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
        return;
    }

    // Validate the StreamUsageEnum as per resource management and stream priorities.
    err =
        mDelegate.ValidateStreamUsage(transportOptions.streamUsage, transportOptions.videoStreamID, transportOptions.audioStreamID);
    if (err != CHIP_NO_ERROR)
    {
        auto status = to_underlying(StatusCodeEnum::kInvalidStream);
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Stream", AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, status);
        return;
    }

    uint16_t connectionID = GenerateConnectionID();

    if (connectionID == kMaxConnectionId)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Max Connections Exhausted",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
        return;
    }

    std::shared_ptr<TransportOptionsStorage> transportOptionsPtr{ new (std::nothrow) TransportOptionsStorage(transportOptions) };

    if (transportOptionsPtr == nullptr)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Memory Allocation failed for transportOptions",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
        return;
    }

    TransportConfigurationStorage outTransportConfiguration(connectionID, transportOptionsPtr);

    Status status = mDelegate.AllocatePushTransport(*transportOptionsPtr, connectionID);

    if (status == Status::Success)
    {
        // add connection to CurrentConnections
        FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

        TransportConfigurationStorageWithFabricIndex transportConfiguration({ outTransportConfiguration, peerFabricIndex });

        UpsertStreamTransportConnection(transportConfiguration);
        response.transportConfiguration = outTransportConfiguration;

        // ExpiryTime Handling
        if (transportOptions.expiryTime.HasValue())
        {
            ScheduleTransportDeallocate(connectionID, transportOptions.expiryTime.Value());
        }

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
    uint16_t connectionID                                                 = commandData.connectionID;
    TransportConfigurationStorageWithFabricIndex * transportConfiguration = FindStreamTransportConnection(connectionID);
    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleDeallocatePushTransport[ep=%d]: ConnectionID Not Found.",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (transportConfiguration->fabricIndex != ctx.mCommandHandler.GetAccessingFabricIndex())
    {
        ChipLogError(Zcl, "HandleDeallocatePushTransport[ep=%d]: ConnectionID Not Found.",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Call the delegate
    auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(mDelegate.DeallocatePushTransport(connectionID));

    if (delegateStatus.IsSuccess() == true)
    {
        // Remove connection from CurrentConnections
        RemoveStreamTransportConnection(connectionID);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, delegateStatus);
}

void PushAvStreamTransportServer::HandleModifyPushTransport(HandlerContext & ctx,
                                                            const Commands::ModifyPushTransport::DecodableType & commandData)
{
    Status status           = Status::Success;
    uint16_t connectionID   = commandData.connectionID;
    auto & transportOptions = commandData.transportOptions;

    // Contraints check on incoming transport Options

    VerifyOrReturn(transportOptions.streamUsage != StreamUsageEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid streamUsage ",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    VerifyOrReturn(transportOptions.videoStreamID.HasValue() || transportOptions.audioStreamID.HasValue(), {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing videoStreamID and audioStreamID",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    VerifyOrReturn(transportOptions.url.size() <= 2000, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing videoStreamID and audioStreamID",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
    });

    auto & triggerOptions = transportOptions.triggerOptions;

    VerifyOrReturn(triggerOptions.triggerType != TransportTriggerTypeEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid triggerType ",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion)
    {
        VerifyOrReturn(triggerOptions.motionZones.HasValue(), {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing motion zones ",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        });

        if (triggerOptions.motionZones.Value().IsNull() == false)
        {
            auto & motionZonesList = triggerOptions.motionZones;
            auto iter              = motionZonesList.Value().Value().begin();

            while (iter.Next())
            {
                auto & transportZoneOption = iter.GetValue();

                if (mFeatures.Has(Feature::kPerZoneSensitivity))
                {
                    VerifyOrReturn(transportZoneOption.sensitivity.HasValue(), {
                        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing Zone Sensitivity ",
                                     AttributeAccessInterface::GetEndpointId().Value());
                        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
                    });

                    VerifyOrReturn(transportZoneOption.sensitivity.Value() >= 1 && transportZoneOption.sensitivity.Value() <= 10, {
                        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Zone Sensitivity Constraint Error",
                                     AttributeAccessInterface::GetEndpointId().Value());
                        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
                    });
                }
            }
        }

        if (mFeatures.Has(Feature::kPerZoneSensitivity) == false)
        {
            VerifyOrReturn(triggerOptions.motionSensitivity.HasValue(), {
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing Motion Sensitivity ",
                             AttributeAccessInterface::GetEndpointId().Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            });
            VerifyOrReturn(
                triggerOptions.motionSensitivity.Value().Value() >= 1 && triggerOptions.motionSensitivity.Value().Value() <= 10, {
                    ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Motion Sensitivity Constraint Error",
                                 AttributeAccessInterface::GetEndpointId().Value());
                    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
                });
        }

        VerifyOrReturn(triggerOptions.motionTimeControl.HasValue(), {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing Motion Time Control ",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        });

        VerifyOrReturn(triggerOptions.motionTimeControl.Value().initialDuration >= 1, {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Motion Time Control (InitialDuration) Constraint Error",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        });

        VerifyOrReturn(triggerOptions.motionTimeControl.Value().maxDuration >= 1, {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Motion Time Control (MaxDuration) Constraint Error",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        });
    }

    if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion ||
        triggerOptions.triggerType == TransportTriggerTypeEnum::kCommand)
    {
        VerifyOrReturn(triggerOptions.maxPreRollLen.HasValue(), {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Missing Max Pre Roll Len field ",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        });
    }

    VerifyOrReturn(transportOptions.ingestMethod != IngestMethodsEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Ingest Method ",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    TransportConfigurationStorageWithFabricIndex * transportConfiguration = FindStreamTransportConnection(connectionID);

    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: ConnectionID Not Found.",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (transportConfiguration->fabricIndex != ctx.mCommandHandler.GetAccessingFabricIndex())
    {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: ConnectionID Not Found.",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (mDelegate.GetTransportStatus(connectionID) == PushAvStreamTransportStatusEnum::kBusy)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: Connection is Busy",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
        return;
    }

    std::shared_ptr<TransportOptionsStorage> transportOptionsPtr{ new (std::nothrow) TransportOptionsStorage(transportOptions) };

    if (transportOptionsPtr == nullptr)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: Memory Allocation failed for transportOptions",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
        return;
    }
    // Call the delegate
    status = mDelegate.ModifyPushTransport(connectionID, transportOptions);

    if (status == Status::Success)
    {
        transportConfiguration->transportConfiguration.SetTransportOptionsPtr(transportOptionsPtr);
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
                connectionIDList.push_back(transportConnection.transportConfiguration.connectionID);
            }
        }
    }
    else
    {
        TransportConfigurationStorageWithFabricIndex * transportConfiguration = FindStreamTransportConnection(connectionID.Value());
        if (transportConfiguration == nullptr)
        {
            ChipLogError(Zcl, "HandleSetTransportStatus[ep=%d]: ConnectionID Not Found.",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }
        if (transportConfiguration->fabricIndex != ctx.mCommandHandler.GetAccessingFabricIndex())
        {
            ChipLogError(Zcl, "HandleSetTransportStatus[ep=%d]: ConnectionID Not Found.",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }
        connectionIDList.push_back(connectionID.Value());
    }
    // Call the delegate
    status = mDelegate.SetTransportStatus(connectionIDList, transportStatus);
    if (status == Status::Success)
    {
        for (auto & connID : connectionIDList)
        {
            for (auto & transportConnection : mCurrentConnections)
            {
                if (transportConnection.transportConfiguration.connectionID == connID)
                {
                    transportConnection.transportConfiguration.transportStatus = transportStatus;
                }
            }
        }
    }
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void PushAvStreamTransportServer::HandleManuallyTriggerTransport(
    HandlerContext & ctx, const Commands::ManuallyTriggerTransport::DecodableType & commandData)
{
    Status status           = Status::Success;
    uint16_t connectionID   = commandData.connectionID;
    auto & activationReason = commandData.activationReason;

    VerifyOrReturn(activationReason != TriggerActivationReasonEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Invalid Activation Reason ",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> timeControl = commandData.timeControl;

    if (timeControl.HasValue())
    {
        VerifyOrReturn(timeControl.Value().initialDuration >= 1, {
            ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Motion Time Control (InitialDuration) Constraint Error",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        });

        VerifyOrReturn(timeControl.Value().maxDuration >= 1, {
            ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Motion Time Control (MaxDuration) Constraint Error",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        });
    }

    TransportConfigurationStorageWithFabricIndex * transportConfiguration = FindStreamTransportConnection(connectionID);

    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: ConnectionID Not Found.",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (transportConfiguration->fabricIndex != ctx.mCommandHandler.GetAccessingFabricIndex())
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: ConnectionID Not Found.",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (mDelegate.GetTransportStatus(connectionID) == PushAvStreamTransportStatusEnum::kBusy)
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Connection is Busy",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
        return;
    }

    if (transportConfiguration->transportConfiguration.transportStatus == TransportStatusEnum::kInactive)
    {
        auto clusterStatus = to_underlying(StatusCodeEnum::kInvalidTransportStatus);
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Invalid Transport status",
                     AttributeAccessInterface::GetEndpointId().Value());
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, clusterStatus);
        return;
    }
    if (transportConfiguration->transportConfiguration.transportOptions.HasValue())
    {
        if (transportConfiguration->transportConfiguration.transportOptions.Value().triggerOptions.triggerType ==
            TransportTriggerTypeEnum::kContinuous)
        {

            auto clusterStatus = to_underlying(StatusCodeEnum::kInvalidTriggerType);
            ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Invalid Trigger type",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, clusterStatus);
            return;
        }
        if (transportConfiguration->transportConfiguration.transportOptions.Value().triggerOptions.triggerType ==
                TransportTriggerTypeEnum::kCommand &&
            timeControl.HasValue() == false)
        {

            ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Time control field not present",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
            return;
        }
    }

    // When trigger type is motion in the allocated transport but triggering it manually
    if (timeControl.HasValue() == false)
    {
        timeControl = transportConfiguration->transportConfiguration.transportOptions.Value().triggerOptions.motionTimeControl;
    }

    // Call the delegate
    status = mDelegate.ManuallyTriggerTransport(connectionID, activationReason, timeControl);

    if (status == Status::Success)
    {
        mDelegate.GeneratePushTransportBeginEvent(connectionID, TransportTriggerTypeEnum::kCommand, MakeOptional(activationReason));
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void PushAvStreamTransportServer::HandleFindTransport(HandlerContext & ctx,
                                                      const Commands::FindTransport::DecodableType & commandData)
{
    Commands::FindTransportResponse::Type response;

    Optional<DataModel::Nullable<uint16_t>> connectionID = commandData.connectionID;

    std::vector<TransportConfigurationStruct> transportConfigurations;

    if ((connectionID.HasValue() == false) || connectionID.Value().IsNull())
    {
        if (mCurrentConnections.size() == 0)
        {
            ChipLogError(Zcl, "HandleFindTransport[ep=%d]: ConnectionID not found",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }

        for (auto & connection : mCurrentConnections)
        {
            if (connection.fabricIndex == ctx.mCommandHandler.GetAccessingFabricIndex())
            {
                transportConfigurations.push_back(connection.transportConfiguration);
            }
        }
    }
    else
    {
        TransportConfigurationStorageWithFabricIndex * transportConfiguration =
            FindStreamTransportConnection(connectionID.Value().Value());
        if (transportConfiguration == nullptr)
        {
            ChipLogError(Zcl, "HandleFindTransport[ep=%d]: ConnectionID not found",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }
        if (transportConfiguration->fabricIndex != ctx.mCommandHandler.GetAccessingFabricIndex())
        {
            ChipLogError(Zcl, "HandleFindTransport[ep=%d]: ConnectionID Not Found.",
                         AttributeAccessInterface::GetEndpointId().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }
        transportConfigurations.push_back(transportConfiguration->transportConfiguration);
    }

    if (transportConfigurations.size() == 0)
    {
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, Status::NotFound);
    }

    response.transportConfigurations =
        DataModel::List<const TransportConfigurationStruct>(transportConfigurations.data(), transportConfigurations.size());

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

Status PushAvStreamTransportDelegate::GeneratePushTransportBeginEvent(const uint16_t connectionID,
                                                                      const TransportTriggerTypeEnum triggerType,
                                                                      const Optional<TriggerActivationReasonEnum> activationReason)
{
    Events::PushTransportBegin::Type event;
    EventNumber eventNumber;

    event.connectionID     = connectionID;
    event.triggerType      = triggerType;
    event.activationReason = activationReason;

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate PushAVTransportBegin event: %" CHIP_ERROR_FORMAT, mEndpointId,
                     err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

Status PushAvStreamTransportDelegate::GeneratePushTransportEndEvent(const uint16_t connectionID,
                                                                    const TransportTriggerTypeEnum triggerType,
                                                                    const Optional<TriggerActivationReasonEnum> activationReason)
{
    Events::PushTransportEnd::Type event;
    EventNumber eventNumber;

    event.connectionID     = connectionID;
    event.triggerType      = triggerType;
    event.activationReason = activationReason;

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate PushAVTransportEnd event: %" CHIP_ERROR_FORMAT, mEndpointId,
                     err.Format());
        return Status::Failure;
    }
    return Status::Success;
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
