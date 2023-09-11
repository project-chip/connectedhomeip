/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines the CHIP device event queue which operates in a FIFO context (first-in first-out),
 *      and provides a specific set of member functions to access its elements wth thread safety.
 */

#include <platform/DeviceSafeQueue.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

void DeviceSafeQueue::Push(const ChipDeviceEvent & event)
{
    std::unique_lock<std::mutex> lock(mEventQueueLock);
    mEventQueue.push(event);
}

bool DeviceSafeQueue::Empty()
{
    std::unique_lock<std::mutex> lock(mEventQueueLock);
    return mEventQueue.empty();
}

ChipDeviceEvent DeviceSafeQueue::PopFront()
{
    std::unique_lock<std::mutex> lock(mEventQueueLock);

    const ChipDeviceEvent event = mEventQueue.front();
    mEventQueue.pop();

    return event;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
