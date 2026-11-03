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

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>

#include <controller/python/matter/case_capture/CASEHandshakeMetricsRecord.h>

namespace chip {
namespace python {

// Holds handshakes that have reached a conclusion until a consumer thread collects them.
//
// The producer is the CHIP event loop inside the tracing hooks, so Publish does the least work
// that correctness allows: take a short lock, copy, signal, return. It never enters Python and
// never waits for a consumer, which is what keeps a slow or absent listener from showing up in
// the timings being measured.
class CompletedCASEHandshakeQueue
{
public:
    // Accepts records again, keeping at most `depth` of them, and discards anything left from a
    // previous run.
    void Open(uint32_t depth);

    // Stops accepting records, drops the ones held, and wakes every waiter so each can see that
    // notifications were stopped and unwind.
    void Close();

    // Called on the CHIP event loop thread, from inside the measured path.
    void Publish(const PychipCASEHandshakeMetricsRecord & record);

    // Called on a consumer thread with the interpreter lock released. Returns false when the
    // timeout expires or the queue was closed while waiting. `droppedCount` is the running total
    // of records there was no room for.
    bool Wait(PychipCASEHandshakeMetricsRecord & out, uint32_t timeoutMs, uint32_t & droppedCount);

private:
    std::mutex mMutex;
    std::condition_variable mRecordAvailable;
    std::deque<PychipCASEHandshakeMetricsRecord> mQueue;
    size_t mDepth          = kCASEHandshakeMetricsNotificationQueueDefaultDepth;
    uint32_t mDroppedCount = 0;
    bool mIsOpen           = false;
};

// The one queue the metrics backend publishes to and the Python consumer waits on.
CompletedCASEHandshakeQueue & CompletedCASEHandshakes();

} // namespace python
} // namespace chip
