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
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <functional>
#include <platform/CHIPDeviceLayer.h>
#include <rtc/rtc.hpp>
#include <webrtc-manager/WebRTCProviderClient.h>
#include <webrtc-manager/WebRTCRequestorDelegate.h>

class WebRTCManager
{
public:
    using ICECandidateStruct         = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
    using WebRTCSessionStruct        = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
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

    CHIP_ERROR HandleOffer(const WebRTCSessionStruct & session, const WebRTCRequestorDelegate::OfferArgs & args);

    CHIP_ERROR HandleAnswer(const WebRTCSessionStruct & session, const std::string & sdp);

    CHIP_ERROR HandleICECandidates(const WebRTCSessionStruct & session, const std::vector<ICECandidateStruct> & candidates);

    CHIP_ERROR Connect(chip::Controller::DeviceCommissioner & commissioner, chip::NodeId nodeId, chip::EndpointId endpointId);

    CHIP_ERROR ProvideOffer(chip::app::DataModel::Nullable<uint16_t> sessionId, StreamUsageEnum streamUsage,
                            chip::Optional<chip::app::DataModel::Nullable<uint16_t>> videoStreamId,
                            chip::Optional<chip::app::DataModel::Nullable<uint16_t>> audioStreamId);

    CHIP_ERROR SolicitOffer(StreamUsageEnum streamUsage, chip::Optional<chip::app::DataModel::Nullable<uint16_t>> videoStreamId,
                            chip::Optional<chip::app::DataModel::Nullable<uint16_t>> audioStreamId);

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

    // Close and reset the RTP sockets
    void CloseRTPSockets();

    // PeerConnection callback handlers. The handlers are executed on the Matter thread.
    void OnLocalDescriptionGenerated(const std::shared_ptr<rtc::PeerConnection> & connection, const rtc::Description & desc);
    void OnLocalCandidateGathered(const std::shared_ptr<rtc::PeerConnection> & connection, const rtc::Candidate & candidate);
    void OnConnectionStateChanged(const std::shared_ptr<rtc::PeerConnection> & connection, rtc::PeerConnection::State state);
    void OnGatheringStateChanged(const std::shared_ptr<rtc::PeerConnection> & connection,
                                 rtc::PeerConnection::GatheringState state);

    chip::app::LazyRegisteredServerCluster<chip::app::Clusters::WebRTCTransportRequestor::WebRTCTransportRequestorCluster>
        mWebRTCRegisteredServerCluster;

    WebRTCProviderClient mWebRTCProviderClient;
    WebRTCRequestorDelegate mWebRTCRequestorDelegate;

    std::shared_ptr<rtc::PeerConnection> mPeerConnection;

    enum class LocalSdpState
    {
        Idle,
        PendingOffer,
        PendingAnswer
    };

    struct PendingSdpContext
    {
        LocalSdpState state = LocalSdpState::Idle;
        uint16_t sessionId  = 0;
        chip::app::DataModel::Nullable<uint16_t> nullableSessionId;
        StreamUsageEnum streamUsage = {};
        chip::Optional<chip::app::DataModel::Nullable<uint16_t>> videoStreamId;
        chip::Optional<chip::app::DataModel::Nullable<uint16_t>> audioStreamId;

        void Reset() { *this = PendingSdpContext{}; }
    };
    PendingSdpContext mPendingSdpContext;

    // Local vector to store the ICE Candidate info coming from the WebRTC stack
    std::vector<ICECandidateInfo> mLocalCandidates;

    std::shared_ptr<rtc::Track> mTrack;
    std::shared_ptr<rtc::Track> mAudioTrack;

    // Callback to notify when session is established
    SessionEstablishedCallback mSessionEstablishedCallback;

    // Track the current video stream ID for the session
    uint16_t mCurrentVideoStreamId = 0;

    // UDP socket for RTP forwarding
    int mRTPSocket      = -1;
    int mAudioRTPSocket = -1;
};
