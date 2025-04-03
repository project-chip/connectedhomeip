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
#include <app/clusters/webrtc-transport-provider-server/webrtc-transport-provider-server.h>
#include <rtc/rtc.hpp>

namespace Camera {

class WebRTCProviderManager : public chip::app::Clusters::WebRTCTransportProvider::Delegate
{
public:
    WebRTCProviderManager() = default;
    ~WebRTCProviderManager() { CloseConnection(); };

    void CloseConnection();

    CHIP_ERROR HandleSolicitOffer(const OfferRequestArgs & args,
                                  chip::app::Clusters::WebRTCTransportProvider::WebRTCSessionStruct & outSession,
                                  bool & outDeferredOffer) override;

    CHIP_ERROR
    HandleProvideOffer(const ProvideOfferRequestArgs & args,
                       chip::app::Clusters::WebRTCTransportProvider::WebRTCSessionStruct & outSession,
                       const chip::ScopedNodeId & peerId, chip::EndpointId originatingEndpointId) override;

    CHIP_ERROR HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer) override;

    CHIP_ERROR HandleProvideICECandidates(uint16_t sessionId, const std::vector<std::string> & candidates) override;

    CHIP_ERROR HandleEndSession(uint16_t sessionId, chip::app::Clusters::WebRTCTransportProvider::WebRTCEndReasonEnum reasonCode,
                                chip::app::DataModel::Nullable<uint16_t> videoStreamID,
                                chip::app::DataModel::Nullable<uint16_t> audioStreamID) override;

    CHIP_ERROR ValidateStreamUsage(chip::app::Clusters::WebRTCTransportProvider::StreamUsageEnum streamUsage,
                                   const chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & videoStreamId,
                                   const chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & audioStreamId) override;

private:
    void ScheduleAnswerSend();
    CHIP_ERROR SendAnswerCommand(uint16_t sessionId, const std::string & sdpAnswer, const chip::ScopedNodeId & peerId,
                                 chip::EndpointId originatingEndpointId);

    std::shared_ptr<rtc::PeerConnection> mPeerConnection;
    std::shared_ptr<rtc::DataChannel> mDataChannel;

    chip::ScopedNodeId mPeerId;
    chip::EndpointId mOriginatingEndpointId;

    uint16_t mCurrentSessionId = 0;
    std::string mSdpAnswer;
};

} // namespace Camera
