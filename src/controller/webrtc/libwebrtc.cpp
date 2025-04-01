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

#include "libwebrtc.h"
#include "callbacks.h"

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>

#include <rtc/description.hpp>
#include <rtc/rtc.h>
#include <rtc/rtc.hpp>

namespace chipwebrtc {

class RTCClient
{
public:
    int client_id;
    std::shared_ptr<rtc::PeerConnection> pc;
    rtc::Configuration config;
    SdpOfferCallback offer_cb;
    SdpAnswerCallback answer_cb;
    IceCallback ice_cb;
    ErrorCallback error_cb;
    PeerConnectedCallback peer_connected_cb;
    PeerDisconnectedCallback peer_disconnected_cb;
    StatsCollectedCallback stats_cb;

    RTCClient(int id) { client_id = id; }
};

// Function to read file content into a string
char * readFile(const char * filename)
{
    FILE * file = fopen(filename, "r");
    if (!file)
    {
        printf("Could not open %s\n", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char * content = (char *) malloc(size + 1);
    fread(content, 1, size, file);
    content[size] = '\0';
    fclose(file);
    return content;
}

// Function to read file content into a string
std::string readFileString(std::string filename)
{
    std::ifstream ifs("myfile.txt");
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    return content;
}

// Function to create a new WebRTC client
void * create_webrtc_client(int id)
{
    RTCClient * client = new RTCClient(id);
    return (void *) client;
}

// Function to initialize the peer connection
void initialise_peer_connection(void * Client)
{
    rtc::InitLogger(rtc::LogLevel::Verbose);

    RTCClient * client = static_cast<RTCClient *>(Client);

    // Add default ICE servers to the configuration
    client->config.iceServers.emplace_back("stun.l.google.com:19302");

    // Create a new peer connection
    client->pc = std::make_shared<rtc::PeerConnection>(client->config);

    // Set up event handlers for the peer connection
    client->pc->onLocalDescription([client](rtc::Description description) {
        std::cout << "Local Description (Paste this to the other peer):" << std::endl;
        std::cout << std::string(description) << std::endl;
        std::string desc_string = std::string(description);
        client->offer_cb(desc_string.c_str(), client->client_id);
    });

    client->pc->onLocalCandidate([client](rtc::Candidate candidate) {
        std::cout << "Local Candidate (Paste this to the other peer after the local description):" << std::endl;
        std::string cand_string = std::string(candidate);
        std::cout << cand_string << std::endl << std::endl;
        client->ice_cb(cand_string.c_str(), client->client_id);
    });

    client->pc->onStateChange([client](rtc::PeerConnection::State state) {
        std::cout << "[State: " << state << "]" << std::endl;
        if (state == rtc::PeerConnection::State::Connected)
        {
            if (client->peer_connected_cb)
            {
                client->peer_connected_cb(client->client_id);
            }
        }
        else if (state == rtc::PeerConnection::State::Disconnected)
        {
            if (client->peer_disconnected_cb)
            {
                client->peer_disconnected_cb(client->client_id);
            }
        }
    });

    client->pc->onGatheringStateChange(
        [](rtc::PeerConnection::GatheringState state) { std::cout << "[Gathering State: " << state << "]" << std::endl; });

    // Create a data channel for communication
    auto dc = client->pc->createDataChannel("test");

    // Set up event handlers for the data channel
    dc->onOpen([&]() { std::cout << "[DataChannel open: " << dc->label() << "]" << std::endl; });
    dc->onClosed([&]() { std::cout << "[DataChannel closed: " << dc->label() << "]" << std::endl; });
    dc->onMessage([](auto data) {
        if (std::holds_alternative<std::string>(data))
        {
            std::cout << "[Received: " << std::get<std::string>(data) << "]" << std::endl;
        }
    });
}

// Function to destroy a WebRTC client
void destroy_client(void * Client) {}

// API to close the peer connection and free up resources
void close_peer_connection(void * Client)
{
    if (Client == nullptr)
    {
        std::cout << "Client is null" << std::endl;
        return;
    }

    RTCClient * rtcClient = static_cast<RTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        std::cout << "PeerConnection is null" << std::endl;
        return;
    }

    rtcClient->pc->close();
    delete rtcClient;
}

// Function to get statistics from the peer connection
void GetStats(void * Client)
{
    if (Client == nullptr)
    {
        std::cout << "Client is null" << std::endl;
        return;
    }

    RTCClient * rtcClient = static_cast<RTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        std::cout << "PeerConnection is null" << std::endl;
        return;
    }

    // Retrieve statistics from the peer connection
    size_t bytesSent     = rtcClient->pc->bytesSent();
    size_t bytesReceived = rtcClient->pc->bytesReceived();

    // Print the data statistics
    std::cout << "Stats: Total bytes sent: " << bytesSent << ", Total bytes received: " << bytesReceived << std::endl;
}

// Function to create an offer for the peer connection
void CreateOffer(void * Client)
{
    if (Client == nullptr)
    {
        std::cout << "Client is null" << std::endl;
        return;
    }

    RTCClient * rtcClient = static_cast<RTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        std::cout << "PeerConnection is null" << std::endl;
        return;
    }

    // Create an offer for the peer connection
    rtc::Description description = rtcClient->pc->createOffer();

    // Call the offer callback with the offer description
    if (rtcClient->offer_cb)
    {
        rtcClient->offer_cb(description.typeString().c_str(), rtcClient->client_id);
    }
}

// Function to create an answer for the peer connection
void CreateAnswer(void * Client, const std::string & offer, std::function<void(std::string)> callback, int index) {}

// Function to get the local session description
const char * get_local_sdp(void * Client)
{
    if (Client == nullptr)
    {
        std::cout << "Client is null" << std::endl;
        return "";
    }

    RTCClient * rtcClient = static_cast<RTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        std::cout << "PeerConnection is null" << std::endl;
        return "";
    }

    // Return the local session description as a string
    return rtcClient->pc->localDescription()->typeString().c_str();
}

// Function to set the remote session description
void SetAnswer(void * Client, const std::string & answer) {}

// Function to set the remote candidate
void SetCandidate(void * Client, const std::string & candidate)
{
    if (Client == nullptr)
    {
        std::cout << "Client is null" << std::endl;
        return;
    }

    RTCClient * rtcClient = static_cast<RTCClient *>(Client);
    if (rtcClient->pc == nullptr)
    {
        std::cout << "PeerConnection is null" << std::endl;
        return;
    }

    // Add the remote candidate to the peer connection
    rtcClient->pc->addRemoteCandidate(rtc::Candidate(candidate));
}

// Function to send data over the data channel
void send_data(void * Client, const std::string & data) {}

// Function to set callbacks for various events
void set_callbacks(void * Client, SdpOfferCallback offer_callback, SdpAnswerCallback answer_callback, IceCallback ice_callback,
                   ErrorCallback error_callback, PeerConnectedCallback peer_connected_callback,
                   PeerDisconnectedCallback peer_disconnected_callback, StatsCollectedCallback stats_callback)
{
    RTCClient * client = static_cast<RTCClient *>(Client);

    // Set the callbacks for various events
    client->offer_cb             = offer_callback;
    client->answer_cb            = answer_callback;
    client->ice_cb               = ice_callback;
    client->error_cb             = error_callback;
    client->peer_connected_cb    = peer_connected_callback;
    client->peer_disconnected_cb = peer_disconnected_callback;
    client->stats_cb             = stats_callback;
}

} // namespace chipwebrtc
