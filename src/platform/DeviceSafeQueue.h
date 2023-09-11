/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    DeviceSafeQueue(const DeviceSafeQueue &) = delete;
    DeviceSafeQueue & operator=(const DeviceSafeQueue &) = delete;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
