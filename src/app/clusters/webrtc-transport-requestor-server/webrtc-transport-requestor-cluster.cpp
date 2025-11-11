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
#include "webrtc-transport-requestor-cluster.h"

#include <app/server-cluster/AttributeListBuilder.h>
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
    Commands::Offer::kMetadataEntry,
    Commands::Answer::kMetadataEntry,
    Commands::ICECandidates::kMetadataEntry,
    Commands::End::kMetadataEntry,
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
    DefaultServerCluster({ endpointId, Id }), mDelegate(delegate)
{}

DataModel::ActionReturnStatus WebRTCTransportRequestorServer::ReadAttribute(const DataModel::ReadAttributeRequest & request,
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
            return Status::InvalidCommand;
        }
        return HandleOffer(*handler, req);
    }
    case Commands::Answer::Id: {
        Commands::Answer::DecodableType req;
        if (DataModel::Decode(input_arguments, req) != CHIP_NO_ERROR)
        {
            return Status::InvalidCommand;
        }
        return HandleAnswer(*handler, req);
    }
    case Commands::ICECandidates::Id: {
        Commands::ICECandidates::DecodableType req;
        if (DataModel::Decode(input_arguments, req) != CHIP_NO_ERROR)
        {
            return Status::InvalidCommand;
        }
        return HandleICECandidates(*handler, req);
    }
    case Commands::End::Id: {
        Commands::End::DecodableType req;
        if (DataModel::Decode(input_arguments, req) != CHIP_NO_ERROR)
        {
            return Status::InvalidCommand;
        }
        return HandleEnd(*handler, req);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR WebRTCTransportRequestorServer::AcceptedCommands(const ConcreteClusterPath & path,
                                                            ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR WebRTCTransportRequestorServer::Attributes(const ConcreteClusterPath & path,
                                                      ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), {}, {});
}

DataModel::ActionReturnStatus WebRTCTransportRequestorServer::HandleOffer(const CommandHandler & commandHandler,
                                                                          const Commands::Offer::DecodableType & req)
{
    uint16_t sessionId = req.webRTCSessionID;

    // Get the session struct to pass to delegate
    NodeId peerNodeId           = GetNodeIdFromCtx(commandHandler);
    FabricIndex peerFabricIndex = commandHandler.GetAccessingFabricIndex();

    WebRTCSessionStruct * session = FindSession(sessionId, peerNodeId, peerFabricIndex);
    VerifyOrReturnValue(session != nullptr, Status::NotFound);

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
            return Status::InvalidCommand;
        }
    }

    // Convert ICETransportPolicy from CharSpan to std::string.
    if (req.ICETransportPolicy.HasValue())
    {
        args.iceTransportPolicy.SetValue(std::string(req.ICETransportPolicy.Value().data(), req.ICETransportPolicy.Value().size()));
    }

    // Delegate processing: handle the SDP offer, gather ICE candidates, SDP answer, etc.
    return mDelegate.HandleOffer(*session, args);
}

DataModel::ActionReturnStatus WebRTCTransportRequestorServer::HandleAnswer(const CommandHandler & commandHandler,
                                                                           const Commands::Answer::DecodableType & req)
{
    uint16_t sessionId = req.webRTCSessionID;
    auto sdpSpan       = req.sdp;

    // Get the session struct to pass to delegate
    NodeId peerNodeId           = GetNodeIdFromCtx(commandHandler);
    FabricIndex peerFabricIndex = commandHandler.GetAccessingFabricIndex();

    WebRTCSessionStruct * session = FindSession(sessionId, peerNodeId, peerFabricIndex);
    VerifyOrReturnValue(session != nullptr, Status::NotFound);

    std::string sdpAnswer(sdpSpan.data(), sdpSpan.size());

    // Delegate handles Answer command received.
    return mDelegate.HandleAnswer(*session, sdpAnswer);
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

        // Validate SDPMid constraint: if present, must have min length 1
        if (!candidate.SDPMid.IsNull() && candidate.SDPMid.Value().empty())
        {
            ChipLogError(Zcl, "HandleICECandidates: SDPMid must have minimum length of 1 when present");
            return Status::ConstraintError;
        }

        candidates.push_back(candidate);
    }

    // Check the validity of the list.
    CHIP_ERROR err = iter.GetStatus();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleICECandidates: ICECandidates list error: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::InvalidCommand;
    }

    // Check ice candidates min 1 contraint.
    if (candidates.empty())
    {
        ChipLogError(Zcl, "HandleICECandidates: No ICE candidates provided.");
        return Status::ConstraintError;
    }

    // Get the session struct to pass to delegate
    NodeId peerNodeId           = GetNodeIdFromCtx(commandHandler);
    FabricIndex peerFabricIndex = commandHandler.GetAccessingFabricIndex();

    WebRTCSessionStruct * session = FindSession(sessionId, peerNodeId, peerFabricIndex);
    VerifyOrReturnValue(session != nullptr, Status::NotFound);

    // Pass the full session struct so delegate can identify which camera this is for
    return mDelegate.HandleICECandidates(*session, candidates);
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
        return Status::ConstraintError;
    }

    // Get the session struct to pass to delegate
    NodeId peerNodeId           = GetNodeIdFromCtx(commandHandler);
    FabricIndex peerFabricIndex = commandHandler.GetAccessingFabricIndex();

    WebRTCSessionStruct * session = FindSession(sessionId, peerNodeId, peerFabricIndex);
    VerifyOrReturnValue(session != nullptr, Status::NotFound);

    // Pass the full session struct so delegate can identify which camera this is for
    CHIP_ERROR delegateResult = mDelegate.HandleEnd(*session, reason);
    RemoveSession(sessionId, peerNodeId, peerFabricIndex);

    return delegateResult;
}

// Helper functions
WebRTCSessionStruct * WebRTCTransportRequestorServer::FindSession(uint16_t sessionId, NodeId peerNodeId, FabricIndex fabricIndex)
{
    for (auto & session : mCurrentSessions)
    {
        if (session.id == sessionId && session.peerNodeID == peerNodeId && session.GetFabricIndex() == fabricIndex)
        {
            return &session;
        }
    }

    return nullptr;
}

WebRTCTransportRequestorServer::UpsertResultEnum WebRTCTransportRequestorServer::UpsertSession(const WebRTCSessionStruct & session)
{
    // Search for a session in the current sessions using the full tuple <id, peerNodeID, fabricIndex>
    // This is critical because different cameras can allocate the same sessionId
    UpsertResultEnum result;
    auto it = std::find_if(mCurrentSessions.begin(), mCurrentSessions.end(), [&session](const auto & existing) {
        return existing.id == session.id && existing.peerNodeID == session.peerNodeID &&
            existing.GetFabricIndex() == session.GetFabricIndex();
    });

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

    NotifyAttributeChanged(CurrentSessions::Id);

    return result;
}

void WebRTCTransportRequestorServer::RemoveSession(uint16_t sessionId, NodeId peerNodeId, FabricIndex fabricIndex)
{
    size_t originalSize = mCurrentSessions.size();
    // Remove the session if the full tuple <sessionId, peerNodeId, fabricIndex> matches
    // This is critical: different cameras can have the same sessionId, so we must match all three fields
    mCurrentSessions.erase(std::remove_if(mCurrentSessions.begin(), mCurrentSessions.end(),
                                          [sessionId, peerNodeId, fabricIndex](const WebRTCSessionStruct & s) {
                                              return s.id == sessionId && s.peerNodeID == peerNodeId &&
                                                  s.GetFabricIndex() == fabricIndex;
                                          }),
                           mCurrentSessions.end());

    // Check whether session was removed
    if (mCurrentSessions.size() < originalSize)
    {
        // Notify the stack that the CurrentSessions attribute has changed.
        NotifyAttributeChanged(CurrentSessions::Id);
    }
}

} // namespace WebRTCTransportRequestor
} // namespace Clusters
} // namespace app
} // namespace chip
