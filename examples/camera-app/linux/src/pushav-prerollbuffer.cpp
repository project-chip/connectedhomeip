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

PreRollBuffer::PreRollBuffer(int64_t maxPreBufferLengthMs, size_t maxTotalBytes) :
    maxPreBufferLengthMs(maxPreBufferLengthMs), maxTotalBytes(maxTotalBytes)
{}

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
    bufferSizes[streamKey] += size; // Track total bytes in buffer for this stream key

    TrimBuffer(queue);
    PushBufferToTransport(); // Automatically flush after each frame push
}

void PreRollBuffer::PushBufferToTransport()
{
    int64_t currentTime  = NowMs();
    int64_t pushMarginMs = 500;
    std::vector<BufferSink *> sinksToRemove;

    for (const auto & [sink, streamKeys] : sinkSubscriptions)
    {
        if (!sink->sendAudio && !sink->sendVideo)
        {
            sinksToRemove.push_back(sink);
            continue;
        }

        // Calculated prerollbuffer that can be provided
        int64_t effectiveDelay = std::min(sink->requestedPreBufferLengthMs, maxPreBufferLengthMs);

        for (const std::string & streamKey : streamKeys)
        {
            auto it = buffers.find(streamKey);
            if (it == buffers.end())
            {
                // No frames for this stream key yet
                continue;
            }

	    int64_t targetTime = currentTime - effectiveDelay;
            for (const auto & frame : it->second)
            {
                if ((frame->ptsMs >= targetTime) && (frame->ptsMs < targetTime + pushMarginMs) &&
                    (frame->deliveredTo.find(sink) == frame->deliveredTo.end()))
                {
                    // Frame is not older than the requested prebuffer length and hasn't been delivered to this sink yet
                    if (streamKey[0] == 'a' && sink->sendAudio)
                    {
                        sink->sendAudio(frame->data.get(), frame->size, streamKey);
                    }
                    else if (streamKey[0] == 'v' && sink->sendVideo)
                    {
                        sink->sendVideo(frame->data.get(), frame->size, streamKey);
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

void PreRollBuffer::TrimBuffer(std::deque<std::shared_ptr<PreRollFrame>> & buffer)
{
    int64_t currentTime = NowMs();
    if (buffer.empty())
    {
        return;
    }

    size_t & size = bufferSizes[buffer.front()->streamKey];

    while (!buffer.empty())
    {
        auto & front = buffer.front();
        if ((currentTime - front->ptsMs > maxPreBufferLengthMs) || (size > maxTotalBytes))
        {
            size -= front->size;
            buffer.pop_front();
        }
        else
        {
            break;
        }
    }
}

int64_t PreRollBuffer::NowMs() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
