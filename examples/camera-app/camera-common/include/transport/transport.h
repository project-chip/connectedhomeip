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

#include <app-common/zap-generated/cluster-objects.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>
#pragma once
// Base class for media transports(WebRTC, PushAV)
// Media Transports would implement this interface for the Media controller to
// use.
// Media Transports would be registered with the Media controller and indicated
// by the CanSend<Video/Audio> when they are ready for data transfer.
// Before being destroyed, they must be unregistered from the Media controller.
class Transport
{
public:
    // Send video data for a given stream ID
    virtual void SendVideo(const chip::ByteSpan & data, int64_t timestamp, uint16_t videoStreamID) = 0;

    // Send audio data for a given stream ID
    virtual void SendAudio(const chip::ByteSpan & data, int64_t timestamp, uint16_t audioStreamID) = 0;

    // Send synchronixed audio/video data for given audio and video stream IDs
    virtual void SendAudioVideo(const chip::ByteSpan & data, uint16_t videoStreamID, uint16_t audioStreamID) = 0;

    // Indicates that the transport is ready to send video data
    virtual bool CanSendVideo() = 0;

    // Indicates that the transport is ready to send audio data
    virtual bool CanSendAudio() = 0;

    virtual ~Transport() = default;

    // SFrame End-to-End Encryption configuration (optional)
    // For transport types that support SFrame (e.g., WebRTC), this will contain the encryption config.
    // For transport types that don't support SFrame (e.g., PushAV), this will remain empty (!HasValue()).
    chip::Optional<chip::app::Clusters::WebRTCTransportProvider::Structs::SFrameStruct::Type> sFrameConfig;
};
