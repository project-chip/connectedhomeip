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

#include "webrtc-requestor-delegate.h"

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

CHIP_ERROR WebRTCRequestorDelegate::HandleOffer(const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
                                                const OfferArgs & aArgs)
{
    // Offers arrive only in the SolicitOffer flow, which this example does not use yet
    ChipLogError(AppServer, "AvAnalysisNode: unexpected Offer for WebRTC session %u", aSession.id);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR WebRTCRequestorDelegate::HandleAnswer(const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
                                                 const std::string & aSdpAnswer)
{
    VerifyOrReturnError(mPeerController != nullptr && mWebRTCClient != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(AppServer, "AvAnalysisNode: Answer received for WebRTC session %u", aSession.id);
    ReturnErrorOnFailure(mPeerController->ApplyAnswer(aSession.id, aSdpAnswer));
    mWebRTCClient->NotifyAnswered(aSession.id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR
WebRTCRequestorDelegate::HandleICECandidates(
    const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
    const std::vector<Clusters::WebRTCTransportRequestor::ICECandidateStruct> & aCandidates)
{
    VerifyOrReturnError(mPeerController != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(AppServer, "AvAnalysisNode: %u ICE candidates received for WebRTC session %u",
                    static_cast<unsigned>(aCandidates.size()), aSession.id);
    for (const auto & candidate : aCandidates)
    {
        ReturnErrorOnFailure(
            mPeerController->AddRemoteCandidate(aSession.id, std::string(candidate.candidate.data(), candidate.candidate.size())));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCRequestorDelegate::HandleEnd(const Clusters::WebRTCTransportRequestor::WebRTCSessionStruct & aSession,
                                              Clusters::WebRTCTransportRequestor::WebRTCEndReasonEnum aReason)
{
    VerifyOrReturnError(mWebRTCClient != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(AppServer, "AvAnalysisNode: End received for WebRTC session %u (reason %u)", aSession.id,
                    static_cast<unsigned>(aReason));
    // The client settles the stream state and releases the peer connection through OnSessionClosed
    mWebRTCClient->NotifyEnded(aSession.id);
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
