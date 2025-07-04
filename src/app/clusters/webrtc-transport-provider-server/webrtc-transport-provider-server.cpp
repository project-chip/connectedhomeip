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
#include "webrtc-transport-provider-server.h"

#include <protocols/interaction_model/StatusCode.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <iterator>
#include <memory>

using namespace chip;
using namespace chip::app;
using chip::Protocols::InteractionModel::Status;

using ICEServerDecodableStruct = chip::app::Clusters::Globals::Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct      = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
using ICECandidateStruct       = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
using StreamUsageEnum          = chip::app::Clusters::Globals::StreamUsageEnum;
using WebRTCEndReasonEnum      = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

namespace {

static constexpr uint16_t kMaxSessionId = 65534;

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

WebRTCTransportProviderServer::WebRTCTransportProviderServer(Delegate & delegate, EndpointId endpointId) :
    AttributeAccessInterface(MakeOptional(endpointId), WebRTCTransportProvider::Id),
    CommandHandlerInterface(MakeOptional(endpointId), WebRTCTransportProvider::Id), mDelegate(delegate)
{}

WebRTCTransportProviderServer::~WebRTCTransportProviderServer()
{
    Shutdown();
}

CHIP_ERROR WebRTCTransportProviderServer::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void WebRTCTransportProviderServer::Shutdown()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

// AttributeAccessInterface
CHIP_ERROR WebRTCTransportProviderServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    // The only attribute from the spec is "CurrentSessions" (attribute ID 0x0000),
    // which is a list[WebRTCSessionStruct].
    if (aPath.mClusterId == Id && aPath.mAttributeId == Attributes::CurrentSessions::Id)
    {
        // We encode mCurrentSessions as a list of WebRTCSessionStruct
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
            for (auto & session : mCurrentSessions)
            {
                ReturnErrorOnFailure(encoder.Encode(session));
            }
            return CHIP_NO_ERROR;
        });
    }

    // If not our attribute, let default logic handle
    return CHIP_NO_ERROR;
}

// CommandHandlerInterface
void WebRTCTransportProviderServer::InvokeCommand(HandlerContext & ctx)
{
    ChipLogDetail(Zcl, "WebRTCTransportProvider: InvokeCommand called with CommandId=0x%08" PRIx32, ctx.mRequestPath.mCommandId);

    switch (ctx.mRequestPath.mCommandId)
    {
    case Commands::SolicitOffer::Id:
        CommandHandlerInterface::HandleCommand<Commands::SolicitOffer::DecodableType>(
            ctx, [this](HandlerContext & subCtx, const auto & req) { HandleSolicitOffer(subCtx, req); });
        break;

    case Commands::ProvideOffer::Id:
        CommandHandlerInterface::HandleCommand<Commands::ProvideOffer::DecodableType>(
            ctx, [this](HandlerContext & subCtx, const auto & req) { HandleProvideOffer(subCtx, req); });
        break;

    case Commands::ProvideAnswer::Id:
        CommandHandlerInterface::HandleCommand<Commands::ProvideAnswer::DecodableType>(
            ctx, [this](HandlerContext & subCtx, const auto & req) { HandleProvideAnswer(subCtx, req); });
        break;

    case Commands::ProvideICECandidates::Id:
        CommandHandlerInterface::HandleCommand<Commands::ProvideICECandidates::DecodableType>(
            ctx, [this](HandlerContext & subCtx, const auto & req) { HandleProvideICECandidates(subCtx, req); });
        break;

    case Commands::EndSession::Id:
        CommandHandlerInterface::HandleCommand<Commands::EndSession::DecodableType>(
            ctx, [this](HandlerContext & subCtx, const auto & req) { HandleEndSession(subCtx, req); });
        break;

    default:
        // Mark unrecognized command as UnsupportedCommand
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedCommand);
        break;
    }
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

    MatterReportingAttributeChangeCallback(AttributeAccessInterface::GetEndpointId().Value(), WebRTCTransportProvider::Id,
                                           WebRTCTransportProvider::Attributes::CurrentSessions::Id);

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
        // Notify the stack that the CurrentSessions attribute has changed.
        MatterReportingAttributeChangeCallback(AttributeAccessInterface::GetEndpointId().Value(), WebRTCTransportProvider::Id,
                                               WebRTCTransportProvider::Attributes::CurrentSessions::Id);
    }
}

WebRTCSessionStruct * WebRTCTransportProviderServer::CheckForMatchingSession(HandlerContext & ctx, uint16_t sessionId)
{
    WebRTCSessionStruct * session = FindSession(sessionId);
    if (session == nullptr)
    {
        return nullptr;
    }

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Ensure the session’s peer matches the current command invoker
    if (peerNodeId != session->peerNodeID || peerFabricIndex != session->GetFabricIndex())
    {
        return nullptr;
    }

    return session;
}

uint16_t WebRTCTransportProviderServer::GenerateSessionId()
{
    static uint16_t lastSessionId = 1;

    do
    {
        uint16_t candidateId = lastSessionId++;

        // Handle wrap-around per spec
        if (lastSessionId > kMaxSessionId)
        {
            lastSessionId = 1;
        }

        if (FindSession(candidateId) == nullptr)
        {
            return candidateId;
        }
    } while (true);
}

// Command Handlers
void WebRTCTransportProviderServer::HandleSolicitOffer(HandlerContext & ctx, const Commands::SolicitOffer::DecodableType & req)
{
    // Validate the streamUsage field against the allowed enum values.
    if (req.streamUsage == StreamUsageEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleSolicitOffer: Invalid streamUsage value %u.", to_underlying(req.streamUsage));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    bool privacyModeActive = false;
    if (mDelegate.IsPrivacyModeActive(privacyModeActive) != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleSolicitOffer: Cannot determine privacy mode state");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
        return;
    }

    if (privacyModeActive)
    {
        ChipLogError(Zcl, "HandleSolicitOffer: Privacy mode is enabled");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
        return;
    }

    // Validate VideoStreamID against AllocatedVideoStreams if present and not null
    if (req.videoStreamID.HasValue() && !req.videoStreamID.Value().IsNull())
    {
        // Delegate should validate against AllocatedVideoStreams
        if (mDelegate.ValidateVideoStreamID(req.videoStreamID.Value().Value()) != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleSolicitOffer: VideoStreamID %u does not match AllocatedVideoStreams",
                         req.videoStreamID.Value().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
            return;
        }
    }

    // Validate AudioStreamID against AllocatedAudioStreams if present and not null
    if (req.audioStreamID.HasValue() && !req.audioStreamID.Value().IsNull())
    {
        // Delegate should validate against AllocatedAudioStreams
        if (mDelegate.ValidateAudioStreamID(req.audioStreamID.Value().Value()) != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleSolicitOffer: AudioStreamID %u does not match AllocatedAudioStreams",
                         req.audioStreamID.Value().Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
            return;
        }
    }

    // Prepare the arguments for the delegate.
    Delegate::OfferRequestArgs args;
    args.sessionId             = GenerateSessionId();
    args.streamUsage           = req.streamUsage;
    args.videoStreamId         = req.videoStreamID;
    args.audioStreamId         = req.audioStreamID;
    args.peerNodeId            = GetNodeIdFromCtx(ctx.mCommandHandler);
    args.fabricIndex           = ctx.mCommandHandler.GetAccessingFabricIndex();
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
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
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

    auto status = Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleSolicitOffer(args, outSession, deferredOffer));
    if (!status.IsSuccess())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
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

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, resp);
}

void WebRTCTransportProviderServer::HandleProvideOffer(HandlerContext & ctx, const Commands::ProvideOffer::DecodableType & req)
{
    auto webRTCSessionID = req.webRTCSessionID;
    auto videoStreamID   = req.videoStreamID;
    auto audioStreamID   = req.audioStreamID;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    WebRTCSessionStruct outSession;

    // Prepare delegate arguments for the session
    Delegate::ProvideOfferRequestArgs args;

    // Validate the streamUsage field against the allowed enum values.
    if (req.streamUsage == StreamUsageEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleProvideOffer: Invalid streamUsage value %u.", to_underlying(req.streamUsage));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    // If WebRTCSessionID is not null and does not match a value in CurrentSessions: Respond with NOT_FOUND.
    if (!webRTCSessionID.IsNull())
    {
        uint16_t sessionId                    = webRTCSessionID.Value();
        WebRTCSessionStruct * existingSession = CheckForMatchingSession(ctx, sessionId);
        if (existingSession == nullptr)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
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
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
            return;
        }

        if (privacyModeActive)
        {
            ChipLogError(Zcl, "HandleProvideOffer: Privacy mode is enabled");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
            return;
        }

        // Validate VideoStreamID if present and not null
        if (videoStreamID.HasValue() && !videoStreamID.Value().IsNull())
        {
            if (mDelegate.ValidateVideoStreamID(videoStreamID.Value().Value()) != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "HandleProvideOffer: VideoStreamID %u does not match AllocatedVideoStreams",
                             videoStreamID.Value().Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
                return;
            }
        }
        else if (videoStreamID.HasValue() && videoStreamID.Value().IsNull())
        {
            // VideoStreamID is present and is null - need to automatically select
            // First check if there are any video streams allocated
            if (!mDelegate.HasAllocatedVideoStreams())
            {
                ChipLogError(Zcl, "HandleProvideOffer: No video streams currently allocated");
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
                return;
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
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
                return;
            }
        }
        else if (audioStreamID.HasValue() && audioStreamID.Value().IsNull())
        {
            // AudioStreamID is present and is null - need to automatically select
            // First check if there are any audio streams allocated
            if (!mDelegate.HasAllocatedAudioStreams())
            {
                ChipLogError(Zcl, "HandleProvideOffer: No audio streams currently allocated");
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
                return;
            }
            // Automatic selection will be handled by the delegate in HandleProvideOffer.
        }

        // Check resource management and stream priorities
        CHIP_ERROR err = mDelegate.ValidateStreamUsage(req.streamUsage, req.videoStreamID, req.audioStreamID);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleProvideOffer: Cannot meet resource management conditions");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
            return;
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
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
        }

        args.iceServers.SetValue(std::move(localIceServers));
    }

    // Convert ICETransportPolicy from CharSpan to std::string.
    if (req.ICETransportPolicy.HasValue())
    {
        args.iceTransportPolicy.SetValue(std::string(req.ICETransportPolicy.Value().data(), req.ICETransportPolicy.Value().size()));
    }

    // Delegate processing: process the SDP offer, create session, increment reference counts.
    auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleProvideOffer(args, outSession));
    if (!delegateStatus.IsSuccess())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, delegateStatus);
        return;
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

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, resp);
}

void WebRTCTransportProviderServer::HandleProvideAnswer(HandlerContext & ctx, const Commands::ProvideAnswer::DecodableType & req)
{
    // Extract command fields from the request.
    uint16_t sessionId = req.webRTCSessionID;
    auto sdpSpan       = req.sdp;

    WebRTCSessionStruct * existingSession = CheckForMatchingSession(ctx, sessionId);
    if (existingSession == nullptr)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    std::string sdpAnswer(sdpSpan.data(), sdpSpan.size());
    ctx.mCommandHandler.AddStatus(
        ctx.mRequestPath, Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleProvideAnswer(sessionId, sdpAnswer)));
}

void WebRTCTransportProviderServer::HandleProvideICECandidates(HandlerContext & ctx,
                                                               const Commands::ProvideICECandidates::DecodableType & req)
{
    // Extract command fields from the request.
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
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    WebRTCSessionStruct * existingSession = CheckForMatchingSession(ctx, sessionId);
    if (existingSession == nullptr)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Delegate the handling of ICE candidates.
    ctx.mCommandHandler.AddStatus(
        ctx.mRequestPath,
        Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleProvideICECandidates(sessionId, candidates)));
}

void WebRTCTransportProviderServer::HandleEndSession(HandlerContext & ctx, const Commands::EndSession::DecodableType & req)
{
    // Extract command fields from the request.
    uint16_t sessionId = req.webRTCSessionID;
    auto reason        = req.reason;

    // Validate the reason field against the allowed enum values.
    if (reason == WebRTCEndReasonEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleEndSession: Invalid reason value %u.", to_underlying(reason));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    WebRTCSessionStruct * existingSession = CheckForMatchingSession(ctx, sessionId);
    if (existingSession == nullptr)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Delegate handles decrementing reference counts on video/audio streams if applicable.
    CHIP_ERROR err = mDelegate.HandleEndSession(sessionId, reason, existingSession->videoStreamID, existingSession->audioStreamID);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::ClusterStatusCode(err));

    // Remove the session entry from CurrentSessions.
    RemoveSession(sessionId);
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
