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

#include <memory>

using namespace chip;
using namespace chip::app;

using chip::Protocols::InteractionModel::Status;

namespace {

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

WebRTCTransportProviderServer::WebRTCTransportProviderServer(Delegate * delegate, EndpointId endpointId) :
    AttributeAccessInterface(MakeOptional(endpointId), WebRTCTransportProvider::Id),
    CommandHandlerInterface(MakeOptional(endpointId), WebRTCTransportProvider::Id), mDelegate(delegate)
{}

WebRTCTransportProviderServer::~WebRTCTransportProviderServer()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR WebRTCTransportProviderServer::Init()
{
    // Register ourselves for attribute read/write
    AttributeAccessInterfaceRegistry::Instance().Register(this);

    // Register ourselves as the command handler
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR WebRTCTransportProviderServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    // The only attribute from the spec is "CurrentSessions" (attribute ID 0x0000),
    // which is a list[WebRTCSessionStruct].
    if (aPath.mClusterId == Id && aPath.mAttributeId == Attributes::CurrentSessions::Id)
    {
        // We encode mCurrentSessions as a list of WebRTCSessionStruct::Type
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

    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "No delegate set for WebRTCTransportProviderServer");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        return;
    }

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

void WebRTCTransportProviderServer::AddOrUpdateSession(const WebRTCSessionStruct & session)
{
    bool found = false;

    // Search for a session with the same ID and update if found
    for (auto & existing : mCurrentSessions)
    {
        if (existing.id == session.id)
        {
            existing = session;
            found    = true;
            break;
        }
    }

    // If not found, add a new entry
    if (!found)
    {
        mCurrentSessions.push_back(session);
    }

    // Notify the stack that the CurrentSessions attribute has changed
    MatterReportingAttributeChangeCallback(AttributeAccessInterface::GetEndpointId().Value(), WebRTCTransportProvider::Id,
                                           WebRTCTransportProvider::Attributes::CurrentSessions::Id);
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

uint16_t WebRTCTransportProviderServer::GenerateSessionID()
{
    static uint16_t lastSessionId = 0;
    uint16_t candidateId          = lastSessionId++;

    // Handle wrap-around per spec
    if (lastSessionId > 65534)
        lastSessionId = 0;

    // Ensure uniqueness
    while (FindSession(candidateId) != nullptr)
    {
        candidateId = lastSessionId++;
        if (lastSessionId > 65534)
            lastSessionId = 0;
    }

    return candidateId;
}

// Command Handlers
void WebRTCTransportProviderServer::HandleSolicitOffer(HandlerContext & ctx, const Commands::SolicitOffer::DecodableType & req)
{
    // According to the Matter WebRTC Transport Provider specification,
    // at least one of VideoStreamID or AudioStreamID must be present.
    if (!req.videoStreamID.HasValue() && !req.audioStreamID.HasValue())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    // Prepare arguments for Delegate::HandleSolicitOffer using SolicitOfferRequestArgs struct
    Delegate::SolicitOfferRequestArgs args;
    args.id                 = GenerateSessionID();
    args.streamUsage        = req.streamUsage;
    args.videoStreamId      = req.videoStreamID;
    args.audioStreamId      = req.audioStreamID;
    args.iceServers         = req.ICEServers;
    args.iceTransportPolicy = req.ICETransportPolicy;
    args.metadataOptions    = req.metadataOptions;
    args.peerNodeId         = GetNodeIdFromCtx(ctx.mCommandHandler);
    args.peerFabricIndex    = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Delegate processing
    WebRTCSessionStruct outSession;
    bool deferredOffer = false;
    CHIP_ERROR err     = mDelegate->HandleSolicitOffer(args, outSession, deferredOffer);

    // Convert the CHIP_ERROR to an Interaction Model status code
    Protocols::InteractionModel::Status status = StatusIB(err).mStatus;
    if (status != Protocols::InteractionModel::Status::Success)
    {
        // If the delegate could not allocate or encountered a problem,
        // respond with the appropriate status. For example, if out of resources,
        // the delegate might return an error that maps to ResourceExhausted or Failure.
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Store the WebRTCSessionStruct in the CurrentSessions
    AddOrUpdateSession(outSession);

    // Send the response
    Commands::SolicitOfferResponse::Type resp;
    resp.webRTCSessionID = outSession.id;
    resp.deferredOffer   = deferredOffer;
    resp.videoStreamID.SetValue(outSession.videoStreamID);
    resp.audioStreamID.SetValue(outSession.audioStreamID);

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, resp);
}

void WebRTCTransportProviderServer::HandleProvideOffer(HandlerContext & ctx, const Commands::ProvideOffer::DecodableType & req)
{
    // Extract command fields from the request.
    auto webRTCSessionID = req.webRTCSessionID;
    auto videoStreamID   = req.videoStreamID;
    auto audioStreamID   = req.audioStreamID;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    uint16_t sessionId = 0;

    // Determine if this is a new session or a re-Offer
    if (webRTCSessionID.IsNull())
    {
        // This is a request for a new session
        // If both the VideoStreamID and AudioStreamID are not present
        if (!videoStreamID.HasValue() && !audioStreamID.HasValue())
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }

        sessionId = GenerateSessionID();
    }
    else
    {
        // This is a re-offer. Check if the session ID is in our provider list
        sessionId                             = webRTCSessionID.Value();
        WebRTCSessionStruct * existingSession = FindSession(sessionId);

        if (!existingSession)
        {
            // If WebRTCSessionID is not null and does not match a value in current sessions.
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }

        // Also check that the existing session belongs to the same PeerNodeID / Fabric
        // If it doesn’t match, respond with CONSTRAINT_ERROR
        if (peerNodeId != existingSession->peerNodeID || peerFabricIndex != existingSession->peerFabricIndex)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
    }

    // Delegate processing
    WebRTCSessionStruct outSession;

    Delegate::ProvideOfferRequestArgs args;
    args.id                 = sessionId;
    args.streamUsage        = req.streamUsage;
    args.sdp                = req.sdp;
    args.videoStreamId      = videoStreamID;
    args.audioStreamId      = audioStreamID;
    args.iceServers         = req.ICEServers;
    args.iceTransportPolicy = req.ICETransportPolicy;
    args.metadataOptions    = req.metadataOptions;
    args.peerNodeId         = GetNodeIdFromCtx(ctx.mCommandHandler);
    args.peerFabricIndex    = ctx.mCommandHandler.GetAccessingFabricIndex();

    CHIP_ERROR err = mDelegate->HandleProvideOffer(args, outSession);

    // Convert the CHIP_ERROR to an Interaction Model status code
    Protocols::InteractionModel::Status status = StatusIB(err).mStatus;
    if (status != Protocols::InteractionModel::Status::Success)
    {
        // If the delegate could not allocate or encountered a problem,
        // respond with the appropriate status. For example, if out of resources,
        // the delegate might return an error that maps to ResourceExhausted or Failure.
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Store the WebRTCSessionStruct in the CurrentSessions
    AddOrUpdateSession(outSession);

    // Send the response
    Commands::ProvideOfferResponse::Type resp;
    resp.webRTCSessionID = outSession.id;
    resp.audioStreamID.SetValue(outSession.videoStreamID);
    resp.audioStreamID.SetValue(outSession.audioStreamID);

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, resp);
}

void WebRTCTransportProviderServer::HandleProvideAnswer(HandlerContext & ctx, const Commands::ProvideAnswer::DecodableType & req)
{
    // Extract command fields from the request.
    uint16_t sessionId = req.webRTCSessionID;
    auto sdpAnswer     = req.sdp;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    WebRTCSessionStruct * existingSession = FindSession(sessionId);

    if (!existingSession)
    {
        // If WebRTCSessionID is not null and does not match a value in current sessions.
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    // Also check that the existing session belongs to the same PeerNodeID / Fabric
    // If it doesn’t match, respond with CONSTRAINT_ERROR
    if (peerNodeId != existingSession->peerNodeID || peerFabricIndex != existingSession->peerFabricIndex)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    Status status  = Protocols::InteractionModel::Status::Success;
    CHIP_ERROR err = mDelegate->HandleProvideAnswer(sessionId, sdpAnswer);
    if (err != CHIP_NO_ERROR)
    {
        status = StatusIB(err).mStatus;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void WebRTCTransportProviderServer::HandleProvideICECandidates(HandlerContext & ctx,
                                                               const Commands::ProvideICECandidates::DecodableType & req)
{
    // Extract command fields from the request.
    uint16_t sessionId = req.webRTCSessionID;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    std::vector<std::string> candidates;
    auto iter = req.ICECandidates.begin();
    while (iter.Next())
    {
        // Get current CharSpan
        const chip::CharSpan & candidateSpan = iter.GetValue();

        // Convert to std::string
        candidates.emplace_back(candidateSpan.data(), candidateSpan.size());
    }

    WebRTCSessionStruct * existingSession = FindSession(sessionId);

    if (!existingSession)
    {
        // If WebRTCSessionID is not null and does not match a value in current sessions.
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    // Also check that the existing session belongs to the same PeerNodeID / Fabric
    // If it doesn’t match, respond with CONSTRAINT_ERROR
    if (peerNodeId != existingSession->peerNodeID || peerFabricIndex != existingSession->peerFabricIndex)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    Status status  = Protocols::InteractionModel::Status::Success;
    CHIP_ERROR err = mDelegate->HandleProvideICECandidates(sessionId, candidates);
    if (err != CHIP_NO_ERROR)
    {
        status = StatusIB(err).mStatus;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void WebRTCTransportProviderServer::HandleEndSession(HandlerContext & ctx, const Commands::EndSession::DecodableType & req)
{
    // Extract command fields from the request.
    uint16_t sessionId = req.webRTCSessionID;
    auto reason        = req.reason;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    WebRTCSessionStruct * existingSession = FindSession(sessionId);

    if (!existingSession)
    {
        // If WebRTCSessionID is not null and does not match a value in current sessions.
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    // Also check that the existing session belongs to the same PeerNodeID / Fabric
    // If it doesn’t match, respond with CONSTRAINT_ERROR
    if (peerNodeId != existingSession->peerNodeID || peerFabricIndex != existingSession->peerFabricIndex)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    Status status  = Protocols::InteractionModel::Status::Success;
    CHIP_ERROR err = mDelegate->HandleEndSession(sessionId, reason);
    if (err != CHIP_NO_ERROR)
    {
        status = StatusIB(err).mStatus;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);

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
