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

#include "pushav-prerollbuffer.h"
#include <algorithm>
#include <cstring>
#include <lib/support/logging/CHIPLogging.h>

PreRollBuffer::PreRollBuffer(size_t maxTotalBytes) : maxTotalBytes(maxTotalBytes) {}

void PreRollBuffer::PushFrameToBuffer(const std::string & streamKey, const char * data, size_t size)
{
    auto frame       = std::make_shared<PreRollFrame>();
    frame->streamKey = streamKey;
    frame->data      = std::make_unique<char[]>(size);
    memcpy(frame->data.get(), data, size);
    frame->size  = size;
    frame->ptsMs = NowMs();
    auto & queue = buffers[streamKey]; // Get or create the queue for this stream key
    queue.push_back(frame);
    contentBufferSize += size; // Track total bytes in buffer for all streams
    TrimBuffer();
    PushBufferToTransport(); // Automatically flush after each frame push
}

void PreRollBuffer::PushBufferToTransport()
{
    int64_t currentTime = NowMs();
    std::vector<BufferSink *> sinksToRemove;

    for (auto & [sink, streamKeys] : sinkSubscriptions)
    {
        if (!sink->transport)
        {
            sinksToRemove.push_back(sink);
            continue;
        }

        // if (sink->transport->GetTransportStatus() /*0:Active, 1:Inactive */)
        // {
        //     // Remove delivery records when transport is inactive
        //     for (auto & [_, bufferQueue] : buffers)
        //     {
        //         for (auto & frame : bufferQueue)
        //         {
        //             frame->deliveredTo.erase(sink);
        //         }
        //     }
        //     continue;
        // }

        int64_t minTimeToDeliver = (sink->requestedPreBufferLengthMs == 0 && sink->transport->CanSendVideo())
            ? currentTime - sink->minKeyframeIntervalMs
            : currentTime - sink->requestedPreBufferLengthMs;
        for (const std::string & streamKey : streamKeys)
        {
            auto it = buffers.find(streamKey);
            if (it == buffers.end())
            {
                // No frames for this stream key yet
                continue;
            }

            for (const auto & frame : it->second)
            {
                if (frame->ptsMs < minTimeToDeliver)
                {
                    continue;
                }
                if (frame->deliveredTo.find(sink) != frame->deliveredTo.end())
                {
                    continue;
                }
                else
                {
                    // Frame is not older than the requested prebuffer length and hasn't been delivered to this sink yet
                    if (streamKey[0] == 'a' && sink->transport->CanSendAudio())
                    {
                        sink->transport->SendAudio(frame->data.get(), frame->size,
                                                   static_cast<uint16_t>(std::stoi(streamKey.substr(1))));
                    }
                    else if (streamKey[0] == 'v' && sink->transport->CanSendVideo())
                    {
                        sink->transport->SendVideo(frame->data.get(), frame->size,
                                                   static_cast<uint16_t>(std::stoi(streamKey.substr(1))));
                    }
                    else
                    {
                        continue; // Cannot send
                    }
                    // Mark as delivered to this sink to avoid duplicate delivery
                    frame->deliveredTo.insert(sink);
                }
            }
        }
    }

    // Remove sinks with no valid senders
    for (BufferSink * sink : sinksToRemove)
    {
        DeregisterTransportFromBuffer(sink);
    }
}

void PreRollBuffer::RegisterTransportToBuffer(BufferSink * sink, const std::unordered_set<std::string> & streamKeys)
{
    sinkSubscriptions[sink] = streamKeys;
}

void PreRollBuffer::DeregisterTransportFromBuffer(BufferSink * sink)
{
    sinkSubscriptions.erase(sink);
}

void PreRollBuffer::TrimBuffer()
{
    while (contentBufferSize > maxTotalBytes)
    {
        std::shared_ptr<PreRollFrame> oldest = nullptr;
        std::string oldestStreamKey;

        // Find the oldest frame across all buffers
        for (auto & [streamKey, buffer] : buffers)
        {
            if (!buffer.empty())
            {
                auto & candidate = buffer.front();
                if (!oldest || candidate->ptsMs < oldest->ptsMs)
                {
                    oldest          = candidate;
                    oldestStreamKey = streamKey;
                }
            }
        }

        if (oldest)
        {
            // Remove it

            buffers[oldestStreamKey].pop_front();
            maxTotalBytes -= oldest->size;
        }
        else
        {
            break; // Nothing to remove
        }
    }
}

int64_t PreRollBuffer::NowMs() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
