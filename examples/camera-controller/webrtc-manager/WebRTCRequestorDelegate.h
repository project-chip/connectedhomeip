/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-enums.h>
#include <app/clusters/webrtc-transport-requestor-server/webrtc-transport-requestor-server.h>

class WebRTCRequestorDelegate : public chip::app::Clusters::WebRTCTransportRequestor::WebRTCTransportRequestorDelegate
{
public:
    using ICECandidateStruct  = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
    using WebRTCEndReasonEnum = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

    WebRTCRequestorDelegate()  = default;
    ~WebRTCRequestorDelegate() = default;

    CHIP_ERROR HandleOffer(uint16_t sessionId, const OfferArgs & args) override;

    CHIP_ERROR HandleAnswer(uint16_t sessionId, const std::string & sdpAnswer) override;

    CHIP_ERROR HandleICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates) override;

    CHIP_ERROR HandleEnd(uint16_t sessionId, WebRTCEndReasonEnum reasonCode) override;
};
