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

void * pychip_webrtc_CreateWebrtcClient(int id)
{
    return chip::webrtc::CreateWebrtcClient(id);
}

void pychip_webrtc_InitialiseConnection(void * client)
{
    return chip::webrtc::InitialisePeerConnection(client);
}

void pychip_webrtc_CloseConnection(void * client)
{
    return chip::webrtc::ClosePeerConnection(client);
}

void pychip_webrtc_DestroyClient(void * client)
{
    return chip::webrtc::DestroyClient(client);
}

void pychip_webrtc_GetStats(void * client)
{
    return chip::webrtc::GetStats(client);
}

void pychip_webrtc_CreateOffer(void * client)
{
    chip::webrtc::CreateOffer(client);
}

const char * pychip_webrtc_GetLocalSDP(void * client)
{
    return chip::webrtc::GetLocalSdp(client);
}

void pychip_webrtc_SetAnswer(void * client, const char * answer)
{
    chip::webrtc::SetAnswer(client, answer);
}

void pychip_webrtc_SetCandidate(void * client, const char * candidate)
{
    chip::webrtc::SetCandidate(client, candidate);
}

void pychip_webrtc_SetCallbacks(void * client, SdpOfferCallback offer_cb, SdpAnswerCallback answer_cb, IceCallback ice_cb,
                                ErrorCallback error_cb, PeerConnectedCallback peer_connected_cb,
                                PeerDisconnectedCallback peer_disconnected_cb, StatsCollectedCallback stats_callback)
{
    chip::webrtc::SetCallbacks(client, offer_cb, answer_cb, ice_cb, error_cb, peer_connected_cb, peer_disconnected_cb,
                               stats_callback);
}
}
