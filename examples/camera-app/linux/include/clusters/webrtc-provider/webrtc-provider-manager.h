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
#include "webrtc-abstract.h"
#include <app-common/zap-generated/cluster-enums.h>
#include <app/CASESessionManager.h>
#include <app/clusters/webrtc-transport-provider-server/WebRTCTransportProviderCluster.h>
#include <media-controller.h>
#include <webrtc-transport.h>

#include <map>

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportProvider {

using ICEServerDecodableStruct = chip::app::Clusters::Globals::Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct      = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
using ICECandidateStruct       = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
using StreamUsageEnum          = chip::app::Clusters::Globals::StreamUsageEnum;
using WebRTCEndReasonEnum      = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

class WebRTCProviderManager : public Delegate
{
public:
    WebRTCProviderManager() :
        mOnConnectedCallback(OnDeviceConnected, this), mOnConnectionFailureCallback(OnDeviceConnectionFailure, this)
    {}

    ~WebRTCProviderManager() { CloseConnection(); };

    void Init();

    void CloseConnection();

    void SetMediaController(MediaController * mediaController);

    void SetWebRTCTransportProvider(WebRTCTransportProviderCluster * webRTCTransportProvider);

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
                                   chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & videoStreamId,
                                   chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & audioStreamId) override;

    void SetCameraDevice(CameraDeviceInterface * aCameraDevice);

    CHIP_ERROR ValidateVideoStreamID(uint16_t videoStreamId) override;

    CHIP_ERROR ValidateAudioStreamID(uint16_t audioStreamId) override;

    CHIP_ERROR IsStreamUsageSupported(StreamUsageEnum streamUsage) override;

    CHIP_ERROR IsHardPrivacyModeActive(bool & isActive) override;

    CHIP_ERROR IsSoftRecordingPrivacyModeActive(bool & isActive) override;

    CHIP_ERROR IsSoftLivestreamPrivacyModeActive(bool & isActive) override;

    bool HasAllocatedVideoStreams() override;

    bool HasAllocatedAudioStreams() override;

    CHIP_ERROR ValidateSFrameConfig(uint16_t cipherSuite, size_t baseKeyLength) override;

    CHIP_ERROR IsUTCTimeNull(bool & isNull) override;

    void LiveStreamPrivacyModeChanged(bool privacyModeEnabled);

private:
    std::string ExtractMidFromSdp(const std::string & sdp, const std::string & mediaType);

    void ScheduleOfferSend(uint16_t sessionId);

    void ScheduleICECandidatesSend(uint16_t sessionId);

    void ScheduleAnswerSend(uint16_t sessionId);

    void ScheduleEndSend(uint16_t sessionId);

    void RegisterWebrtcTransport(uint16_t sessionId);

    void UnregisterWebrtcTransport(uint16_t sessionId);

    CHIP_ERROR SendOfferCommand(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle,
                                uint16_t sessionId);

    CHIP_ERROR SendAnswerCommand(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle,
                                 uint16_t sessionId);

    CHIP_ERROR SendICECandidatesCommand(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle,
                                        uint16_t sessionId);

    CHIP_ERROR SendEndCommand(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle,
                              uint16_t sessionId, WebRTCEndReasonEnum endReason);

    CHIP_ERROR AcquireAudioVideoStreams(uint16_t sessionId);

    CHIP_ERROR ReleaseAudioVideoStreams(uint16_t sessionId);

    static void OnDeviceConnected(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                  const chip::SessionHandle & sessionHandle);

    static void OnDeviceConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    // WebRTC Callbacks
    void OnLocalDescription(const std::string & sdp, SDPType type, const uint16_t sessionId);
    void OnConnectionStateChanged(bool connected, const uint16_t sessionId);

    WebrtcTransport * GetTransport(uint16_t sessionId);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;

    std::unordered_map<uint16_t, std::unique_ptr<WebrtcTransport>> mWebrtcTransportMap;
    // This is to retrieve the sessionIds for a given ScopedNodeId (NodeId + FabricIndex)
    std::map<ScopedNodeId, uint16_t> mSessionIdMap;

    MediaController * mMediaController = nullptr;

    WebRTCTransportProviderCluster * mWebRTCTransportProvider = nullptr;

    // Handle to the Camera Device interface. For accessing other
    // clusters, if required.
    CameraDeviceInterface * mCameraDevice = nullptr;

    bool mSoftLiveStreamPrivacyEnabled = false;
};

} // namespace WebRTCTransportProvider
} // namespace Clusters
} // namespace app
} // namespace chip
