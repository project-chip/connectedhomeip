#include "webrtc-kvs_esp_port.h"
#include <time.h>

#include "camera-device.h"
#include "jsmn.h"
#include "webrtc-transport.h"
#include <atomic>
#include <iomanip>
#include <webrtc_bridge.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportProvider;

using namespace Camera;
static size_t gSDPLength = CONFIG_MAX_LARGE_BUFFER_SIZE_BYTES;
extern CameraDevice gCameraDevice;

char peerClientId[SS_MAX_SIGNALING_CLIENT_ID_LEN + 1];

std::string json_unescape(const std::string &input) {
  std::string output;
  size_t i = 0;
  while (i < input.length()) {
    if (input[i] == '\\' && i + 1 < input.length()) {
      char next = input[i + 1];
      switch (next) {
      case '\"':
        output += '\"';
        i += 2;
        break;
      case '\\':
        output += '\\';
        i += 2;
        break;
      case 'b':
        output += '\b';
        i += 2;
        break;
      case 'f':
        output += '\f';
        i += 2;
        break;
      case 'n':
        output += '\n';
        i += 2;
        break;
      case 'r':
        output += '\r';
        i += 2;
        break;
      case 't':
        output += '\t';
        i += 2;
        break;
      case 'u':
        if (i + 5 < input.length()) {
          std::istringstream iss(input.substr(i + 2, 4));
          unsigned int code;
          if (iss >> std::hex >> code) {
            output += static_cast<char>(code); // For ASCII-range only
          }
          i += 6;
        } else {
          output += '?'; // malformed
          i += 2;
        }
        break;
      default:
        output += next;
        i += 2;
      }
    } else {
      output += input[i];
      ++i;
    }
  }
  return output;
}

std::string json_escape(const std::string &input) {
  std::string output;
  for (char c : input) {
    switch (c) {
    case '\"':
      output += "\\\"";
      break;
    case '\\':
      output += "\\\\";
      break;
    case '\b':
      output += "\\b";
      break;
    case '\f':
      output += "\\f";
      break;
    case '\n':
      output += "\\n";
      break;
    case '\r':
      output += "\\r";
      break;
    case '\t':
      output += "\\t";
      break;
    default:
      if (static_cast<unsigned char>(c) < 0x20) {
        char buf[7];
        snprintf(buf, sizeof(buf), "\\u%04x", c);
        output += buf;
      } else {
        output += c;
      }
    }
  }
  return output;
}

int extract_sdp(const char *json, char *sdp_buf, size_t sdp_buf_len) {
  jsmn_parser parser;
  jsmntok_t tokens[64];
  int ret;

  jsmn_init(&parser);
  ret = jsmn_parse(&parser, json, strlen(json), tokens,
                   sizeof(tokens) / sizeof(tokens[0]));
  if (ret < 0) {
    printf("Failed to parse JSON: %d\n", ret);
    return -1;
  }

  for (int i = 1; i < ret; i++) {
    if (tokens[i].type == JSMN_STRING &&
        strncmp(json + tokens[i].start, "sdp",
                tokens[i].end - tokens[i].start) == 0) {
      int len = tokens[i + 1].end - tokens[i + 1].start;
      if (len >= sdp_buf_len)
        len = sdp_buf_len - 1;
      strncpy(sdp_buf, json + tokens[i + 1].start, len);
      sdp_buf[len] = '\0';
      return 0;
    }
  }

  return -1; // SDP not found
}

int extract_candidate(const char *json, char *sdp_buf, size_t sdp_buf_len) {
  jsmn_parser parser;
  jsmntok_t tokens[64];
  int ret;

  jsmn_init(&parser);
  ret = jsmn_parse(&parser, json, strlen(json), tokens,
                   sizeof(tokens) / sizeof(tokens[0]));
  if (ret < 0) {
    printf("Failed to parse JSON: %d\n", ret);
    return -1;
  }

  for (int i = 1; i < ret; i++) {
    if (tokens[i].type == JSMN_STRING &&
        strncmp(json + tokens[i].start, "candidate",
                tokens[i].end - tokens[i].start) == 0) {
      int len = tokens[i + 1].end - tokens[i + 1].start;
      if (len >= sdp_buf_len)
        len = sdp_buf_len - 1;
      strncpy(sdp_buf, json + tokens[i + 1].start, len);
      sdp_buf[len] = '\0';
      return 0;
    }
  }

  return -1; // SDP not found
}

void webrtc_bridge_message_received_cb(void *data, int len) {
  // handle message
  printf("Received Message from P4-Streamer: \n%.*s\n", len, (char *)data);

  signaling_msg_t *msg = new signaling_msg_t;

  msg->payload = NULL;
  msg->payloadLen = 0;

  size_t serialized_len = 0;
  deserialize_signaling_message((const char *)data, len, msg);

  char sdp_buf[gSDPLength];
  switch (msg->messageType) {
  case SIGNALING_MSG_TYPE_OFFER:
    if (extract_sdp(msg->payload, sdp_buf, sizeof(sdp_buf)) == 0) {
      printf("Extracted SDP:\n%s\n", sdp_buf);
    }
    break;
  case SIGNALING_MSG_TYPE_ANSWER:
    if (extract_sdp(msg->payload, sdp_buf, sizeof(sdp_buf)) == 0) {
      printf("Extracted SDP:\n%s\n", sdp_buf);
    }
    break;
  case SIGNALING_MSG_TYPE_ICE_CANDIDATE:
    if (extract_candidate(msg->payload, sdp_buf, sizeof(sdp_buf)) == 0) {
      printf("Extracted Candidate:\n%s\n", sdp_buf);
    }
    break;
  default:
    printf("Unknown message type\n");
    goto cleanup;
    break;
  }

  {
    uint16_t sessionId = 0;
    snprintf(peerClientId, sizeof(peerClientId), "%s", msg->peerClientId);
    printf("Peer Client ID: \n%s\n", peerClientId);

    sessionId = static_cast<uint16_t>(
        strtoul(peerClientId, nullptr, 0)); // base 0 auto-detects "0x"

    printf("Session ID: %u\n", sessionId);

    std::string unescaped_msg = json_unescape(std::string(sdp_buf));

    printf("unescaped msg: \n%s\n", unescaped_msg.c_str());

    if (msg->messageType == SIGNALING_MSG_TYPE_OFFER) {
      WebRTCTransportProvider::Delegate &delegateRef =
          gCameraDevice.GetWebRTCProviderDelegate();
      auto *webrtcMgr = static_cast<WebRTCProviderManager *>(&delegateRef);
      if (webrtcMgr != nullptr) {
        WebrtcTransport *transport = webrtcMgr->GetTransport(sessionId);
        if (transport != nullptr) {
          transport->OnLocalDescription(unescaped_msg, SDPType::Offer);
          printf("Set SDP Offer to WebRTCProviderManager\n");
        } else {
          printf("Transport is not found for sessionID: %u\n", sessionId);
        }
      } else {
        printf("Delegate is not of type WebRTCProviderManager\n");
      }
    } else if (msg->messageType == SIGNALING_MSG_TYPE_ANSWER) {
      WebRTCTransportProvider::Delegate &delegateRef =
          gCameraDevice.GetWebRTCProviderDelegate();
      auto *webrtcMgr = static_cast<WebRTCProviderManager *>(&delegateRef);
      if (webrtcMgr != nullptr) {
        WebrtcTransport *transport = webrtcMgr->GetTransport(sessionId);
        if (transport != nullptr) {
          transport->OnLocalDescription(unescaped_msg, SDPType::Answer);
          printf("Set SDP Answer to WebRTCProviderManager\n");
        } else {
          printf("Transport is not found for sessionID: %u\n", sessionId);
        }
        printf("Set SDP Answer to WebRTCProviderManager\n");
      } else {
        printf("Delegate is not of type WebRTCProviderManager\n");
      }

    } else if (msg->messageType == SIGNALING_MSG_TYPE_ICE_CANDIDATE) {
      WebRTCTransportProvider::Delegate &delegateRef =
          gCameraDevice.GetWebRTCProviderDelegate();
      auto *webrtcMgr = static_cast<WebRTCProviderManager *>(&delegateRef);
      if (webrtcMgr != nullptr) {
        WebrtcTransport *transport = webrtcMgr->GetTransport(sessionId);
        if (transport != nullptr) {
          transport->OnICECandidate(unescaped_msg); // todo: session id based
          printf("Set Candidate to WebRTCProviderManager\n");
        } else {
          printf("Transport is not found for sessionID: %u\n", sessionId);
        }
        printf("Set Candidate to WebRTCProviderManager\n");
      } else {
        printf("Delegate is not of type WebRTCProviderManager\n");
      }
    }
  }

cleanup:
  if (msg->payload) {
    free(msg->payload);
  }
  delete msg;
}

static std::string generateRandomPeerConnectionId() {
  // Seed the random number generator
  srand((unsigned int)time(NULL));

  // Generate a random 16-bit number
  unsigned short randomId = (unsigned short)(rand() & 0xFFFF);

  // Format it into a hex string
  char buffer[8];
  snprintf(buffer, sizeof(buffer), "0x%04X", randomId);

  return std::string(buffer);
}

static std::atomic<unsigned int> peerConnectionCounter{0}; // Starts from 0

static std::string generateMonotonicPeerConnectionId() {
  // Atomically get the current ID and increment it
  unsigned int id =
      peerConnectionCounter.fetch_add(1, std::memory_order_relaxed);

  // Convert the ID to a zero-padded 8-digit hex string
  std::ostringstream oss;
  oss << "0x" << std::setw(8) << std::setfill('0') << std::hex << id;

  return oss.str();
}

KVSWebRTCPeerConnection::KVSWebRTCPeerConnection() {
  std::string peerConnectionId = generateMonotonicPeerConnectionId();
  mPeerConnection = std::make_shared<EspWebRTCPeerConnection>(peerConnectionId);
}

void KVSWebRTCPeerConnection::SetCallbacks(
    OnLocalDescriptionCallback onLocalDescription,
    OnICECandidateCallback onICECandidate,
    OnConnectionStateCallback onConnectionState, OnTrackCallback onTrack) {
  // Received messages are forwarded using webrtc_bridge_send_message
}

void KVSWebRTCPeerConnection::Close() {
  // KVSWebRTC close is handled by the KVSWebRTCManager.
}

void KVSWebRTCPeerConnection::CreateOffer(uint16_t sessionId) {
  // Set local description in KVSWebRTC
  // Answer is received via webrtc_bridge_message_received_cb
  signaling_msg_t *message = new signaling_msg_t;
  message->version = 0;
  message->messageType = SIGNALING_MSG_TYPE_TRIGGER_OFFER;
  snprintf(message->correlationId, sizeof(message->correlationId), "%u",
           sessionId);
  std::string peerConnectionId = this->GetPeerConnectionId();
  snprintf(peerClientId, sizeof(peerClientId), "%s", peerConnectionId.c_str());
  memcpy(message->peerClientId, peerClientId, sizeof(peerClientId));
  message->payloadLen = 0;
  message->payload = NULL;

  size_t serialized_len = 0;
  char *serialized_msg = serialize_signaling_message(message, &serialized_len);
  if (serialized_msg) {
    webrtc_bridge_send_message(serialized_msg, serialized_len);
  }

  delete message;
}

void KVSWebRTCPeerConnection::CreateAnswer() {
  // Answer is received from KVSWebRTCManager when offer is sent and received
  // via webrtc_bridge.
}

void KVSWebRTCPeerConnection::SetRemoteDescription(const std::string &sdp,
                                                   SDPType type) {
  // handles SDP Offer received from webrtc requestor.
  // Send SDP to KVSWebRTCManager.
  char *sdp_json = (char *)malloc(gSDPLength);
  signaling_msg_t *message = new signaling_msg_t;

  std::string escaped_sdp = json_escape(sdp);
  size_t json_len = 0;
  if (type == SDPType::Offer) {
    json_len = sprintf(sdp_json, "{\"type\": \"offer\", \"sdp\": \"%s\"}",
                       escaped_sdp.c_str());
    printf("OFFER: \n%s\n", sdp_json);
    message->messageType = SIGNALING_MSG_TYPE_OFFER;
  } else if (type == SDPType::Answer) {
    json_len = sprintf(sdp_json, "{\"type\": \"answer\", \"sdp\": \"%s\"}",
                       escaped_sdp.c_str());
    printf("ANSWER: \n%s\n", sdp_json);
    message->messageType = SIGNALING_MSG_TYPE_ANSWER;
  }

  message->version = 0;
  std::string peerConnectionId = this->GetPeerConnectionId();
  snprintf(peerClientId, sizeof(peerClientId), "%s", peerConnectionId.c_str());
  memcpy(message->peerClientId, peerClientId, sizeof(peerClientId));
  message->payloadLen = json_len;
  message->payload = sdp_json;

  size_t serialized_len = 0;
  char *serialized_msg = serialize_signaling_message(message, &serialized_len);
  if (serialized_msg) {
    webrtc_bridge_send_message(serialized_msg, serialized_len);
  }

  printf("SDP LENGTH: %d\n", serialized_len);

  delete message;
  free(sdp_json);
}

void KVSWebRTCPeerConnection::AddRemoteCandidate(const std::string &candidate,
                                                 const std::string &mid) {

  // Send webrtc requestor's candidates to KVSWebRTCManager.
  char *candidate_json = (char *)malloc(1024);

  std::string escaped_sdp =
      json_escape(std::string(candidate.begin(), candidate.end()));
  size_t json_len =
      sprintf(candidate_json, "{\"candidate\": \"%s\"}", escaped_sdp.c_str());

  printf("CANDIDATE: \n%s\n", candidate_json);

  signaling_msg_t *message = new signaling_msg_t;
  message->version = 0;
  message->messageType = SIGNALING_MSG_TYPE_ICE_CANDIDATE;
  std::string peerConnectionId = this->GetPeerConnectionId();
  snprintf(peerClientId, sizeof(peerClientId), "%s", peerConnectionId.c_str());
  memcpy(message->peerClientId, peerClientId, sizeof(peerClientId));
  message->payloadLen = json_len;
  message->payload = candidate_json;

  size_t serialized_len = 0;
  char *serialized_msg = serialize_signaling_message(message, &serialized_len);
  if (serialized_msg) {
    webrtc_bridge_send_message(serialized_msg, serialized_len);
  }

  printf("Candidate length: %d\n", serialized_len);

  delete message;
  free(candidate_json);
}

std::shared_ptr<WebRTCTrack>
KVSWebRTCPeerConnection::AddTrack(MediaType mediaType) {
  // Addition of tracks is handled by the KVSWebRTCManager.
  return nullptr;
}
std::string KVSWebRTCPeerConnection::GetPeerConnectionId() {
  return mPeerConnection->GetPeerConnectionId();
}

std::shared_ptr<WebRTCPeerConnection> CreateWebRTCPeerConnection() {
  return std::make_shared<KVSWebRTCPeerConnection>();
}
