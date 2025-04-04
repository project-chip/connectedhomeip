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
#include <platform/CHIPDeviceLayer.h>
#include <rtc/rtc.hpp>
#include <sys/socket.h>
#include <webrtc-manager/WebRTCProviderClient.h>
#include <webrtc-manager/WebRTCRequestorDelegate.h>

#define LOCALHOST_IP "127.0.0.1"
#define VIDEO_STREAM_DEST_PORT 5000
#define VIDEO_H264_CODEC 96
#define VIDEO_BITRATE 3000

class WebRTCManager
{
public:
    using ICECandidateStruct = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
    using StreamUsageEnum    = chip::app::Clusters::Globals::StreamUsageEnum;

    static WebRTCManager & Instance()
    {
        static WebRTCManager instance;
        return instance;
    }

    void Init();

    CHIP_ERROR HandleOffer(uint16_t sessionId, const WebRTCRequestorDelegate::OfferArgs & args);

    CHIP_ERROR HandleAnswer(uint16_t sessionId, const std::string & sdp);

    CHIP_ERROR HandleICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates);

    CHIP_ERROR Connnect(chip::Controller::DeviceCommissioner & commissioner, chip::NodeId nodeId, chip::EndpointId endpointId);

    CHIP_ERROR ProvideOffer(chip::app::DataModel::Nullable<uint16_t> sessionId, StreamUsageEnum streamUsage);

    CHIP_ERROR SolicitOffer(StreamUsageEnum streamUsage);

    CHIP_ERROR ProvideAnswer(uint16_t sessionId, const std::string & sdp);

    CHIP_ERROR ProvideICECandidates(uint16_t sessionId);

private:
    // Make the constructor private to enforce the singleton pattern
    WebRTCManager();
    ~WebRTCManager();

    chip::app::Clusters::WebRTCTransportRequestor::WebRTCTransportRequestorServer mWebRTCRequestorServer;

    WebRTCProviderClient mWebRTCProviderClient;
    WebRTCRequestorDelegate mWebRTCRequestorDelegate;

    std::shared_ptr<rtc::PeerConnection> mPeerConnection;
    std::shared_ptr<rtc::DataChannel> mDataChannel;

    std::string mLocalDescription;
    // Local vector to store the ICE Candidate strings coming from the WebRTC
    // stack
    std::vector<std::string> mLocalCandidates;

    // a track to receive video stream
    rtc::Description::Video mMedia;
    std::shared_ptr<rtc::Track> mTrack;
    // media handler to depacketize the incoming RTP stream
    std::shared_ptr<rtc::H264RtpDepacketizer> mDepacketizer;
    // socket to send over the video stream
    int sock;
    sockaddr_in socket_address;

    bool initializeSocket();
};
