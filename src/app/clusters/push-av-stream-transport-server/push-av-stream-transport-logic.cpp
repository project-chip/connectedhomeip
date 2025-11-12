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
#include <set>

static constexpr uint16_t kMaxConnectionId = 65535; // This is also invalid connectionID
static constexpr uint16_t kMaxEndpointId   = 65534;

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

PushAvStreamTransportServerLogic::~PushAvStreamTransportServerLogic() {}

CHIP_ERROR PushAvStreamTransportServerLogic::Init()
{
    LoadPersistentAttributes();
    return CHIP_NO_ERROR;
}

void PushAvStreamTransportServerLogic::Shutdown()
{
    for (const auto & timerContext : mTimerContexts)
    {
        DeviceLayer::SystemLayer().CancelTimer(PushAVStreamTransportDeallocateCallback, static_cast<void *>(timerContext.get()));
    }
}

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
    // Load currentConnections
    ChipLogFailure(mDelegate->LoadCurrentConnections(mCurrentConnections), Zcl,
                   "PushAVStreamTransport: Unable to load allocated connections from the KVS.");

    // Signal delegate that all persistent configuration attributes have been loaded.
    TEMPORARY_RETURN_IGNORED mDelegate->PersistentAttributesLoadedCallback();
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

bool PushAvStreamTransportServerLogic::ValidateUrl(const std::string & url)
{
    const std::string https = "https://";

    // Check minimum length and https prefix
    if (url.size() <= https.size() || url.substr(0, https.size()) != https)
    {
        return false;
    }

    // Check that URL does not contain fragment character '#'
    if (url.find('#') != std::string::npos)
    {
        ChipLogError(Camera, "URL contains fragment character '#'");
        return false;
    }

    // Check that URL does not contain query character '?'
    if (url.find('?') != std::string::npos)
    {
        ChipLogError(Camera, "URL contains query character '?'");
        return false;
    }

    // Check that URL ends with a forward slash '/'
    if (url.back() != '/')
    {
        ChipLogError(Camera, "URL does not end with '/'");
        return false;
    }

    // Extract host part
    size_t hostStart = https.size();
    size_t hostEnd   = url.find('/', hostStart);
    std::string host = url.substr(hostStart, hostEnd - hostStart);

    // Basic host validation: ensure non-empty
    if (host.empty())
    {
        ChipLogError(Camera, "URL does not contain a valid host.");
        return false;
    }

    // Accept any host as long as non-empty
    return true;
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
    VerifyOrReturnValue(transportOptions.TLSEndpointID <= kMaxEndpointId, Status::ConstraintError,
                        ChipLogError(Zcl,
                                     "Transport Options verification from command data[ep=%d]: EndpointID field Constraint Error",
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

        CMAFInterfaceEnum cmafInterface = containerOptions.CMAFContainerOptions.Value().CMAFInterface;

        VerifyOrReturnValue(
            cmafInterface == CMAFInterfaceEnum::kInterface1 || cmafInterface == CMAFInterfaceEnum::kInterface2DASH ||
                cmafInterface == CMAFInterfaceEnum::kInterface2HLS,
            Status::ConstraintError,
            ChipLogError(Zcl,
                         "Transport Options verification from command data[ep=%d]: CMAF Container Options CMAFInterface not valid",
                         mEndpointId));

        uint16_t segmentDuration = containerOptions.CMAFContainerOptions.Value().segmentDuration;

        VerifyOrReturnValue(segmentDuration >= kMinSegmentDuration && segmentDuration <= kMaxSegmentDuration,
                            Status::ConstraintError,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: CMAF Container Options Segment "
                                         "Duration field not within allowed range",
                                         mEndpointId));

        uint16_t chunkDuration = containerOptions.CMAFContainerOptions.Value().chunkDuration;

        VerifyOrReturnValue(chunkDuration >= 0 && chunkDuration <= segmentDuration / 2, Status::ConstraintError,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: CMAF Container Options Chunk "
                                         "Duration field not within allowed range",
                                         mEndpointId));

        VerifyOrReturnValue(
            containerOptions.CMAFContainerOptions.Value().trackName.size() >= 1 &&
                containerOptions.CMAFContainerOptions.Value().trackName.size() <= kMaxTrackNameLength,
            Status::ConstraintError,
            ChipLogError(Zcl,
                         "Transport Options verification from command data[ep=%d]: CMAF Container Options Track Name "
                         "Error, actual length: %" PRIu32 " not "
                         "within expected range of 1 to 16",
                         mEndpointId, static_cast<uint32_t>(containerOptions.CMAFContainerOptions.Value().trackName.size())));

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

    // Handle constraint checks against the provided command fields
    Status transportOptionsValidityStatus = ValidateIncomingTransportOptions(transportOptions);

    VerifyOrDo(transportOptionsValidityStatus == Status::Success, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: TransportOptions of command data is not Valid", mEndpointId);
        handler.AddStatus(commandPath, transportOptionsValidityStatus);
        return std::nullopt;
    });

    // Validate the TLS Endpoint
    if (mTLSClientManagementDelegate != nullptr)
    {
        CHIP_ERROR tlsEndpointValidityStatus = mTLSClientManagementDelegate->FindProvisionedEndpointByID(
            commandPath.mEndpointId, handler.GetAccessingFabricIndex(), commandData.transportOptions.TLSEndpointID,
            [&](auto & TLSEndpoint) -> CHIP_ERROR {
                // Use heap allocation for large certificate buffers to reduce stack usage
                auto rootCertBuffer   = std::make_unique<PersistentStore<CHIP_CONFIG_TLS_PERSISTED_ROOT_CERT_BYTES>>();
                auto clientCertBuffer = std::make_unique<PersistentStore<CHIP_CONFIG_TLS_PERSISTED_CLIENT_CERT_BYTES>>();

                Tls::CertificateTable::BufferedClientCert clientCertEntry(*clientCertBuffer);
                Tls::CertificateTable::BufferedRootCert rootCertEntry(*rootCertBuffer);

                if (mTlsCertificateManagementDelegate != nullptr)
                {
                    auto & table = mTlsCertificateManagementDelegate->GetCertificateTable();
                    ReturnErrorOnFailure(table.GetClientCertificateEntry(handler.GetAccessingFabricIndex(),
                                                                         TLSEndpoint.ccdid.Value(), clientCertEntry));
                    ReturnErrorOnFailure(
                        table.GetRootCertificateEntry(handler.GetAccessingFabricIndex(), TLSEndpoint.caid, rootCertEntry));
                    mDelegate->SetTLSCerts(clientCertEntry, rootCertEntry);
                }
                else
                {
                    // For tests, create empty cert entries
                    mDelegate->SetTLSCerts(clientCertEntry, rootCertEntry);
                }
                return CHIP_NO_ERROR;
            });

        VerifyOrDo(tlsEndpointValidityStatus == CHIP_NO_ERROR, {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: TLSEndpointID of command data is not valid/Provisioned",
                         mEndpointId);
            auto status = to_underlying(StatusCodeEnum::kInvalidTLSEndpoint);
            TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
            return std::nullopt;
        });
    }
    else
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: TLS Client Management Delegate is not set", mEndpointId);
        auto status = to_underlying(StatusCodeEnum::kInvalidTLSEndpoint);
        TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
        return std::nullopt;
    }

    // IngestMethod has alread been validated against the constraints above, this is handling the subsequent required spec logic
    // to cross-reference use against the ContainerType
    IngestMethodsEnum ingestMethod = commandData.transportOptions.ingestMethod;

    bool isFormatSupported = false;

    for (auto & supportsFormat : mSupportedFormats)
    {
        if ((supportsFormat.ingestMethod == ingestMethod) &&
            (supportsFormat.containerFormat == commandData.transportOptions.containerOptions.containerType))
        {
            isFormatSupported = true;
            break;
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
        TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
        return std::nullopt;
    }

    if (transportOptions.containerOptions.containerType == ContainerFormatEnum::kCmaf &&
        transportOptions.containerOptions.CMAFContainerOptions.HasValue())
    {
        bool isValidUrl = ValidateUrl(std::string(transportOptions.url.data(), transportOptions.url.size()));

        if (isValidUrl == false)
        {
            auto status = to_underlying(StatusCodeEnum::kInvalidURL);
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Url", mEndpointId);
            TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
            return std::nullopt;
        }
    }

    bool isValidStreamUsage = mDelegate->ValidateStreamUsage(transportOptions.streamUsage);
    if (isValidStreamUsage == false)
    {
        auto status = to_underlying(StatusCodeEnum::kInvalidStreamUsage);
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: StreamUsage not present in the StreamUsagePriorities list",
                     mEndpointId);
        TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
        return std::nullopt;
    }

    // Validate the motion zones in the trigger options
    if ((transportOptions.triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion) &&
        (transportOptions.triggerOptions.motionZones.HasValue()) && (!transportOptions.triggerOptions.motionZones.Value().IsNull()))
    {

        auto & motionZonesList = transportOptions.triggerOptions.motionZones;
        auto iterDupCheck      = motionZonesList.Value().Value().begin();
        size_t zoneListSize    = 0;
        TEMPORARY_RETURN_IGNORED motionZonesList.Value().Value().ComputeSize(&zoneListSize);

        // If there are duplicate entries, reject the command
        std::set<uint16_t> zoneIDsFound;
        bool nullFound = false;

        while (iterDupCheck.Next())
        {
            auto & transportZoneOption = iterDupCheck.GetValue();
            if (!transportZoneOption.zone.IsNull())
            {
                uint16_t zoneID = transportZoneOption.zone.Value();
                if (zoneIDsFound.count(zoneID) == 0)
                {
                    // Zone ID not found, add to set of IDs
                    zoneIDsFound.emplace(zoneID);
                }
                else
                {
                    // This is a duplicate
                    ChipLogError(
                        Zcl, "Transport Options verification from command data[ep=%d]: Duplicate Zone ID (=%d) in Motion Zones. ",
                        mEndpointId, zoneID);
                    return Status::AlreadyExists;
                }
            }
            else
            {
                if (nullFound)
                {
                    // This is the second null, therefore also a duplicate entry
                    ChipLogError(
                        Zcl, "Transport Options verification from command data[ep=%d]: Duplicate Null Zone ID in Motion Zones. ",
                        mEndpointId);
                    return Status::AlreadyExists;
                }
                nullFound = true;
            }
        }

        bool isValidZoneSize = mDelegate->ValidateMotionZoneListSize(zoneListSize);
        VerifyOrReturnValue(
            isValidZoneSize, Status::DynamicConstraintError,
            ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Invalid Motion Zone Size ", mEndpointId));

        auto iter = motionZonesList.Value().Value().begin();
        while (iter.Next())
        {
            auto & transportZoneOption = iter.GetValue();

            // The Zone can be Null, meaning this trigger applies to all zones, if not null, verify with the delegate that the
            // ZoneID (in the ZoneManagement cluster instance) is valid
            if (!transportZoneOption.zone.IsNull())
            {
                Status zoneIdStatus = mDelegate->ValidateZoneId(transportZoneOption.zone.Value());
                if (zoneIdStatus != Status::Success)
                {
                    auto status = to_underlying(StatusCodeEnum::kInvalidZone);
                    ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid ZoneId", mEndpointId);
                    TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
                    return std::nullopt;
                }
            }
        }
    }

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
                mDelegate->SetVideoStream(transportOptions.videoStreamID.Value().Value()));

            if (!delegateStatus.IsSuccess())
            {
                auto cluster_status = to_underlying(StatusCodeEnum::kInvalidStream);
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Video Stream ", mEndpointId);
                TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, cluster_status);
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
                mDelegate->SetAudioStream(transportOptions.audioStreamID.Value().Value()));

            if (!delegateStatus.IsSuccess())
            {
                auto cluster_status = to_underlying(StatusCodeEnum::kInvalidStream);
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Audio Stream ", mEndpointId);
                TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, cluster_status);
                return std::nullopt;
            }
        }
    }

    if (transportOptions.containerOptions.containerType == ContainerFormatEnum::kCmaf &&
        transportOptions.containerOptions.CMAFContainerOptions.HasValue())
    {
        // SegmentDuration validation is restricted to video streams because SegmentDuration must be a multiple of
        // KeyFrameInterval. See spec issue: https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/12322
        if (transportOptionsPtr->videoStreamID.HasValue())
        {
            bool isValidSegmentDuration = mDelegate->ValidateSegmentDuration(
                transportOptions.containerOptions.CMAFContainerOptions.Value().segmentDuration, transportOptionsPtr->videoStreamID);
            if (isValidSegmentDuration == false)
            {
                auto segmentDurationStatus = to_underlying(StatusCodeEnum::kInvalidOptions);
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Segment Duration not within allowed range", mEndpointId);
                TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, segmentDurationStatus);
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

    FabricIndex accessingFabricIndex = handler.GetAccessingFabricIndex();

    status = mDelegate->AllocatePushTransport(*transportOptionsPtr, connectionID, accessingFabricIndex);

    if (status == Status::Success)
    {
        // add connection to CurrentConnections
        TransportConfigurationStorage outTransportConfiguration(connectionID, transportOptionsPtr);

        outTransportConfiguration.transportStatus = TransportStatusEnum::kInactive;

        outTransportConfiguration.SetFabricIndex(accessingFabricIndex);

        UpsertStreamTransportConnection(outTransportConfiguration);

        response.transportConfiguration = outTransportConfiguration;

        // ExpiryTime Handling
        if (transportOptions.expiryTime.HasValue())
        {
            TEMPORARY_RETURN_IGNORED ScheduleTransportDeallocate(connectionID, transportOptions.expiryTime.Value());
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

    if (mDelegate->GetTransportBusyStatus(connectionID) == PushAvStreamTransportStatusEnum::kBusy)
    {
        ChipLogError(Zcl, "HandleDeallocatePushTransport[ep=%d]: Connection is Busy", mEndpointId);
        handler.AddStatus(commandPath, Status::Busy);
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

    // Note, we will always have Transport Options as they're mandatory in the initial allocate
    status = CheckPrivacyModes(transportConfiguration->transportOptions.Value().streamUsage);
    if (status != Status::Success)
    {
        handler.AddStatus(commandPath, status);
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
        TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, clusterStatus);
        return std::nullopt;
    }

    if (transportConfiguration->transportOptions.Value().triggerOptions.triggerType == TransportTriggerTypeEnum::kContinuous)
    {
        auto clusterStatus = to_underlying(StatusCodeEnum::kInvalidTriggerType);
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Invalid Trigger type", mEndpointId);
        TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, clusterStatus);
        return std::nullopt;
    }

    if (transportConfiguration->transportOptions.Value().triggerOptions.triggerType == TransportTriggerTypeEnum::kCommand &&
        !timeControl.HasValue())
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Time control field not present", mEndpointId);
        handler.AddStatus(commandPath, Status::DynamicConstraintError);
        return std::nullopt;
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

Status PushAvStreamTransportServerLogic::CheckPrivacyModes(StreamUsageEnum streamUsage)
{
    bool hardPrivacyModeActive = false;

    CHIP_ERROR err = mDelegate->IsHardPrivacyModeActive(hardPrivacyModeActive);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "PushAvStreamTransport:CheckPrivacyModes: Failed to check Hard Privacy mode: %" CHIP_ERROR_FORMAT,
                     err.Format());
        return Status::Failure;
    }

    if (hardPrivacyModeActive)
    {
        ChipLogError(Zcl, "PushAvStreamTransport:CheckPrivacyModes: Hard Privacy mode is enabled");
        return Status::InvalidInState;
    }

    bool softLivestreamPrivacyModeActive = false;
    err                                  = mDelegate->IsSoftLivestreamPrivacyModeActive(softLivestreamPrivacyModeActive);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl,
                     "PushAvStreamTransport:CheckPrivacyModes: Failed to check Soft LivestreamPrivacy mode: %" CHIP_ERROR_FORMAT,
                     err.Format());
        return Status::Failure;
    }

    if (softLivestreamPrivacyModeActive && streamUsage == StreamUsageEnum::kLiveView)
    {
        ChipLogError(Zcl,
                     "PushAvStreamTransport:CheckPrivacyModes: Soft LivestreamPrivacy mode is enabled and StreamUsage is LiveView");
        return Status::InvalidInState;
    }

    bool softRecordingPrivacyModeActive = false;
    err                                 = mDelegate->IsSoftRecordingPrivacyModeActive(softRecordingPrivacyModeActive);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl,
                     "PushAvStreamTransport:CheckPrivacyModes: Failed to check SoftRecordingPrivacyModeActive: %" CHIP_ERROR_FORMAT,
                     err.Format());
        return Status::Failure;
    }

    if (softRecordingPrivacyModeActive && (streamUsage == StreamUsageEnum::kRecording || streamUsage == StreamUsageEnum::kAnalysis))
    {
        ChipLogError(Zcl,
                     "PushAvStreamTransport:CheckPrivacyModes: Soft RecordingPrivacy mode is enabled and StreamUsage is Recording "
                     "or Analysis");
        return Status::InvalidInState;
    }

    return Status::Success;
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

Status PushAvStreamTransportServerLogic::GeneratePushTransportEndEvent(const uint16_t connectionID)
{
    Events::PushTransportEnd::Type event;
    EventNumber eventNumber;

    event.connectionID = connectionID;

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate PushAVTransportEnd event: %" CHIP_ERROR_FORMAT, mEndpointId,
                     err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

Status PushAvStreamTransportServerLogic::NotifyTransportStarted(uint16_t connectionID, TransportTriggerTypeEnum triggerType,
                                                                Optional<TriggerActivationReasonEnum> activationReason)
{
    ChipLogProgress(Zcl, "NotifyTransportStarted called for connectionID %u with triggerType %u", connectionID,
                    to_underlying(triggerType));

    // Validate that the connection exists
    TransportConfigurationStorage * transportConfig = FindStreamTransportConnection(connectionID);
    if (transportConfig == nullptr)
    {
        ChipLogError(Zcl, "NotifyTransportStarted: ConnectionID %u not found", connectionID);
        return Status::NotFound;
    }

    // Generate the PushTransportBegin event
    return GeneratePushTransportBeginEvent(connectionID, triggerType, activationReason);
}

Status PushAvStreamTransportServerLogic::NotifyTransportStopped(uint16_t connectionID, TransportTriggerTypeEnum triggerType)
{
    ChipLogProgress(Zcl, "NotifyTransportStopped called for connectionID %u with triggerType %u", connectionID,
                    to_underlying(triggerType));

    // Validate that the connection exists
    TransportConfigurationStorage * transportConfig = FindStreamTransportConnection(connectionID);
    if (transportConfig == nullptr)
    {
        ChipLogError(Zcl, "NotifyTransportStopped: ConnectionID %u not found", connectionID);
        return Status::NotFound;
    }

    // Generate the PushTransportEnd event
    return GeneratePushTransportEndEvent(connectionID);
}

} // namespace Clusters
} // namespace app
} // namespace chip
