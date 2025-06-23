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

#include <string>

#include "Callbacks.h"

extern "C" {

namespace chip {
namespace webrtc {
typedef void * WebRTCClientHandle;

typedef void (*LocalDescriptionCallback)(const char * sdp, const char * type, void * user_data);
typedef void (*IceCandidateCallback)(const char * candidate, const char * mid, void * user_data);

WebRTCClientHandle webrtc_client_create();

void webrtc_client_destroy(WebRTCClientHandle handle);

void webrtc_client_create_peer_connection(WebRTCClientHandle handle, const char * stun_url);

void webrtc_client_create_offer(WebRTCClientHandle handle);

void webrtc_client_create_answer(WebRTCClientHandle handle);

void webrtc_client_set_remote_description(WebRTCClientHandle handle, const char * sdp, const char * type);

void webrtc_client_add_ice_candidate(WebRTCClientHandle handle, const char * candidate, const char * mid);

void webrtc_client_set_local_description_callback(WebRTCClientHandle handle, LocalDescriptionCallback cb, void * user_data);

void webrtc_client_set_ice_candidate_callback(WebRTCClientHandle handle, IceCandidateCallback cb, void * user_data);

} // namespace webrtc
} // namespace chip

} // extern "C"
