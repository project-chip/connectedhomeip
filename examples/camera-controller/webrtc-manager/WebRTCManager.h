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

#include <app-common/zap-generated/cluster-objects.h>
#include <functional>
#include <platform/CHIPDeviceLayer.h>
#include <rtc/rtc.hpp>
#include <webrtc-manager/WebRTCProviderClient.h>
#include <webrtc-manager/WebRTCRequestorDelegate.h>

class WebRTCManager
{
public:
    using ICECandidateStruct         = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
    using StreamUsageEnum            = chip::app::Clusters::Globals::StreamUsageEnum;
    using SessionEstablishedCallback = std::function<void(uint16_t streamId)>;

    static WebRTCManager & Instance()
    {
        static WebRTCManager instance;
        return instance;
    }

    void Init();

    /**
     * @brief Set callback to be invoked when WebRTC session is established
     *
     * @param callback Function to call when session is established
     */
    void SetSessionEstablishedCallback(SessionEstablishedCallback callback) { mSessionEstablishedCallback = callback; }

    CHIP_ERROR HandleOffer(uint16_t sessionId, const WebRTCRequestorDelegate::OfferArgs & args);

    CHIP_ERROR HandleAnswer(uint16_t sessionId, const std::string & sdp);

    CHIP_ERROR HandleICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates);

    CHIP_ERROR Connnect(chip::Controller::DeviceCommissioner & commissioner, chip::NodeId nodeId, chip::EndpointId endpointId);

    CHIP_ERROR ProvideOffer(chip::app::DataModel::Nullable<uint16_t> sessionId, StreamUsageEnum streamUsage,
                            chip::Optional<chip::app::DataModel::Nullable<uint16_t>> videoStreamId,
                            chip::Optional<chip::app::DataModel::Nullable<uint16_t>> audioStreamId);

    CHIP_ERROR SolicitOffer(StreamUsageEnum streamUsage);

    CHIP_ERROR ProvideAnswer(uint16_t sessionId, const std::string & sdp);

    CHIP_ERROR ProvideICECandidates(uint16_t sessionId);

    /**
     * @brief Close the WebRTC connection and clean up resources
     */
    void Disconnect();

private:
    // Make the constructor private to enforce the singleton pattern
    WebRTCManager();
    ~WebRTCManager();

    chip::app::Clusters::WebRTCTransportRequestor::WebRTCTransportRequestorServer mWebRTCRequestorServer;

    WebRTCProviderClient mWebRTCProviderClient;
    WebRTCRequestorDelegate mWebRTCRequestorDelegate;

    std::shared_ptr<rtc::PeerConnection> mPeerConnection;

    uint16_t mPendingSessionId = 0;
    std::string mLocalDescription;

    // Local vector to store the ICE Candidate strings coming from the WebRTC stack
    std::vector<std::string> mLocalCandidates;

    std::shared_ptr<rtc::Track> mTrack;

    // Callback to notify when session is established
    SessionEstablishedCallback mSessionEstablishedCallback;

    // Track the current video stream ID for the session
    uint16_t mCurrentVideoStreamId = 0;

    // UDP socket for RTP forwarding
    int mRTPSocket = -1;

    // Close and reset the RTP socket
    void CloseRTPSocket();
};
