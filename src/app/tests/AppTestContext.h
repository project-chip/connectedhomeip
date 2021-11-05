/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "system/SystemClock.h"
#include <messaging/tests/MessagingContext.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlunit-test.h>

namespace chip {
namespace Test {

/**
 * @brief The context of test cases for messaging layer. It wil initialize network layer and system layer, and create
 *        two secure sessions, connected with each other. Exchanges can be created for each secure session.
 */
class AppContext : public MessagingContext
{
public:
    /// Initialize the underlying layers and test suite pointer
    CHIP_ERROR Init();

    // Shutdown all layers, finalize operations
    CHIP_ERROR Shutdown();

    /*
     * For unit-tests that simulate end-to-end transmission and reception of messages in loopback mode,
     * this mode better replicates a real-functioning stack that correctly handles the processing
     * of a transmitted message as an asynchronous, bottom half handler dispatched after the current execution context has
     completed.
     * This is achieved using SystemLayer::ScheduleWork.

     * This should be used on conjunction with the DrainAndServiceIO function below to correctly service and drain the event queue.
     *
     */
    void EnableAsyncDispatch()
    {
        auto & impl = mTransportManager.GetTransport().GetImplAtIndex<0>();
        impl.EnableAsyncDispatch(&mIOContext.GetSystemLayer());
    }

    /*
     * This drives the servicing of events using the embedded IOContext while there are pending
     * messages in the loopback transport's pending message queue. This should run to completion
     * in well-behaved logic (i.e there isn't an indefinite ping-pong of messages transmitted back
     * and forth).
     *
     * Consequently, this is guarded with a user-provided timeout to ensure we don't have unit-tests that stall
     * in CI due to bugs in the code that is being tested.
     *
     * This DOES NOT ensure that all pending events are serviced to completion (i.e timers, any ScheduleWork calls).
     *
     */
    void DrainAndServiceIO(System::Clock::Timeout maxWait = chip::System::Clock::Seconds16(5))
    {
        auto & impl                        = mTransportManager.GetTransport().GetImplAtIndex<0>();
        System::Clock::Timestamp startTime = System::SystemClock().GetMonotonicTimestamp();

        while (impl.HasPendingMessages())
        {
            mIOContext.DriveIO();
            if ((System::SystemClock().GetMonotonicTimestamp() - startTime) >= maxWait)
            {
                break;
            }
        }
    }

    static int Initialize(void * context)
    {
        auto * ctx = static_cast<AppContext *>(context);
        return ctx->Init() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
    }

    static int InitializeAsync(void * context)
    {
        auto * ctx = static_cast<AppContext *>(context);

        VerifyOrReturnError(ctx->Init() == CHIP_NO_ERROR, FAILURE);
        ctx->EnableAsyncDispatch();

        return SUCCESS;
    }

    static int Finalize(void * context)
    {
        auto * ctx = static_cast<AppContext *>(context);
        return ctx->Shutdown() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
    }

private:
    chip::TransportMgr<chip::Test::LoopbackTransport> mTransportManager;
    chip::Test::IOContext mIOContext;
};

} // namespace Test
} // namespace chip
