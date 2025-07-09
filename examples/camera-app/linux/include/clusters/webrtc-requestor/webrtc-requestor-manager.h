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

#include "camera-device-interface.h"
#include <app-common/zap-generated/cluster-enums.h>
#include <app/CASESessionManager.h>
#include <app/clusters/webrtc-transport-requestor-server/webrtc-transport-requestor-server.h>
#include <app/dynamic_server/AccessControl.h>
#include <media-controller.h>
#include <rtc/rtc.hpp>
#include <webrtc-transport.h>

#include <unordered_map>

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportRequestor {

class WebRTCRequestorManager : public WebRTCTransportRequestorDelegate
{
public:
    using ICECandidateStruct  = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
    using WebRTCEndReasonEnum = chip::app::Clusters::Globals::WebRTCEndReasonEnum;
    WebRTCRequestorManager() :
        mOnConnectedCallback(OnDeviceConnected, this), mOnConnectionFailureCallback(OnDeviceConnectionFailure, this)
    {}

    ~WebRTCRequestorManager() { CloseConnection(); }

    void Init(chip::ScopedNodeId peerId, chip::EndpointId endpointId);

    void SetMediaController(MediaController * mediaController);

    void CloseConnection();

    CHIP_ERROR HandleOffer(uint16_t sessionId, const OfferArgs & args) override;

    CHIP_ERROR HandleAnswer(uint16_t sessionId, const std::string & sdpAnswer) override;

    CHIP_ERROR HandleICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates) override;

    CHIP_ERROR HandleEnd(uint16_t sessionId, WebRTCEndReasonEnum reasonCode) override;

    void ScheduleProvideOfferSend();

    void ScheduleProvideICECandidatesSend();

    void SetWebRTCRequestorServer(WebRTCTransportRequestorServer * server) { mWebRTCRequestorServer = server; }

    void SetCameraDevice(CameraDeviceInterface * aCameraDevice) { mCameraDevice = aCameraDevice; }

private:
    enum class CommandType : uint8_t
    {
        kUndefined            = 0,
        kProvideOffer         = 1,
        kAnswer               = 2,
        kProvideICECandidates = 3,
    };

    static void OnDeviceConnected(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                  const chip::SessionHandle & sessionHandle);

    static void OnDeviceConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    void SendProvideOffer();
    CHIP_ERROR SendProvideOfferCommand(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    void HandleProvideOfferResponse(const WebRTCTransportProvider::Commands::ProvideOfferResponse::DecodableType & value);

    CHIP_ERROR SendProvideICECandidatesCommand(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);

    CHIP_ERROR AcquireAudioVideoStreams();

    CHIP_ERROR ReleaseAudioVideoStreams();

    void RegisterWebrtcTransport(uint16_t sessionId);

    WebRTCTransportRequestorServer * mWebRTCRequestorServer;
    MediaController * mMediaController = nullptr;

    std::shared_ptr<rtc::PeerConnection> mPeerConnection;
    std::shared_ptr<rtc::Track> mVideoTrack;
    std::shared_ptr<rtc::Track> mAudioTrack;
    uint16_t mVideoStreamID;
    uint16_t mAudioStreamID;
    uint16_t mCurrentSessionId;

    chip::ScopedNodeId mPeerId;
    chip::EndpointId mOriginatingEndpointId;
    std::string mLocalSdp;

    std::vector<std::string> mLocalCandidates;

    CommandType mCommandType = CommandType::kUndefined;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;

    std::unordered_map<uint16_t, std::unique_ptr<WebrtcTransport>> mWebrtcTransportMap;

    // Handle to the Camera Device interface. For accessing other
    // clusters, if required.
    CameraDeviceInterface * mCameraDevice = nullptr;
};

} // namespace WebRTCTransportRequestor
} // namespace Clusters
} // namespace app
} // namespace chip
