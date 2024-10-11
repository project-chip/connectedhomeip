/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This is a unit test suite for <tt>chip::System::WakeEvent</tt>
 *
 */

#include <pw_unit_test/framework.h>

#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemConfig.h>
#include <system/SystemError.h>
#include <system/SystemLayerImpl.h>

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <pthread.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

using namespace chip::System;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

namespace chip {
namespace System {
class WakeEventTest
{
public:
    static int GetReadFD(const WakeEvent & wakeEvent) { return wakeEvent.GetReadFD(); }
};
} // namespace System
} // namespace chip

namespace {

class TestSystemWakeEvent : public ::testing::Test
{
public:
    void SetUp()
    {
        mSystemLayer.Init();
        mWakeEvent.Open(mSystemLayer);
    }

    void TearDown()
    {
        mWakeEvent.Close(mSystemLayer);
        mSystemLayer.Shutdown();
    }

    ::chip::System::LayerImpl mSystemLayer;
    WakeEvent mWakeEvent;
    fd_set mReadSet;
    fd_set mWriteSet;
    fd_set mErrorSet;

    int SelectWakeEvent(timeval timeout = {})
    {
        // NOLINTBEGIN(clang-analyzer-security.insecureAPI.bzero)
        //
        // NOTE: darwin uses bzero to clear out FD sets. This is not a security concern.
        FD_ZERO(&mReadSet);
        FD_ZERO(&mWriteSet);
        FD_ZERO(&mErrorSet);
        // NOLINTEND(clang-analyzer-security.insecureAPI.bzero)

        FD_SET(WakeEventTest::GetReadFD(mWakeEvent), &mReadSet);
        return select(WakeEventTest::GetReadFD(mWakeEvent) + 1, &mReadSet, &mWriteSet, &mErrorSet, &timeout);
    }
};

TEST_F(TestSystemWakeEvent, TestOpen)
{
    EXPECT_GE(WakeEventTest::GetReadFD(mWakeEvent), 0);
    EXPECT_EQ(SelectWakeEvent(), 0);
}

TEST_F(TestSystemWakeEvent, TestNotify)
{
    EXPECT_EQ(SelectWakeEvent(), 0);

    // Check that select() succeeds after Notify() has been called
    mWakeEvent.Notify();
    EXPECT_EQ(SelectWakeEvent(), 1);
    EXPECT_TRUE(FD_ISSET(WakeEventTest::GetReadFD(mWakeEvent), &mReadSet));

    // ...and state of the event is not cleared automatically
    EXPECT_EQ(SelectWakeEvent(), 1);
    EXPECT_TRUE(FD_ISSET(WakeEventTest::GetReadFD(mWakeEvent), &mReadSet));
}

TEST_F(TestSystemWakeEvent, TestConfirm)
{
    // Check that select() succeeds after Notify() has been called
    mWakeEvent.Notify();
    EXPECT_EQ(SelectWakeEvent(), 1);
    EXPECT_TRUE(FD_ISSET(WakeEventTest::GetReadFD(mWakeEvent), &mReadSet));

    // Check that Confirm() clears state of the event
    mWakeEvent.Confirm();
    EXPECT_EQ(SelectWakeEvent(), 0);
}

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
void * WaitForEvent(void * aContext)
{
    // wait 5 seconds
    auto * context = static_cast<TestSystemWakeEvent *>(aContext);
    return reinterpret_cast<void *>(context->SelectWakeEvent(timeval{ 5, 0 }));
}

TEST_F(TestSystemWakeEvent, TestBlockingSelect)
{
    // Spawn a thread waiting for the event
    pthread_t tid = 0;
    EXPECT_EQ(0, pthread_create(&tid, nullptr, WaitForEvent, this));

    mWakeEvent.Notify();
    void * selectResult = nullptr;
    EXPECT_EQ(0, pthread_join(tid, &selectResult));
    EXPECT_EQ(selectResult, reinterpret_cast<void *>(1));
}
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

TEST_F(TestSystemWakeEvent, TestClose)
{
    mWakeEvent.Close(mSystemLayer);

    const auto notifFD = WakeEventTest::GetReadFD(mWakeEvent);

    // Check that Close() has cleaned up itself and reopen is possible
    EXPECT_EQ(mWakeEvent.Open(mSystemLayer), CHIP_NO_ERROR);
    EXPECT_LT(notifFD, 0);
}
} // namespace

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
