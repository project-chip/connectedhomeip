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

#include <atomic>
#include <cstdint>
#include <media-controller.h>
#include <thread>

enum class StreamType
{
    kVideo,
    kAudio,
};

// Network Stream Source
class NetworkStreamSource
{
public:
    NetworkStreamSource() {}
    virtual ~NetworkStreamSource() {}
    void Init(MediaController * aMediaController, uint16_t aSrcPort, StreamType streamType);
    void Start(uint16_t streamId);
    void Stop();

private:
    void ListenForStreamOnSocket();

    MediaController * mMediaController = nullptr;
    uint16_t mSrcPort;
    uint16_t mStreamId;
    StreamType mStreamType;
    std::atomic<bool> mStreamSourceActive = false;

    std::vector<std::thread> streamThreads;
};
