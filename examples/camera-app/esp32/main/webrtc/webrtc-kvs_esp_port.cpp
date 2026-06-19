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
#include "webrtc-kvs_esp_port.h"
#include "camera-device.h"
#include "webrtc-kvs_esp_port_utils.h"
#include "webrtc-transport.h"
#include <signaling_serializer.h>
#include <webrtc_bridge.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportProvider;

using namespace Camera;

static char peerClientId[SS_MAX_SIGNALING_CLIENT_ID_LEN + 1];

extern CameraDevice gCameraDevice;

KVSWebRTCPeerConnection::KVSWebRTCPeerConnection()
{
    std::string peerConnectionId = generateMonotonicPeerConnectionId();
    mPeerConnection              = std::make_shared<EspWebRTCPeerConnection>(peerConnectionId);
}

void KVSWebRTCPeerConnection::SetCallbacks(OnLocalDescriptionCallback onLocalDescription, OnICECandidateCallback onICECandidate,
                                           OnConnectionStateCallback onConnectionState, OnTrackCallback onTrack)
{
    // Received messages are forwarded using webrtc_bridge_send_message
}

void KVSWebRTCPeerConnection::Close()
{
    // KVSWebRTC close is handled by the KVSWebRTCManager.
}

void KVSWebRTCPeerConnection::CreateOffer(uint16_t sessionId)
{
    // Set local description in KVSWebRTC
    // Answer is received via webrtc_bridge_message_received_cb
    std::unique_ptr<signaling_msg_t> message(new (std::nothrow) signaling_msg_t());
    if (message == nullptr)
    {
        ChipLogError(Camera, "CreateOffer: failed to allocate signaling_msg_t");
        return;
    }
    message->version     = 0;
    message->messageType = SIGNALING_MSG_TYPE_TRIGGER_OFFER;
    snprintf(message->correlationId, sizeof(message->correlationId), "%u", sessionId);
    std::string peerConnectionId = this->GetPeerConnectionId();
    snprintf(peerClientId, sizeof(peerClientId), "%s", peerConnectionId.c_str());
    memcpy(message->peerClientId, peerClientId, sizeof(peerClientId));
    message->payloadLen = 0;
    message->payload    = NULL;

    size_t serialized_len = 0;
    char * serialized_msg = serialize_signaling_message(message.get(), &serialized_len);
    if (serialized_msg)
    {
        webrtc_bridge_send_message(serialized_msg, serialized_len);
    }
}

void KVSWebRTCPeerConnection::CreateAnswer()
{
    // Answer is received from KVSWebRTCManager when offer is sent and received
    // via webrtc_bridge.
}

void KVSWebRTCPeerConnection::SetRemoteDescription(const std::string & sdp, SDPType type)
{
    // handles SDP Offer received from webrtc requestor.
    // Send SDP to KVSWebRTCManager.
    std::unique_ptr<signaling_msg_t> message(new (std::nothrow) signaling_msg_t());
    if (message == nullptr)
    {
        ChipLogError(Camera, "SetRemoteDescription: failed to allocate signaling_msg_t");
        return;
    }

    const char * sdp_fmt = nullptr;
    if (type == SDPType::Offer)
    {
        sdp_fmt              = "{\"type\": \"offer\", \"sdp\": \"%s\"}";
        message->messageType = SIGNALING_MSG_TYPE_OFFER;
    }
    else if (type == SDPType::Answer)
    {
        sdp_fmt              = "{\"type\": \"answer\", \"sdp\": \"%s\"}";
        message->messageType = SIGNALING_MSG_TYPE_ANSWER;
    }
    else
    {
        ChipLogError(Camera, "SetRemoteDescription: unsupported SDP type");
        return;
    }

    std::string escaped_sdp = json_escape(sdp);

    // Compute the exact buffer size required for the formatted JSON (+1 for the null terminator).
    int needed = snprintf(nullptr, 0, sdp_fmt, escaped_sdp.c_str());
    if (needed < 0)
    {
        ChipLogError(Camera, "SetRemoteDescription: failed to compute sdp_json size");
        return;
    }
    size_t sdp_json_size = static_cast<size_t>(needed) + 1;

    char * sdp_json = (char *) heap_caps_malloc_prefer(sdp_json_size, 2, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM, MALLOC_CAP_INTERNAL);
    if (sdp_json == nullptr)
    {
        ChipLogError(Camera, "SetRemoteDescription: failed to allocate sdp_json");
        return;
    }

    size_t json_len = snprintf(sdp_json, sdp_json_size, sdp_fmt, escaped_sdp.c_str());
    ChipLogProgress(Camera, "SDP: \n%s\n", sdp_json);

    message->version             = 0;
    std::string peerConnectionId = this->GetPeerConnectionId();
    snprintf(peerClientId, sizeof(peerClientId), "%s", peerConnectionId.c_str());
    memcpy(message->peerClientId, peerClientId, sizeof(peerClientId));
    message->payloadLen = json_len;
    message->payload    = sdp_json;

    size_t serialized_len = 0;
    char * serialized_msg = serialize_signaling_message(message.get(), &serialized_len);
    if (serialized_msg)
    {
        webrtc_bridge_send_message(serialized_msg, serialized_len);
    }

    ChipLogProgress(Camera, "SDP LENGTH: %d", serialized_len);

    free(sdp_json);
}

void KVSWebRTCPeerConnection::AddRemoteCandidate(const std::string & candidate, const std::string & mid)
{

    // Send webrtc requestor's candidates to KVSWebRTCManager.
    static constexpr const char * kCandidateFmt = "{\"candidate\": \"%s\"}";
    std::string escaped_sdp                     = json_escape(std::string(candidate.begin(), candidate.end()));

    // Compute the exact buffer size required for the formatted JSON (+1 for the null terminator).
    int needed = snprintf(nullptr, 0, kCandidateFmt, escaped_sdp.c_str());
    if (needed < 0)
    {
        ChipLogError(Camera, "AddRemoteCandidate: failed to compute candidate_json size");
        return;
    }
    size_t candidate_json_size = static_cast<size_t>(needed) + 1;

    char * candidate_json = (char *) malloc(candidate_json_size);
    if (candidate_json == nullptr)
    {
        ChipLogError(Camera, "AddRemoteCandidate: failed to allocate candidate_json");
        return;
    }

    size_t json_len = snprintf(candidate_json, candidate_json_size, kCandidateFmt, escaped_sdp.c_str());

    ChipLogProgress(Camera, "CANDIDATE: \n%s\n", candidate_json);

    std::unique_ptr<signaling_msg_t> message(new (std::nothrow) signaling_msg_t());
    if (message == nullptr)
    {
        ChipLogError(Camera, "AddRemoteCandidate: failed to allocate signaling_msg_t");
        free(candidate_json);
        return;
    }
    message->version             = 0;
    message->messageType         = SIGNALING_MSG_TYPE_ICE_CANDIDATE;
    std::string peerConnectionId = this->GetPeerConnectionId();
    snprintf(peerClientId, sizeof(peerClientId), "%s", peerConnectionId.c_str());
    memcpy(message->peerClientId, peerClientId, sizeof(peerClientId));
    message->payloadLen = json_len;
    message->payload    = candidate_json;

    size_t serialized_len = 0;
    char * serialized_msg = serialize_signaling_message(message.get(), &serialized_len);
    if (serialized_msg)
    {
        webrtc_bridge_send_message(serialized_msg, serialized_len);
    }

    ChipLogProgress(Camera, "Candidate length: %d", serialized_len);

    free(candidate_json);
}

std::shared_ptr<WebRTCTrack> KVSWebRTCPeerConnection::AddTrack(MediaType mediaType)
{
    // Addition of tracks is handled by the KVSWebRTCManager.
    return nullptr;
}
std::string KVSWebRTCPeerConnection::GetPeerConnectionId()
{
    return mPeerConnection->GetPeerConnectionId();
}

std::shared_ptr<WebRTCPeerConnection> CreateWebRTCPeerConnection()
{
    return std::make_shared<KVSWebRTCPeerConnection>();
}
