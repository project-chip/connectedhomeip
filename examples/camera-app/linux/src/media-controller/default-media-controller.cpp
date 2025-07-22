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
#include "default-media-controller.h"
#include <algorithm>
#include <lib/support/logging/CHIPLogging.h>

void DefaultMediaController::RegisterTransport(Transport * transport, uint16_t videoStreamID, uint16_t audioStreamID)
{
    ChipLogProgress(Camera, "Registering transport: videoStreamID=%u, audioStreamID=%u", videoStreamID, audioStreamID);

    std::lock_guard<std::mutex> lock(mConnectionsMutex);
    mConnections.push_back({ transport, videoStreamID, audioStreamID });

    ChipLogProgress(Camera, "Transport registered successfully. Total connections: %u", (unsigned) mConnections.size());
}

void DefaultMediaController::UnregisterTransport(Transport * transport)
{
    std::lock_guard<std::mutex> lock(mConnectionsMutex);
    mConnections.erase(std::remove_if(mConnections.begin(), mConnections.end(),
                                      [transport](const Connection & c) { return c.transport == transport; }),
                       mConnections.end());
}

void DefaultMediaController::DistributeVideo(const char * data, size_t size, uint16_t videoStreamID)
{
    std::lock_guard<std::mutex> lock(mConnectionsMutex);
    for (const Connection & connection : mConnections)
    {
        if (connection.videoStreamID == videoStreamID && connection.transport && connection.transport->CanSendVideo())
        {
            connection.transport->SendVideo(data, size, videoStreamID);
        }
    }
}

void DefaultMediaController::DistributeAudio(const char * data, size_t size, uint16_t audioStreamID)
{
    std::lock_guard<std::mutex> lock(mConnectionsMutex);
    for (const Connection & connection : mConnections)
    {
        if (connection.audioStreamID == audioStreamID && connection.transport && connection.transport->CanSendAudio())
        {
            connection.transport->SendAudio(data, size, audioStreamID);
        }
    }
}
