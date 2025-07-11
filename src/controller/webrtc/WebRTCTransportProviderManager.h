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
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/CASESessionManager.h>
#include <app/clusters/webrtc-transport-provider-server/webrtc-transport-provider-server.h>
#include <rtc/rtc.hpp>

#if CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
#include <app/dynamic_server/AccessControl.h>
#endif

// The Python callbacks to call when certain events happen in WebRTCTransportProvider.
using ProvideOfferCallback         = int (*)(uint16_t, const char *);
using ProvideAnswerCallback        = int (*)(uint16_t, const char *);
using ProvideICECandidatesCallback = int (*)(uint16_t, const char **, const int);
using ProvideEndCallback           = int (*)(uint16_t, uint8_t);
using SessionIdCreatedCallback     = void (*)(uint16_t, uint64_t);

using ICEServerDecodableStruct = chip::app::Clusters::Globals::Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct      = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
using ICECandidateStruct       = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
using StreamUsageEnum          = chip::app::Clusters::Globals::StreamUsageEnum;
using WebRTCEndReasonEnum      = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

class WebRTCTransportProviderManager : public chip::app::Clusters::WebRTCTransportProvider::Delegate
{
public:
    static WebRTCTransportProviderManager & Instance()
    {
        static WebRTCTransportProviderManager instance;
        return instance;
    }
    void Init();

    void InitCallbacks(ProvideOfferCallback provideOfferCallback, ProvideAnswerCallback provideAnswerCallback,
                       ProvideICECandidatesCallback provideICECandidatesCallback, ProvideEndCallback provideEndCallback,
                       SessionIdCreatedCallback sessionIdCreatedCallback);

    CHIP_ERROR HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession,
                                  bool & outDeferredOffer) override;

    CHIP_ERROR
    HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession) override;

    CHIP_ERROR HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer) override;

    CHIP_ERROR HandleProvideICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates) override;

    CHIP_ERROR HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode,
                                chip::app::DataModel::Nullable<uint16_t> videoStreamID,
                                chip::app::DataModel::Nullable<uint16_t> audioStreamID) override;

    CHIP_ERROR ValidateStreamUsage(StreamUsageEnum streamUsage,
                                   const chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & videoStreamId,
                                   const chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & audioStreamId) override;

    CHIP_ERROR ValidateVideoStreamID(uint16_t videoStreamId) override;

    CHIP_ERROR ValidateAudioStreamID(uint16_t audioStreamId) override;

    CHIP_ERROR IsPrivacyModeActive(bool & isActive) override;

    bool HasAllocatedVideoStreams() override;

    bool HasAllocatedAudioStreams() override;

private:
#if CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
    int webRTCProviderDynamicEndpointId = kWebRTCProviderDynamicEndpointId;
#else
    int webRTCProviderDynamicEndpointId = 1;
#endif

    WebRTCTransportProviderManager() : mWebRTCProviderServer(*this, webRTCProviderDynamicEndpointId) {};
    ~WebRTCTransportProviderManager() = default;

    chip::app::Clusters::WebRTCTransportProvider::WebRTCTransportProviderServer mWebRTCProviderServer;
};
