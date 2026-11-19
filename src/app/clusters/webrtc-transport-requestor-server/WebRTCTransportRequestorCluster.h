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
#pragma once

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/WebRTCTransportRequestor/ClusterId.h>
#include <clusters/WebRTCTransportRequestor/Commands.h>
#include <clusters/WebRTCTransportRequestor/Enums.h>
#include <clusters/WebRTCTransportRequestor/Metadata.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>
#include <string>
#include <sys/types.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportRequestor {

using ICEServerDecodableStruct = chip::app::Clusters::Globals::Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct      = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
using ICECandidateStruct       = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
using StreamUsageEnum          = chip::app::Clusters::Globals::StreamUsageEnum;
using WebRTCEndReasonEnum      = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

/**
 * A WebRTCSessionStruct that owns the storage for its VideoStreams and AudioStreams lists, so that it stays valid after
 * the lists it was copied from are gone.
 */
class WebRTCSessionStorage : public WebRTCSessionStruct
{
public:
    explicit WebRTCSessionStorage(const WebRTCSessionStruct & session) { Assign(session); }
    WebRTCSessionStorage(const WebRTCSessionStorage & other) : WebRTCSessionStruct() { Assign(other); }

    WebRTCSessionStorage & operator=(const WebRTCSessionStorage & other)
    {
        Assign(other);
        return *this;
    }

    WebRTCSessionStorage & operator=(const WebRTCSessionStruct & session)
    {
        Assign(session);
        return *this;
    }

private:
    void Assign(const WebRTCSessionStruct & session);

    std::vector<uint16_t> mVideoStreams;
    std::vector<uint16_t> mAudioStreams;
};

/** @brief
 *  Defines methods for implementing application-specific logic for the WebRTCTransportRequestor Cluster.
 */
class Delegate
{
public:
    Delegate() = default;

    virtual ~Delegate() = default;

    struct OfferArgs
    {
        std::string sdp;
        Optional<std::vector<ICEServerDecodableStruct>> iceServers;
        Optional<std::string> iceTransportPolicy;
        NodeId peerNodeId;
    };

    /**
     * @brief
     *   Handles the Offer command received by the server.
     *
     * @param[in] session  The WebRTC session for this offer.
     * @param[in] args     Structure containing all input arguments for the command.
     *
     * @return CHIP_ERROR
     *   - Returns error if the session is invalid or the candidates cannot be processed
     */
    virtual CHIP_ERROR HandleOffer(const WebRTCSessionStruct & session, const OfferArgs & args) = 0;
    /**
     * @brief
     *   Handles the Answer command received by the server.
     *
     * @param[in] session    The WebRTC session for this answer.
     * @param[in] sdpAnswer  SDP answer received.
     *
     * @return CHIP_ERROR
     *   - Returns error if the session is invalid or the candidates cannot be processed
     */
    virtual CHIP_ERROR HandleAnswer(const WebRTCSessionStruct & session, const std::string & sdpAnswer) = 0;
    /**
     * @brief
     *   Called when the server receives the ICECandidates command.
     *
     * @param[in] session     The WebRTC session for these candidates.
     * @param[in] candidates  List of ICE candidate structs.
     * Note: The callee cannot reference the `candidates` vector after this call
     * returns, and must copy the contents over for later use, if required.
     *
     * @return CHIP_ERROR
     *   - Returns error if the session is invalid or the candidates cannot be processed
     */
    virtual CHIP_ERROR HandleICECandidates(const WebRTCSessionStruct & session,
                                           const std::vector<ICECandidateStruct> & candidates) = 0;
    /**
     * @brief
     *   Called when the server receives the End command.
     *
     * @param[in] session     The WebRTC session to end.
     * @param[in] reasonCode  Reason to end the session.
     *
     * @return CHIP_ERROR
     *   - Returns error if the session is invalid or the candidates cannot be processed
     */
    virtual CHIP_ERROR HandleEnd(const WebRTCSessionStruct & session, WebRTCEndReasonEnum reasonCode) = 0;
};

class WebRTCTransportRequestorCluster : public DefaultServerCluster
{
public:
    enum class UpsertResultEnum : uint8_t
    {
        kInserted = 0x00,
        kUpdated  = 0x01,
    };

    /**
     * @brief
     *  Creates a WebRTCTransportRequestorCluster instance with a given endpoint and delegate.
     *
     * @param endpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param delegate A reference to the delegate to be used by this server.
     *                 The caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    WebRTCTransportRequestorCluster(EndpointId endpointId, Delegate & delegate);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    /**
     * @brief
     *   Gets a copy of the current sessions from the Requestor server.
     *
     * Each returned session owns its VideoStreams and AudioStreams lists, so the copy stays valid when the cluster's
     * sessions change afterwards.
     */
    std::vector<WebRTCSessionStorage> GetCurrentSessions() const { return mCurrentSessions; }

    /**
     * @brief
     * Inserts a new session or updates an existing one based on session ID.
     *
     * The cluster keeps its own copy of the session, including its VideoStreams and AudioStreams lists, so the caller's
     * lists only need to be valid during this call.
     *
     * @param session The session data to insert or update.
     * @return kInserted if a new session was added, kUpdated if an existing one was modified.
     */
    UpsertResultEnum UpsertSession(const WebRTCSessionStruct & session);

    /**
     * A session the requestor started with SolicitOffer or ProvideOffer, as given by the request and the provider's
     * response. The lists only need to be valid during RecordOfferedSession() / MakeOfferedSession().
     */
    struct OfferedSession
    {
        uint16_t sessionId          = 0;
        NodeId peerNodeId           = kUndefinedNodeId;
        FabricIndex fabricIndex     = kUndefinedFabricIndex;
        EndpointId peerEndpointId   = kInvalidEndpointId;
        StreamUsageEnum streamUsage = StreamUsageEnum::kUnknownEnumValue;

        // From the request.
        Optional<DataModel::List<const uint16_t>> requestedVideoStreams;
        Optional<DataModel::List<const uint16_t>> requestedAudioStreams;
        Optional<bool> requestedMetadataEnabled;
        // A ProvideOffer with a non-null WebRTCSessionID.
        bool isReOffer = false;

        // From the provider's response, which reports stream IDs only when the request used VideoStreamID / AudioStreamID.
        Optional<DataModel::Nullable<uint16_t>> reportedVideoStreamID;
        Optional<DataModel::Nullable<uint16_t>> reportedAudioStreamID;
    };

    /**
     * @brief Builds the CurrentSessions entry for an offered session.
     *
     * As WebRTCSessionStruct defines them, VideoStreams / AudioStreams hold every stream of the session: the requested
     * list, or else the single stream the provider reported. VideoStreamID / AudioStreamID hold the first entry, or null.
     * MetadataEnabled is the requested value.
     */
    static WebRTCSessionStorage MakeOfferedSession(const OfferedSession & offer);

    /**
     * @brief Inserts or updates the CurrentSessions entry for an offered session, built by MakeOfferedSession().
     *
     * A re-offer of a session that is already in CurrentSessions leaves the entry unchanged: a re-offer only changes the
     * SDP, since StreamUsage, the streams and MetadataEnabled apply to new sessions only.
     */
    void RecordOfferedSession(const OfferedSession & offer);

    /**
     * @brief
     *   Removes a session identified by its session ID, peer node ID, and fabric index from the internal list of current sessions.
     *
     * @param sessionId  The session ID of the session to remove.
     * @param peerNodeId The peer node ID of the session to remove.
     * @param fabricIndex The fabric index of the session to remove.
     *                   If the session is not found, the call is a no‑op.
     */
    void RemoveSession(uint16_t sessionId, NodeId peerNodeId, FabricIndex fabricIndex);

private:
    Delegate & mDelegate;
    std::vector<WebRTCSessionStorage> mCurrentSessions;

    // Helper functions
    WebRTCSessionStruct * FindSession(uint16_t sessionId, NodeId peerNodeId, FabricIndex fabricIndex);

    // Command handlers
    DataModel::ActionReturnStatus HandleOffer(const CommandHandler & commandHandler, const Commands::Offer::DecodableType & req);
    DataModel::ActionReturnStatus HandleAnswer(const CommandHandler & commandHandler, const Commands::Answer::DecodableType & req);
    DataModel::ActionReturnStatus HandleICECandidates(const CommandHandler & commandHandler,
                                                      const Commands::ICECandidates::DecodableType & req);
    DataModel::ActionReturnStatus HandleEnd(const CommandHandler & commandHandler, const Commands::End::DecodableType & req);
};

} // namespace WebRTCTransportRequestor
} // namespace Clusters
} // namespace app
} // namespace chip
