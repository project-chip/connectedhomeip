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
#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportProvider {

using ICEServerStruct     = Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct = Structs::WebRTCSessionStruct::Type;

/**
 * @brief
 *   A delegate interface for the WebRTC Transport Provider cluster, allowing
 *   OEM or application-specific logic to be implemented outside of the cluster
 *   server itself. The server calls these methods to do the "real work."
 */
class Delegate
{
public:
    virtual ~Delegate() = default;

    /**
     * @brief
     *   This method is called when the server receives the SolicitOffer command.
     *
     *   It is used by the requestor to ask the provider to initiate a new Offer/Answer
     *   negotiation. The specification states that the provider may be in a standby mode
     *   (low power), thus a "deferred offer" scenario might occur if the device needs time
     *   to fully power resources needed for streaming.
     *
     * @param[in]  id               A local ID for the session being allocated
     * @param[in]  streamUsage      Indicates the usage for this session
     * @param[in]  videoStreamId    The requested VideoStreamID:
     *                                - Not present => No video requested
     *                                - Null => Automatic assignment requested
     *                                - Valid => Use the specified ID
     * @param[in]  audioStreamId    The requested AudioStreamID:
     *                                - Not present => No audio requested
     *                                - Null => Automatic assignment requested
     *                                - Valid => Use the specified ID
     * @param[in]  iceServers       Optional list of ICE server configurations (TURN/STUN).
     * @param[in]  iceTransportPolicy  If present, indicates whether ICE transport is 'all' or 'relay' only.
     * @param[in]  metadataOptions  Reserved for future use (value = 0).
     * @param[in]  peerNodeId       The Node ID of the controller or requestor.
     * @param[in]  peerFabricIndex  The FabricIndex of the requestor.
     * @param[out] outSession       On success, must be populated with the new session's details.
     * @param[out] outDeferredOffer True if the device is in standby or needs to defer generating an Offer.
     *
     * @return CHIP_ERROR
     *   - CHIP_NO_ERROR if a session was successfully created or prepared for negotiation
     *   - Appropriate error otherwise (e.g. out of resources)
     */
    virtual CHIP_ERROR HandleSolicitOffer(uint16_t id, StreamUsageEnum streamUsage,
                                          const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                          const Optional<DataModel::Nullable<uint16_t>> & audioStreamId,
                                          const Optional<DataModel::DecodableList<ICEServerStruct>> & iceServers,
                                          const Optional<chip::CharSpan> & iceTransportPolicy,
                                          const Optional<chip::BitMask<WebRTCMetadataOptionsBitmap>> & metadataOptions,
                                          NodeId peerNodeId, FabricIndex peerFabricIndex, WebRTCSessionStruct & outSession,
                                          bool & outDeferredOffer) = 0;

    /**
     * @brief
     *   This method is called when the server receives the ProvideOffer command.
     *
     *   ProvideOffer is used by a requestor to either:
     *     - Create a brand new session by sending an SDP Offer
     *     - Re-offer an SDP to modify an existing session (e.g., enabling two-way talk).
     *
     * @param[in]  id                The existing session ID or 0 if new (depending on your usage).
     * @param[in]  streamUsage       The stream usage (camera feed, doorbell, etc.).
     * @param[in]  sdp               The SDP Offer (RFC 8866).
     * @param[in]  videoStreamId     Requested or existing VideoStreamID.
     * @param[in]  audioStreamId     Requested or existing AudioStreamID.
     * @param[in]  iceServers        Optional ICE server list.
     * @param[in]  iceTransportPolicy If present, indicates 'all' or 'relay' for candidate gathering.
     * @param[in]  metadataOptions   Reserved (value = 0).
     * @param[in]  peerNodeId        Requestor’s NodeId.
     * @param[in]  peerFabricIndex   Requestor’s FabricIndex.
     * @param[out] outSession        Must be populated with the final session info (session ID, streams, etc.).
     *
     * @return CHIP_ERROR
     *   - CHIP_NO_ERROR on success
     *   - Appropriate error if the request is invalid or resources are exhausted
     */
    virtual CHIP_ERROR HandleProvideOffer(uint16_t id, StreamUsageEnum streamUsage, const chip::CharSpan & sdp,
                                          const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                          const Optional<DataModel::Nullable<uint16_t>> & audioStreamId,
                                          const Optional<DataModel::DecodableList<ICEServerStruct>> & iceServers,
                                          const Optional<chip::CharSpan> & iceTransportPolicy,
                                          const Optional<chip::BitMask<WebRTCMetadataOptionsBitmap>> & metadataOptions,
                                          NodeId peerNodeId, FabricIndex peerFabricIndex, WebRTCSessionStruct & outSession) = 0;

    /**
     * @brief
     *   Called when the server receives the ProvideAnswer command.
     *
     *   In the standard WebRTC Offer/Answer flow, once the provider sends an Offer (or
     *   the requestor has re-offered), the other side will respond with an SDP Answer.
     *   This method is invoked to store and process that Answer.
     *
     * @param[in] sessionId  The identifier of the current session.
     * @param[in] sdpAnswer  The SDP Answer string.
     *
     * @return CHIP_ERROR
     *   - CHIP_NO_ERROR if the Answer is accepted
     *   - Appropriate error if invalid or unexpected
     */
    virtual CHIP_ERROR HandleProvideAnswer(uint16_t sessionId, const chip::CharSpan & sdpAnswer) = 0;

    /**
     * @brief
     *   Called when the server receives the ProvideICECandidates command.
     *
     *   After the initial Offer/Answer exchange, additional ICE Candidates may be
     *   discovered. This command allows the requestor to provide those candidates
     *   (i.e., Trickle ICE).
     *
     * @param[in] sessionId  The current session ID.
     * @param[in] candidates A list of ICE candidate strings.
     *
     * @return CHIP_ERROR
     *   - CHIP_NO_ERROR on success
     *   - An error if the session is invalid or the candidates cannot be processed
     */
    virtual CHIP_ERROR HandleProvideICECandidates(uint16_t sessionId, const std::vector<std::string> & candidates) = 0;

    /**
     * @brief
     *   Called when the server or the requestor ends a session via the EndSession command.
     *
     * @param[in] sessionId  The ID of the session to end.
     * @param[in] reasonCode Reason for ending the session (e.g. normal closure, resource limit).
     *
     * @return CHIP_ERROR
     *   - CHIP_NO_ERROR on success
     *   - Error if no matching session is found or some cleanup error occurs
     */
    virtual CHIP_ERROR HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode) = 0;
};

class WebRTCTransportProviderServer : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    /**
     * @brief
     *   Constructs the WebRTCTransportProviderServer with the specified delegate and endpoint.
     *
     * @param[in] delegate   A pointer to an implementation of the Delegate interface. Must remain
     *                       valid for the lifetime of this object.
     * @param[in] endpointId The Endpoint where the WebRTC Transport Provider cluster is published.
     */
    WebRTCTransportProviderServer(Delegate * delegate, EndpointId endpointId);

    /**
     * @brief
     *   Destructor. Cleans up any internal data, but does not destroy the delegate.
     */
    ~WebRTCTransportProviderServer() override;

    /**
     * @brief
     *   Initializes this server instance.
     *
     * @return CHIP_ERROR
     *   - CHIP_NO_ERROR on successful registration
     *   - Error code if the registration failed
     */
    CHIP_ERROR Init();

private:
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    void InvokeCommand(HandlerContext & ctx) override;

    // Helper functions
    WebRTCSessionStruct * FindSession(uint16_t sessionId);
    void AddOrUpdateSession(const WebRTCSessionStruct & session);
    void RemoveSession(uint16_t sessionId);
    uint16_t GenerateSessionID();

    // Command Handlers
    void HandleSolicitOffer(HandlerContext & ctx, const Commands::SolicitOffer::DecodableType & req);
    void HandleProvideOffer(HandlerContext & ctx, const Commands::ProvideOffer::DecodableType & req);
    void HandleProvideAnswer(HandlerContext & ctx, const Commands::ProvideAnswer::DecodableType & req);
    void HandleProvideICECandidates(HandlerContext & ctx, const Commands::ProvideICECandidates::DecodableType & req);
    void HandleEndSession(HandlerContext & ctx, const Commands::EndSession::DecodableType & req);

    Delegate * mDelegate = nullptr;
    std::vector<WebRTCSessionStruct> mCurrentSessions;
};

} // namespace WebRTCTransportProvider
} // namespace Clusters
} // namespace app
} // namespace chip
