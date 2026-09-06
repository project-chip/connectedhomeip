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

#include "webrtc-peer-controller.h"

#include <app/clusters/av-analysis-server/DefaultAvAnalysisWebRTCClient.h>
#include <app/clusters/webrtc-transport-requestor-server/WebRTCTransportRequestorCluster.h>

namespace chip {
namespace app {

/**
 * Receives the camera's inbound WebRTC signaling commands (Answer, ICECandidates, End) for the
 * sessions this node initiates through the AV Analysis cluster's WebRTC client.
 */
class WebRTCRequestorDelegate : public Clusters::WebRTCTransportRequestor::Delegate
{
public:
    void Init(WebRTCPeerController * aPeerController, Clusters::DefaultAvAnalysisWebRTCClient * aWebRTCClient)
    {
        mPeerController = aPeerController;
        mWebRTCClient   = aWebRTCClient;
    }

    CHIP_ERROR HandleOffer(const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
                           const OfferArgs & aArgs) override;
    CHIP_ERROR HandleAnswer(const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
                            const std::string & aSdpAnswer) override;
    CHIP_ERROR
    HandleICECandidates(const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
                        const std::vector<Clusters::WebRTCTransportRequestor::ICECandidateStruct> & aCandidates) override;
    CHIP_ERROR HandleEnd(const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
                         Clusters::WebRTCTransportRequestor::WebRTCEndReasonEnum aReason) override;

private:
    // Sends the candidates gathered for the session to its camera, the trickle phase after the Answer
    void SendLocalCandidates(uint16_t aWebRTCSessionId);

    WebRTCPeerController * mPeerController                  = nullptr;
    Clusters::DefaultAvAnalysisWebRTCClient * mWebRTCClient = nullptr;
};

} // namespace app
} // namespace chip
