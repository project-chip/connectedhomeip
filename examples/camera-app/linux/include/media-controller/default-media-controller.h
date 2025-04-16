
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

#include <media-controller.h>
#include <mutex>
#include <vector>

// Default Media Controller
class DefaultMediaController : public MediaController
{
public:
    DefaultMediaController() {}
    virtual ~DefaultMediaController() {}
    // Transports register themselves with the media-controller for receiving
    // media from stream sources.
    void RegisterTransport(Transport * transport, uint16_t videoStreamID, uint16_t audioStreamID) override;
    // Transports must first unregister from the media-controller when they are
    // getting destroyed.
    void UnregisterTransport(Transport * transport) override;
    // Media controller goes through registered transports and dispatches media
    // if the transport is ready.
    void DistributeVideo(const char * data, size_t size, uint16_t videoStreamID) override;
    void DistributeAudio(const char * data, size_t size, uint16_t audioStreamID) override;

private:
    std::vector<Connection> connections;
    std::mutex connectionsMutex;
};
