/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <transport/TransportMgr.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

namespace chip {
namespace Test {

class LoopbackTransportManager
{
public:
    /// Initialize the underlying layers.
    CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(mIOContext.Init());
        GetLoopback().InitLoopbackTransport(&mIOContext.GetSystemLayer());
        ReturnErrorOnFailure(mTransportManager.Init("LOOPBACK"));
        return CHIP_NO_ERROR;
    }

    // Shutdown all layers, finalize operations
    void Shutdown()
    {
        GetLoopback().ShutdownLoopbackTransport();
        mIOContext.Shutdown();
    }

    System::Layer & GetSystemLayer() { return mIOContext.GetSystemLayer(); }
    LoopbackTransport & GetLoopback() { return mTransportManager.GetTransport().template GetImplAtIndex<0>(); }
    TransportMgrBase & GetTransportMgr() { return mTransportManager; }
    IOContext & GetIOContext() { return mIOContext; }

    /*
     * This drives the servicing of events using the embedded IOContext while there are pending
     * messages in the loopback transport's pending message queue. This should run to completion
     * in well-behaved logic (i.e there isn't an indefinite ping-pong of messages transmitted back
     * and forth).
     *
     * Consequently, this is guarded with a user-provided timeout to ensure we don't have unit-tests that stall
     * in CI due to bugs in the code that is being tested.
     *
     * This DOES NOT ensure that all pending events are serviced to completion
     * (i.e timers, any ScheduleWork calls), but does:
     *
     * 1) Guarantee that every call will make some progress on ready-to-run
     *    things, by calling DriveIO at least once.
     * 2) Try to ensure that any ScheduleWork calls that happend directly as a
     *    result of message reception, and any messages those async tasks send,
     *    get handled before DrainAndServiceIO returns.
     */
    void DrainAndServiceIO(System::Clock::Timeout maxWait = chip::System::Clock::Seconds16(5))
    {
        auto & impl                        = GetLoopback();
        System::Clock::Timestamp startTime = System::SystemClock().GetMonotonicTimestamp();

        while (true)
        {
            bool hadPendingMessages = impl.HasPendingMessages();
            while (impl.HasPendingMessages())
            {
                mIOContext.DriveIO();
                if ((System::SystemClock().GetMonotonicTimestamp() - startTime) >= maxWait)
                {
                    return;
                }
            }

            // Processing those messages might have queued some run-ASAP async
            // work.  Make sure to process that too, in case it generates
            // response messages.
            mIOContext.DriveIO();
            if (!hadPendingMessages && !impl.HasPendingMessages())
            {
                // We're not making any progress on messages.  Just stop.
                break;
            }
            // No need to check our timer here: either impl.HasPendingMessages()
            // is true and we will check it next iteration, or it's false and we
            // will either stop on the next iteration or it will become true and
            // we will check the timer then.
        }
    }

private:
    Test::IOContext mIOContext;
    TransportMgr<LoopbackTransport> mTransportManager;
};

} // namespace Test
} // namespace chip
