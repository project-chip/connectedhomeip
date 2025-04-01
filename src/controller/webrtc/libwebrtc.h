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

#include <functional>
#include <string>

#include "callbacks.h"

namespace chipwebrtc {
/**
 * @brief
 *    This function creates a new WebRTC client with the specified ID.
 *
 * @param[in]  id
 *     The unique identifier for the WebRTC client.
 *
 * @return void *
 *     A pointer to the newly created WebRTC client.
 */
void * create_webrtc_client(int id);

/**
 * @brief
 *    This function destroys a WebRTC client.
 *
 * @param[in]  Client
 *     A pointer to the WebRTC client to be destroyed.
 *
 * @return void
 */
void destroy_client(void * Client);

/**
 * @brief
 *    This function initializes the peer connection for a WebRTC client.
 *
 * @param[in]  Client
 *     A pointer to the WebRTC client for which to initialize the peer connection.
 *
 * @return void
 */
void initialise_peer_connection(void * Client);

/**
 * @brief
 *    This function closes the peer connection and frees up resources for a WebRTC client.
 *
 * @param[in]  Client
 *     A pointer to the WebRTC client for which to close the peer connection.
 *
 * @return void
 */
void close_peer_connection(void * Client);

/**
 * @brief
 *    This function retrieves statistics from the peer connection of a WebRTC client.
 *
 * @param[in]  Client
 *     A pointer to the WebRTC client for which to retrieve statistics.
 *
 * @return void
 */
void GetStats(void * Client);

/**
 * @brief
 *    This function creates an offer for the peer connection of a WebRTC client.
 *
 * @param[in]  Client
 *     A pointer to the WebRTC client for which to create an offer.
 *
 * @return void
 */
void CreateOffer(void * Client);

/**
 * @brief
 *    This function creates an answer for the peer connection of a WebRTC client.
 *
 * @param[in]  Client
 *     A pointer to the WebRTC client for which to create an answer.
 *
 * @param[in]  offer
 *     The offer string received from the other peer.
 *
 * @param[in]  callback
 *     A callback function to be called with the answer string when it is ready.
 *
 * @param[in]  index
 *     An index parameter that can be used by the implementation (if needed).
 *
 * @return void
 */
void CreateAnswer(void * Client, const std::string & offer, std::function<void(std::string)> callback, int index);

/**
 * @brief
 *    This function retrieves the local session description string for a WebRTC client.
 *
 * @param[in]  Client
 *     A pointer to the WebRTC client for which to retrieve the local session description.
 *
 * @return const char *
 *     A pointer to the local session description string.
 */
const char * get_local_sdp(void * Client);

/**
 * @brief
 *    This function sets the remote session description for a WebRTC client.
 *
 * @param[in]  Client
 *     A pointer to the WebRTC client for which to set the remote session description.
 *
 * @param[in]  answer
 *     The answer string received from the other peer.
 *
 * @return void
 */
void SetAnswer(void * Client, const std::string & answer);

/**
 * @brief
 *    This function sets a remote candidate for a WebRTC client.
 *
 * @param[in]  Client
 *     A pointer to the WebRTC client for which to set the remote candidate.
 *
 * @param[in]  candidate
 *     The candidate string received from the other peer.
 *
 * @return void
 */
void SetCandidate(void * Client, const std::string & candidate);

/**
 * @brief
 *    This function sends data over the data channel of a WebRTC client.
 *
 * @param[in]  Client
 *     A pointer to the WebRTC client for which to send data.
 *
 * @param[in]  data
 *     The data string to be sent over the data channel.
 *
 * @return void
 */
void send_data(void * Client, const std::string & data);

/**
 * @brief
 *    This function sets various callbacks for a WebRTC client.
 *
 * @param[in]  Client
 *     A pointer to the WebRTC client for which to set callbacks.
 *
 * @param[in]  offer_callback
 *     A callback function to be called when an offer is received.
 *
 * @param[in]  answer_callback
 *     A callback function to be called when an answer is received.
 *
 * @param[in]  ice_callback
 *     A callback function to be called when an ICE candidate is received.
 *
 * @param[in]  error_callback
 *     A callback function to be called when an error occurs.
 *
 * @param[in]  peer_connected_callback
 *     A callback function to be called when a peer connection is established.
 *
 * @param[in]  peer_disconnected_callback
 *     A callback function to be called when a peer connection is disconnected.
 *
 * @param[in]  stats_callback
 *     A callback function to be called when statistics are collected.
 *
 * @return void
 */
void set_callbacks(void * Client, SdpOfferCallback offer_callback, SdpAnswerCallback answer_callback, IceCallback ice_callback,
                   ErrorCallback error_callback, PeerConnectedCallback peer_connected_callback,
                   PeerDisconnectedCallback peer_disconnected_callback, StatsCollectedCallback stats_callback);

} // namespace chipwebrtc
