/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <string>
#include <vector>

#include "WebRTCTransportRequestorManager.h"
#include <app/clusters/webrtc-transport-requestor-server/WebRTCTransportRequestorCluster.h>
#include <controller/webrtc/access_control/WebRTCAccessControl.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void WebRTCTransportRequestorManager::Init()
{
    Controller::AccessControl::InitAccessControl(kWebRTCRequesterDynamicEndpointId);

    mWebRTCRegisteredServerCluster.Create(kWebRTCRequesterDynamicEndpointId, *this);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mWebRTCRegisteredServerCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register WebRTCTransportRequestor on endpoint %u: %" CHIP_ERROR_FORMAT,
                     kWebRTCRequesterDynamicEndpointId, err.Format());
    }
}

void WebRTCTransportRequestorManager::Shutdown()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mWebRTCRegisteredServerCluster.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "WebRTCTransportRequestor unregister error: %" CHIP_ERROR_FORMAT, err.Format());
    }
    mWebRTCRegisteredServerCluster.Destroy();
}

void WebRTCTransportRequestorManager::InitCallbacks(OnOfferCallback onOnOfferCallback, OnAnswerCallback onAnswerCallback,
                                                    OnICECandidatesCallback onICECandidatesCallback, OnEndCallback onEndCallback)
{
    mOnOfferCallback         = onOnOfferCallback;
    mOnAnswerCallback        = onAnswerCallback;
    mOnICECandidatesCallback = onICECandidatesCallback;
    mOnEndCallback           = onEndCallback;
}

CHIP_ERROR WebRTCTransportRequestorManager::HandleOffer(const WebRTCSessionStruct & session, const OfferArgs & args)
{
    VerifyOrReturnError(mOnOfferCallback != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::string offer = args.sdp;
    return mOnOfferCallback(session.id, offer.c_str());
}

CHIP_ERROR WebRTCTransportRequestorManager::HandleAnswer(const WebRTCSessionStruct & session, const std::string & sdpAnswer)
{
    VerifyOrReturnError(mOnAnswerCallback != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::string answer = sdpAnswer;
    return mOnAnswerCallback(session.id, answer.c_str());
}

CHIP_ERROR WebRTCTransportRequestorManager::HandleICECandidates(const WebRTCSessionStruct & session,
                                                                const std::vector<ICECandidateStruct> & candidates)
{
    VerifyOrReturnError(mOnICECandidatesCallback != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::vector<OwnedIceCandidate> remoteCandidates;
    remoteCandidates.reserve(candidates.size());
    std::vector<IceCandidate> cStrings;
    cStrings.reserve(candidates.size());

    if (candidates.empty())
    {
        ChipLogError(Camera, "Candidate list is empty. At least one candidate is expected.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (const auto & candidate : candidates)
    {
        OwnedIceCandidate aIceCandidate;
        aIceCandidate.candidate     = std::make_unique<std::string>(candidate.candidate.begin(), candidate.candidate.end());
        bool isSdpMidNull           = candidate.SDPMid.IsNull();
        aIceCandidate.sdpMid        = isSdpMidNull
                   ? nullptr
                   : std::make_unique<std::string>(candidate.SDPMid.Value().begin(), candidate.SDPMid.Value().end());
        aIceCandidate.sdpMLineIndex = candidate.SDPMLineIndex.IsNull() ? -1 : static_cast<int>(candidate.SDPMLineIndex.Value());
        aIceCandidate.view = IceCandidate{ aIceCandidate.candidate->c_str(), isSdpMidNull ? nullptr : aIceCandidate.sdpMid->c_str(),
                                           aIceCandidate.sdpMLineIndex };
        remoteCandidates.push_back(std::move(aIceCandidate));
    }

    for (const auto & candidate : remoteCandidates)
    {
        cStrings.push_back(candidate.view);
    }

    return mOnICECandidatesCallback(session.id, cStrings.data(), static_cast<int>(cStrings.size()));
}

CHIP_ERROR WebRTCTransportRequestorManager::HandleEnd(const WebRTCSessionStruct & session,
                                                      WebRTCTransportRequestor::WebRTCEndReasonEnum reasonCode)
{
    VerifyOrReturnError(mOnEndCallback != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mOnEndCallback(session.id, static_cast<uint8_t>(reasonCode));
}

void WebRTCTransportRequestorManager::UpsertSession(
    const chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type & session)
{
    mWebRTCRegisteredServerCluster.Cluster().UpsertSession(session);
}
