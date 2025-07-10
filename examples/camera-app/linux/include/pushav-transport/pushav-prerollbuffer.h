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
 *    distributed under the License is distributed on an "AS IS" BASIS,vvv
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <chrono>
#include <string>

// TODO: This dummy file to be removed after merging prerollbuffer implementation

struct RawBufferPacket
{
    const char * data;
    size_t size;
    std::chrono::steady_clock::time_point timestamp;
    bool isVideo;
    std::string streamId;

    RawBufferPacket(const char * d, size_t s, std::chrono::steady_clock::time_point t, bool v, std::string id) :
        data(d), size(s), timestamp(t), isVideo(v), streamId(id)
    {}
};

class PushAvPreRollBuffer
{
public:
    PushAvPreRollBuffer(int preRollLength, int timeBase)
    {
        // Dummy implementation
    }

    ~PushAvPreRollBuffer() = default;

    void AddPacket(RawBufferPacket packet)
    {
        // Dummy implementation
    }

    RawBufferPacket FetchPacket()
    {
        // Return empty packet
        return RawBufferPacket(nullptr, 0, std::chrono::steady_clock::time_point(), false, "");
    }
};
