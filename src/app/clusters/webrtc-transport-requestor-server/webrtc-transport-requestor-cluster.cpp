/*
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
#include <app/clusters/webrtc-transport-requestor-server/webrtc-transport-requestor-cluster.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportRequestor;
using namespace chip::app::Clusters::WebRTCTransportRequestor::Attributes;
using chip::Protocols::InteractionModel::ClusterStatusCode;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr uint16_t kMaxSessionId = 65534;

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    WebRTCTransportRequestor::Commands::Offer::kMetadataEntry,
    WebRTCTransportRequestor::Commands::Answer::kMetadataEntry,
    WebRTCTransportRequestor::Commands::ICECandidates::kMetadataEntry,
    WebRTCTransportRequestor::Commands::End::kMetadataEntry,
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

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportRequestor {

WebRTCTransportRequestorServer::WebRTCTransportRequestorServer(EndpointId endpointId, Delegate & delegate) :
    DefaultServerCluster({ endpointId, WebRTCTransportRequestor::Id }), mDelegate(delegate)
{}

DataModel::ActionReturnStatus WebRTCTransportRequestorServer::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                            AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::CurrentSessions::Id:
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            for (auto & session : mCurrentSessions)
            {
                ReturnErrorOnFailure(listEncoder.Encode(session));
            }
            return CHIP_NO_ERROR;
        });
    case ClusterRevision::Id:
        return encoder.Encode(WebRTCTransportRequestor::kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> WebRTCTransportRequestorServer::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                           TLV::TLVReader & input_arguments,
                                                                                           CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::Offer::Id: {
        Commands::Offer::DecodableType req;
        if (DataModel::Decode(input_arguments, req) != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        return HandleOffer(*handler, req);
    }
    case Commands::Answer::Id: {
        Commands::Answer::DecodableType req;
        if (DataModel::Decode(input_arguments, req) != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        return HandleAnswer(*handler, req);
    }
    case Commands::ICECandidates::Id: {
        Commands::ICECandidates::DecodableType req;
        if (DataModel::Decode(input_arguments, req) != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        return HandleICECandidates(*handler, req);
    }
    case Commands::End::Id: {
        Commands::End::DecodableType req;
        if (DataModel::Decode(input_arguments, req) != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        return HandleEnd(*handler, req);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR WebRTCTransportRequestorServer::AcceptedCommands(const ConcreteClusterPath & path,
                                                            ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

uint16_t WebRTCTransportRequestorServer::GenerateSessionId()
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
    VerifyOrDie(false);
}

bool WebRTCTransportRequestorServer::IsPeerNodeSessionValid(uint16_t sessionId, const CommandHandler & commandHandler)
{
    NodeId peerNodeId           = GetNodeIdFromCtx(commandHandler);
    FabricIndex peerFabricIndex = commandHandler.GetAccessingFabricIndex();

    // Check if the session ID is in the existing sessions list
    WebRTCSessionStruct * existingSession = FindSession(sessionId);

    if (!existingSession)
    {
        return false;
    }

    // Also check that the existing session belongs to the same PeerNodeID / Fabric
    // If it doesn't match, return false
    return (peerNodeId == existingSession->peerNodeID) && (peerFabricIndex == existingSession->GetFabricIndex());
}

DataModel::ActionReturnStatus WebRTCTransportRequestorServer::HandleOffer(const CommandHandler & commandHandler,
                                                                          const Commands::Offer::DecodableType & req)
{
    uint16_t sessionId = req.webRTCSessionID;

    if (!IsPeerNodeSessionValid(sessionId, commandHandler))
    {
        return Protocols::InteractionModel::Status::NotFound;
    }

    // Create arguments for Delegate.
    Delegate::OfferArgs args;
    args.sdp = std::string(req.sdp.data(), req.sdp.size());

    // Convert ICE servers list from DecodableList to vector.
    if (req.ICEServers.HasValue())
    {
        auto iterator = req.ICEServers.Value().begin();
        while (iterator.Next())
        {
            args.iceServers.Value().push_back(iterator.GetValue());
        }

        // Check the list validity
        CHIP_ERROR err = iterator.GetStatus();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleOffer: ICEServers list error: %" CHIP_ERROR_FORMAT, err.Format());
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
    }

    // Convert ICETransportPolicy from CharSpan to std::string.
    if (req.ICETransportPolicy.HasValue())
    {
        args.iceTransportPolicy.SetValue(std::string(req.ICETransportPolicy.Value().data(), req.ICETransportPolicy.Value().size()));
    }

    // Delegate processing: handle the SDP offer, gather ICE candidates, SDP answer, etc.
    return mDelegate.HandleOffer(sessionId, args);
}

DataModel::ActionReturnStatus WebRTCTransportRequestorServer::HandleAnswer(const CommandHandler & commandHandler,
                                                                           const Commands::Answer::DecodableType & req)
{
    uint16_t sessionId = req.webRTCSessionID;
    auto sdpSpan       = req.sdp;

    if (!IsPeerNodeSessionValid(sessionId, commandHandler))
    {
        return Protocols::InteractionModel::Status::NotFound;
    }

    std::string sdpAnswer(sdpSpan.data(), sdpSpan.size());

    // Delegate handles Answer command received.
    return mDelegate.HandleAnswer(sessionId, sdpAnswer);
}

DataModel::ActionReturnStatus
WebRTCTransportRequestorServer::HandleICECandidates(const CommandHandler & commandHandler,
                                                    const Commands::ICECandidates::DecodableType & req)
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
    CHIP_ERROR err = iter.GetStatus();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleICECandidates: ICECandidates list error: %" CHIP_ERROR_FORMAT, err.Format());
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    // Check ice candidates min 1 contraint.
    if (candidates.empty())
    {
        ChipLogError(Zcl, "HandleICECandidates: No ICE candidates provided.");
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    // Check if the session, NodeID are valid
    if (!IsPeerNodeSessionValid(sessionId, commandHandler))
    {
        return Protocols::InteractionModel::Status::NotFound;
    }

    return mDelegate.HandleICECandidates(sessionId, candidates);
}

DataModel::ActionReturnStatus WebRTCTransportRequestorServer::HandleEnd(const CommandHandler & commandHandler,
                                                                        const Commands::End::DecodableType & req)
{
    uint16_t sessionId = req.webRTCSessionID;
    auto reason        = req.reason;

    // Validate the reason field against the allowed enum values.
    if (reason == WebRTCEndReasonEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleEnd: Invalid reason value %u.", to_underlying(reason));
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    if (!IsPeerNodeSessionValid(sessionId, commandHandler))
    {
        return Protocols::InteractionModel::Status::NotFound;
    }

    CHIP_ERROR delegateResult = mDelegate.HandleEnd(sessionId, reason);

    RemoveSession(sessionId);

    return delegateResult;
}

// Helper functions
WebRTCSessionStruct * WebRTCTransportRequestorServer::FindSession(uint16_t sessionId)
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

WebRTCTransportRequestorServer::UpsertResultEnum WebRTCTransportRequestorServer::UpsertSession(const WebRTCSessionStruct & session)
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

    NotifyAttributeChanged(WebRTCTransportRequestor::Attributes::CurrentSessions::Id);

    return result;
}

void WebRTCTransportRequestorServer::RemoveSession(uint16_t sessionId)
{
    size_t originalSize = mCurrentSessions.size();
    // Remove the session if sessionId is matching
    mCurrentSessions.erase(std::remove_if(mCurrentSessions.begin(), mCurrentSessions.end(),
                                          [sessionId](const WebRTCSessionStruct & s) { return s.id == sessionId; }),
                           mCurrentSessions.end());

    // Check whether session was removed
    if (mCurrentSessions.size() < originalSize)
    {
        // Notify the stack that the CurrentSessions attribute has changed.
        NotifyAttributeChanged(WebRTCTransportRequestor::Attributes::CurrentSessions::Id);
    }
}

} // namespace WebRTCTransportRequestor
} // namespace Clusters
} // namespace app
} // namespace chip
