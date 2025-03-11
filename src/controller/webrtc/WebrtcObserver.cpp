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

#include "WebrtcObserver.h"
#include "callbacks.h"

#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

#include "absl/memory/memory.h"
#include "absl/types/optional.h"
#include "api/stats/rtc_stats_collector_callback.h"
#include "api/stats/rtc_stats_report.h"
#include "rtc_base/strings/json.h"
#include "json/json.h"
#include <api/jsep.h>
#include <api/peer_connection_interface.h>
#include <api/rtc_error.h>
#include <rtc_base/logging.h>
#include <rtc_base/physical_socket_server.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/thread.h>

namespace {

const char kCandidateSdpMidName[]        = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[]           = "candidate";

} // namespace

void PCStatsCollector::OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport> & report)
{
    if (client->stats_cb)
        client->stats_cb((report->ToJson()).c_str(), client->client_id);
    else
        RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Stats callback not initialised";
}

WebrtcObserver::WebrtcObserver(RTCClient * Client) : client(Client)
{
    SetSDPObserver = new rtc::RefCountedObject<DummySetSessionDescriptionObserver>(Client->client_id);
}

void WebrtcObserver::OnSuccess(webrtc::SessionDescriptionInterface * desc)
{
    peer_connection->SetLocalDescription(SetSDPObserver.get(), desc);
    std::string sdp;
    desc->ToString(&sdp);
    webrtc::SdpType type = desc->GetType();
    if (type == webrtc::SdpType::kOffer)
    {
        if (client->offer_cb)
        {
            (client->offer_cb)(sdp.c_str(), client->client_id);
        }
        else
        {
            RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Offer callback not initialised";
        }
    }
    else if (type == webrtc::SdpType::kAnswer)
    {
        if (client->answer_cb)
        {
            (client->answer_cb)(sdp.c_str(), client->client_id);
        }
        else
        {
            RTC_LOG(LS_ERROR) << "[LIBWEBRTC] answer callback not initialised";
        }
    }
    else
    {
        RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Invalid SDP";
        if (client->error_cb)
            (client->error_cb)("Invalid SDP", client->client_id);
        else
            RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Error callback not initialised";
    }
}

void WebrtcObserver::OnFailure(webrtc::RTCError Error)
{
    if (client->error_cb)
        (client->error_cb)("error in SDP", client->client_id);
    else
        RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Error callback not initialised";
}

void WebrtcObserver::OnIceCandidate(const webrtc::IceCandidateInterface * candidate)
{

    RTC_LOG(LS_INFO) << "[LIBWEBRTC] Ice connection callback";
    Json::Value jmessage;

    jmessage[kCandidateSdpMidName]        = candidate->sdp_mid();
    jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
    std::string sdp;
    if (!candidate->ToString(&sdp))
    {
        RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Failed to serialize candidate";
        return;
    }
    jmessage[kCandidateSdpName] = sdp;

    Json::StreamWriterBuilder factory;

    std::string * msg = new std::string(Json::writeString(factory, jmessage));

    const char * cand_string = msg->c_str();

    std::string candidate_str;
    candidate->ToString(&candidate_str);

    if (client->candidate_count < 30)
        client->candidates[client->candidate_count] = cand_string;
    client->candidate_count++;
    RTC_LOG(LS_VERBOSE) << candidate_str;
}

void WebrtcObserver::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{
    RTC_LOG(LS_INFO) << "OnIceConnectionChange callback new state: " << new_state;
    if (new_state == webrtc::PeerConnectionInterface::kIceConnectionDisconnected)
    {
        if (client->peer_disconnected_cb)
            (client)->peer_disconnected_cb(1);
        else
            RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Peer disconnected callback not initialised";
    }
}

void WebrtcObserver::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
    RTC_LOG(LS_INFO) << "[LIBWEBRTC] OnIceGatheringChange callback new state: " << new_state;
    if (new_state == webrtc::PeerConnectionInterface::kIceGatheringComplete)
    {
        if (client->ice_cb)
        {
            client->ice_cb(client->candidates, client->client_id);
        }
        else
        {
            RTC_LOG(LS_ERROR) << "[LIBWEBRTC] ICE callback not initialised";
        }
    }
}

RTCClient::RTCClient(int id)
{
    webrtc_observer      = new rtc::RefCountedObject<WebrtcObserver>(this);
    offer_cb             = nullptr;
    answer_cb            = nullptr;
    ice_cb               = nullptr;
    error_cb             = nullptr;
    peer_connected_cb    = nullptr;
    peer_disconnected_cb = nullptr;
    stats_cb             = nullptr;
    client_id            = id;
    pc_stats_collector_  = new rtc::RefCountedObject<PCStatsCollector>(this);
    candidate_count      = 0;
}
