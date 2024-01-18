/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *      This file declares the CHIP device event queue which operates in a FIFO context (first-in first-out),
 *      and provides a specific set of member functions to access its elements wth thread safety.
 */

#pragma once

#include <mutex>
#include <queue>

#include <lib/core/CHIPCore.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *  @class DeviceSafeQueue
 *
 *  @brief
 *      This class represents a thread-safe message queue implemented with C++ Standard Library, the message queue
 *      is used by the CHIP event loop to hold incoming messages. Each message is sequentially dequeued, decoded,
 *      and then an action is performed.
 *
 */
class DeviceSafeQueue
{
public:
    DeviceSafeQueue()  = default;
    ~DeviceSafeQueue() = default;

    void Push(const ChipDeviceEvent & event);
    bool Empty();
    ChipDeviceEvent PopFront();

private:
    std::queue<ChipDeviceEvent> mEventQueue;
    std::mutex mEventQueueLock;

    DeviceSafeQueue(const DeviceSafeQueue &)             = delete;
    DeviceSafeQueue & operator=(const DeviceSafeQueue &) = delete;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
