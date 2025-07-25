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
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
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

WebRTCTransportRequestorServer::WebRTCTransportRequestorServer(EndpointId endpointId, WebRTCTransportRequestorDelegate & delegate) :
    DefaultServerCluster({ endpointId, WebRTCTransportRequestor::Id }), mDelegate(delegate)
{}

DataModel::ActionReturnStatus WebRTCTransportRequestorServer::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                            AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::CurrentSessions::Id: {
        CHIP_ERROR err = encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            for (auto & session : mCurrentSessions)
            {
                ReturnErrorOnFailure(listEncoder.Encode(session));
            }
            return CHIP_NO_ERROR;
        });

        if (err == CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::Success;
        }
        else
        {
            return Protocols::InteractionModel::Status::Failure;
        }
    }
    case ClusterRevision::Id: {
        if (encoder.Encode(WebRTCTransportRequestor::kRevision) == CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::Success;
        }
        else
        {
            return Protocols::InteractionModel::Status::Failure;
        }
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> WebRTCTransportRequestorServer::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                           TLV::TLVReader & input_arguments,
                                                                                           CommandHandler * handler)
{
    ChipLogDetail(Zcl, "WebRTCTransportRequestorServer: InvokeCommand called with CommandId=0x%08" PRIx32, request.path.mCommandId);

    switch (request.path.mCommandId)
    {
    case Commands::Offer::Id: {
        Commands::Offer::DecodableType req;
        if (DataModel::Decode(input_arguments, req) != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        return HandleOffer(req, *handler);
    }
    case Commands::Answer::Id: {
        Commands::Answer::DecodableType req;
        if (DataModel::Decode(input_arguments, req) != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        return HandleAnswer(req, *handler);
    }
    case Commands::ICECandidates::Id: {
        Commands::ICECandidates::DecodableType req;
        if (DataModel::Decode(input_arguments, req) != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        return HandleICECandidates(req, *handler);
    }
    case Commands::End::Id: {
        Commands::End::DecodableType req;
        if (DataModel::Decode(input_arguments, req) != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        return HandleEnd(req, *handler);
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
    if (peerNodeId != existingSession->peerNodeID || peerFabricIndex != existingSession->GetFabricIndex())
    {
        return false;
    }

    return true;
}

DataModel::ActionReturnStatus WebRTCTransportRequestorServer::HandleOffer(const Commands::Offer::DecodableType & req,
                                                                          const CommandHandler & commandHandler)
{
    // Extract command fields from the request
    uint16_t sessionId = req.webRTCSessionID;

    // Check if the session, NodeID are valid
    if (!IsPeerNodeSessionValid(sessionId, commandHandler))
    {
        return Protocols::InteractionModel::Status::NotFound;
    }

    // Create arguments for WebRTCTransportRequestorDelegate.
    WebRTCTransportRequestorDelegate::OfferArgs args;
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
            ChipLogError(Zcl, "HandleOffer: ICECandidates list error: %" CHIP_ERROR_FORMAT, err.Format());
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
    }

    // Convert ICETransportPolicy from CharSpan to std::string.
    if (req.ICETransportPolicy.HasValue())
    {
        args.iceTransportPolicy.SetValue(std::string(req.ICETransportPolicy.Value().data(), req.ICETransportPolicy.Value().size()));
    }

    // Delegate processing: handle the SDP offer, gather ICE candidates, SDP answer, etc.
    CHIP_ERROR delegateResult = mDelegate.HandleOffer(sessionId, args);
    if (delegateResult == CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::Success;
    }
    else if (delegateResult == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }
}

DataModel::ActionReturnStatus WebRTCTransportRequestorServer::HandleAnswer(const Commands::Answer::DecodableType & req,
                                                                           const CommandHandler & commandHandler)
{
    // Extract command fields from the request
    uint16_t sessionId = req.webRTCSessionID;
    auto sdpSpan       = req.sdp;

    // Check if the session, NodeID are valid
    if (!IsPeerNodeSessionValid(sessionId, commandHandler))
    {
        return Protocols::InteractionModel::Status::NotFound;
    }

    std::string sdpAnswer(sdpSpan.data(), sdpSpan.size());

    // Delegate handles Answer command received.
    CHIP_ERROR delegateResult = mDelegate.HandleAnswer(sessionId, sdpAnswer);
    if (delegateResult == CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::Success;
    }
    else if (delegateResult == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }
}

DataModel::ActionReturnStatus
WebRTCTransportRequestorServer::HandleICECandidates(const Commands::ICECandidates::DecodableType & req,
                                                    const CommandHandler & commandHandler)
{
    // Extract command fields from the request
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

    // Check ice candidates min 1 constraint.
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

    // Handle ICE candidates in Delegate
    CHIP_ERROR delegateResult = mDelegate.HandleICECandidates(sessionId, candidates);
    if (delegateResult == CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::Success;
    }
    else if (delegateResult == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }
}

DataModel::ActionReturnStatus WebRTCTransportRequestorServer::HandleEnd(const Commands::End::DecodableType & req,
                                                                        const CommandHandler & commandHandler)
{
    // Extract command fields from the request
    uint16_t sessionId = req.webRTCSessionID;
    auto reason        = req.reason;

    // Validate the reason field against the allowed enum values.
    if (reason == WebRTCEndReasonEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleEnd: Invalid reason value %u.", static_cast<uint8_t>(reason));
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    // Check if the session, NodeID are valid
    if (!IsPeerNodeSessionValid(sessionId, commandHandler))
    {
        return Protocols::InteractionModel::Status::NotFound;
    }

    // Handle End command in Delegate
    CHIP_ERROR delegateResult = mDelegate.HandleEnd(sessionId, reason);

    // Store the WebRTCSessionStruct in the CurrentSessions
    RemoveSession(sessionId);

    if (delegateResult == CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::Success;
    }
    else if (delegateResult == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }
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
