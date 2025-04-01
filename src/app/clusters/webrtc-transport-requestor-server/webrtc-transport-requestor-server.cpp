/*
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
 */

/****************************************************************************'
 * @file
 * @brief Implementation for the WebRTC Transport Requestor Server Cluster
 ***************************************************************************/

#include "webrtc-transport-requestor-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/reporting/reporting.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::WebRTCTransportRequestor;
using namespace chip::app::Clusters::WebRTCTransportRequestor::Attributes;
using chip::Protocols::InteractionModel::ClusterStatusCode;
using chip::Protocols::InteractionModel::Status;

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
namespace WebRTCTransportRequestor {

WebRTCTransportRequestorServer::WebRTCTransportRequestorServer(EndpointId endpointId, WebRTCTransportRequestorDelegate & delegate) :
    AttributeAccessInterface(MakeOptional(endpointId), WebRTCTransportRequestor::Id),
    CommandHandlerInterface(MakeOptional(endpointId), WebRTCTransportRequestor::Id), mDelegate(delegate)
{
    // Set WebRTCTransportRequestorServer in delegate here
}

WebRTCTransportRequestorServer::~WebRTCTransportRequestorServer()
{
    // Reset WebRTCTransportRequestorServer in delegate here

    Shutdown();
}

CHIP_ERROR WebRTCTransportRequestorServer::Init()
{
    // Register this WebRTCTransportRequestorServer for attributes read/write
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);

    // Register this WebRTCTransportRequestorServer to handle commands
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    return CHIP_NO_ERROR;
}

void WebRTCTransportRequestorServer::Shutdown()
{
    // Unregister the handlers
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

// AttributeAccessInterface
CHIP_ERROR WebRTCTransportRequestorServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    // if attributee id matches, encode current sessions and return
    if (aPath.mClusterId == Id && aPath.mAttributeId == Attributes::CurrentSessions::Id)
    {
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
            for (auto & session : mCurrentSessions)
            {
                ReturnErrorOnFailure(encoder.Encode(session));
            }
            return CHIP_NO_ERROR;
        });
    }

    // if attribute id doesn't match, return no error
    return CHIP_NO_ERROR;
}

// CommandHandlerInterface
void WebRTCTransportRequestorServer::InvokeCommand(HandlerContext & ctx)
{
    ChipLogDetail(Zcl, "WebRTCTransportRequestorServer: InvokeCommand called with CommandId=0x%08" PRIx32,
                  ctx.mRequestPath.mCommandId);

    switch (ctx.mRequestPath.mCommandId)
    {
    case Commands::Offer::Id:
        CommandHandlerInterface::HandleCommand<Commands::Offer::DecodableType>(
            ctx, [this](HandlerContext & ctx_, const auto & req) { HandleOffer(ctx_, req); });
        break;
    case Commands::Answer::Id:
        CommandHandlerInterface::HandleCommand<Commands::Answer::DecodableType>(
            ctx, [this](HandlerContext & ctx_, const auto & req) { HandleAnswer(ctx_, req); });
        break;
    case Commands::ICECandidates::Id:
        CommandHandlerInterface::HandleCommand<Commands::ICECandidates::DecodableType>(
            ctx, [this](HandlerContext & ctx_, const auto & req) { HandleICECandidates(ctx_, req); });
        break;
    case Commands::End::Id:
        CommandHandlerInterface::HandleCommand<Commands::End::DecodableType>(
            ctx, [this](HandlerContext & ctx_, const auto & req) { HandleEnd(ctx_, req); });
        break;

    default:
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedCommand);
        break;
    }
}

uint16_t WebRTCTransportRequestorServer::GenerateSessionId()
{
    static uint16_t lastSessionId = 0;
    uint16_t candidateId          = 0;

    while (true)
    {
        candidateId = lastSessionId++;

        // Handle wrap-around per spec
        if (lastSessionId > kMaxSessionId)
        {
            lastSessionId = 0;
        }

        if (FindSession(candidateId) == nullptr)
        {
            break;
        }
    }

    return candidateId;
}

bool WebRTCTransportRequestorServer::IsPeerNodeSessionValid(uint16_t sessionId, HandlerContext & ctx)
{
    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Check if the session ID is in the existing sessions list
    WebRTCSessionTypeStruct * existingSession = FindSession(sessionId);

    if (!existingSession)
    {
        // If sessionId is not null and does not match a value in current sessions.
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return false;
    }

    // Also check that the existing session belongs to the same PeerNodeID / Fabric
    // If it doesn’t match, respond with CONSTRAINT_ERROR
    if (peerNodeId != existingSession->peerNodeID || peerFabricIndex != existingSession->GetFabricIndex())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return false;
    }

    return true;
}

void WebRTCTransportRequestorServer::HandleOffer(HandlerContext & ctx, const Commands::Offer::DecodableType & req)
{
    // Extract command fields from the request
    uint16_t sessionId = req.webRTCSessionID;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Check if the session ID is in the existing sessions list
    WebRTCSessionTypeStruct * existingSession = FindSession(sessionId);
    // Create arguments for WebRTCTransportRequestorDelegate.
    WebRTCTransportRequestorDelegate::OfferArgs args;
    WebRTCSessionTypeStruct outSession;

    if (!existingSession)
    {
        // As per specification, sessionId represents the established session
        args.sessionId = sessionId;
        // TODO: For Session validation, SolicitOfferResponse SHALL create a WebRTCSessionTypeStruct with the received
        // webRTCSessionID and store it to CurrentSessions.
        outSession.id         = sessionId;
        outSession.peerNodeID = peerNodeId;
        outSession.SetFabricIndex(peerFabricIndex);
    }
    else
    {
        // Also check that the existing session belongs to the same PeerNodeID / Fabric
        // If it doesn’t match, respond with CONSTRAINT_ERROR
        if (peerNodeId != existingSession->peerNodeID || peerFabricIndex != existingSession->GetFabricIndex())
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
    }

    args.sdp = std::string(req.sdp.data(), req.sdp.size());

    // Convert ICE servers list from DecodableList to vector.
    if (req.ICEServers.HasValue())
    {
        auto iterator = req.ICEServers.Value().begin();
        while (iterator.Next())
        {
            args.iceServers.Value().push_back(iterator.GetValue());
        }
    }

    // Convert ICETransportPolicy from CharSpan to std::string.
    if (req.ICETransportPolicy.HasValue())
    {
        args.iceTransportPolicy.SetValue(std::string(req.ICETransportPolicy.Value().data(), req.ICETransportPolicy.Value().size()));
    }

    // Delegate processing: handle the SDP offer, gather ICE candidates, SDP answer, etc.
    Protocols::InteractionModel::ClusterStatusCode delegateStatus =
        Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleOffer(sessionId, args, outSession));
    if (!delegateStatus.IsSuccess())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, delegateStatus);
        return;
    }

    // Store the new WebRTCSessionTypeStruct in CurrentSessions.
    UpsertSession(outSession);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, delegateStatus);
}

void WebRTCTransportRequestorServer::HandleAnswer(HandlerContext & ctx, const Commands::Answer::DecodableType & req)
{
    // Extract command fields from the request
    uint16_t sessionId = req.webRTCSessionID;
    auto sdpSpan       = req.sdp;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Check if the session ID is in the existing sessions list
    WebRTCSessionTypeStruct * existingSession = FindSession(sessionId);

    WebRTCSessionTypeStruct outSession;

    if (!existingSession)
    {
        // TODO: For Session validation, ProvideOfferResponse SHALL create a WebRTCSessionTypeStruct with the received
        // webRTCSessionID and store it to the CurrentSessions.
        outSession.id         = sessionId;
        outSession.peerNodeID = peerNodeId;
        outSession.SetFabricIndex(peerFabricIndex);
    }
    else
    {
        // Also check that the existing session belongs to the same PeerNodeID / Fabric
        // If it doesn’t match, respond with CONSTRAINT_ERROR
        if (peerNodeId != existingSession->peerNodeID || peerFabricIndex != existingSession->GetFabricIndex())
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
    }

    std::string sdpAnswer(sdpSpan.data(), sdpSpan.size());

    // Delegate handles Answer command received.
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath,
                                  Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleAnswer(sessionId, sdpAnswer)));

    // Store the new WebRTCSessionTypeStruct in CurrentSessions.
    UpsertSession(outSession);
}

void WebRTCTransportRequestorServer::HandleICECandidates(HandlerContext & ctx, const Commands::ICECandidates::DecodableType & req)
{
    // Extract command fields from the request
    uint16_t sessionId = req.webRTCSessionID;

    std::vector<std::string> candidates;
    auto iter = req.ICECandidates.begin();
    while (iter.Next())
    {
        // Get current candidate CharSpan and convert to std::string.
        const CharSpan & candidateSpan = iter.GetValue();
        candidates.emplace_back(candidateSpan.data(), candidateSpan.size());
    }

    // Check the validity of the list.
    CHIP_ERROR err = iter.GetStatus();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleICECandidates: ICECandidates list error: %s", chip::ErrorStr(err));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    // Check if the session, NodeID are valid
    if (!IsPeerNodeSessionValid(sessionId, ctx))
        return;

    // Handle ICE candidates in Delegate
    ctx.mCommandHandler.AddStatus(
        ctx.mRequestPath, Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleICECandidates(sessionId, candidates)));
}

void WebRTCTransportRequestorServer::HandleEnd(HandlerContext & ctx, const Commands::End::DecodableType & req)
{
    // Extract command fields from the request
    uint16_t sessionId = req.webRTCSessionID;
    auto reason        = req.reason;

    // Validate the reason field against the allowed enum values.
    if (reason == WebRTCEndReasonEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleEnd: Invalid reason value %u.", static_cast<uint8_t>(reason));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    // Check if the session, NodeID are valid
    if (!IsPeerNodeSessionValid(sessionId, ctx))
        return;

    // Handle End command in Delegate
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath,
                                  Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleEnd(sessionId, reason)));

    // Store the WebRTCSessionTypeStruct in the CurrentSessions
    RemoveSession(sessionId);
}

// Helper functions
WebRTCSessionTypeStruct * WebRTCTransportRequestorServer::FindSession(uint16_t sessionId)
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

WebRTCTransportRequestorServer::UpsertResultEnum
WebRTCTransportRequestorServer::UpsertSession(const WebRTCSessionTypeStruct & session)
{
    // Search for a session in the current sessions
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

    MatterReportingAttributeChangeCallback(AttributeAccessInterface::GetEndpointId().Value(), WebRTCTransportRequestor::Id,
                                           WebRTCTransportRequestor::Attributes::CurrentSessions::Id);

    return result;
}

void WebRTCTransportRequestorServer::RemoveSession(uint16_t sessionId)
{
    size_t originalSize = mCurrentSessions.size();
    // Remove the session if sessionId is matching
    mCurrentSessions.erase(std::remove_if(mCurrentSessions.begin(), mCurrentSessions.end(),
                                          [sessionId](const WebRTCSessionTypeStruct & s) { return s.id == sessionId; }),
                           mCurrentSessions.end());

    // Check whether session was removed
    if (mCurrentSessions.size() < originalSize)
    {
        // Notify the stack that the CurrentSessions attribute has changed.
        MatterReportingAttributeChangeCallback(AttributeAccessInterface::GetEndpointId().Value(), WebRTCTransportRequestor::Id,
                                               WebRTCTransportRequestor::Attributes::CurrentSessions::Id);
    }
}

} // namespace WebRTCTransportRequestor
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterWebRTCTransportRequestorPluginServerInitCallback() {}
