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
#include <app/ConcreteAttributePath.h>
#include <app/reporting/reporting.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportRequestor {

class WebRTCRequestorDelegate;

using ICEServerStruct     = Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct = Structs::WebRTCSessionStruct::Type;

class WebRTCRequestorServer : private AttributeAccessInterface, private CommandHandlerInterface
{
public:
    /**
     * @brief
     *  Creates a WebRTCRequestorServer instance with a given endpoint and delegate.
     *
     * @param endpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param delegate A reference to the delegate to be used by this server.
     *                 The caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    WebRTCRequestorServer(EndpointId endpointId, WebRTCRequestorDelegate * delegate);

    /**
     * @brief
     *  WebRTCRequestorServer destructor for clean up.
     *
     */
    ~WebRTCRequestorServer();

    /**
     * * @brief
     * Initializes the WebRTCRequestorServer instance.
     *
     * @return Returns an error if the registration fails.
     */
    CHIP_ERROR Init();

private:
    WebRTCRequestorDelegate * mDelegate = nullptr;
    std::vector<WebRTCSessionStruct> mCurrentSessions;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;

    // Helper functions
    WebRTCSessionStruct * FindSession(uint16_t sessionId);
    void AddOrUpdateSession(const WebRTCSessionStruct & session);
    void RemoveSession(uint16_t sessionId);

    // Command handlers
    void HandleOffer(HandlerContext & ctx, const Commands::Offer::DecodableType & req);
    void HandleAnswer(HandlerContext & ctx, const Commands::Answer::DecodableType & req);
    void HandleICECandidates(HandlerContext & ctx, const Commands::ICECandidates::DecodableType & req);
    void HandleEnd(HandlerContext & ctx, const Commands::End::DecodableType & req);
};

/** @brief
 *  Defines methods for implementing application-specific logic for the WebRTCRequestor Cluster.
 */
class WebRTCRequestorDelegate
{
public:
    WebRTCRequestorDelegate() = default;

    virtual ~WebRTCRequestorDelegate() = default;

    /**
     * @brief
     *   Handles the Offer command received by the server.
     *
     * @param[in]  sessionId Current session ID.
     * @param[in]  sdpOffer  SDP offer received.
     * @param[in]  iceServers ICE Servers received to generated public ICE candidates.
     * @param[in]  iceTransportPolicy Allowed ICE candidate policy (e.g.; 'all', 'relay') to generate ICE candidates
     * @param[in]  peerNodeId The Node ID of the provider.
     * @param[in]  peerFabricIndex The FabricIndex of the provider.
     *
     * @param[out] outSession New session struct is created with the new session details received.
     *
     * @return CHIP_ERROR
     *   - Returns error if the session is invalid or the candidates cannot be processed
     */
    virtual CHIP_ERROR HandleOffer(uint16_t sessionId, const chip::CharSpan & sdpOffer,
                                   const Optional<DataModel::DecodableList<ICEServerStruct>> & iceServers,
                                   const Optional<chip::CharSpan> & iceTransportPolicy, NodeId peerNodeId,
                                   FabricIndex peerFabricIndex, WebRTCSessionStruct & outSession) = 0;
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
    virtual CHIP_ERROR HandleAnswer(uint16_t sessionId, const chip::CharSpan & sdpAnswer) = 0;
    /**
     * @brief
     *   Called when the server receives the ICECandidates command.
     *
     * @param[in] sessionId  Current session ID.
     * @param[in] candidates List of ICE candidate strings.
     *
     * @return CHIP_ERROR
     *   - Returns error if the session is invalid or the candidates cannot be processed
     */
    virtual CHIP_ERROR HandleICECandidates(uint16_t sessionId, const std::vector<std::string> & candidates) = 0;
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

protected:
    friend class WebRTCRequestorServer;

    WebRTCRequestorServer * mWebRTCRequestorServer = nullptr;

    // sets the Chime Server pointer
    void SetWebRTCRequestorServer(WebRTCRequestorServer * WebRTCRequestorServer) { mWebRTCRequestorServer = WebRTCRequestorServer; }
    WebRTCRequestorServer * GetWebRTCRequestorServer() const { return mWebRTCRequestorServer; }
};

} // namespace WebRTCTransportRequestor
} // namespace Clusters
} // namespace app
} // namespace chip
