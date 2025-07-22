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

#include "WebRTCTransportProviderManager.h"
#include <string>
#include <vector>

namespace chip {
namespace python {

// The callback methods provided by python.
ProvideOfferCallback gProvideOfferCallback                 = nullptr;
ProvideAnswerCallback gProvideAnswerCallback               = nullptr;
ProvideICECandidatesCallback gProvideICECandidatesCallback = nullptr;
ProvideEndCallback gEndSessionCallback                     = nullptr;
SessionIdCreatedCallback gSessionIdCreatedCallback         = nullptr;
} // namespace python
} // namespace chip

using namespace chip::python;

void WebRTCTransportProviderManager::Init()
{
#if CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
    chip::app::dynamic_server::InitAccessControl();
#endif
    mWebRTCProviderServer.Init();
}

void WebRTCTransportProviderManager::InitCallbacks(ProvideOfferCallback provideOfferCallback,
                                                   ProvideAnswerCallback provideAnswerCallback,
                                                   ProvideICECandidatesCallback provideICECandidatesCallback,
                                                   ProvideEndCallback provideEndCallback,
                                                   SessionIdCreatedCallback sessionIdCreatedCallback)
{
    gProvideOfferCallback         = provideOfferCallback;
    gProvideAnswerCallback        = provideAnswerCallback;
    gProvideICECandidatesCallback = provideICECandidatesCallback;
    gEndSessionCallback           = provideEndCallback;
    gSessionIdCreatedCallback     = sessionIdCreatedCallback;
}

CHIP_ERROR WebRTCTransportProviderManager::HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession,
                                                              bool & outDeferredOffer)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR WebRTCTransportProviderManager::HandleProvideOffer(const ProvideOfferRequestArgs & args,
                                                              WebRTCSessionStruct & outSession)
{
    ChipLogProgress(Camera, "HandleProvideOffer called");
    if (gSessionIdCreatedCallback)
        gSessionIdCreatedCallback(args.sessionId, args.peerNodeId);

    // Initialize a new WebRTC session from the SolicitOfferRequestArgs
    outSession.id          = args.sessionId;
    outSession.peerNodeID  = args.peerNodeId;
    outSession.streamUsage = args.streamUsage;
    outSession.fabricIndex = args.fabricIndex;

    // Resolve or allocate a VIDEO stream
    if (args.videoStreamId.HasValue())
    {
        if (args.videoStreamId.Value().IsNull())
        {
            // Automatic stream selection behavior is ambiguous for controller role. Do nothing for now.
        }
        else
        {
            outSession.videoStreamID = args.videoStreamId.Value();
        }
    }
    else
    {
        outSession.videoStreamID.SetNull();
    }

    // Resolve or allocate an AUDIO stream
    if (args.audioStreamId.HasValue())
    {
        if (args.audioStreamId.Value().IsNull())
        {
            // Automatic stream selection behavior is ambiguous for controller role. Do nothing for now.
        }
        else
        {
            outSession.audioStreamID = args.audioStreamId.Value();
        }
    }
    else
    {
        outSession.audioStreamID.SetNull();
    }

    std::string offer = args.sdp;
    int err           = gProvideOfferCallback(args.sessionId, offer.c_str());
    return err == 0 ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR WebRTCTransportProviderManager::HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR WebRTCTransportProviderManager::HandleProvideICECandidates(uint16_t sessionId,
                                                                      const std::vector<ICECandidateStruct> & candidates)
{
    std::vector<std::string> remoteCandidates;
    remoteCandidates.reserve(candidates.size());
    std::vector<const char *> cStrings;
    cStrings.reserve(candidates.size());

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

    int err = gProvideICECandidatesCallback(sessionId, cStrings.data(), static_cast<int>(cStrings.size()));
    return err == 0 ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR WebRTCTransportProviderManager::HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode,
                                                            chip::app::DataModel::Nullable<uint16_t> videoStreamID,
                                                            chip::app::DataModel::Nullable<uint16_t> audioStreamID)
{
    int err = gEndSessionCallback(sessionId, static_cast<uint8_t>(reasonCode));
    return err == 0 ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

// Below Delegate Calls are still ambiguous with their role with controller. Returning default positive values for now.
// TODO Implement delegate methods

CHIP_ERROR
WebRTCTransportProviderManager::ValidateStreamUsage(StreamUsageEnum streamUsage,
                                                    const chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & videoStreamId,
                                                    const chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & audioStreamId)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCTransportProviderManager::ValidateVideoStreamID(uint16_t videoStreamId)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCTransportProviderManager::ValidateAudioStreamID(uint16_t audioStreamId)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCTransportProviderManager::IsPrivacyModeActive(bool & isActive)
{
    isActive = false;
    return CHIP_NO_ERROR;
}

bool WebRTCTransportProviderManager::HasAllocatedVideoStreams()
{
    return true;
}

bool WebRTCTransportProviderManager::HasAllocatedAudioStreams()
{
    return true;
}
