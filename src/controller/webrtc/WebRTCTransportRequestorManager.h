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

#include <app/ConcreteCommandPath.h>
#include <app/clusters/webrtc-transport-requestor-server/webrtc-transport-requestor-server.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>

#if CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
#include <app/dynamic_server/AccessControl.h>
#endif

// The Python callbacks to call when certain events happen in WebRTCTransportRequestor.
using OnOfferCallback         = int (*)(uint16_t, const char *);
using OnAnswerCallback        = int (*)(uint16_t, const char *);
using OnICECandidatesCallback = int (*)(uint16_t, const char **, const int);
using OnEndCallback           = int (*)(uint16_t, uint8_t);

class WebRTCTransportRequestorManager : public chip::app::Clusters::WebRTCTransportRequestor::WebRTCTransportRequestorDelegate
{
public:
    using ICECandidateStruct  = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
    using WebRTCEndReasonEnum = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

    static WebRTCTransportRequestorManager & Instance()
    {
        static WebRTCTransportRequestorManager instance;
        return instance;
    }

    // methods to be called by python
    void Init();

    void InitCallbacks(OnOfferCallback onOnOfferCallback, OnAnswerCallback onAnswerCallback,
                       OnICECandidatesCallback onICECandidatesCallback, OnEndCallback onEndCallback);

    // delegate methods
    CHIP_ERROR HandleOffer(uint16_t sessionId, const OfferArgs & args) override;

    CHIP_ERROR HandleAnswer(uint16_t sessionId, const std::string & sdpAnswer) override;

    CHIP_ERROR HandleICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates) override;

    CHIP_ERROR HandleEnd(uint16_t sessionId, WebRTCEndReasonEnum reasonCode) override;

    // method to be called by provider client
    void UpsertSession(const chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type & session);

private:
    WebRTCTransportRequestorManager() : mWebRTCRequestorServer(webRTCRequesterDynamicEndpointId, *this){};
    ~WebRTCTransportRequestorManager() = default;

#if CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
    int webRTCRequesterDynamicEndpointId = kWebRTCRequesterDynamicEndpointId;
#else
    int webRTCRequesterDynamicEndpointId = 1;
#endif

    chip::app::Clusters::WebRTCTransportRequestor::WebRTCTransportRequestorServer mWebRTCRequestorServer;
};
