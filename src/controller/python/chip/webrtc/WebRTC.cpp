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

#include "WebRTC.h"
#include <WebRTCClient.h>

#include <memory>
#include <thread>
#include <unistd.h>

#include <rtc/description.hpp>
#include <rtc/rtc.h>
#include <rtc/rtc.hpp>

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace webrtc {

static std::mutex g_mutex;
static std::map<WebRTCClientHandle, std::shared_ptr<WebRTCClient>> g_clients;

WebRTCClientHandle webrtc_client_create()
{
    auto client               = std::make_shared<WebRTCClient>();
    WebRTCClientHandle handle = reinterpret_cast<WebRTCClientHandle>(client.get());
    std::lock_guard<std::mutex> lock(g_mutex);
    g_clients[handle] = client;
    return handle;
}

void webrtc_client_destroy(WebRTCClientHandle handle)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_clients.erase(handle);
}

PyChipError webrtc_client_create_peer_connection(WebRTCClientHandle handle, const char * stun_url)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        CHIP_ERROR err = it->second->CreatePeerConnection(stun_url);
        return ToPyChipError(err);
    }

    return ToPyChipError(CHIP_NO_ERROR);
}

void webrtc_client_create_offer(WebRTCClientHandle handle)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        it->second->CreateOffer();
    }
}

void webrtc_client_create_answer(WebRTCClientHandle handle)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        it->second->CreateAnswer();
    }
}

void webrtc_client_set_remote_description(WebRTCClientHandle handle, const char * sdp, const char * type)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        it->second->SetRemoteDescription(sdp, type);
    }
}

void webrtc_client_add_ice_candidate(WebRTCClientHandle handle, const char * candidate, const char * mid)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        it->second->AddIceCandidate(candidate, mid);
    }
}

void webrtc_client_set_local_description_callback(WebRTCClientHandle handle, LocalDescriptionCallback cb, void * user_data)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        it->second->OnLocalDescription(
            [cb, user_data](const std::string & sdp, const std::string & type) { cb(sdp.c_str(), type.c_str(), user_data); });
    }
}

void webrtc_client_set_ice_candidate_callback(WebRTCClientHandle handle, IceCandidateCallback cb, void * user_data)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        it->second->OnIceCandidate([cb, user_data](const std::string & candidate, const std::string & mid) {
            cb(candidate.c_str(), mid.c_str(), user_data);
        });
    }
}

const char * webrtc_get_local_description(WebRTCClientHandle handle)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        return it->second->GetLocalDescription();
    }
    return "";
}

int webrtc_get_peer_connection_state(WebRTCClientHandle handle)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        return it->second->GetPeerConnectionState();
    }
    return -1;
}

void webrtc_client_set_gathering_complete_callback(WebRTCClientHandle handle, GatheringCompleteCallback cb)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        it->second->OnGatheringComplete(cb);
    }
}

void webrtc_client_set_state_change_callback(WebRTCClientHandle handle, OnStateChangeCallback cb)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        it->second->OnStateChange(cb);
    }
}

void webrtc_provider_client_init(WebRTCClientHandle handle, uint32_t nodeId, uint8_t fabricIndex, uint16_t endpoint)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        it->second->WebRTCProviderClientInit(nodeId, fabricIndex, endpoint);
    }
}

void webrtc_provider_client_init_commandsender_callbacks(WebRTCClientHandle handle,
                                                         OnCommandSenderResponseCallback onCommandSenderResponseCallback,
                                                         OnCommandSenderErrorCallback onCommandSenderErrorCallback,
                                                         OnCommandSenderDoneCallback onCommandSenderDoneCallback)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        it->second->WebRTCProviderClientInitCallbacks(onCommandSenderResponseCallback, onCommandSenderErrorCallback,
                                                      onCommandSenderDoneCallback);
    }
}

PyChipError webrtc_provider_client_send_command(WebRTCClientHandle handle, void * appContext, uint16_t endpointId,
                                                uint32_t clusterId, uint32_t commandId, const uint8_t * payload, size_t length)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_clients.find(handle);
    if (it != g_clients.end())
    {
        return it->second->SendCommand(appContext, endpointId, clusterId, commandId, payload, length);
    }
    return ToPyChipError(CHIP_ERROR_INTERNAL);
}

} // namespace webrtc
} // namespace chip
