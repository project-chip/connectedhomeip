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
#include "camera-device.h"
#include "pushav-prerollbuffer.h"
#include <algorithm>
#include <lib/support/logging/CHIPLogging.h>

void DefaultMediaController::SetCameraDevice(Camera::CameraDevice * device)
{
    mCameraDevice = device;
    if (mCameraDevice)
    {
        size_t bufferSize = mCameraDevice->GetPreRollBufferSize();
        mPreRollBuffer.SetMaxTotalBytes(bufferSize * 1000);
        ChipLogProgress(Camera, "PreRollBuffer size set to %zu bytes from CameraDevice.", bufferSize);
    }
    else
    {
        // Handle case where device is null, perhaps revert to a default or log an error
        ChipLogError(Camera, "CameraDevice is null in DefaultMediaController::SetCameraDevice. PreRollBuffer size not set.");
        mPreRollBuffer.SetMaxTotalBytes(509600); // Fallback to a small default
    }
}

void DefaultMediaController::RegisterTransport(Transport * transport, const std::vector<uint16_t> & videoStreams,
                                               const std::vector<uint16_t> & audioStreams)
{
    ChipLogProgress(Camera, "Registering transport: videoStreams count=%u, audioStreams count=%u",
                    static_cast<unsigned>(videoStreams.size()), static_cast<unsigned>(audioStreams.size()));

    std::lock_guard<std::mutex> lock(mConnectionsMutex);
    mConnections.push_back({ transport, videoStreams, audioStreams });

    auto * bufferSink     = new BufferSink();
    bufferSink->transport = transport;
    // 0: Deliver with the minimum I-frame duration
    // 1: Deliver with a delay of up to 1 ms (default)
    // Other values: Deliver with the specified delay
    bufferSink->requestedPreBufferLengthMs = 1;
    bufferSink->registrationTimeMs         = mPreRollBuffer.NowMs();
    bufferSink->hasDeliveredFirstFrame     = false;

    if (mCameraDevice)
    {
        bufferSink->minKeyframeIntervalMs = mCameraDevice->GetMinKeyframeIntervalMs();
        ChipLogProgress(Camera, "MinKeyframeIntervalMs set to %lld ms from CameraDevice.",
                        static_cast<long long>(bufferSink->minKeyframeIntervalMs));
    }
    else
    {
        bufferSink->minKeyframeIntervalMs = 1000; // Fallback default if CameraDevice not set
        ChipLogError(Camera, "CameraDevice not set in DefaultMediaController. Using default MinKeyframeIntervalMs.");
    }

    std::unordered_set<std::string> streamKeys;
    for (uint16_t audioStream : audioStreams)
    {
        streamKeys.insert("a" + std::to_string(audioStream));
        ChipLogProgress(Camera, "  Registered audioStream=%u", audioStream);
    }

    for (uint16_t videoStream : videoStreams)
    {
        streamKeys.insert("v" + std::to_string(videoStream));
        ChipLogProgress(Camera, "  Registered videoStream=%u", videoStream);
    }

    mPreRollBuffer.RegisterTransportToBuffer(bufferSink, streamKeys);
    mSinkMap[transport] = bufferSink;
    ChipLogProgress(Camera, "Transport registered successfully. Total connections: %u", (unsigned) mConnections.size());
}

void DefaultMediaController::UnregisterTransport(Transport * transport)
{
    std::lock_guard<std::mutex> lock(mConnectionsMutex);
    mConnections.erase(std::remove_if(mConnections.begin(), mConnections.end(),
                                      [transport](const Connection & c) { return c.transport == transport; }),
                       mConnections.end());
    auto it = mSinkMap.find(transport);
    if (it != mSinkMap.end())
    {
        mPreRollBuffer.DeregisterTransportFromBuffer(it->second);
        delete it->second;
        mSinkMap.erase(it);
        ChipLogProgress(Camera, "Sink deregistered for transport.");
    }
}

void DefaultMediaController::DistributeVideo(const uint8_t * data, size_t size, uint16_t videoStreamID, int64_t timestamp)
{
    std::string streamKey = "v" + std::to_string(videoStreamID);
    mPreRollBuffer.PushFrameToBuffer(streamKey, data, size, timestamp);
}

void DefaultMediaController::DistributeAudio(const uint8_t * data, size_t size, uint16_t audioStreamID, int64_t timestamp)
{
    std::string streamKey = "a" + std::to_string(audioStreamID);
    mPreRollBuffer.PushFrameToBuffer(streamKey, data, size, timestamp);
}

void DefaultMediaController::SetPreRollLength(Transport * transport, uint16_t preRollBufferLength)

{
    auto it = mSinkMap.find(transport);
    if (it != mSinkMap.end() && it->second != nullptr)
    {
        it->second->requestedPreBufferLengthMs = preRollBufferLength;
        ChipLogProgress(Camera, "Delay updated for transport to %u ms", preRollBufferLength);
    }
    else
    {
        ChipLogError(Camera, "SetDelay: Transport not registered");
    }
}

Transport * DefaultMediaController::GetTransportForVideoStream(uint16_t videoStreamID)
{
    std::lock_guard<std::mutex> lock(mConnectionsMutex);
    for (const auto & conn : mConnections)
    {
        for (uint16_t id : conn.videoStreams)
        {
            if (id == videoStreamID)
            {
                return conn.transport;
            }
        }
    }
    return nullptr;
}

Transport * DefaultMediaController::GetTransportForAudioStream(uint16_t audioStreamID)
{
    std::lock_guard<std::mutex> lock(mConnectionsMutex);
    for (const auto & conn : mConnections)
    {
        for (uint16_t id : conn.audioStreams)
        {
            if (id == audioStreamID)
            {
                return conn.transport;
            }
        }
    }
    return nullptr;
}

void DefaultMediaController::ResetTransportSinkState(Transport * transport)
{
    auto it = mSinkMap.find(transport);
    if (it != mSinkMap.end() && it->second != nullptr)
    {
        it->second->registrationTimeMs     = mPreRollBuffer.NowMs();
        it->second->hasDeliveredFirstFrame = false;
        ChipLogProgress(Camera, "Reset sink state for transport: registrationTimeMs=%lld",
                        static_cast<long long>(it->second->registrationTimeMs));
    }
    else
    {
        ChipLogError(Camera, "ResetTransportSinkState: Transport not registered");
    }
}
