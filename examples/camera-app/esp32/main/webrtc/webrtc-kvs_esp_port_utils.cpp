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
#include "camera-device.h"
#include "webrtc-provider-manager.h"
#include <iomanip>
#include <jsmn.h>
#include <signaling_serializer.h>
#include <sstream>
#include <string.h>
#include <webrtc-kvs_esp_port_utils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportProvider;

using namespace Camera;

static const char * TAG = "webrtc-kvs_esp_port_utils";

static char peerClientId[SS_MAX_SIGNALING_CLIENT_ID_LEN + 1];

extern CameraDevice gCameraDevice;

static std::atomic<unsigned int> peerConnectionCounter{ 0 }; // Starts from 0

static std::string json_unescape(const std::string & input)
{
    std::string output;
    size_t i = 0;
    while (i < input.length())
    {
        if (input[i] == '\\' && i + 1 < input.length())
        {
            char next = input[i + 1];
            switch (next)
            {
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
                if (i + 5 < input.length())
                {
                    std::istringstream iss(input.substr(i + 2, 4));
                    unsigned int code;
                    if (iss >> std::hex >> code)
                    {
                        output += static_cast<char>(code); // For ASCII-range only
                    }
                    i += 6;
                }
                else
                {
                    output += '?'; // malformed
                    i += 2;
                }
                break;
            default:
                output += next;
                i += 2;
            }
        }
        else
        {
            output += input[i];
            ++i;
        }
    }
    return output;
}

std::string json_escape(const std::string & input)
{
    std::string output;
    for (char c : input)
    {
        switch (c)
        {
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
            if (static_cast<unsigned char>(c) < 0x20)
            {
                char buf[7];
                snprintf(buf, sizeof(buf), "\\u%04x", c);
                output += buf;
            }
            else
            {
                output += c;
            }
        }
    }
    return output;
}

// Extracts the string value of the given top-level JSON key into out_buf (null-terminated, clamped to out_buf_len).
// Returns 0 on success, -1 if the input is invalid, the JSON cannot be parsed, or the key is not found.
static int extract_json_field(const char * json, const char * key, char * out_buf, size_t out_buf_len)
{
    if (json == nullptr || key == nullptr || out_buf == nullptr || out_buf_len == 0)
    {
        ChipLogError(Camera, "extract_json_field: invalid arguments for key '%s'", key ? key : "(null)");
        return -1;
    }

    jsmn_parser parser;
    jsmntok_t tokens[64];

    jsmn_init(&parser);
    int ret = jsmn_parse(&parser, json, strlen(json), tokens, sizeof(tokens) / sizeof(tokens[0]));
    if (ret < 0)
    {
        ChipLogError(Camera, "Failed to parse JSON: %d", ret);
        return -1;
    }

    size_t key_len = strlen(key);
    for (int i = 1; i + 1 < ret; i++)
    {
        if (tokens[i].type != JSMN_STRING)
        {
            continue;
        }
        size_t name_len = static_cast<size_t>(tokens[i].end - tokens[i].start);
        if (name_len != key_len || strncmp(json + tokens[i].start, key, key_len) != 0)
        {
            continue;
        }

        int tok_len = tokens[i + 1].end - tokens[i + 1].start;
        if (tok_len < 0)
        {
            return -1;
        }
        // Clamp to the destination buffer, leaving room for the null terminator.
        size_t copy_len = static_cast<size_t>(tok_len);
        if (copy_len > out_buf_len - 1)
        {
            copy_len = out_buf_len - 1;
        }
        memcpy(out_buf, json + tokens[i + 1].start, copy_len);
        out_buf[copy_len] = '\0';
        return 0;
    }

    return -1; // key not found
}

void webrtc_bridge_message_received_cb(const void * data, int len)
{
    // handle message
    ChipLogProgress(Camera, "Received Message from P4-Streamer: %.*s", len, (const char *) data);

    // Use nothrow to check for allocation failure
    std::unique_ptr<signaling_msg_t> msg(new (std::nothrow) signaling_msg_t());
    if (msg == nullptr)
    {
        ChipLogError(Camera, "webrtc_bridge_message_received_cb: failed to allocate signaling_msg_t");
        return;
    }

    deserialize_signaling_message((const char *) data, len, msg.get());

    if (msg->payload == nullptr || msg->payloadLen == 0)
    {
        ChipLogError(Camera, "webrtc_bridge_message_received_cb: message has no payload");
        return;
    }

    // The extracted SDP/candidate is a substring of the JSON payload, so the payload length
    // (+1 for the null terminator) is a safe upper bound for the output buffer.
    size_t sdp_buf_len = msg->payloadLen + 1;
    char * sdp_buf     = (char *) heap_caps_malloc_prefer(sdp_buf_len, 2, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM, MALLOC_CAP_INTERNAL);
    if (sdp_buf == nullptr)
    {
        ChipLogError(Camera, "webrtc_bridge_message_received_cb: failed to allocate sdp_buf");
        return;
    }

    switch (msg->messageType)
    {
    case SIGNALING_MSG_TYPE_OFFER:
    case SIGNALING_MSG_TYPE_ANSWER:
        if (extract_json_field(msg->payload, "sdp", sdp_buf, sdp_buf_len) == 0)
        {
            ESP_LOGD(TAG, "Extracted SDP:\n%s\n", sdp_buf);
        }
        break;
    case SIGNALING_MSG_TYPE_ICE_CANDIDATE:
        if (extract_json_field(msg->payload, "candidate", sdp_buf, sdp_buf_len) == 0)
        {
            ESP_LOGD(TAG, "Extracted Candidate:\n%s\n", sdp_buf);
        }
        break;
    default:
        ESP_LOGE(TAG, "Unknown message type\n");
        goto cleanup;
        break;
    }

    {
        uint16_t sessionId = 0;
        snprintf(peerClientId, sizeof(peerClientId), "%s", msg->peerClientId);
        ChipLogProgress(Camera, "Peer Client ID: %s", peerClientId);

        sessionId = static_cast<uint16_t>(strtoul(peerClientId, nullptr, 0)); // base 0 auto-detects "0x"

        ChipLogProgress(Camera, "Session ID: %u", sessionId);

        std::string unescaped_msg = json_unescape(std::string(sdp_buf));

        ESP_LOGD(TAG, "unescaped msg: \n%s\n", unescaped_msg.c_str());

        if (msg->messageType == SIGNALING_MSG_TYPE_OFFER)
        {
            WebRTCTransportProvider::Delegate & delegateRef = gCameraDevice.GetWebRTCProviderDelegate();
            auto * webrtcMgr                                = static_cast<WebRTCProviderManager *>(&delegateRef);
            if (webrtcMgr != nullptr)
            {
                WebrtcTransport * transport = webrtcMgr->GetTransport(sessionId);
                if (transport != nullptr)
                {
                    transport->OnLocalDescription(unescaped_msg, SDPType::Offer);
                    ChipLogProgress(Camera, "Set SDP Offer to WebRTCProviderManager");
                }
                else
                {
                    ChipLogError(Camera, "Transport is not found for sessionID: %u", sessionId);
                }
            }
            else
            {
                ChipLogError(Camera, "Delegate is not of type WebRTCProviderManager");
            }
        }
        else if (msg->messageType == SIGNALING_MSG_TYPE_ANSWER)
        {
            WebRTCTransportProvider::Delegate & delegateRef = gCameraDevice.GetWebRTCProviderDelegate();
            auto * webrtcMgr                                = static_cast<WebRTCProviderManager *>(&delegateRef);
            if (webrtcMgr != nullptr)
            {
                WebrtcTransport * transport = webrtcMgr->GetTransport(sessionId);
                if (transport != nullptr)
                {
                    transport->OnLocalDescription(unescaped_msg, SDPType::Answer);
                    ChipLogProgress(Camera, "Set SDP Answer to WebRTCProviderManager");
                }
                else
                {
                    ChipLogError(Camera, "Transport is not found for sessionID: %u", sessionId);
                }
            }
            else
            {
                ChipLogError(Camera, "Delegate is not of type WebRTCProviderManager");
            }
        }
        else if (msg->messageType == SIGNALING_MSG_TYPE_ICE_CANDIDATE)
        {
            WebRTCTransportProvider::Delegate & delegateRef = gCameraDevice.GetWebRTCProviderDelegate();
            auto * webrtcMgr                                = static_cast<WebRTCProviderManager *>(&delegateRef);
            if (webrtcMgr != nullptr)
            {
                WebrtcTransport * transport = webrtcMgr->GetTransport(sessionId);
                if (transport != nullptr)
                {
                    transport->OnICECandidate(unescaped_msg); // todo: session id based
                    ChipLogProgress(Camera, "Set Candidate to WebRTCProviderManager");
                }
                else
                {
                    ChipLogError(Camera, "Transport is not found for sessionID: %u", sessionId);
                }
            }
            else
            {
                ChipLogError(Camera, "Delegate is not of type WebRTCProviderManager");
            }
        }
    }

cleanup:
    if (msg->payload)
    {
        free(msg->payload);
    }
    if (sdp_buf)
    {
        heap_caps_free(sdp_buf);
    }
}

std::string generateMonotonicPeerConnectionId()
{
    // Atomically get the current ID and increment it
    unsigned int id = peerConnectionCounter.fetch_add(1, std::memory_order_relaxed);

    // Convert the ID to a zero-padded 8-digit hex string
    std::ostringstream oss;
    oss << "0x" << std::setw(8) << std::setfill('0') << std::hex << id;

    return oss.str();
}
