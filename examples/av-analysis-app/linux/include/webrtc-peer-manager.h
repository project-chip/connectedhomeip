/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "webrtc-peer-controller.h"

#include <rtc/rtc.hpp>

namespace chip {
namespace app {

/**
 * The Linux implementation of WebRTCPeerController: owns the libdatachannel peer connections behind
 * the AV Analysis WebRTC client, one per WebRTC session, keyed by the camera together with the
 * session id it assigned: those ids are unique only within one camera. A connection is created when
 * the client asks for an offer, bound to its session when the camera assigns one, and released when
 * the session is over.
 *
 * The controller methods run on the Matter thread; libdatachannel's own callbacks are rescheduled
 * onto it before touching any state here.
 */
class WebRTCPeerManager : public WebRTCPeerController
{
public:
    WebRTCPeerManager();
    ~WebRTCPeerManager() override;

    // AvAnalysisWebRTCPeerDelegate
    CHIP_ERROR CreateOffer(OfferCallback & aCallback) override;
    void OnSessionAssigned(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId) override;
    void OnOfferAbandoned() override;
    void OnSessionClosed(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId) override;

    // WebRTCPeerController inbound signaling
    CHIP_ERROR ApplyAnswer(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId, const std::string & aSdp) override;
    CHIP_ERROR AddRemoteCandidate(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId,
                                  const std::string & aCandidate) override;
    std::vector<LocalICECandidate> TakeLocalCandidates(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId) override;

private:
    // The camera and the id it assigned: ids are unique only within a camera, so both identify a session
    using SessionKey = std::pair<ScopedNodeId, uint16_t>;

    struct PeerSession
    {
        std::shared_ptr<rtc::PeerConnection> peerConnection;
        std::shared_ptr<rtc::Track> videoTrack;
        // Gathered local candidates, buffered until they are taken for sending to the camera
        std::vector<LocalICECandidate> localCandidates;
    };

    // The assigned session a given connection belongs to
    std::map<SessionKey, PeerSession>::iterator FindAssignedSession(const std::shared_ptr<rtc::PeerConnection> & aPeerConnection);
    // The session a given connection belongs to: the pending one, or an assigned entry
    PeerSession * FindSession(const std::shared_ptr<rtc::PeerConnection> & aPeerConnection);
    // A connection's state change, on the Matter thread
    void OnPeerConnectionStateChanged(const std::shared_ptr<rtc::PeerConnection> & aPeerConnection,
                                      rtc::PeerConnection::State aState);

    // A pending session always has a connection
    bool HasPendingSession() const { return mPendingSession.peerConnection != nullptr; }
    // Closes the pending connection, if any, and clears the pending session
    void ReleasePendingSession();

    // Created by CreateOffer; moves into mSessions once the camera assigns the id
    PeerSession mPendingSession;

    std::map<SessionKey, PeerSession> mSessions;
};

} // namespace app
} // namespace chip
