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
#include "Callbacks.h"

#include <memory>
#include <thread>
#include <unistd.h>

#include <rtc/description.hpp>
#include <rtc/rtc.h>
#include <rtc/rtc.hpp>

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace webrtc {

class WebRTCClient
{
public:
    int client_id;
    std::shared_ptr<rtc::PeerConnection> pc;
    std::shared_ptr<rtc::DataChannel> dc;
    rtc::Configuration config;
    SdpOfferCallback offerCb;
    SdpAnswerCallback answerCb;
    IceCallback iceCb;
    ErrorCallback errorCb;
    PeerConnectedCallback peerConnectedCb;
    PeerDisconnectedCallback peerDisconnectedCb;
    StatsCollectedCallback statsCb;

    WebRTCClient(int id) { client_id = id; }
};

// Function to create a new WebRTC client
void * CreateWebrtcClient(int id)
{
    WebRTCClient * client = new WebRTCClient(id);
    return (void *) client;
}

// Function to initialize the peer connection
void InitialisePeerConnection(void * Client)
{
    rtc::InitLogger(rtc::LogLevel::Verbose);

    WebRTCClient * client = static_cast<WebRTCClient *>(Client);

    // Create a new peer connection
    client->pc = std::make_shared<rtc::PeerConnection>(client->config);

    // Set up event handlers for the peer connection
    client->pc->onLocalDescription([client](rtc::Description description) {
        std::string desc_string = std::string(description);
        ChipLogProgress(NotSpecified, "Local Description (Paste this to the other peer): %s", desc_string.c_str());
        if (client->offerCb)
        {
            client->offerCb(desc_string.c_str(), client->client_id);
        }
    });

    client->pc->onLocalCandidate([client](rtc::Candidate candidate) {
        std::string cand_string = std::string(candidate);
        ChipLogProgress(NotSpecified, "Local Candidate (Paste this to the other peer after the local description): %s",
                        cand_string.c_str());
        if (client->iceCb)
        {
            client->iceCb(cand_string.c_str(), client->client_id);
        }
    });

    client->pc->onStateChange([client](rtc::PeerConnection::State state) {
        ChipLogProgress(NotSpecified, "[State: %u]", static_cast<unsigned>(state));
        if (state == rtc::PeerConnection::State::Connected)
        {
            if (client->peerConnectedCb)
            {
                client->peerConnectedCb(client->client_id);
            }
        }
        else if (state == rtc::PeerConnection::State::Disconnected)
        {
            if (client->peerDisconnectedCb)
            {
                client->peerDisconnectedCb(client->client_id);
            }
        }
    });

    client->pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
        ChipLogProgress(NotSpecified, "[Gathering State: %u]", static_cast<unsigned>(state));
    });

    // Create a data channel for communication
    client->dc = client->pc->createDataChannel("test");
    if (Client == nullptr)
    {
        ChipLogError(NotSpecified, "Client is null");
        return;
    }

    WebRTCClient * rtcClient = static_cast<WebRTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        ChipLogError(NotSpecified, "PeerConnection is null");
        return;
    }

    // Set up event handlers for the data channel
    client->dc->onOpen([&]() { ChipLogProgress(NotSpecified, "[DataChannel open: %s]", client->dc->label().c_str()); });
    client->dc->onClosed([&]() { ChipLogProgress(NotSpecified, "[DataChannel closed: %s]", client->dc->label().c_str()); });
    client->dc->onMessage([](auto data) {
        if (std::holds_alternative<std::string>(data))
        {
            ChipLogProgress(NotSpecified, "[Received message: %s]", std::get<std::string>(data).c_str());
        }
    });
}

// API to destroy WebRTC client
void DestroyClient(void * Client)
{
    if (Client == nullptr)
    {
        ChipLogError(NotSpecified, "Client is null");
        return;
    }

    WebRTCClient * rtcClient = static_cast<WebRTCClient *>(Client);
    delete rtcClient;
}

// API to close the peer connection and free up resources
void ClosePeerConnection(void * Client)
{
    if (Client == nullptr)
    {
        ChipLogError(NotSpecified, "Client is null");
        return;
    }

    WebRTCClient * rtcClient = static_cast<WebRTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        ChipLogError(NotSpecified, "PeerConnection is null");
        return;
    }

    rtcClient->pc->close();
}

// Function to get statistics from the peer connection
void GetStats(void * Client)
{
    if (Client == nullptr)
    {
        ChipLogError(NotSpecified, "Client is null");
        return;
    }

    WebRTCClient * rtcClient = static_cast<WebRTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        ChipLogError(NotSpecified, "PeerConnection is null");
        return;
    }

    // Retrieve statistics from the peer connection
    size_t bytesSent     = rtcClient->pc->bytesSent();
    size_t bytesReceived = rtcClient->pc->bytesReceived();

    // Print the data statistics
    ChipLogProgress(NotSpecified, "Stats: Total bytes sent: %lu Total bytes received: %lu", bytesSent, bytesReceived);
}

// Function to create an offer for the peer connection
void CreateOffer(void * Client)
{
    if (Client == nullptr)
    {
        ChipLogError(NotSpecified, "Client is null");
        return;
    }

    WebRTCClient * rtcClient = static_cast<WebRTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        ChipLogError(NotSpecified, "PeerConnection is null");
        return;
    }

    // Create an offer for the peer connection
    rtc::Description description = rtcClient->pc->createOffer();

    // Call the offer callback with the offer description
    if (rtcClient->offerCb)
    {
        std::string sdpOffer = description.typeString();
        rtcClient->offerCb(sdpOffer.c_str(), rtcClient->client_id);
    }
}

// Function to create an answer for the peer connection
void CreateAnswer(void * Client, const std::string & offer, std::function<void(std::string)> callback, int index)
{
    if (Client == nullptr)
    {
        ChipLogError(NotSpecified, "Client is null");
        return;
    }

    WebRTCClient * rtcClient = static_cast<WebRTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        ChipLogError(NotSpecified, "PeerConnection is null");
        return;
    }

    // Set the remote offer
    rtcClient->pc->setRemoteDescription(rtc::Description(offer, "offer"));

    // Create an answer for the peer connection
    rtc::Description answer = rtcClient->pc->createAnswer();

    // Call the provided callback with the answer description
    rtcClient->answerCb(answer.typeString().c_str(), rtcClient->client_id);
}

// Function to get the local session description
const char * GetLocalSdp(void * Client)
{
    if (Client == nullptr)
    {
        ChipLogError(NotSpecified, "Client is null");
        return "";
    }

    WebRTCClient * rtcClient = static_cast<WebRTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        ChipLogError(NotSpecified, "PeerConnection is null");
        return "";
    }

    // Return the local session description as a string
    return rtcClient->pc->localDescription()->typeString().c_str();
}

// Function to set the remote session description
void SetAnswer(void * Client, const std::string & answer)
{
    if (Client == nullptr)
    {
        ChipLogError(NotSpecified, "Client is null");
        return;
    }

    WebRTCClient * rtcClient = static_cast<WebRTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        ChipLogError(NotSpecified, "PeerConnection is null");
        return;
    }

    // Create a remote description from the answer string
    rtc::Description remoteDescription(answer, "answer");

    // Set the remote description on the PeerConnection
    rtcClient->pc->setRemoteDescription(remoteDescription);

    ChipLogProgress(NotSpecified, "Remote description set successfully");
}

// Function to set the remote candidate
void SetCandidate(void * Client, const std::string & candidate)
{
    if (Client == nullptr)
    {
        ChipLogError(NotSpecified, "Client is null");
        return;
    }

    WebRTCClient * rtcClient = static_cast<WebRTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        ChipLogError(NotSpecified, "PeerConnection is null");
        return;
    }

    // Add the remote candidate to the peer connection
    rtcClient->pc->addRemoteCandidate(rtc::Candidate(candidate));
}

// Function to send data over the data channel
void SendData(void * Client, const std::string & data)
{
    if (Client == nullptr)
    {
        ChipLogError(NotSpecified, "Client is null");
        return;
    }

    WebRTCClient * rtcClient = static_cast<WebRTCClient *>(Client);
    if (rtcClient->dc == nullptr)
    {
        ChipLogError(NotSpecified, "Datachannel is null");
        return;
    }

    rtcClient->dc->send(data);
}

// Function to set callbacks for various events
void SetCallbacks(void * Client, SdpOfferCallback offer_callback, SdpAnswerCallback answer_callback, IceCallback ice_callback,
                  ErrorCallback error_callback, PeerConnectedCallback peer_connected_callback,
                  PeerDisconnectedCallback peer_disconnected_callback, StatsCollectedCallback stats_callback)
{
    WebRTCClient * client = static_cast<WebRTCClient *>(Client);

    // Set the callbacks for various events
    client->offerCb            = offer_callback;
    client->answerCb           = answer_callback;
    client->iceCb              = ice_callback;
    client->errorCb            = error_callback;
    client->peerConnectedCb    = peer_connected_callback;
    client->peerDisconnectedCb = peer_disconnected_callback;
    client->statsCb            = stats_callback;
}

} // namespace webrtc
} // namespace chip
