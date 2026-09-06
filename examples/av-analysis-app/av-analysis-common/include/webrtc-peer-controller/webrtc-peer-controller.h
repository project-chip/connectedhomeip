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

#include <string>
#include <vector>

#include <app/clusters/av-analysis-server/DefaultAvAnalysisWebRTCClient.h>

namespace chip {
namespace app {

/**
 * Platform-supplied WebRTC peer connections: the SDK client's peer delegate and signaling for established sessions.
 */
class WebRTCPeerController : public Clusters::AvAnalysisWebRTCPeerDelegate
{
public:
    // An ICE candidate this node gathered, with the media section it belongs to
    struct LocalICECandidate
    {
        std::string candidate;
        std::string sdpMid;
    };

    /**
     * Learns of peer connections that failed after their session was assigned
     */
    class FailureObserver
    {
    public:
        virtual ~FailureObserver() = default;

        /**
         * The session's peer connection reached the Failed or Closed state
         */
        virtual void OnPeerConnectionFailed(uint16_t aWebRTCSessionId) = 0;
    };

    void SetFailureObserver(FailureObserver * aObserver) { mFailureObserver = aObserver; }

    /**
     * Applies the camera's SDP answer to the session's peer connection.
     */
    virtual CHIP_ERROR ApplyAnswer(uint16_t aWebRTCSessionId, const std::string & aSdp) = 0;

    /**
     * Adds a remote ICE candidate trickled by the camera to the session's peer connection.
     */
    virtual CHIP_ERROR AddRemoteCandidate(uint16_t aWebRTCSessionId, const std::string & aCandidate) = 0;

    /**
     * Hands over the candidates the session's peer connection has gathered so far and forgets them,
     * so each is sent to the camera once.
     */
    virtual std::vector<LocalICECandidate> TakeLocalCandidates(uint16_t aWebRTCSessionId) = 0;

protected:
    FailureObserver * mFailureObserver = nullptr;
};

} // namespace app
} // namespace chip
