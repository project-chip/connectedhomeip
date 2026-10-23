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
#include <app/clusters/webrtc-transport-requestor-server/WebRTCTransportRequestorCluster.h>

class WebRTCRequestorDelegate : public chip::app::Clusters::WebRTCTransportRequestor::Delegate
{
public:
    using ICECandidateStruct  = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
    using WebRTCSessionStruct = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
    using WebRTCEndReasonEnum = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

    WebRTCRequestorDelegate()  = default;
    ~WebRTCRequestorDelegate() = default;

    CHIP_ERROR HandleOffer(const WebRTCSessionStruct & session, const OfferArgs & args) override;

    CHIP_ERROR HandleAnswer(const WebRTCSessionStruct & session, const std::string & sdpAnswer) override;

    CHIP_ERROR HandleICECandidates(const WebRTCSessionStruct & session,
                                   const std::vector<ICECandidateStruct> & candidates) override;

    CHIP_ERROR HandleEnd(const WebRTCSessionStruct & session, WebRTCEndReasonEnum reasonCode) override;
};
