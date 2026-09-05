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
    CHIP_ERROR HandleOffer(const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
                           const OfferArgs & aArgs) override
    {
        // Offers arrive only in the SolicitOffer flow, which this example does not use yet
        ChipLogError(AppServer, "AvAnalysisNode: unexpected Offer for WebRTC session %u", aSession.id);
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR HandleAnswer(const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
                            const std::string & aSdpAnswer) override
    {
        // TODO: apply the answer to the session's peer connection and notify the WebRTC client
        ChipLogProgress(AppServer, "AvAnalysisNode: Answer received for WebRTC session %u", aSession.id);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleICECandidates(const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
                                   const std::vector<Clusters::WebRTCTransportRequestor::ICECandidateStruct> & aCandidates) override
    {
        // TODO: add the candidates to the session's peer connection
        ChipLogProgress(AppServer, "AvAnalysisNode: %u ICE candidates received for WebRTC session %u",
                        static_cast<unsigned>(aCandidates.size()), aSession.id);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleEnd(const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
                         Clusters::WebRTCTransportRequestor::WebRTCEndReasonEnum aReason) override
    {
        // TODO: release the session's peer connection and notify the WebRTC client
        ChipLogProgress(AppServer, "AvAnalysisNode: End received for WebRTC session %u (reason %u)", aSession.id,
                        static_cast<unsigned>(aReason));
        return CHIP_NO_ERROR;
    }
};

} // namespace app
} // namespace chip
