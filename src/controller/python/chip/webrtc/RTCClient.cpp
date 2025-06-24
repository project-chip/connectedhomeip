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

#include <controller/webrtc/WebRTC.h>

using namespace chip;
using namespace chip::webrtc;

extern "C" {

void * pychip_webrtc_client_create()
{
    return chip::webrtc::webrtc_client_create();
}

void pychip_webrtc_client_destroy(void * client)
{
    return chip::webrtc::webrtc_client_destroy(client);
}

void pychip_webrtc_client_create_peer_connection(void * client, char * stun_url)
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
}
