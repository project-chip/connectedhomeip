/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "webrtc-transport-provider-cluster.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/StatusCode.h>

#include <iterator>
#include <memory>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportProvider;
using namespace chip::app::Clusters::WebRTCTransportProvider::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr uint16_t kMaxSessionId = 65534;

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    Commands::SolicitOffer::kMetadataEntry,         Commands::ProvideOffer::kMetadataEntry, Commands::ProvideAnswer::kMetadataEntry,
    Commands::ProvideICECandidates::kMetadataEntry, Commands::EndSession::kMetadataEntry,
};

constexpr DataModel::AttributeEntry kMandatoryAttributes[] = {
    CurrentSessions::kMetadataEntry,
};

NodeId GetNodeIdFromCtx(const CommandHandler & commandHandler)
{
    auto descriptor = commandHandler.GetSubjectDescriptor();

    if (descriptor.authMode != Access::AuthMode::kCase)
    {
        return kUndefinedNodeId;
    }
    return descriptor.subject;
}

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportProvider {

WebRTCTransportProviderServer::WebRTCTransportProviderServer(EndpointId endpointId, Delegate & delegate) :
    DefaultServerCluster({ endpointId, Id }), mDelegate(delegate)
{}

DataModel::ActionReturnStatus WebRTCTransportProviderServer::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case CurrentSessions::Id:
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            for (auto & session : mCurrentSessions)
            {
                ReturnErrorOnFailure(listEncoder.Encode(session));
            }
            return CHIP_NO_ERROR;
        });
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case FeatureMap::Id:
        // TODO: Allow delegate to specify supported features
        // Currently hardcoded to 0 (no features supported)
        // METADATA feature (bit 0) should be configurable based on delegate capabilities
        return encoder.Encode<uint32_t>(0);
    default:
        return Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> WebRTCTransportProviderServer::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                          TLV::TLVReader & input_arguments,
                                                                                          CommandHandler * handler)
{
    FabricIndex accessingFabricIndex = handler->GetAccessingFabricIndex();

    switch (request.path.mCommandId)
    {
    case Commands::SolicitOffer::Id: {
        Commands::SolicitOffer::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleSolicitOffer(*handler, req);
    }
    case Commands::ProvideOffer::Id: {
        Commands::ProvideOffer::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleProvideOffer(*handler, req);
    }
    case Commands::ProvideAnswer::Id: {
        Commands::ProvideAnswer::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleProvideAnswer(*handler, req);
    }
    case Commands::ProvideICECandidates::Id: {
        Commands::ProvideICECandidates::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleProvideICECandidates(*handler, req);
    }
    case Commands::EndSession::Id: {
        Commands::EndSession::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleEndSession(*handler, req);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR WebRTCTransportProviderServer::AcceptedCommands(const ConcreteClusterPath & path,
                                                           ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR WebRTCTransportProviderServer::Attributes(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryAttributes), Span<AttributeListBuilder::OptionalAttributeEntry>());
}

// Helper functions
WebRTCSessionStruct * WebRTCTransportProviderServer::FindSession(uint16_t sessionId)
{
    for (auto & session : mCurrentSessions)
    {
        if (session.id == sessionId)
        {
            return &session;
        }
    }

    return nullptr;
}

WebRTCTransportProviderServer::UpsertResultEnum WebRTCTransportProviderServer::UpsertSession(const WebRTCSessionStruct & session)
{
    UpsertResultEnum result;
    auto it = std::find_if(mCurrentSessions.begin(), mCurrentSessions.end(),
                           [id = session.id](const auto & existing) { return existing.id == id; });

    if (it != mCurrentSessions.end())
    {
        *it    = session;
        result = UpsertResultEnum::kUpdated;
    }
    else
    {
        mCurrentSessions.push_back(session);
        result = UpsertResultEnum::kInserted;
    }

    NotifyAttributeChanged(Attributes::CurrentSessions::Id);

    return result;
}

void WebRTCTransportProviderServer::RemoveSession(uint16_t sessionId)
{
    size_t originalSize = mCurrentSessions.size();

    // Erase-Remove idiom
    mCurrentSessions.erase(std::remove_if(mCurrentSessions.begin(), mCurrentSessions.end(),
                                          [sessionId](const WebRTCSessionStruct & s) { return s.id == sessionId; }),
                           mCurrentSessions.end());

    // If a session was removed, the size will be smaller.
    if (mCurrentSessions.size() < originalSize)
    {
        NotifyAttributeChanged(Attributes::CurrentSessions::Id);
    }
}

WebRTCSessionStruct * WebRTCTransportProviderServer::CheckForMatchingSession(const CommandHandler & commandHandler,
                                                                             uint16_t sessionId)
{
    WebRTCSessionStruct * session = FindSession(sessionId);
    if (session == nullptr)
    {
        return nullptr;
    }

    NodeId peerNodeId           = GetNodeIdFromCtx(commandHandler);
    FabricIndex peerFabricIndex = commandHandler.GetAccessingFabricIndex();

    // Ensure the session’s peer matches the current command invoker
    if (peerNodeId != session->peerNodeID || peerFabricIndex != session->GetFabricIndex())
    {
        return nullptr;
    }

    return session;
}

uint16_t WebRTCTransportProviderServer::GenerateSessionId()
{
    static uint16_t lastSessionId = 0;
    uint16_t candidateId          = 0;

    // Try at most kMaxSessionId+1 attempts to find a free ID
    // This ensures we never loop infinitely even if all IDs are somehow in use
    for (uint16_t attempts = 0; attempts <= kMaxSessionId; attempts++)
    {
        candidateId = lastSessionId++;

        // Handle wrap-around per spec
        if (lastSessionId > kMaxSessionId)
        {
            lastSessionId = 0;
        }

        if (FindSession(candidateId) == nullptr)
        {
            return candidateId;
        }
    }

    // This should never happen in practice since we support 65534 sessions
    // and typical applications will have far fewer active sessions
    ChipLogError(Zcl, "All session IDs are in use!");
    chipDie();
}

// Command Handlers
DataModel::ActionReturnStatus WebRTCTransportProviderServer::HandleSolicitOffer(CommandHandler & commandHandler,
                                                                                const Commands::SolicitOffer::DecodableType & req)
{
    // Validate the streamUsage field against the allowed enum values.
    if (req.streamUsage == StreamUsageEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleSolicitOffer: Invalid streamUsage value %u.", to_underlying(req.streamUsage));
        return Status::ConstraintError;
    }

    bool privacyModeActive = false;
    if (mDelegate.IsPrivacyModeActive(privacyModeActive) != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleSolicitOffer: Cannot determine privacy mode state");
        return Status::InvalidInState;
    }

    if (privacyModeActive)
    {
        ChipLogError(Zcl, "HandleSolicitOffer: Privacy mode is enabled");
        return Status::InvalidInState;
    }

    // At least one of Video Stream ID and Audio Stream ID has to be present
    if (!req.videoStreamID.HasValue() && !req.audioStreamID.HasValue())
    {
        ChipLogError(Zcl, "HandleSolicitOffer: one of VideoStreamID or AudioStreamID must be present");
        return Status::InvalidCommand;
    }

    // Validate VideoStreamID against AllocatedVideoStreams.
    // If present and null then a stream has to have been allocated.
    // If present and not null, then the stream ID has to exist
    if (req.videoStreamID.HasValue())
    {
        if (req.videoStreamID.Value().IsNull())
        {
            // Is there an allocated stream, delegate handles matching against an allocated stream in the HandleSolicitOffer method
            if (!mDelegate.HasAllocatedVideoStreams())
            {
                ChipLogError(Zcl, "HandleSolicitOffer: video requested when there are no AllocatedVideoStreams");
                return Status::InvalidInState;
            }
        }
        else
        {
            // Delegate should validate against AllocatedVideoStreams
            if (mDelegate.ValidateVideoStreamID(req.videoStreamID.Value().Value()) != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "HandleSolicitOffer: VideoStreamID %u does not match AllocatedVideoStreams",
                             req.videoStreamID.Value().Value());
                return Status::DynamicConstraintError;
            }
        }
    }

    // Validate AudioStreamID against AllocatedAudioStreams if present
    // If present and null then a stream has to have been allocated.
    // If present and not null, then the stream ID has to exist
    if (req.audioStreamID.HasValue())
    {
        if (req.audioStreamID.Value().IsNull())
        {
            // Is there an allocated stream, delegate handles matching against an allocated stream in the HandleSolicitOffer method
            if (!mDelegate.HasAllocatedAudioStreams())
            {
                ChipLogError(Zcl, "HandleSolicitOffer: audio requested when there are no AllocatedAudioStreams");
                return Status::InvalidInState;
            }
        }
        else
        {
            // Delegate should validate against AllocatedAudioStreams
            if (mDelegate.ValidateAudioStreamID(req.audioStreamID.Value().Value()) != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "HandleSolicitOffer: AudioStreamID %u does not match AllocatedAudioStreams",
                             req.audioStreamID.Value().Value());
                return Status::DynamicConstraintError;
            }
        }
    }

    // Prepare the arguments for the delegate.
    Delegate::OfferRequestArgs args;
    args.sessionId             = GenerateSessionId();
    args.streamUsage           = req.streamUsage;
    args.videoStreamId         = req.videoStreamID;
    args.audioStreamId         = req.audioStreamID;
    args.peerNodeId            = GetNodeIdFromCtx(commandHandler);
    args.fabricIndex           = commandHandler.GetAccessingFabricIndex();
    args.originatingEndpointId = req.originatingEndpointID;

    if (req.ICEServers.HasValue())
    {
        std::vector<ICEServerDecodableStruct> localIceServers;

        auto iter = req.ICEServers.Value().begin();
        while (iter.Next())
        {
            // Just move the decodable struct as-is, only valid during this method call
            localIceServers.push_back(std::move(iter.GetValue()));
        }

        // Check the validity of the list.
        CHIP_ERROR listErr = iter.GetStatus();
        if (listErr != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleSolicitOffer: ICECandidates list error: %" CHIP_ERROR_FORMAT, listErr.Format());
            return Status::InvalidCommand;
        }

        args.iceServers.SetValue(std::move(localIceServers));
    }

    if (req.ICETransportPolicy.HasValue())
    {
        args.iceTransportPolicy.SetValue(std::string(req.ICETransportPolicy.Value().data(), req.ICETransportPolicy.Value().size()));
    }

    // Delegate processing:
    // The delegate implementation SHALL:
    // - Populate a new WebRTCSessionStruct with the requested values.
    // - If in standby mode, set deferredOffer to true (and perform steps to exit standby within 30 seconds).
    // - If not in standby mode, ensure that VideoStreamID and AudioStreamID are valid (or allocate/select new streams if null).
    // - Internally increment the ReferenceCount on any used video/audio streams.
    // - If resources cannot be allocated, invoke End with Reason OutOfResources.
    WebRTCSessionStruct outSession;
    bool deferredOffer = false;

    auto status = ClusterStatusCode(mDelegate.HandleSolicitOffer(args, outSession, deferredOffer));
    if (!status.IsSuccess())
    {
        return status;
    }

    // Store or update the session.
    if (UpsertSession(outSession) == UpsertResultEnum::kInserted)
    {
        ChipLogProgress(Zcl, "WebRTCTransportProvider: Inserted a new session with ID=%u.", outSession.id);
    }
    else
    {
        ChipLogProgress(Zcl, "WebRTCTransportProvider: Updated existing session with ID=%u.", outSession.id);
    }

    // Prepare the SolicitOfferResponse command.
    Commands::SolicitOfferResponse::Type resp;
    resp.webRTCSessionID = outSession.id;
    resp.deferredOffer   = deferredOffer;

    // If VideoStreamID was in the request, it should be in the response
    if (req.videoStreamID.HasValue())
    {
        resp.videoStreamID.SetValue(outSession.videoStreamID);
    }

    // If AudioStreamID was in the request, it should be in the response
    if (req.audioStreamID.HasValue())
    {
        resp.audioStreamID.SetValue(outSession.audioStreamID);
    }

    ConcreteCommandPath requestPath(mPath.mEndpointId, Id, Commands::SolicitOffer::Id);
    commandHandler.AddResponse(requestPath, resp);

    return Status::Success;
}

DataModel::ActionReturnStatus WebRTCTransportProviderServer::HandleProvideOffer(CommandHandler & commandHandler,
                                                                                const Commands::ProvideOffer::DecodableType & req)
{
    auto webRTCSessionID = req.webRTCSessionID;
    auto videoStreamID   = req.videoStreamID;
    auto audioStreamID   = req.audioStreamID;

    NodeId peerNodeId           = GetNodeIdFromCtx(commandHandler);
    FabricIndex peerFabricIndex = commandHandler.GetAccessingFabricIndex();

    WebRTCSessionStruct outSession;

    // Prepare delegate arguments for the session
    Delegate::ProvideOfferRequestArgs args;

    // Validate the streamUsage field against the allowed enum values.
    if (req.streamUsage == StreamUsageEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleProvideOffer: Invalid streamUsage value %u.", to_underlying(req.streamUsage));
        return Status::ConstraintError;
    }

    // If WebRTCSessionID is not null and does not match a value in CurrentSessions: Respond with NOT_FOUND.
    if (!webRTCSessionID.IsNull())
    {
        uint16_t sessionId                    = webRTCSessionID.Value();
        WebRTCSessionStruct * existingSession = CheckForMatchingSession(commandHandler, sessionId);
        if (existingSession == nullptr)
        {
            return Status::NotFound;
        }

        // Use the existing session for further processing (re-offer case).
        outSession = *existingSession;

        // re-use the same session id for offer processing in delegate
        args.sessionId = sessionId;
    }
    else
    {
        // WebRTCSessionID is null - new session request

        // Check privacy mode settings - if either is true, fail with INVALID_IN_STATE
        bool privacyModeActive = false;
        if (mDelegate.IsPrivacyModeActive(privacyModeActive) != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleProvideOffer: Cannot determine privacy mode state");
            return Status::InvalidInState;
        }

        if (privacyModeActive)
        {
            ChipLogError(Zcl, "HandleProvideOffer: Privacy mode is enabled");
            return Status::InvalidInState;
        }

        // At least one of Video Stream ID and Audio Stream ID has to be present
        if (!req.videoStreamID.HasValue() && !req.audioStreamID.HasValue())
        {
            ChipLogError(Zcl, "HandleProvideOffer: one of VideoStreamID or AudioStreamID must be present");
            return Status::InvalidCommand;
        }

        // Validate VideoStreamID against AllocatedVideoStreams.
        // If present and null then a stream has to have been allocated.
        // If present and not null, then the stream ID has to exist
        if (videoStreamID.HasValue() && !videoStreamID.Value().IsNull())
        {
            if (mDelegate.ValidateVideoStreamID(videoStreamID.Value().Value()) != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "HandleProvideOffer: VideoStreamID %u does not match AllocatedVideoStreams",
                             videoStreamID.Value().Value());
                return Status::DynamicConstraintError;
            }
        }
        else if (videoStreamID.HasValue() && videoStreamID.Value().IsNull())
        {
            // VideoStreamID is present and is null - need to automatically select
            // First check if there are any video streams allocated
            if (!mDelegate.HasAllocatedVideoStreams())
            {
                ChipLogError(Zcl, "HandleProvideOffer: No video streams currently allocated");
                return Status::InvalidInState;
            }
            // Automatic selection will be handled by the delegate in HandleProvideOffer.
        }

        // Validate AudioStreamID if present and not null.
        if (audioStreamID.HasValue() && !audioStreamID.Value().IsNull())
        {
            if (mDelegate.ValidateAudioStreamID(audioStreamID.Value().Value()) != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "HandleProvideOffer: AudioStreamID %u does not match AllocatedAudioStreams",
                             audioStreamID.Value().Value());
                return Status::DynamicConstraintError;
            }
        }
        else if (audioStreamID.HasValue() && audioStreamID.Value().IsNull())
        {
            // AudioStreamID is present and is null - need to automatically select
            // First check if there are any audio streams allocated
            if (!mDelegate.HasAllocatedAudioStreams())
            {
                ChipLogError(Zcl, "HandleProvideOffer: No audio streams currently allocated");
                return Status::InvalidInState;
            }
            // Automatic selection will be handled by the delegate in HandleProvideOffer.
        }

        // Check resource management and stream priorities
        CHIP_ERROR err = mDelegate.ValidateStreamUsage(req.streamUsage, req.videoStreamID, req.audioStreamID);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleProvideOffer: Cannot meet resource management conditions");
            return Status::ResourceExhausted;
        }

        // Generate new sessiond id
        args.sessionId = GenerateSessionId();
    }

    args.streamUsage           = req.streamUsage;
    args.videoStreamId         = videoStreamID;
    args.audioStreamId         = audioStreamID;
    args.peerNodeId            = peerNodeId;
    args.fabricIndex           = peerFabricIndex;
    args.sdp                   = std::string(req.sdp.data(), req.sdp.size());
    args.originatingEndpointId = req.originatingEndpointID;

    // Convert ICE servers list from DecodableList to vector.
    if (req.ICEServers.HasValue())
    {
        std::vector<ICEServerDecodableStruct> localIceServers;

        auto iter = req.ICEServers.Value().begin();
        while (iter.Next())
        {
            localIceServers.push_back(std::move(iter.GetValue()));
        }

        CHIP_ERROR listErr = iter.GetStatus();
        if (listErr != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleProvideOffer: ICEServers list error: %" CHIP_ERROR_FORMAT, listErr.Format());
            return Status::InvalidCommand;
        }

        args.iceServers.SetValue(std::move(localIceServers));
    }

    // Convert ICETransportPolicy from CharSpan to std::string.
    if (req.ICETransportPolicy.HasValue())
    {
        args.iceTransportPolicy.SetValue(std::string(req.ICETransportPolicy.Value().data(), req.ICETransportPolicy.Value().size()));
    }

    // Delegate processing: process the SDP offer, create session, increment reference counts.
    auto status = Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleProvideOffer(args, outSession));
    if (!status.IsSuccess())
    {
        return status;
    }

    // Update/Insert the WebRTCSessionStruct in CurrentSessions.
    UpsertSession(outSession);

    // Build and send the ProvideOfferResponse.
    Commands::ProvideOfferResponse::Type resp;
    resp.webRTCSessionID = outSession.id;

    // Set VideoStreamID only if present in the original request.
    if (req.videoStreamID.HasValue())
    {
        resp.videoStreamID.SetValue(outSession.videoStreamID);
    }

    // Set AudioStreamID only if present in the original request.
    if (req.audioStreamID.HasValue())
    {
        resp.audioStreamID.SetValue(outSession.audioStreamID);
    }

    ConcreteCommandPath requestPath(mPath.mEndpointId, Id, Commands::ProvideOffer::Id);
    commandHandler.AddResponse(requestPath, resp);

    return Status::Success;
}

DataModel::ActionReturnStatus WebRTCTransportProviderServer::HandleProvideAnswer(CommandHandler & commandHandler,
                                                                                 const Commands::ProvideAnswer::DecodableType & req)
{
    uint16_t sessionId = req.webRTCSessionID;
    auto sdpSpan       = req.sdp;

    WebRTCSessionStruct * existingSession = CheckForMatchingSession(commandHandler, sessionId);
    if (existingSession == nullptr)
    {
        return Status::NotFound;
    }

    std::string sdpAnswer(sdpSpan.data(), sdpSpan.size());
    return mDelegate.HandleProvideAnswer(sessionId, sdpAnswer);
}

DataModel::ActionReturnStatus
WebRTCTransportProviderServer::HandleProvideICECandidates(CommandHandler & commandHandler,
                                                          const Commands::ProvideICECandidates::DecodableType & req)
{
    uint16_t sessionId = req.webRTCSessionID;

    std::vector<ICECandidateStruct> candidates;
    auto iter = req.ICECandidates.begin();
    while (iter.Next())
    {
        // Get current candidate.
        const ICECandidateStruct & candidate = iter.GetValue();
        candidates.push_back(candidate);
    }

    // Check the validity of the list.
    CHIP_ERROR listErr = iter.GetStatus();
    if (listErr != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleProvideICECandidates: ICECandidates list error: %" CHIP_ERROR_FORMAT, listErr.Format());
        return Status::InvalidCommand;
    }

    WebRTCSessionStruct * existingSession = CheckForMatchingSession(commandHandler, sessionId);
    if (existingSession == nullptr)
    {
        return Status::NotFound;
    }

    // Delegate the handling of ICE candidates.
    return mDelegate.HandleProvideICECandidates(sessionId, candidates);
}

DataModel::ActionReturnStatus WebRTCTransportProviderServer::HandleEndSession(CommandHandler & commandHandler,
                                                                              const Commands::EndSession::DecodableType & req)
{
    // Extract command fields from the request.
    uint16_t sessionId = req.webRTCSessionID;
    auto reason        = req.reason;

    // Validate the reason field against the allowed enum values.
    if (reason == WebRTCEndReasonEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleEndSession: Invalid reason value %u.", to_underlying(reason));
        return Status::ConstraintError;
    }

    WebRTCSessionStruct * existingSession = CheckForMatchingSession(commandHandler, sessionId);
    if (existingSession == nullptr)
    {
        return Status::NotFound;
    }

    // Delegate handles decrementing reference counts on video/audio streams if applicable.
    CHIP_ERROR err = mDelegate.HandleEndSession(sessionId, reason, existingSession->videoStreamID, existingSession->audioStreamID);

    // Remove the session entry from CurrentSessions.
    RemoveSession(sessionId);

    return err;
}

} // namespace WebRTCTransportProvider
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterWebRTCTransportProviderPluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initializing WebRTC Transport Provider cluster.");
}

void MatterWebRTCTransportProviderPluginServerShutdownCallback()
{
    ChipLogProgress(Zcl, "Shutdown WebRTC Transport Provider cluster.");
}
