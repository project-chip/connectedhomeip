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

PreRollBuffer::PreRollBuffer() : mMaxTotalBytes(4096), mContentBufferSize(0) {}

void PreRollBuffer::SetMaxTotalBytes(size_t size)
{
    ChipLogProgress(Camera, "Setting max total bytes to %zu", size);
    mMaxTotalBytes = size;
    TrimBuffer();
}
void PreRollBuffer::PushFrameToBuffer(const std::string & streamKey, const uint8_t * data, size_t size, int64_t timestampMs)
{
    TrimBuffer();
    {
        std::lock_guard<std::mutex> lock(mBufferMutex);
        auto frame       = std::make_shared<PreRollFrame>();
        frame->streamKey = streamKey;
        frame->data      = std::make_unique<uint8_t[]>(size);
        memcpy(frame->data.get(), data, size);
        frame->size  = size;
        frame->ptsMs = timestampMs;
        auto & queue = mBuffers[streamKey]; // Get or create the queue for this stream key
        queue.push_back(frame);
        mContentBufferSize += size; // Track total bytes in buffer for all streams
    }                               // lock_guard released here
    PushBufferToTransport();        // Automatically flush after each frame push
}

void PreRollBuffer::PushBufferToTransport()
{
    std::lock_guard<std::mutex> lock(mBufferMutex);
    std::vector<BufferSink *> sinksToRemove;

    for (auto & [sink, streamKeys] : mSinkSubscriptions)
    {
        if (!sink->transport)
        {
            sinksToRemove.push_back(sink);
            continue;
        }

        // Determine the cutoff time for frame delivery.
        // This cutoff only matters for the INITIAL delivery when a sink is first registered,
        // to decide which buffered frames to send. Once hasDeliveredFirstFrame is true,
        // we deliver all new frames as they arrive (duplicate detection via deliveredTo handles the rest).
        int64_t minTimeToDeliver;
        if (!sink->hasDeliveredFirstFrame)
        {
            // For new sinks, deliver frames from registration time minus the pre-buffer length
            // This ensures frames aren't filtered out if the track takes time to become ready
            minTimeToDeliver = (sink->requestedPreBufferLengthMs == 0)
                ? sink->registrationTimeMs - sink->minKeyframeIntervalMs
                : sink->registrationTimeMs - sink->requestedPreBufferLengthMs;
        }
        else
        {
            // After first frame delivered, accept all frames (deliveredTo set prevents duplicates)
            // Setting to 0 effectively disables the timestamp filter
            minTimeToDeliver = 0;
        }

        for (const std::string & streamKey : streamKeys)
        {
            auto it = mBuffers.find(streamKey);
            if (it == mBuffers.end())
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
                    //  Frame is not older than the requested prebuffer length and hasn't been delivered to this sink yet
                    chip::ByteSpan data(frame->data.get(), frame->size);
                    if (streamKey[0] == 'a' && sink->transport->CanSendAudio())
                    {
                        sink->transport->SendAudio(data, frame->ptsMs, static_cast<uint16_t>(std::stoi(streamKey.substr(1))));
                    }
                    else if (streamKey[0] == 'v' && sink->transport->CanSendVideo())
                    {
                        sink->transport->SendVideo(data, frame->ptsMs, static_cast<uint16_t>(std::stoi(streamKey.substr(1))));
                    }
                    else
                    {
                        continue; // Cannot send or unknown stream key prefix
                    }
                    // Mark as delivered to this sink to avoid duplicate delivery
                    frame->deliveredTo.insert(sink);
                    // Mark that we've successfully delivered at least one frame to this sink
                    sink->hasDeliveredFirstFrame = true;
                }
            }
        }
    }
    // Remove sinks with no valid transport (already under lock)
    for (BufferSink * sink : sinksToRemove)
    {
        ChipLogProgress(Camera, "Removing transport from buffer %p (no valid transport)", sink);
        mSinkSubscriptions.erase(sink);
    }
}

void PreRollBuffer::RegisterTransportToBuffer(BufferSink * sink, const std::unordered_set<std::string> & streamKeys)
{
    std::lock_guard<std::mutex> lock(mBufferMutex);
    ChipLogProgress(Camera, "Registering transport to buffer %p", sink);
    mSinkSubscriptions[sink] = streamKeys;
}

void PreRollBuffer::DeregisterTransportFromBuffer(BufferSink * sink)
{
    std::lock_guard<std::mutex> lock(mBufferMutex);
    ChipLogProgress(Camera, "Deregistering transport from buffer %p", sink);
    mSinkSubscriptions.erase(sink);
}

void PreRollBuffer::TrimBuffer()
{
    std::lock_guard<std::mutex> lock(mBufferMutex);
    while (mContentBufferSize > mMaxTotalBytes)
    {
        std::shared_ptr<PreRollFrame> oldest = nullptr;
        std::string oldestStreamKey;

        // Find the oldest frame across all buffers
        for (auto & [streamKey, buffer] : mBuffers)
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
            // Remove oldest from buffer
            mBuffers[oldestStreamKey].pop_front();
            mContentBufferSize -= oldest->size;
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
