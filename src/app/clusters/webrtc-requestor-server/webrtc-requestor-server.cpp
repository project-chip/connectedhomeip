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
 * @brief Implementation for the WebRTC Requestor Server Cluster
 ***************************************************************************/

#include "webrtc-requestor-server.h"

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
namespace WebRTCTransportRequestor {

WebRTCRequestorServer::WebRTCRequestorServer(EndpointId endpointId, WebRTCRequestorDelegate * delegate) :
    AttributeAccessInterface(MakeOptional(endpointId), WebRTCTransportRequestor::Id),
    CommandHandlerInterface(MakeOptional(endpointId), WebRTCTransportRequestor::Id), mDelegate(delegate)
{
    // Set this WebRTCRequestorServer to delegate here
}

WebRTCRequestorServer::~WebRTCRequestorServer()
{
    // Reset WebRTCRequestorServer in delegate here

    // unregister the handlers
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR WebRTCRequestorServer::Init()
{
    // Register this WebRTCRequestorServer for attributes read/write
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);

    // Register this WebRTCRequestorServer to handle commands
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR WebRTCRequestorServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    // if attributee id matches, encode current sessions and return
    if (aPath.mAttributeId == CurrentSessions::Id)
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
void WebRTCRequestorServer::InvokeCommand(HandlerContext & ctx)
{
    ChipLogDetail(Zcl, "WebRTCRequestorServer: InvokeCommand called with CommandId=0x%08" PRIx32, ctx.mRequestPath.mCommandId);

    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "No delegate set for WebRTCRequestorServer");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        return;
    }

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
        break;
    }
}

void WebRTCRequestorServer::HandleOffer(HandlerContext & ctx, const Commands::Offer::DecodableType & req)
{
    // Extract command fields from the request
    auto sdpOffer           = req.sdp;
    auto iceServers         = req.ICEServers;
    auto iceTransportPolicy = req.ICETransportPolicy;
    uint16_t sessionId      = req.webRTCSessionID;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Check if the session ID is in the requestor list
    WebRTCSessionStruct * existingSession = FindSession(sessionId);

    if (!existingSession)
    {
        // If sessionId is not null and does not match a value in current sessions.
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

    // Delegate processing
    WebRTCSessionStruct outSession;
    CHIP_ERROR err =
        mDelegate->HandleOffer(sessionId, sdpOffer, iceServers, iceTransportPolicy, peerNodeId, peerFabricIndex, outSession);

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

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);

    // Store the WebRTCSessionStruct in the CurrentSessions
    AddOrUpdateSession(outSession);
}

void WebRTCRequestorServer::HandleAnswer(HandlerContext & ctx, const Commands::Answer::DecodableType & req)
{
    // Extract command fields from the request
    auto sdpAnswer     = req.sdp;
    uint16_t sessionId = req.webRTCSessionID;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Check if the session ID is in the requestor list
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

    // Delegate processing
    CHIP_ERROR err = mDelegate->HandleAnswer(sessionId, sdpAnswer);

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

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void WebRTCRequestorServer::HandleICECandidates(HandlerContext & ctx, const Commands::ICECandidates::DecodableType & req)
{
    // Extract command fields from the request
    uint16_t sessionId = req.webRTCSessionID;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Check if the session ID is in the requestor list
    WebRTCSessionStruct * existingSession = FindSession(sessionId);

    if (!existingSession)
    {
        // If the sessionId is not null and does not match a value in current sessions.
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

    std::vector<std::string> candidates;
    auto iter = req.ICECandidates.begin();
    while (iter.Next())
    {
        // Get current CharSpan
        const chip::CharSpan & candidateSpan = iter.GetValue();

        // Convert to std::string
        candidates.emplace_back(candidateSpan.data(), candidateSpan.size());
    }
    // Delegate processing
    CHIP_ERROR err = mDelegate->HandleICECandidates(sessionId, candidates);

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

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void WebRTCRequestorServer::HandleEnd(HandlerContext & ctx, const Commands::End::DecodableType & req)
{
    // Extract command fields from the request
    uint16_t sessionId = req.webRTCSessionID;
    auto reason        = req.reason;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Check if the session ID is in the requestor list
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

    // Delegate processing
    CHIP_ERROR err = mDelegate->HandleEnd(sessionId, reason);

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

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);

    // Store the WebRTCSessionStruct in the CurrentSessions
    RemoveSession(sessionId);
}

// Helper functions
WebRTCSessionStruct * WebRTCRequestorServer::FindSession(uint16_t sessionId)
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

void WebRTCRequestorServer::AddOrUpdateSession(const WebRTCSessionStruct & session)
{
    // Search for a session with the same ID
    for (auto & existing : mCurrentSessions)
    {
        if (existing.id == session.id)
        {
            // Update it in-place
            existing = session;
            return;
        }
    }

    mCurrentSessions.emplace_back(session);
}

void WebRTCRequestorServer::RemoveSession(uint16_t sessionId)
{
    mCurrentSessions.erase(std::remove_if(mCurrentSessions.begin(), mCurrentSessions.end(),
                                          [sessionId](const WebRTCSessionStruct & s) { return s.id == sessionId; }),
                           mCurrentSessions.end());
}

} // namespace WebRTCTransportRequestor
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterWebRTCTransportRequestorPluginServerInitCallback() {}

bool emberAfWebRTCTransportRequestorClusterOfferCallback(
    chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
    chip::app::Clusters::WebRTCTransportRequestor::Commands::Offer::DecodableType const &)
{
    // TODO
    return false;
}

bool emberAfWebRTCTransportRequestorClusterAnswerCallback(
    chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
    chip::app::Clusters::WebRTCTransportRequestor::Commands::Answer::DecodableType const &)
{
    // TODO
    return false;
}

bool emberAfWebRTCTransportRequestorClusterICECandidatesCallback(
    chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
    chip::app::Clusters::WebRTCTransportRequestor::Commands::ICECandidates::DecodableType const &)
{
    // TODO
    return false;
}

bool emberAfWebRTCTransportRequestorClusterEndCallback(
    chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
    chip::app::Clusters::WebRTCTransportRequestor::Commands::End::DecodableType const &)
{
    // TODO
    return false;
}
