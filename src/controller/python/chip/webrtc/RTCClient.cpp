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

#include <controller/python/chip/webrtc/WebRTC.h>
#include <lib/core/CHIPError.h>

using namespace chip;
using namespace chip::webrtc;

extern "C" {

void * pychip_webrtc_client_create()
{
    return chip::webrtc::webrtc_client_create();
}

void pychip_webrtc_client_destroy(void * client)
{
    chip::webrtc::webrtc_client_destroy(client);
}

PyChipError pychip_webrtc_client_create_peer_connection(void * client, char * stun_url)
{
    return chip::webrtc::webrtc_client_create_peer_connection(client, stun_url);
}

void pychip_webrtc_client_create_offer(void * client)
{
    chip::webrtc::webrtc_client_create_offer(client);
}

void pychip_webrtc_client_create_answer(void * client)
{
    chip::webrtc::webrtc_client_create_answer(client);
}

void pychip_webrtc_client_set_remote_description(void * client, char * sdp, char * type)
{
    chip::webrtc::webrtc_client_set_remote_description(client, sdp, type);
}

void pychip_webrtc_client_add_ice_candidate(void * client, char * candidate, char * mid)
{
    chip::webrtc::webrtc_client_add_ice_candidate(client, candidate, mid);
}

void pychip_webrtc_client_set_local_description_callback(void * client, LocalDescriptionCallback cb, void * user_data)
{
    chip::webrtc::webrtc_client_set_local_description_callback(client, cb, user_data);
}

void pychip_webrtc_client_set_ice_candidate_callback(void * client, IceCandidateCallback cb, void * user_data)
{
    chip::webrtc::webrtc_client_set_ice_candidate_callback(client, cb, user_data);
}

const char * pychip_webrtc_get_local_description(WebRTCClientHandle handle)
{
    return chip::webrtc::webrtc_get_local_description(handle);
}

int pychip_webrtc_get_peer_connection_state(WebRTCClientHandle handle)
{
    return chip::webrtc::webrtc_get_peer_connection_state(handle);
}

void pychip_webrtc_client_set_gathering_complete_callback(WebRTCClientHandle handle, GatheringCompleteCallback cb)
{
    chip::webrtc::webrtc_client_set_gathering_complete_callback(handle, cb);
}

void pychip_webrtc_client_set_state_change_callback(WebRTCClientHandle handle, OnStateChangeCallback cb)
{
    chip::webrtc::webrtc_client_set_state_change_callback(handle, cb);
}

void pychip_webrtc_provider_client_init(WebRTCClientHandle handle, uint32_t nodeId, uint8_t fabricIndex, uint16_t endpoint)
{
    chip::webrtc::webrtc_provider_client_init(handle, nodeId, fabricIndex, endpoint);
}

void pychip_webrtc_provider_client_init_commandsender_callbacks(WebRTCClientHandle handle,
                                                                OnCommandSenderResponseCallback onCommandSenderResponseCallback,
                                                                OnCommandSenderErrorCallback onCommandSenderErrorCallback,
                                                                OnCommandSenderDoneCallback onCommandSenderDoneCallback)
{
    chip::webrtc::webrtc_provider_client_init_commandsender_callbacks(handle, onCommandSenderResponseCallback,
                                                                      onCommandSenderErrorCallback, onCommandSenderDoneCallback);
}

PyChipError pychip_webrtc_provider_client_send_command(WebRTCClientHandle handle, void * appContext, uint16_t endpointId,
                                                       uint32_t clusterId, uint32_t commandId, const uint8_t * payload,
                                                       size_t length)
{
    return chip::webrtc::webrtc_provider_client_send_command(handle, appContext, endpointId, clusterId, commandId, payload, length);
}
}
