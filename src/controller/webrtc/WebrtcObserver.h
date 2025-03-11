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

#include "callbacks.h"

#include <memory>
#include <string>

#include "api/create_peerconnection_factory.h"
#include "api/peer_connection_interface.h"
#include "api/stats/rtc_stats_collector_callback.h"
#include "api/stats/rtc_stats_report.h"
#include <rtc_base/logging.h>

struct RTCClient;

class DummySetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver
{
public:
    DummySetSessionDescriptionObserver() {}
    DummySetSessionDescriptionObserver(int peer_id) { callback_id = peer_id; }
    virtual void OnSuccess() { RTC_LOG(LS_INFO) << "[LIBWEBRTC] SetSessionDescription success, client id: " << callback_id; }
    virtual void OnFailure(webrtc::RTCError error)
    {
        RTC_LOG(LS_INFO) << "[LIBWEBRTC] SetSessionDescription failed, client id: " << callback_id
                         << ", error details: " << ToString(error.type()) << ": " << error.message();
    }
    int callback_id;
};

class PCStatsCollector : public webrtc::RTCStatsCollectorCallback
{
private:
    RTCClient * client;

public:
    void OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport> & report) override;

    PCStatsCollector(RTCClient * Client) : client(Client) {}
};

class WebrtcObserver : public webrtc::PeerConnectionObserver,
                       public webrtc::DataChannelObserver,
                       public webrtc::CreateSessionDescriptionObserver
{
public:
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection;
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel;
    rtc::scoped_refptr<DummySetSessionDescriptionObserver> SetSDPObserver;

    // PC Observer implementation

    void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {};

    void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {};

    void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {};

    void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override {

    };

    void OnRenegotiationNeeded() override { RTC_LOG(LS_INFO) << "[LIBWEBRTC] OnRenegotiationNeeded callback"; };

    void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;

    void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

    void OnIceCandidate(const webrtc::IceCandidateInterface * candidate) override;

    // DataObserver implementation

    void OnStateChange() override {};

    void OnMessage(const webrtc::DataBuffer & buffer) override {};

    void OnBufferedAmountChange(uint64_t previous_amount) override {};

    // CreateSessionDescription Observer implementation

    void OnSuccess(webrtc::SessionDescriptionInterface * desc) override;

    void OnFailure(webrtc::RTCError error) override;

    WebrtcObserver(RTCClient * Client);

private:
    RTCClient * client;
};

class RTCClient
{
public:
    int client_id;
    const char * candidates[30];
    int candidate_count;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory;
    webrtc::PeerConnectionInterface::RTCConfiguration configuration;
    rtc::scoped_refptr<PCStatsCollector> pc_stats_collector_;
    rtc::scoped_refptr<WebrtcObserver> webrtc_observer;
    SdpOfferCallback offer_cb;
    SdpAnswerCallback answer_cb;
    IceCallback ice_cb;
    ErrorCallback error_cb;
    PeerConnectedCallback peer_connected_cb;
    PeerDisconnectedCallback peer_disconnected_cb;
    StatsCollectedCallback stats_cb;
    std::unique_ptr<rtc::Thread> network_thread;
    std::unique_ptr<rtc::Thread> signaling_thread;
    std::unique_ptr<rtc::Thread> worker_thread;

    RTCClient(int id);
};
