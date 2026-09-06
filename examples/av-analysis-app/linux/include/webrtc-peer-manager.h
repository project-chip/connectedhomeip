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
#include <vector>

#include "webrtc-peer-controller.h"

#include <rtc/rtc.hpp>

namespace chip {
namespace app {

/**
 * The Linux implementation of WebRTCPeerController: owns the libdatachannel peer connections behind
 * the AV Analysis WebRTC client, one per WebRTC session, keyed by the camera-assigned session id. A
 * connection is created when the client asks for an offer, bound to its id when the camera assigns
 * one, and released when the session is over.
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
    void OnSessionAssigned(uint16_t aWebRTCSessionId) override;
    void OnOfferAbandoned() override;
    void OnSessionClosed(uint16_t aWebRTCSessionId) override;

    // WebRTCPeerController inbound signaling
    CHIP_ERROR ApplyAnswer(uint16_t aWebRTCSessionId, const std::string & aSdp) override;
    CHIP_ERROR AddRemoteCandidate(uint16_t aWebRTCSessionId, const std::string & aCandidate) override;

private:
    struct PeerSession
    {
        std::shared_ptr<rtc::PeerConnection> peerConnection;
        std::shared_ptr<rtc::Track> videoTrack;
        // Gathered local candidates, buffered until the client can trickle them to the camera
        std::vector<std::string> localCandidates;
    };

    // The session a given connection belongs to: the pending one, or an assigned entry
    PeerSession * FindSession(const std::shared_ptr<rtc::PeerConnection> & aPeerConnection);

    // A pending session always has a connection
    bool HasPendingSession() const { return mPendingSession.peerConnection != nullptr; }
    // Closes the pending connection, if any, and clears the pending session
    void ReleasePendingSession();

    // Created by CreateOffer; moves into mSessions once the camera assigns the id
    PeerSession mPendingSession;

    std::map<uint16_t, PeerSession> mSessions;
};

} // namespace app
} // namespace chip
