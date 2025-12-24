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

static size_t gSDPLength = CONFIG_MAX_LARGE_BUFFER_SIZE_BYTES;
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
    signaling_msg_t * message = new signaling_msg_t;
    message->version          = 0;
    message->messageType      = SIGNALING_MSG_TYPE_TRIGGER_OFFER;
    snprintf(message->correlationId, sizeof(message->correlationId), "%u", sessionId);
    std::string peerConnectionId = this->GetPeerConnectionId();
    snprintf(peerClientId, sizeof(peerClientId), "%s", peerConnectionId.c_str());
    memcpy(message->peerClientId, peerClientId, sizeof(peerClientId));
    message->payloadLen = 0;
    message->payload    = NULL;

    size_t serialized_len = 0;
    char * serialized_msg = serialize_signaling_message(message, &serialized_len);
    if (serialized_msg)
    {
        webrtc_bridge_send_message(serialized_msg, serialized_len);
    }

    delete message;
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
    char * sdp_json           = (char *) malloc(gSDPLength);
    signaling_msg_t * message = new signaling_msg_t;

    std::string escaped_sdp = json_escape(sdp);
    size_t json_len         = 0;
    if (type == SDPType::Offer)
    {
        json_len = sprintf(sdp_json, "{\"type\": \"offer\", \"sdp\": \"%s\"}", escaped_sdp.c_str());
        printf("OFFER: \n%s\n", sdp_json);
        message->messageType = SIGNALING_MSG_TYPE_OFFER;
    }
    else if (type == SDPType::Answer)
    {
        json_len = sprintf(sdp_json, "{\"type\": \"answer\", \"sdp\": \"%s\"}", escaped_sdp.c_str());
        printf("ANSWER: \n%s\n", sdp_json);
        message->messageType = SIGNALING_MSG_TYPE_ANSWER;
    }

    message->version             = 0;
    std::string peerConnectionId = this->GetPeerConnectionId();
    snprintf(peerClientId, sizeof(peerClientId), "%s", peerConnectionId.c_str());
    memcpy(message->peerClientId, peerClientId, sizeof(peerClientId));
    message->payloadLen = json_len;
    message->payload    = sdp_json;

    size_t serialized_len = 0;
    char * serialized_msg = serialize_signaling_message(message, &serialized_len);
    if (serialized_msg)
    {
        webrtc_bridge_send_message(serialized_msg, serialized_len);
    }

    printf("SDP LENGTH: %d\n", serialized_len);

    delete message;
    free(sdp_json);
}

void KVSWebRTCPeerConnection::AddRemoteCandidate(const std::string & candidate, const std::string & mid)
{

    // Send webrtc requestor's candidates to KVSWebRTCManager.
    char * candidate_json = (char *) malloc(1024);

    std::string escaped_sdp = json_escape(std::string(candidate.begin(), candidate.end()));
    size_t json_len         = sprintf(candidate_json, "{\"candidate\": \"%s\"}", escaped_sdp.c_str());

    printf("CANDIDATE: \n%s\n", candidate_json);

    signaling_msg_t * message    = new signaling_msg_t;
    message->version             = 0;
    message->messageType         = SIGNALING_MSG_TYPE_ICE_CANDIDATE;
    std::string peerConnectionId = this->GetPeerConnectionId();
    snprintf(peerClientId, sizeof(peerClientId), "%s", peerConnectionId.c_str());
    memcpy(message->peerClientId, peerClientId, sizeof(peerClientId));
    message->payloadLen = json_len;
    message->payload    = candidate_json;

    size_t serialized_len = 0;
    char * serialized_msg = serialize_signaling_message(message, &serialized_len);
    if (serialized_msg)
    {
        webrtc_bridge_send_message(serialized_msg, serialized_len);
    }

    printf("Candidate length: %d\n", serialized_len);

    delete message;
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
