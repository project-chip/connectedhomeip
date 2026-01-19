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

static size_t gSDPLength       = CONFIG_MAX_LARGE_BUFFER_SIZE_BYTES;
static size_t gCandidateLength = 1024;
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
    char * sdp_json = (char *) malloc(gSDPLength);
    std::unique_ptr<signaling_msg_t> message(new (std::nothrow) signaling_msg_t());
    if (message == nullptr)
    {
        ChipLogError(Camera, "SetRemoteDescription: failed to allocate signaling_msg_t");
        return;
    }

    std::string escaped_sdp = json_escape(sdp);
    size_t json_len         = 0;
    if (type == SDPType::Offer)
    {
        json_len = sprintf(sdp_json, "{\"type\": \"offer\", \"sdp\": \"%s\"}", escaped_sdp.c_str());
        ChipLogProgress(Camera, "OFFER: \n%s\n", sdp_json);
        message->messageType = SIGNALING_MSG_TYPE_OFFER;
    }
    else if (type == SDPType::Answer)
    {
        json_len = sprintf(sdp_json, "{\"type\": \"answer\", \"sdp\": \"%s\"}", escaped_sdp.c_str());
        ChipLogProgress(Camera, "ANSWER: \n%s\n", sdp_json);
        message->messageType = SIGNALING_MSG_TYPE_ANSWER;
    }

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
    char * candidate_json = (char *) malloc(gCandidateLength);

    std::string escaped_sdp = json_escape(std::string(candidate.begin(), candidate.end()));
    size_t json_len         = sprintf(candidate_json, "{\"candidate\": \"%s\"}", escaped_sdp.c_str());

    ChipLogProgress(Camera, "CANDIDATE: \n%s\n", candidate_json);

    std::unique_ptr<signaling_msg_t> message(new (std::nothrow) signaling_msg_t());
    if (message == nullptr)
    {
        ChipLogError(Camera, "AddRemoteCandidate: failed to allocate signaling_msg_t");
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
