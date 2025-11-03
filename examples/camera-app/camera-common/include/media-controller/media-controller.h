
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

#pragma once

#include <mutex>
#include <transport.h>
#include <vector>

// Connection object to store transport and stream IDs
struct Connection
{
    Transport * transport;
    uint16_t videoStreamID;
    uint16_t audioStreamID;
};

// Media Controller
class MediaController
{
public:
    MediaController() {}
    virtual ~MediaController() {}
    // Transports register themselves with the media-controller for receiving
    // media from stream sources.
    virtual void RegisterTransport(Transport * transport, uint16_t videoStreamID, uint16_t audioStreamID) = 0;
    // Transports must first unregister from the media-controller when they are
    // getting destroyed.
    virtual void UnregisterTransport(Transport * transport) = 0;
    // Get transport registered for a specific stream ID
    virtual Transport * GetTransportForVideoStream(uint16_t videoStreamID) = 0;
    virtual Transport * GetTransportForAudioStream(uint16_t audioStreamID) = 0;
    // Media controller goes through registered transports and dispatches media
    // if the transport is ready.
    virtual void DistributeVideo(const uint8_t * data, size_t size, uint16_t videoStreamID) = 0;
    virtual void DistributeAudio(const uint8_t * data, size_t size, uint16_t audioStreamID) = 0;
    virtual void SetPreRollLength(Transport * transport, uint16_t PreRollBufferLength)      = 0;
};
