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

#include <lib/core/CHIPError.h>
#include <string>

extern "C" {

namespace chip {
namespace webrtc {
typedef void * WebRTCClientHandle;

typedef void (*LocalDescriptionCallback)(const char * sdp, const char * type, void * user_data);
typedef void (*IceCandidateCallback)(const char * candidate, const char * mid, void * user_data);
typedef void (*GatheringCompleteCallback)();
typedef void (*OnStateChangeCallback)(int state);

/**
 * @brief Creates a new WebRTC client instance.
 *
 * Initializes a new WebRTC client and returns a handle to it.
 *
 * @return WebRTCClientHandle Opaque handle to the created client instance.
 */
WebRTCClientHandle webrtc_client_create();

/**
 * @brief Destroys a WebRTC client instance.
 *
 * Frees all resources associated with the client. Must be called after use.
 *
 * @param handle The client handle to destroy.
 */
void webrtc_client_destroy(WebRTCClientHandle handle);

/**
 * @brief Initializes a peer connection with the specified STUN server.
 *
 * Creates and configures a PeerConnection using the provided STUN server URL.
 *
 * @param handle The client handle.
 * @param stun_url URL of the STUN server (e.g., "stun:stun.l.google.com:19302").
 *
 * @return CHIP_ERROR on error, CHIP_NO_ERROR.
 */
CHIP_ERROR webrtc_client_create_peer_connection(WebRTCClientHandle handle, const char * stun_url);

/**
 * @brief Generates a local SDP offer for the peer connection.
 *
 * Creates an SDP offer and triggers the local description callback if set.
 *
 * @param handle The client handle.
 */
void webrtc_client_create_offer(WebRTCClientHandle handle);

/**
 * @brief Generates a local SDP answer to a received offer.
 *
 * Creates an SDP answer and triggers the local description callback if set.
 *
 * @param handle The client handle.
 */
void webrtc_client_create_answer(WebRTCClientHandle handle);

/**
 * @brief Sets the remote session description for the peer connection.
 *
 * Applies the provided SDP as the remote description (e.g., an answer to an offer).
 *
 * @param handle The client handle.
 * @param sdp The SDP string representing the remote description.
 * @param type Type of the SDP (e.g., "offer", "answer").
 */
void webrtc_client_set_remote_description(WebRTCClientHandle handle, const char * sdp, const char * type);

/**
 * @brief Adds a remote ICE candidate to the peer connection.
 *
 * Provides a candidate gathered from the remote peer to facilitate connectivity.
 *
 * @param handle The client handle.
 * @param candidate The ICE candidate string.
 * @param mid Media line identifier associated with the candidate.
 */
void webrtc_client_add_ice_candidate(WebRTCClientHandle handle, const char * candidate, const char * mid);

/**
 * @brief Sets the callback for local session description updates.
 *
 * Called when the local SDP (offer/answer) is generated or set.
 *
 * @param handle The client handle.
 * @param cb Callback function to invoke with the new SDP.
 * @param user_data User-defined data passed to the callback.
 */
void webrtc_client_set_local_description_callback(WebRTCClientHandle handle, LocalDescriptionCallback cb, void * user_data);

/**
 * @brief Sets the callback for ICE candidate events.
 *
 * Triggered when local ICE candidates are gathered.
 *
 * @param handle The client handle.
 * @param cb Callback function to receive ICE candidates.
 * @param user_data User-defined data for the callback.
 */
void webrtc_client_set_ice_candidate_callback(WebRTCClientHandle handle, IceCandidateCallback cb, void * user_data);

/**
 * @brief Retrieves the local description of the WebRTC client associated with the given handle.
 *
 * Called when latest sdp is required.
 *
 * @param handle The handle of the WebRTC client.
 * @return const char* A pointer to the local description string, or nullptr if the client is not found.
 */
const char * webrtc_get_local_description(WebRTCClientHandle handle);

/**
 * @brief Retrieves the current state of the peer connection for the WebRTC client associated with the given handle.
 *
 * Called when latest peer connection state is required.
 *
 * @param handle The handle of the WebRTC client.
 * @return int The state of the peer connection. Possible values are defined in the libdatachannel.
 * @note This function returns -1 if the client is not found.
 */
int webrtc_get_peer_connection_state(WebRTCClientHandle handle);

/**
 * @brief Sets a callback for when the gathering process for ICE candidates is complete.
 *
 *
 * @param handle The handle of the WebRTC client.
 * @param cb The callback function to be invoked when ice candidate gathering is complete.
 */
void webrtc_client_set_gathering_complete_callback(WebRTCClientHandle handle, GatheringCompleteCallback cb);

/**
 * @brief Sets a callback for when the state of the peer connection changes.
 *
 *
 * @param handle The handle of the WebRTC client.
 * @param cb The callback function to be invoked when the peer connection state changes.
 */
void webrtc_client_set_state_change_callback(WebRTCClientHandle handle, OnStateChangeCallback cb);

} // namespace webrtc
} // namespace chip

} // extern "C"
