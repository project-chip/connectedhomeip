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
