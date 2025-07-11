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
#include <app/clusters/webrtc-transport-requestor-server/webrtc-transport-requestor-server.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace chip {
namespace python {

// The callback methods provided by python.
OnOfferCallback gOnOfferCallback                 = nullptr;
OnAnswerCallback gOnAnswerCallback               = nullptr;
OnICECandidatesCallback gOnICECandidatesCallback = nullptr;
OnEndCallback gOnEndCallback                     = nullptr;

} // namespace python
} // namespace chip

using namespace chip::python;

void WebRTCTransportRequestorManager::Init()
{
#if CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
    dynamic_server::InitAccessControl();
#endif
    mWebRTCRequestorServer.Init();
}

void WebRTCTransportRequestorManager::InitCallbacks(OnOfferCallback onOnOfferCallback, OnAnswerCallback onAnswerCallback,
                                                    OnICECandidatesCallback onICECandidatesCallback, OnEndCallback onEndCallback)
{
    gOnOfferCallback         = onOnOfferCallback;
    gOnAnswerCallback        = onAnswerCallback;
    gOnICECandidatesCallback = onICECandidatesCallback;
    gOnEndCallback           = onEndCallback;
}

CHIP_ERROR WebRTCTransportRequestorManager::HandleOffer(uint16_t sessionId, const OfferArgs & args)
{
    std::string offer = args.sdp;
    int err           = gOnOfferCallback(sessionId, offer.c_str());
    return err == 0 ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR WebRTCTransportRequestorManager::HandleAnswer(uint16_t sessionId, const std::string & sdpAnswer)
{
    std::string answer = sdpAnswer;
    int err            = gOnAnswerCallback(sessionId, answer.c_str());
    return err == 0 ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR WebRTCTransportRequestorManager::HandleICECandidates(uint16_t sessionId,
                                                                const std::vector<ICECandidateStruct> & candidates)
{

    std::vector<std::string> remoteCandidates;
    remoteCandidates.clear();
    std::vector<const char *> cStrings;
    cStrings.clear();

    if (candidates.empty())
    {
        ChipLogError(Camera, "Candidate list is empty. At least one candidate is expected.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (const auto & candidate : candidates)
    {
        remoteCandidates.push_back(std::string(candidate.candidate.begin(), candidate.candidate.end()));
    }

    for (const std::string & candidate : remoteCandidates)
    {
        cStrings.push_back(candidate.c_str());
    }

    int err = gOnICECandidatesCallback(sessionId, cStrings.data(), static_cast<int>(cStrings.size()));
    return err == 0 ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR WebRTCTransportRequestorManager::HandleEnd(uint16_t sessionId, WebRTCTransportRequestor::WebRTCEndReasonEnum reasonCode)
{
    int err = gOnEndCallback(sessionId, static_cast<uint8_t>(reasonCode));
    return err == 0 ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

void WebRTCTransportRequestorManager::UpsertSession(
    const chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type & session)
{
    mWebRTCRequestorServer.UpsertSession(session);
}
