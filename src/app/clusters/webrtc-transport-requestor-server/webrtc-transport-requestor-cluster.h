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

/** @brief
 *  Defines methods for implementing application-specific logic for the WebRTCTransportRequestor Cluster.
 */
class WebRTCTransportRequestorDelegate
{
public:
    WebRTCTransportRequestorDelegate() = default;

    virtual ~WebRTCTransportRequestorDelegate() = default;

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
     * @param[in] args
     *   Structure containing all input arguments for the command.
     *
     * @return CHIP_ERROR
     *   - Returns error if the session is invalid or the candidates cannot be processed
     */
    virtual CHIP_ERROR HandleOffer(uint16_t sessionId, const OfferArgs & args) = 0;
    /**
     * @brief
     *   Handles the Answer command received by the server.
     *
     * @param[in]  sessionId Current session ID.
     * @param[in]  sdpAnswer  SDP answer received.
     *
     * @return CHIP_ERROR
     *   - Returns error if the session is invalid or the candidates cannot be processed
     */
    virtual CHIP_ERROR HandleAnswer(uint16_t sessionId, const std::string & sdpAnswer) = 0;
    /**
     * @brief
     *   Called when the server receives the ICECandidates command.
     *
     * @param[in] sessionId  Current session ID.
     * @param[in] candidates List of ICE candidate structs.
     * Note: The callee cannot reference the `candidates` vector after this call
     * returns, and must copy the contents over for later use, if required.
     *
     * @return CHIP_ERROR
     *   - Returns error if the session is invalid or the candidates cannot be processed
     */
    virtual CHIP_ERROR HandleICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates) = 0;
    /**
     * @brief
     *   Called when the server receives the End command.
     *
     * @param[in] sessionId  Current session ID to end.
     * @param[in] reasonCode Reason to end the session.
     *
     * @return CHIP_ERROR
     *   - Returns error if the session is invalid or the candidates cannot be processed
     */
    virtual CHIP_ERROR HandleEnd(uint16_t sessionId, WebRTCEndReasonEnum reasonCode) = 0;
};

class WebRTCTransportRequestorServer : public DefaultServerCluster
{
public:
    enum class UpsertResultEnum : uint8_t
    {
        kInserted = 0x00,
        kUpdated  = 0x01,
    };

    /**
     * @brief
     *  Creates a WebRTCTransportRequestorServer instance with a given endpoint and delegate.
     *
     * @param endpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param delegate A reference to the delegate to be used by this server.
     *                 The caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    WebRTCTransportRequestorServer(EndpointId endpointId, WebRTCTransportRequestorDelegate & delegate);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    /**
     * @brief
     *   Gets the current sessions from the Requestor server.
     */
    std::vector<WebRTCSessionStruct> GetCurrentSessions() const { return mCurrentSessions; }

    /**
     * @brief
     * Inserts a new session or updates an existing one based on session ID.
     *
     * @param session The session data to insert or update.
     * @return kInserted if a new session was added, kUpdated if an existing one was modified.
     */
    UpsertResultEnum UpsertSession(const WebRTCSessionStruct & session);

    /**
     * @brief
     *   Removes a session identified by its session ID from the internal list of current sessions.
     *
     * @param sessionId  The session ID of the session to remove.
     *                   If the ID is not found, the call is a no‑op.
     */
    void RemoveSession(uint16_t sessionId);

private:
    WebRTCTransportRequestorDelegate & mDelegate;
    std::vector<WebRTCSessionStruct> mCurrentSessions;

    // Helper functions
    WebRTCSessionStruct * FindSession(uint16_t sessionId);
    uint16_t GenerateSessionId();
    bool IsPeerNodeSessionValid(uint16_t sessionId, const CommandHandler & commandHandler);

    // Command handlers
    DataModel::ActionReturnStatus HandleOffer(const Commands::Offer::DecodableType & req, const CommandHandler & commandHandler);
    DataModel::ActionReturnStatus HandleAnswer(const Commands::Answer::DecodableType & req, const CommandHandler & commandHandler);
    DataModel::ActionReturnStatus HandleICECandidates(const Commands::ICECandidates::DecodableType & req,
                                                      const CommandHandler & commandHandler);
    DataModel::ActionReturnStatus HandleEnd(const Commands::End::DecodableType & req, const CommandHandler & commandHandler);
};

} // namespace WebRTCTransportRequestor
} // namespace Clusters
} // namespace app
} // namespace chip
