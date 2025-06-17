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

    std::lock_guard<std::mutex> lock(connectionsMutex);
    connections.push_back({ transport, videoStreamID, audioStreamID });

    ChipLogProgress(Camera, "Transport registered successfully. Total connections: %u", (unsigned) connections.size());
}

void DefaultMediaController::UnregisterTransport(Transport * transport)
{
    std::lock_guard<std::mutex> lock(connectionsMutex);
    connections.erase(std::remove_if(connections.begin(), connections.end(),
                                     [transport](const Connection & c) { return c.transport == transport; }),
                      connections.end());
}

void DefaultMediaController::DistributeVideo(const char * data, size_t size, uint16_t videoStreamID)
{
    std::lock_guard<std::mutex> lock(connectionsMutex);
    for (const Connection & connection : connections)
    {
        if (connection.videoStreamID == videoStreamID && connection.transport && connection.transport->CanSendVideo())
        {
            connection.transport->SendVideo(data, size, videoStreamID);
        }
    }
}

void DefaultMediaController::DistributeAudio(const char * data, size_t size, uint16_t audioStreamID)
{
    std::lock_guard<std::mutex> lock(connectionsMutex);
    for (const Connection & connection : connections)
    {
        if (connection.audioStreamID == audioStreamID && connection.transport && connection.transport->CanSendAudio())
        {
            connection.transport->SendAudio(data, size, audioStreamID);
        }
    }
}
