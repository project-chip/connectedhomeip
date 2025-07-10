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

#include <chrono>
#include <deque>
#include <lib/support/logging/CHIPLogging.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

struct RawBufferPacket
{
    char * data;
    size_t size;
    std::chrono::steady_clock::time_point InputTime;
    bool isVideo = false;
    std::string streamId;
    RawBufferPacket(const char * aData, size_t aSize, std::chrono::steady_clock::time_point iT, bool iV, std::string sI) :
        size(aSize), InputTime(iT), isVideo(iV), streamId(sI)
    {
        data = (char *) std::malloc(size);
        memcpy(data, aData, size);
        ChipLogProgress(Camera, "Added packet of STREAM ID %s", streamId.c_str());
    }
};

class PushAvPreRollBuffer
{
public:
    PushAvPreRollBuffer(long long maxDurationMs, size_t maxContentBufferSize);
    ~PushAvPreRollBuffer();
    void AddPacket(RawBufferPacket packet);
    RawBufferPacket FetchPacket();
    int GetSize();

private:
    std::deque<RawBufferPacket> mBuffer;
    long long mMaxDurationMs;                 // In milliseconds.
    size_t mMaxContentBufferSize;             // In bytes.
    size_t mCurrentSize;                      // Current sizes of each stream. Key is stream index
    mutable std::mutex mPreRollBufferMutex;   // Protects mBuffer and mMaxDurationMs.
    void RemovePackets(std::string streamId); // Removes packets from the buffer.
};
