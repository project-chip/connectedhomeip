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
#include <functional>
#include <memory>
#include <string>
#include <transport.h>
#include <unordered_map>
#include <unordered_set>
struct BufferSink
{
    int64_t requestedPreBufferLengthMs; // 0 means live only
    int64_t minKeyframeIntervalMs;
    Transport * transport;
};

struct PreRollFrame
{
    std::string streamKey;                        // e.g., "a123" or "v456"
    std::unique_ptr<char[]> data;                 // raw frame data
    size_t size;                                  // bytes size
    int64_t ptsMs;                                // receive time
    std::unordered_set<BufferSink *> deliveredTo; // to prevent duplicate sends
};

class PreRollBuffer
{
public:
    PreRollBuffer(size_t maxTotalBytes);
    void PushFrameToBuffer(const std::string & streamKey, const char * data, size_t size);
    void RegisterTransportToBuffer(BufferSink * sink, const std::unordered_set<std::string> & streamKeys);
    void DeregisterTransportFromBuffer(BufferSink * sink);
    int64_t NowMs() const;

private:
    void PushBufferToTransport();
    void TrimBuffer();

    size_t maxTotalBytes;
    size_t contentBufferSize;

    std::unordered_map<std::string, std::deque<std::shared_ptr<PreRollFrame>>> buffers;
    std::unordered_map<BufferSink *, std::unordered_set<std::string>> sinkSubscriptions;
};
