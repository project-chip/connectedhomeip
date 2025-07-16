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
#include "pushav-prerollbuffer.h"
#include <algorithm>
#include <lib/support/logging/CHIPLogging.h>

void DefaultMediaController::RegisterTransport(Transport * transport, uint16_t videoStreamID, uint16_t audioStreamID)
{
    ChipLogProgress(Camera, "Registering transport: videoStreamID=%u, audioStreamID=%u", videoStreamID, audioStreamID);

    std::lock_guard<std::mutex> lock(mConnectionsMutex);
    mConnections.push_back({ transport, videoStreamID, audioStreamID });

    auto * bufferSink                      = new BufferSink();
    bufferSink->requestedPreBufferLengthMs = 0; // by default give only live stream
    bufferSink->sendAudio                  = [transport](const char * data, size_t size, const std::string & streamId) {
        uint16_t sid = static_cast<uint16_t>(std::stoi(streamId.substr(1))); // remove 'a'
        if (transport->CanSendAudio())
            transport->SendAudio(data, size, sid);
    };
    bufferSink->sendVideo = [transport](const char * data, size_t size, const std::string & streamId) {
        uint16_t sid = static_cast<uint16_t>(std::stoi(streamId.substr(1))); // remove 'v'
        if (transport->CanSendVideo())
            transport->SendVideo(data, size, sid);
    };

    std::unordered_set<std::string> streamKeys;
    streamKeys.insert("a" + std::to_string(audioStreamID));
    streamKeys.insert("v" + std::to_string(videoStreamID));

    preRollBuffer.RegisterTransportToBuffer(bufferSink, streamKeys);
    sinkMap[transport] = bufferSink;
    ChipLogProgress(Camera, "Transport registered successfully. Total connections: %u", (unsigned) connections.size());
}

void DefaultMediaController::UnregisterTransport(Transport * transport)
{
    std::lock_guard<std::mutex> lock(connectionsMutex);
    connections.erase(std::remove_if(connections.begin(), connections.end(),
                                     [transport](const Connection & c) { return c.transport == transport; }),
                      connections.end());
    auto it = sinkMap.find(transport);
    if (it != sinkMap.end())
    {
        preRollBuffer.DeregisterTransportFromBuffer(it->second);
        delete it->second;
        sinkMap.erase(it);
        ChipLogProgress(Camera, "Sink deregistered for transport.");
    }
}

void DefaultMediaController::DistributeVideo(const char * data, size_t size, uint16_t videoStreamID)
{
    // std::lock_guard<std::mutex> lock(connectionsMutex);
    // for (const Connection & connection : connections)
    // {
    //     if (connection.videoStreamID == videoStreamID && connection.transport && connection.transport->CanSendVideo())
    //     {
    //         connection.transport->SendVideo(data, size, videoStreamID);
    //     }
    // }
    std::string streamKey = "v" + std::to_string(videoStreamID);
    preRollBuffer.PushFrameToBuffer(streamKey, data, size);
}

void DefaultMediaController::DistributeAudio(const char * data, size_t size, uint16_t audioStreamID)
{
    // std::lock_guard<std::mutex> lock(connectionsMutex);
    // for (const Connection & connection : connections)
    // {
    //     if (connection.audioStreamID == audioStreamID && connection.transport && connection.transport->CanSendAudio())
    //     {
    //         connection.transport->SendAudio(data, size, audioStreamID);
    //     }
    // }
    std::string streamKey = "a" + std::to_string(audioStreamID);
    preRollBuffer.PushFrameToBuffer(streamKey, data, size);
}

void DefaultMediaController::SetPreRollLength(Transport * transport, uint16_t preRollBufferLength)

{
    auto it = sinkMap.find(transport);
    if (it != sinkMap.end() && it->second != nullptr)
    {
        it->second->requestedPreBufferLengthMs = std::min<int64_t>(preRollBufferLength, preRollBuffer.GetMaxPreBufferLengthMs());
        ChipLogProgress(Camera, "Delay updated for transport to %u ms", preRollBufferLength);
    }
    else
    {
        ChipLogError(Camera, "SetDelay: Transport not registered");
    }
}
