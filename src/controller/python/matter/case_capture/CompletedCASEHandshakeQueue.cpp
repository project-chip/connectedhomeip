/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <controller/python/matter/case_capture/CompletedCASEHandshakeQueue.h>

#include <chrono>

namespace chip {
namespace python {

void CompletedCASEHandshakeQueue::Open(uint32_t depth)
{
    const std::lock_guard<std::mutex> lock(mMutex);
    mDepth        = depth;
    mDroppedCount = 0;
    mIsOpen       = true;
    mQueue.clear();
}

void CompletedCASEHandshakeQueue::Close()
{
    {
        const std::lock_guard<std::mutex> lock(mMutex);
        mIsOpen = false;
        mQueue.clear();
    }
    // Wake every waiter so each can see notifications were stopped and unwind.
    mRecordAvailable.notify_all();
}

void CompletedCASEHandshakeQueue::Publish(const PychipCASEHandshakeMetricsRecord & record)
{
    {
        const std::lock_guard<std::mutex> lock(mMutex);
        if (!mIsOpen)
        {
            return;
        }
        if (mQueue.size() >= mDepth)
        {
            // Drop the oldest rather than block the handshake or grow without bound. The
            // count is what makes a consumer that cannot keep up visible to the caller.
            mQueue.pop_front();
            mDroppedCount++;
        }
        mQueue.push_back(record);
    }
    mRecordAvailable.notify_one();
}

bool CompletedCASEHandshakeQueue::Wait(PychipCASEHandshakeMetricsRecord & out, uint32_t timeoutMs, uint32_t & droppedCount)
{
    std::unique_lock<std::mutex> lock(mMutex);
    mRecordAvailable.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] { return !mQueue.empty() || !mIsOpen; });

    droppedCount = mDroppedCount;
    if (mQueue.empty())
    {
        return false; // Timed out, or closed while waiting.
    }
    out = mQueue.front();
    mQueue.pop_front();
    return true;
}

CompletedCASEHandshakeQueue & CompletedCASEHandshakes()
{
    // Function-local, so it is built on first use rather than depending on static init order
    // across translation units.
    static CompletedCASEHandshakeQueue queue;
    return queue;
}

} // namespace python
} // namespace chip
