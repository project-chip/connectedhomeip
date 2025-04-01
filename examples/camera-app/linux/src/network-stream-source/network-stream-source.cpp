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

#include "network-stream-source.h"
#include <arpa/inet.h>
#include <lib/support/logging/CHIPLogging.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr int kVideoBufferSize = 4096;

void NetworkStreamSource::Init(MediaController * aMediaController, uint16_t aSrcPort, StreamType aStreamType)
{
    mMediaController = aMediaController;
    mSrcPort         = aSrcPort;
    mStreamType      = aStreamType;
}

void NetworkStreamSource::Start(uint16_t aStreamId)
{
    mStreamId = aStreamId;

    ChipLogDetail(NotSpecified, "Network socket listener for media stream - start listening ");
    this->mStreamSourceActive = true;
    streamThreads.emplace_back([&]() { ListenForStreamOnSocket(); });
}

void NetworkStreamSource::Stop()
{

    this->mStreamSourceActive = false;

    for (auto & t : streamThreads)
    {
        t.join();
    }
    ChipLogDetail(NotSpecified, "Network socket listener for media stream - stopping listening ");
}

// Network Source (now handles both video and audio)
void NetworkStreamSource::ListenForStreamOnSocket()
{
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1)
    {
        ChipLogError(NotSpecified, "Failed to create socket.");
        return;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port        = htons(mSrcPort);

    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        close(serverSocket);
        ChipLogError(NotSpecified, "Failed to bind socket.");
        return;
    }

    ChipLogDetail(NotSpecified, "Listening on port %u", mSrcPort);

    char buffer[kVideoBufferSize];
    sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);

    while (mStreamSourceActive)
    {
        ssize_t bytesReceived =
            recvfrom(serverSocket, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressLength);
        if (bytesReceived == -1)
        {
            ChipLogError(NotSpecified, "Error receiving data.");
            continue;
        }

        if (mStreamType == StreamType::kVideo)
        {
            mMediaController->DistributeVideo(buffer, bytesReceived, mStreamId);
        }
        else if (mStreamType == StreamType::kAudio)
        {
            mMediaController->DistributeAudio(buffer, bytesReceived, mStreamId);
        }
    }

    close(serverSocket);
}
