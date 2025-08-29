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

#include "constants.h"
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-logic.h>
#include <app/reporting/reporting.h>
#include <app/util/util.h>
#include <assert.h>
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

PushAvStreamTransportServerLogic::PushAvStreamTransportServerLogic(EndpointId aEndpoint, BitFlags<Feature> aFeatures) :
    mEndpointId(aEndpoint), mFeatures(aFeatures),
    mSupportedFormats{ PushAvStreamTransport::SupportedFormatStruct{ PushAvStreamTransport::ContainerFormatEnum::kCmaf,
                                                                     PushAvStreamTransport::IngestMethodsEnum::kCMAFIngest } }
{}

PushAvStreamTransportServerLogic::~PushAvStreamTransportServerLogic()
{
    for (const auto & timerContext : mTimerContexts)
    {
        DeviceLayer::SystemLayer().CancelTimer(PushAVStreamTransportDeallocateCallback, static_cast<void *>(timerContext.get()));
    }
    Shutdown();
}

CHIP_ERROR PushAvStreamTransportServerLogic::Init()
{
    LoadPersistentAttributes();
    return CHIP_NO_ERROR;
}

void PushAvStreamTransportServerLogic::Shutdown() {}

bool PushAvStreamTransportServerLogic::HasFeature(Feature feature) const
{
    return mFeatures.Has(feature);
}

bool PushAvStreamTransportServerLogic::IsNullDelegateWithLogging(EndpointId endpointIdForLogging)
{

    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "No PushAvStreamTransportDelegate set for ep:%u", endpointIdForLogging);
        return true;
    }
    return false;
}

PushAvStreamTransportServerLogic::UpsertResultEnum
PushAvStreamTransportServerLogic::UpsertStreamTransportConnection(const TransportConfigurationStorage & transportConfiguration)
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

    MatterReportingAttributeChangeCallback(mEndpointId, PushAvStreamTransport::Id,
                                           PushAvStreamTransport::Attributes::CurrentConnections::Id);

    return result;
}

void PushAvStreamTransportServerLogic::RemoveStreamTransportConnection(const uint16_t transportConnectionId)
{
    size_t originalSize = mCurrentConnections.size();

    // Erase-Remove idiom
    mCurrentConnections.erase(std::remove_if(mCurrentConnections.begin(), mCurrentConnections.end(),
                                             [transportConnectionId](const TransportConfigurationStorage & s) {
                                                 return s.connectionID == transportConnectionId;
                                             }),
                              mCurrentConnections.end());

    // If a connection was removed, the size will be smaller.
    if (mCurrentConnections.size() < originalSize)
    {
        // Notify the stack that the CurrentConnections attribute has changed.
        MatterReportingAttributeChangeCallback(mEndpointId, PushAvStreamTransport::Id,
                                               PushAvStreamTransport::Attributes::CurrentConnections::Id);
    }
}

void PushAvStreamTransportServerLogic::RemoveTimerAppState(const uint16_t connectionID)
{
    // Erase-Remove idiom
    auto it = std::remove_if(mTimerContexts.begin(), mTimerContexts.end(),
                             [connectionID](const std::shared_ptr<PushAVStreamTransportDeallocateCallbackContext> & s) {
                                 if (s->connectionID == connectionID)
                                 {
                                     DeviceLayer::SystemLayer().CancelTimer(PushAVStreamTransportDeallocateCallback,
                                                                            static_cast<void *>(s.get()));
                                     return true; // Remove from vector
                                 }
                                 return false; // Keep in vector
                             });

    mTimerContexts.erase(it, mTimerContexts.end());
}

void PushAvStreamTransportServerLogic::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Load currentConnections
    err = mDelegate->LoadCurrentConnections(mCurrentConnections);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "PushAVStreamTransport: Unable to load allocated connections from the KVS.");
    }

    // Signal delegate that all persistent configuration attributes have been loaded.
    mDelegate->PersistentAttributesLoadedCallback();
}

TransportConfigurationStorage * PushAvStreamTransportServerLogic::FindStreamTransportConnection(const uint16_t connectionID)
{
    for (auto & transportConnection : mCurrentConnections)
    {
        if (transportConnection.connectionID == connectionID)
        {
            return &transportConnection;
        }
    }
    return nullptr;
}

TransportConfigurationStorage *
PushAvStreamTransportServerLogic::FindStreamTransportConnectionWithinFabric(const uint16_t connectionID, FabricIndex fabricIndex)
{
    for (auto & transportConnection : mCurrentConnections)
    {
        if (transportConnection.connectionID == connectionID)
        {
            if (transportConnection.GetFabricIndex() == fabricIndex)
            {
                return &transportConnection;
            }
        }
    }
    return nullptr;
}

uint16_t PushAvStreamTransportServerLogic::GenerateConnectionID()
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

void PushAvStreamTransportServerLogic::PushAVStreamTransportDeallocateCallback(System::Layer *, void * callbackContext)
{
    PushAVStreamTransportDeallocateCallbackContext * transportDeallocateContext =
        static_cast<PushAVStreamTransportDeallocateCallbackContext *>((callbackContext));

    uint16_t connectionID = transportDeallocateContext->connectionID;

    // Call the delegate
    auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(
        transportDeallocateContext->instance->mDelegate->DeallocatePushTransport(connectionID));

    if (delegateStatus.IsSuccess())
    {
        ChipLogProgress(Zcl, "Push AV Stream Transport Deallocate timer expired. %s", "Deallocating");

        // Remove connection from CurrentConnections
        transportDeallocateContext->instance->RemoveStreamTransportConnection(connectionID);
        transportDeallocateContext->instance->RemoveTimerAppState(connectionID);
    }
    else
    {
        ChipLogError(Zcl, "Push AV Stream Transport Deallocate timer expired. %s", "Deallocation Failed");
    }
}

CHIP_ERROR PushAvStreamTransportServerLogic::ScheduleTransportDeallocate(uint16_t connectionID, uint32_t timeoutSec)
{
    uint32_t timeoutMs = timeoutSec * MILLISECOND_TICKS_PER_SECOND;

    std::shared_ptr<PushAVStreamTransportDeallocateCallbackContext> transportDeallocateContext{ new (
        std::nothrow) PushAVStreamTransportDeallocateCallbackContext{ this, connectionID } };

    if (transportDeallocateContext == nullptr)
    {
        ChipLogError(Zcl, "Failed to allocate memory for deallocate context");
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(timeoutMs),
                                                           PushAVStreamTransportDeallocateCallback,
                                                           static_cast<void *>(transportDeallocateContext.get()));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to schedule deallocate: timeout=%" PRIu32 ", status=%" CHIP_ERROR_FORMAT, timeoutSec,
                     err.Format());
    }
    else
    {
        mTimerContexts.push_back(transportDeallocateContext);
    }

    return err;
}

Status PushAvStreamTransportServerLogic::ValidateIncomingTransportOptions(
    const Structs::TransportOptionsStruct::DecodableType & transportOptions)
{
    // Contraints check on incoming transport Options
    VerifyOrReturnValue(
        transportOptions.streamUsage != StreamUsageEnum::kUnknownEnumValue, Status::ConstraintError,
        ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Invalid streamUsage ", mEndpointId));

    VerifyOrReturnValue(
        transportOptions.videoStreamID.HasValue() || transportOptions.audioStreamID.HasValue(), Status::InvalidCommand,
        ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Missing videoStreamID and audioStreamID",
                     mEndpointId));

    VerifyOrReturnValue(transportOptions.url.size() >= kMinUrlLength && transportOptions.url.size() <= kMaxUrlLength,
                        Status::ConstraintError,
                        ChipLogError(Zcl,
                                     "Transport Options verification from command data[ep=%d]: URL length: %" PRIu32
                                     " not in allowed length range of 13 to 2000",
                                     mEndpointId, static_cast<uint32_t>(transportOptions.url.size())));

    auto & triggerOptions = transportOptions.triggerOptions;

    VerifyOrReturnValue(
        triggerOptions.triggerType != TransportTriggerTypeEnum::kUnknownEnumValue, Status::ConstraintError,
        ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Invalid triggerType ", mEndpointId));

    if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion)
    {
        VerifyOrReturnValue(
            triggerOptions.motionZones.HasValue(), Status::InvalidCommand,
            ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Missing motion zones ", mEndpointId));

        if (!triggerOptions.motionZones.Value().IsNull())
        {
            auto & motionZonesList = triggerOptions.motionZones;
            auto iter              = motionZonesList.Value().Value().begin();

            while (iter.Next())
            {
                auto & transportZoneOption = iter.GetValue();

                if (mFeatures.Has(Feature::kPerZoneSensitivity))
                {
                    VerifyOrReturnValue(
                        transportZoneOption.sensitivity.HasValue(), Status::InvalidCommand,
                        ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Missing Zone Sensitivity ",
                                     mEndpointId));

                    VerifyOrReturnValue(
                        transportZoneOption.sensitivity.Value() >= 1 && transportZoneOption.sensitivity.Value() <= 10,
                        Status::ConstraintError,
                        ChipLogError(Zcl,
                                     "Transport Options verification from command data[ep=%d]: Zone Sensitivity Constraint Error",
                                     mEndpointId));
                }
                else
                {
                    VerifyOrReturnValue(!transportZoneOption.sensitivity.HasValue(), Status::InvalidCommand,
                                        ChipLogError(Zcl,
                                                     "Transport Options verification from command data[ep=%d]: Found Zone "
                                                     "Sensitivity which is not expected.",
                                                     mEndpointId));
                }
            }

            if (iter.GetStatus() != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Motion Zones TLV Validation failed",
                             mEndpointId);
                return Status::InvalidCommand;
            }
        }

        VerifyOrReturnValue(triggerOptions.motionTimeControl.HasValue(), Status::InvalidCommand,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: Missing Motion Time Control ",
                                         mEndpointId));

        VerifyOrReturnValue(
            triggerOptions.motionTimeControl.Value().initialDuration >= 1, Status::ConstraintError,
            ChipLogError(
                Zcl,
                "Transport Options verification from command data[ep=%d]: Motion Time Control (InitialDuration) Constraint Error",
                mEndpointId));

        VerifyOrReturnValue(
            triggerOptions.motionTimeControl.Value().maxDuration >= 1, Status::ConstraintError,
            ChipLogError(
                Zcl, "Transport Options verification from command data[ep=%d]: Motion Time Control (MaxDuration) Constraint Error",
                mEndpointId));
    }
    else
    {

        VerifyOrReturnValue(
            !triggerOptions.motionZones.HasValue(), Status::InvalidCommand,
            ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Found motion zones which is not expected",
                         mEndpointId));

        VerifyOrReturnValue(
            !triggerOptions.motionTimeControl.HasValue(), Status::InvalidCommand,
            ChipLogError(
                Zcl, "Transport Options verification from command data[ep=%d]: Found Motion Time Control which is not expected ",
                mEndpointId));
    }

    if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion && !mFeatures.Has(Feature::kPerZoneSensitivity))
    {
        VerifyOrReturnValue(
            triggerOptions.motionSensitivity.HasValue(), Status::InvalidCommand,
            ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Missing Motion Sensitivity ", mEndpointId));

        if (!triggerOptions.motionSensitivity.Value().IsNull())
        {
            VerifyOrReturnValue(
                triggerOptions.motionSensitivity.Value().Value() >= 1 && triggerOptions.motionSensitivity.Value().Value() <= 10,
                Status::ConstraintError,
                ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Motion Sensitivity Constraint Error",
                             mEndpointId));
        }
    }
    else
    {
        VerifyOrReturnValue(
            !triggerOptions.motionSensitivity.HasValue(), Status::InvalidCommand,
            ChipLogError(Zcl,
                         "Transport Options verification from command data[ep=%d]: Found Motion Sensitivity which is not expected ",
                         mEndpointId));
    }

    if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion ||
        triggerOptions.triggerType == TransportTriggerTypeEnum::kCommand)
    {
        VerifyOrReturnValue(triggerOptions.maxPreRollLen.HasValue(), Status::InvalidCommand,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: Missing Max Pre Roll Len field ",
                                         mEndpointId));
    }
    else
    {
        VerifyOrReturnValue(
            !triggerOptions.maxPreRollLen.HasValue(), Status::InvalidCommand,
            ChipLogError(
                Zcl, "Transport Options verification from command data[ep=%d]: Found Max Pre Roll Len field which is not expected.",
                mEndpointId));
    }

    IngestMethodsEnum ingestMethod = transportOptions.ingestMethod;

    VerifyOrReturnValue(
        ingestMethod != IngestMethodsEnum::kUnknownEnumValue, Status::ConstraintError,
        ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Invalid Ingest Method ", mEndpointId));

    const Structs::ContainerOptionsStruct::Type & containerOptions = transportOptions.containerOptions;

    VerifyOrReturnValue(
        containerOptions.containerType != ContainerFormatEnum::kUnknownEnumValue, Status::ConstraintError,
        ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Invalid Container Format ", mEndpointId));

    if (containerOptions.containerType == ContainerFormatEnum::kCmaf)
    {
        VerifyOrReturnValue(containerOptions.CMAFContainerOptions.HasValue(), Status::InvalidCommand,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: Missing CMAF Container Options ",
                                         mEndpointId));

        if (containerOptions.CMAFContainerOptions.Value().CENCKey.HasValue())
        {
            VerifyOrReturnValue(
                containerOptions.CMAFContainerOptions.Value().CENCKey.Value().size() == kMaxCENCKeyLength, Status::ConstraintError,
                ChipLogError(Zcl,
                             "Transport Options verification from command data[ep=%d]: CMAF Container Options CENC Key constraint "
                             "Error, actual length: %" PRIu32 " not "
                             "equal to expected length of 16",
                             mEndpointId,
                             static_cast<uint32_t>(containerOptions.CMAFContainerOptions.Value().CENCKey.Value().size())));
        }

        if (!mFeatures.Has(Feature::kMetadata))
        {
            VerifyOrReturnValue(!containerOptions.CMAFContainerOptions.Value().metadataEnabled.HasValue(), Status::InvalidCommand,
                                ChipLogError(Zcl,
                                             "Transport Options verification from command data[ep=%d]: Found CMAF Container "
                                             "Options MetadataEnabled which is not expected.",
                                             mEndpointId));
        }

        if (containerOptions.CMAFContainerOptions.Value().CENCKey.HasValue())
        {
            VerifyOrReturnValue(
                containerOptions.CMAFContainerOptions.Value().CENCKeyID.HasValue(), Status::InvalidCommand,
                ChipLogError(Zcl,
                             "Transport Options verification from command data[ep=%d]: Missing CMAF Container Options CENC Key ID ",
                             mEndpointId));

            VerifyOrReturnValue(
                containerOptions.CMAFContainerOptions.Value().CENCKeyID.Value().size() == kMaxCENCKeyIDLength,
                Status::ConstraintError,
                ChipLogError(Zcl,
                             "Transport Options verification from command data[ep=%d]: CMAF Container Options CENC Key ID "
                             "constraint Error, actual "
                             "length: %" PRIu32 " not equal to expected length of 16",
                             mEndpointId,
                             static_cast<uint32_t>(containerOptions.CMAFContainerOptions.Value().CENCKeyID.Value().size())));
        }
        else
        {
            VerifyOrReturnValue(!containerOptions.CMAFContainerOptions.Value().CENCKeyID.HasValue(), Status::InvalidCommand,
                                ChipLogError(Zcl,
                                             "Transport Options verification from command data[ep=%d]: Found CMAF Container "
                                             "Options CENC Key ID which is not expected",
                                             mEndpointId));
        }
    }
    else
    {
        VerifyOrReturnValue(
            !containerOptions.CMAFContainerOptions.HasValue(), Status::InvalidCommand,
            ChipLogError(
                Zcl, "Transport Options verification from command data[ep=%d]: Found CMAF Container Options which is not expected ",
                mEndpointId));
    }

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
PushAvStreamTransportServerLogic::HandleAllocatePushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                              const Commands::AllocatePushTransport::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    Commands::AllocatePushTransportResponse::Type response;
    auto & transportOptions = commandData.transportOptions;

    Status transportOptionsValidityStatus = ValidateIncomingTransportOptions(transportOptions);

    VerifyOrDo(transportOptionsValidityStatus == Status::Success, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: TransportOptions of command data is not Valid", mEndpointId);
        handler.AddStatus(commandPath, transportOptionsValidityStatus);
        return std::nullopt;
    });

    // Todo: TLSEndpointID Validation

    IngestMethodsEnum ingestMethod = commandData.transportOptions.ingestMethod;

    bool isFormatSupported = false;

    for (auto & supportsFormat : mSupportedFormats)
    {
        if ((supportsFormat.ingestMethod == ingestMethod) &&
            (supportsFormat.containerFormat == commandData.transportOptions.containerOptions.containerType))
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
                     mEndpointId, to_underlying(ingestMethod),
                     to_underlying(commandData.transportOptions.containerOptions.containerType));
        handler.AddClusterSpecificFailure(commandPath, status);
        return std::nullopt;
    }

    bool isValidUrl = mDelegate->ValidateUrl(std::string(transportOptions.url.data(), transportOptions.url.size()));

    if (isValidUrl == false)
    {
        auto status = to_underlying(StatusCodeEnum::kInvalidURL);
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Url", mEndpointId);
        handler.AddClusterSpecificFailure(commandPath, status);
        return std::nullopt;
    }

    /*Spec issue for invalid Trigger Type: https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/11701*/
    if (transportOptions.triggerOptions.triggerType == TransportTriggerTypeEnum::kUnknownEnumValue)
    {
        auto status = to_underlying(StatusCodeEnum::kInvalidTriggerType);
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Trigger type", mEndpointId);
        handler.AddClusterSpecificFailure(commandPath, status);
        return std::nullopt;
    }

    // Todo:Validate ZoneId

    // Validate Bandwidth Requirement
    Status status = mDelegate->ValidateBandwidthLimit(transportOptions.streamUsage, transportOptions.videoStreamID,
                                                      transportOptions.audioStreamID);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Resource Exhausted", mEndpointId);
        handler.AddStatus(commandPath, status);
        return std::nullopt;
    }

    std::shared_ptr<TransportOptionsStorage> transportOptionsPtr{ new (std::nothrow) TransportOptionsStorage(transportOptions) };

    if (transportOptionsPtr == nullptr)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Memory Allocation failed for transportOptions", mEndpointId);
        handler.AddStatus(commandPath, Status::ResourceExhausted);
        return std::nullopt;
    }

    if (transportOptions.videoStreamID.HasValue())
    {
        if (transportOptions.videoStreamID.Value().IsNull())
        {
            uint16_t videoStreamID;

            auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(
                mDelegate->SelectVideoStream(transportOptions.streamUsage, videoStreamID));

            if (!delegateStatus.IsSuccess())
            {
                handler.AddStatus(commandPath, delegateStatus);
                return std::nullopt;
            }

            transportOptionsPtr->videoStreamID.SetValue(videoStreamID);
        }
        else
        {
            auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(
                mDelegate->ValidateVideoStream(transportOptions.videoStreamID.Value().Value()));

            if (!delegateStatus.IsSuccess())
            {
                handler.AddStatus(commandPath, delegateStatus);
                return std::nullopt;
            }
        }
    }

    if (transportOptions.audioStreamID.HasValue())
    {
        if (transportOptions.audioStreamID.Value().IsNull())
        {
            uint16_t audioStreamID;

            auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(
                mDelegate->SelectAudioStream(transportOptions.streamUsage, audioStreamID));

            if (!delegateStatus.IsSuccess())
            {
                handler.AddStatus(commandPath, delegateStatus);
                return std::nullopt;
            }

            transportOptionsPtr->audioStreamID.SetValue(audioStreamID);
        }
        else
        {
            auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(
                mDelegate->ValidateAudioStream(transportOptions.audioStreamID.Value().Value()));

            if (!delegateStatus.IsSuccess())
            {
                handler.AddStatus(commandPath, delegateStatus);
                return std::nullopt;
            }
        }
    }

    uint16_t connectionID = GenerateConnectionID();

    if (connectionID == kMaxConnectionId)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Max Connections Exhausted", mEndpointId);
        handler.AddStatus(commandPath, Status::ResourceExhausted);
        return std::nullopt;
    }

    status = mDelegate->AllocatePushTransport(*transportOptionsPtr, connectionID);

    if (status == Status::Success)
    {
        // add connection to CurrentConnections
        FabricIndex peerFabricIndex = handler.GetAccessingFabricIndex();

        TransportConfigurationStorage outTransportConfiguration(connectionID, transportOptionsPtr);

        outTransportConfiguration.transportStatus = TransportStatusEnum::kInactive;

        outTransportConfiguration.SetFabricIndex(peerFabricIndex);

        UpsertStreamTransportConnection(outTransportConfiguration);

        response.transportConfiguration = outTransportConfiguration;

        // ExpiryTime Handling
        if (transportOptions.expiryTime.HasValue())
        {
            ScheduleTransportDeallocate(connectionID, transportOptions.expiryTime.Value());
        }

        handler.AddResponse(commandPath, response);
    }
    else
    {
        handler.AddStatus(commandPath, status);
    }

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> PushAvStreamTransportServerLogic::HandleDeallocatePushTransport(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const Commands::DeallocatePushTransport::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    uint16_t connectionID                                  = commandData.connectionID;
    FabricIndex FabricIndex                                = handler.GetAccessingFabricIndex();
    TransportConfigurationStorage * transportConfiguration = FindStreamTransportConnectionWithinFabric(connectionID, FabricIndex);
    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleDeallocatePushTransport[ep=%d]: ConnectionID Not Found.", mEndpointId);
        handler.AddStatus(commandPath, Status::NotFound);
        return std::nullopt;
    }

    // Call the delegate
    auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(mDelegate->DeallocatePushTransport(connectionID));

    if (delegateStatus.IsSuccess())
    {
        // Remove connection from CurrentConnections
        RemoveStreamTransportConnection(connectionID);
        RemoveTimerAppState(connectionID);
    }

    handler.AddStatus(commandPath, delegateStatus);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
PushAvStreamTransportServerLogic::HandleModifyPushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                            const Commands::ModifyPushTransport::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    Status status           = Status::Success;
    uint16_t connectionID   = commandData.connectionID;
    auto & transportOptions = commandData.transportOptions;

    // Contraints check on incoming transport Options
    Status transportOptionsValidityStatus = ValidateIncomingTransportOptions(transportOptions);

    VerifyOrDo(transportOptionsValidityStatus == Status::Success, {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: TransportOptions of command data is not Valid", mEndpointId);
        handler.AddStatus(commandPath, transportOptionsValidityStatus);
        return std::nullopt;
    });

    FabricIndex fabricIndex = handler.GetAccessingFabricIndex();

    TransportConfigurationStorage * transportConfiguration = FindStreamTransportConnectionWithinFabric(connectionID, fabricIndex);

    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: ConnectionID Not Found.", mEndpointId);
        handler.AddStatus(commandPath, Status::NotFound);
        return std::nullopt;
    }

    if (mDelegate->GetTransportBusyStatus(connectionID) == PushAvStreamTransportStatusEnum::kBusy)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: Connection is Busy", mEndpointId);
        handler.AddStatus(commandPath, Status::Busy);
        return std::nullopt;
    }

    std::shared_ptr<TransportOptionsStorage> transportOptionsPtr{ new (std::nothrow) TransportOptionsStorage(transportOptions) };

    if (transportOptionsPtr == nullptr)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: Memory Allocation failed for transportOptions", mEndpointId);
        handler.AddStatus(commandPath, Status::ResourceExhausted);
        return std::nullopt;
    }

    // Call the delegate
    status = mDelegate->ModifyPushTransport(connectionID, *transportOptionsPtr);

    if (status == Status::Success)
    {
        transportConfiguration->SetTransportOptionsPtr(transportOptionsPtr);
    }

    handler.AddStatus(commandPath, status);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
PushAvStreamTransportServerLogic::HandleSetTransportStatus(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                           const Commands::SetTransportStatus::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    Status status                              = Status::Success;
    DataModel::Nullable<uint16_t> connectionID = commandData.connectionID;
    auto & transportStatus                     = commandData.transportStatus;

    VerifyOrDo(transportStatus != TransportStatusEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleSetTransportStatus[ep=%d]: Invalid Transport Status in command data", mEndpointId);
        handler.AddStatus(commandPath, Status::ConstraintError);
        return std::nullopt;
    });

    std::vector<uint16_t> connectionIDList;

    if (connectionID.IsNull())
    {
        for (auto & transportConnection : mCurrentConnections)
        {
            if (transportConnection.fabricIndex == handler.GetAccessingFabricIndex())
            {
                connectionIDList.push_back(transportConnection.connectionID);
            }
        }
    }
    else
    {
        FabricIndex fabricIndex = handler.GetAccessingFabricIndex();
        TransportConfigurationStorage * transportConfiguration =
            FindStreamTransportConnectionWithinFabric(connectionID.Value(), fabricIndex);
        if (transportConfiguration == nullptr)
        {
            ChipLogError(Zcl, "HandleSetTransportStatus[ep=%d]: ConnectionID Not Found.", mEndpointId);
            handler.AddStatus(commandPath, Status::NotFound);
            return std::nullopt;
        }
        connectionIDList.push_back(connectionID.Value());
    }

    // Call the delegate
    status = mDelegate->SetTransportStatus(connectionIDList, transportStatus);
    if (status == Status::Success)
    {
        for (auto & connID : connectionIDList)
        {
            for (auto & transportConnection : mCurrentConnections)
            {
                if (transportConnection.connectionID == connID)
                {
                    transportConnection.transportStatus = transportStatus;
                }
            }
        }
    }
    handler.AddStatus(commandPath, status);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> PushAvStreamTransportServerLogic::HandleManuallyTriggerTransport(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const Commands::ManuallyTriggerTransport::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    Status status           = Status::Success;
    uint16_t connectionID   = commandData.connectionID;
    auto & activationReason = commandData.activationReason;

    VerifyOrDo(activationReason != TriggerActivationReasonEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Invalid Activation Reason ", mEndpointId);
        handler.AddStatus(commandPath, Status::ConstraintError);
        return std::nullopt;
    });

    Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> timeControl = commandData.timeControl;

    if (timeControl.HasValue())
    {
        VerifyOrDo(timeControl.Value().initialDuration >= 1, {
            ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Motion Time Control (InitialDuration) Constraint Error",
                         mEndpointId);
            handler.AddStatus(commandPath, Status::ConstraintError);
            return std::nullopt;
        });

        VerifyOrDo(timeControl.Value().maxDuration >= 1, {
            ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Motion Time Control (MaxDuration) Constraint Error",
                         mEndpointId);
            handler.AddStatus(commandPath, Status::ConstraintError);
            return std::nullopt;
        });
    }

    FabricIndex fabricIndex                                = handler.GetAccessingFabricIndex();
    TransportConfigurationStorage * transportConfiguration = FindStreamTransportConnectionWithinFabric(connectionID, fabricIndex);

    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: ConnectionID Not Found.", mEndpointId);
        handler.AddStatus(commandPath, Status::NotFound);
        return std::nullopt;
    }

    if (mDelegate->GetTransportBusyStatus(connectionID) == PushAvStreamTransportStatusEnum::kBusy)
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Connection is Busy", mEndpointId);
        handler.AddStatus(commandPath, Status::Busy);
        return std::nullopt;
    }

    if (transportConfiguration->transportStatus == TransportStatusEnum::kInactive)
    {
        auto clusterStatus = to_underlying(StatusCodeEnum::kInvalidTransportStatus);
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Invalid Transport status", mEndpointId);
        handler.AddClusterSpecificFailure(commandPath, clusterStatus);
        return std::nullopt;
    }

    if (transportConfiguration->transportOptions.HasValue())
    {
        if (transportConfiguration->transportOptions.Value().triggerOptions.triggerType == TransportTriggerTypeEnum::kContinuous)
        {
            auto clusterStatus = to_underlying(StatusCodeEnum::kInvalidTriggerType);
            ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Invalid Trigger type", mEndpointId);
            handler.AddClusterSpecificFailure(commandPath, clusterStatus);
            return std::nullopt;
        }

        if (transportConfiguration->transportOptions.Value().triggerOptions.triggerType == TransportTriggerTypeEnum::kCommand &&
            !timeControl.HasValue())
        {
            ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Time control field not present", mEndpointId);
            handler.AddStatus(commandPath, Status::DynamicConstraintError);
            return std::nullopt;
        }
    }

    // When trigger type is motion in the allocated transport but triggering it manually
    if (!timeControl.HasValue())
    {
        timeControl = transportConfiguration->transportOptions.Value().triggerOptions.motionTimeControl;
    }

    // Call the delegate
    status = mDelegate->ManuallyTriggerTransport(connectionID, activationReason, timeControl);

    if (status == Status::Success)
    {
        GeneratePushTransportBeginEvent(connectionID, TransportTriggerTypeEnum::kCommand, MakeOptional(activationReason));
    }

    handler.AddStatus(commandPath, status);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
PushAvStreamTransportServerLogic::HandleFindTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                      const Commands::FindTransport::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    Commands::FindTransportResponse::Type response;

    DataModel::Nullable<uint16_t> connectionID = commandData.connectionID;

    std::vector<Structs::TransportConfigurationStruct::Type> transportConfigurations;

    if (connectionID.IsNull())
    {
        if (mCurrentConnections.size() == 0)
        {
            ChipLogError(Zcl, "HandleFindTransport[ep=%d]: ConnectionID not found", mEndpointId);
            handler.AddStatus(commandPath, Status::NotFound);
            return std::nullopt;
        }

        for (auto & connection : mCurrentConnections)
        {
            if (connection.fabricIndex == handler.GetAccessingFabricIndex())
            {
                transportConfigurations.push_back(connection);
            }
        }
    }
    else
    {
        FabricIndex fabricIndex = handler.GetAccessingFabricIndex();
        TransportConfigurationStorage * transportConfiguration =
            FindStreamTransportConnectionWithinFabric(connectionID.Value(), fabricIndex);
        if (transportConfiguration == nullptr)
        {
            ChipLogError(Zcl, "HandleFindTransport[ep=%d]: ConnectionID not found", mEndpointId);
            handler.AddStatus(commandPath, Status::NotFound);
            return std::nullopt;
        }

        transportConfigurations.push_back(*transportConfiguration);
    }

    if (transportConfigurations.size() == 0)
    {
        handler.AddStatus(commandPath, Status::NotFound);
        return std::nullopt;
    }

    response.transportConfigurations = DataModel::List<const Structs::TransportConfigurationStruct::Type>(
        transportConfigurations.data(), transportConfigurations.size());

    handler.AddResponse(commandPath, response);

    return std::nullopt;
}

Status
PushAvStreamTransportServerLogic::GeneratePushTransportBeginEvent(const uint16_t connectionID,
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

Status PushAvStreamTransportServerLogic::GeneratePushTransportEndEvent(const uint16_t connectionID,
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

} // namespace Clusters
} // namespace app
} // namespace chip
