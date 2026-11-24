/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This is a unit test suite for <tt>chip::System::SystemLayerImplSelect</tt>
 *
 */

#include <gtest/gtest.h>

#include <lib/support/CodeUtils.h>
#include <pw_unit_test/framework.h>
#include <system/SystemClock.h>
#include <system/SystemConfig.h>
#include <system/SystemError.h>
#include <system/SystemLayer.h>
#include <system/SystemLayerImpl.h>

#include <platform/CHIPDeviceLayer.h>

// EventSources are only supported by a select-based LayerSelectLoop
#if !CHIP_SYSTEM_CONFIG_USE_DISPATCH
// The fake PlatformManagerImpl does not drive the system layer event loop
#if !CHIP_DEVICE_LAYER_TARGET_FAKE

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS && CHIP_SYSTEM_CONFIG_POSIX_LOCKING &&                  \
    !CHIP_SYSTEM_CONFIG_USE_LIBEV
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS && CHIP_SYSTEM_CONFIG_POSIX_LOCKING &&
       // !CHIP_SYSTEM_CONFIG_USE_LIBEV

using namespace chip;
using namespace chip::System;
using namespace chip::System::Clock::Literals;

struct TestSource : public LayerImplSelect::EventSource
{
    TestSource() { EXPECT_EQ(wakeEvent.Open(), CHIP_NO_ERROR); }
    ~TestSource() { wakeEvent.Close(); }

    void PrepareEvents(int & maxfd, fd_set & readfds, fd_set & writefds, fd_set & exceptfds, struct timeval & timeout) override
    {
        maxfd = std::max(maxfd, wakeEvent.GetReadFD());
        FD_SET(wakeEvent.GetReadFD(), &readfds);

        if (timeout.tv_sec > delaySeconds)
        {
            timeout.tv_sec  = delaySeconds;
            timeout.tv_usec = 0;
        }
        else if (timeout.tv_sec == delaySeconds)
        {
            timeout.tv_usec = 0;
        }
        prepareCalled++;
    }

    void ProcessEvents(const fd_set & readfds, const fd_set & writefds, const fd_set & exceptfds) override
    {
        // Create mutable copy because Zephyr's FD_ISSET() API requires a non-const fd_set*.
        // The original parameters are const, so we must work on local mutable copies.
        fd_set readfds_copy = readfds;
        if (FD_ISSET(wakeEvent.GetReadFD(), &readfds_copy))
        {
            wakeEvent.Confirm();
            handlerCalled++;
        }
    }

    CHIP_ERROR Notify() { return wakeEvent.Notify(); }

    int prepareCalled = 0;
    int handlerCalled = 0;
    int delaySeconds  = 0;
    WakeEvent wakeEvent;
};

struct TestSystemEventSource : public ::testing::Test
{
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }
};

TEST_F(TestSystemEventSource, OneEventSource)
{
    auto & impl = static_cast<LayerImplSelect &>(chip::DeviceLayer::SystemLayer());

    TestSource source1;

    // Add for the first time
    impl.EventSourceAdd(&source1);
    impl.PrepareEvents();
    impl.WaitForEvents();
    impl.HandleEvents();

    ASSERT_EQ(source1.prepareCalled, 1);
    ASSERT_EQ(source1.handlerCalled, 0);

    // Add the same source for the second time
    impl.EventSourceAdd(&source1);
    impl.PrepareEvents();
    impl.WaitForEvents();
    impl.HandleEvents();

    ASSERT_EQ(source1.prepareCalled, 2);
    ASSERT_EQ(source1.handlerCalled, 0);

    // Notify the source
    ASSERT_EQ(source1.Notify(), CHIP_NO_ERROR);
    impl.PrepareEvents();
    impl.WaitForEvents();
    impl.HandleEvents();
    ASSERT_EQ(source1.prepareCalled, 3);
    ASSERT_EQ(source1.handlerCalled, 1);

    impl.EventSourceClear();
}

TEST_F(TestSystemEventSource, MultipleEventSources)
{
    auto & impl = static_cast<LayerImplSelect &>(chip::DeviceLayer::SystemLayer());

    TestSource source1;
    TestSource source2;

    ASSERT_EQ(source1.Notify(), CHIP_NO_ERROR);
    impl.EventSourceAdd(&source1);
    impl.PrepareEvents();
    impl.WaitForEvents();
    impl.HandleEvents();
    ASSERT_EQ(source1.handlerCalled, 1);

    ASSERT_EQ(source2.Notify(), CHIP_NO_ERROR);
    impl.EventSourceAdd(&source2);
    impl.PrepareEvents();
    impl.WaitForEvents();
    impl.HandleEvents();
    ASSERT_EQ(source2.handlerCalled, 1);

    ASSERT_EQ(source1.Notify(), CHIP_NO_ERROR);
    ASSERT_EQ(source2.Notify(), CHIP_NO_ERROR);

    impl.PrepareEvents();
    impl.WaitForEvents();
    impl.HandleEvents();
    ASSERT_EQ(source1.handlerCalled, 2);
    ASSERT_EQ(source2.handlerCalled, 2);

    impl.EventSourceClear();
}

TEST_F(TestSystemEventSource, RemoveSomeEventSource)
{
    auto & impl = static_cast<LayerImplSelect &>(chip::DeviceLayer::SystemLayer());

    TestSource source1;
    TestSource source2;

    ASSERT_EQ(source1.Notify(), CHIP_NO_ERROR);
    impl.EventSourceAdd(&source1);
    impl.PrepareEvents();
    impl.WaitForEvents();
    impl.HandleEvents();

    ASSERT_EQ(source2.Notify(), CHIP_NO_ERROR);
    impl.EventSourceAdd(&source2);
    impl.PrepareEvents();
    impl.WaitForEvents();
    impl.HandleEvents();

    ASSERT_EQ(source2.Notify(), CHIP_NO_ERROR);
    impl.EventSourceRemove(&source1);
    impl.PrepareEvents();
    impl.WaitForEvents();
    impl.HandleEvents();
    ASSERT_EQ(source1.handlerCalled, 1);
    ASSERT_EQ(source2.handlerCalled, 2);

    impl.EventSourceClear();
}

TEST_F(TestSystemEventSource, MultipleEventSourcesOfDifferentTimeout)
{
    auto & impl = static_cast<LayerImplSelect &>(chip::DeviceLayer::SystemLayer());

    TestSource source1;
    TestSource source2;

    source1.delaySeconds = 2;
    source2.delaySeconds = 3;
    impl.EventSourceAdd(&source1);
    impl.EventSourceAdd(&source2);
    impl.PrepareEvents();
    auto start = System::SystemClock().GetMonotonicTimestamp();
    impl.WaitForEvents();
    auto end = System::SystemClock().GetMonotonicTimestamp();
    impl.HandleEvents();
    auto duration = end - start;
    ASSERT_GE(duration, 2000_ms);
    ASSERT_LT(duration, 3000_ms);

    impl.EventSourceClear();
}

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS && CHIP_SYSTEM_CONFIG_POSIX_LOCKING &&                  \
    !CHIP_SYSTEM_CONFIG_USE_LIBEV

namespace {

void DrainPendingWakeEvents(LayerImplSelect & impl)
{
    TestSource drainSource;
    drainSource.delaySeconds = 0;
    impl.EventSourceAdd(&drainSource);
    impl.PrepareEvents();
    {
        DeviceLayer::StackUnlock unlock;
        impl.WaitForEvents();
    }
    impl.HandleEvents();
    impl.EventSourceRemove(&drainSource);
}

struct CallbackState
{
    explicit CallbackState(SocketEventFlags flag) : expectedFlag(flag) {}

    SocketEventFlags expectedFlag;
    std::atomic<int> count{ 0 };
};

void OnSocketWatchEvent(SocketEvents events, intptr_t data)
{
    auto * state = reinterpret_cast<CallbackState *>(data);
    if (events.Has(state->expectedFlag))
    {
        state->count.fetch_add(1, std::memory_order_relaxed);
    }
}

void VerifyCrossThreadRequestWake(LayerImplSelect & impl, SocketEventFlags flag)
{
    int fds[2]             = { -1, -1 };
    SocketWatchToken token = impl.InvalidSocketWatchToken();
    auto cleanup           = ScopeExit([&]() {
        if (token != impl.InvalidSocketWatchToken())
        {
            DeviceLayer::StackLock lock;
            EXPECT_EQ(impl.StopWatchingSocket(&token), CHIP_NO_ERROR);
            DrainPendingWakeEvents(impl);
        }
        if (fds[0] >= 0)
        {
            EXPECT_EQ(::close(fds[0]), 0);
        }
        if (fds[1] >= 0)
        {
            EXPECT_EQ(::close(fds[1]), 0);
        }
    });

    DeviceLayer::StackLock stackLock;

    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

    ASSERT_EQ(impl.StartWatchingSocket(fds[0], &token), CHIP_NO_ERROR);

    CallbackState callbackState(flag);
    ASSERT_EQ(impl.SetCallback(token, OnSocketWatchEvent, reinterpret_cast<intptr_t>(&callbackState)), CHIP_NO_ERROR);

    DrainPendingWakeEvents(impl);

    TestSource timeoutSource;
    timeoutSource.delaySeconds = 5;
    impl.EventSourceAdd(&timeoutSource);
    impl.PrepareEvents();

    std::mutex mutex;
    std::condition_variable cv;
    bool readyToWait     = false;
    CHIP_ERROR workerErr = CHIP_NO_ERROR;

    std::thread worker([&]() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&]() { return readyToWait; });
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        DeviceLayer::StackLock lock;
        if (flag == SocketEventFlags::kRead)
        {
            workerErr = impl.RequestCallbackOnPendingRead(token);
        }
        else
        {
            workerErr = impl.RequestCallbackOnPendingWrite(token);
        }
    });

    {
        std::lock_guard<std::mutex> lock(mutex);
        readyToWait = true;
    }
    cv.notify_one();

    const auto start = System::SystemClock().GetMonotonicTimestamp();
    System::Clock::Timestamp elapsed{};
    {
        DeviceLayer::StackUnlock unlock;
        impl.WaitForEvents();
        elapsed = System::SystemClock().GetMonotonicTimestamp() - start;
        worker.join();
    }
    impl.HandleEvents();
    impl.EventSourceRemove(&timeoutSource);

    EXPECT_EQ(workerErr, CHIP_NO_ERROR);
    EXPECT_LT(elapsed.count(), 2000u);
    EXPECT_EQ(callbackState.count.load(std::memory_order_relaxed), 0);

    if (flag == SocketEventFlags::kRead)
    {
        const char byte = 1;
        ASSERT_EQ(::write(fds[1], &byte, sizeof(byte)), 1);
    }

    TestSource readinessSource;
    readinessSource.delaySeconds = 5;
    impl.EventSourceAdd(&readinessSource);
    impl.PrepareEvents();
    {
        DeviceLayer::StackUnlock unlock;
        impl.WaitForEvents();
    }
    impl.HandleEvents();
    impl.EventSourceRemove(&readinessSource);
    EXPECT_EQ(callbackState.count.load(std::memory_order_relaxed), 1);

    // Verify the 0 -> 1 guard does not emit a redundant wake when already pending.
    if (flag == SocketEventFlags::kRead)
    {
        EXPECT_EQ(impl.RequestCallbackOnPendingRead(token), CHIP_NO_ERROR);
        EXPECT_EQ(impl.ClearCallbackOnPendingRead(token), CHIP_NO_ERROR);
    }
    else
    {
        EXPECT_EQ(impl.RequestCallbackOnPendingWrite(token), CHIP_NO_ERROR);
        EXPECT_EQ(impl.ClearCallbackOnPendingWrite(token), CHIP_NO_ERROR);
    }

    TestSource noWakeSource;
    noWakeSource.delaySeconds = 1;
    impl.EventSourceAdd(&noWakeSource);
    impl.PrepareEvents();
    const auto noWakeStart = System::SystemClock().GetMonotonicTimestamp();
    {
        DeviceLayer::StackUnlock unlock;
        impl.WaitForEvents();
    }
    const auto noWakeElapsed = System::SystemClock().GetMonotonicTimestamp() - noWakeStart;
    impl.HandleEvents();
    impl.EventSourceRemove(&noWakeSource);
    EXPECT_GE(noWakeElapsed.count(), 900u);
}

} // namespace

TEST_F(TestSystemEventSource, RequestCallbackOnPendingReadWakesSelect)
{
    auto & impl = static_cast<LayerImplSelect &>(chip::DeviceLayer::SystemLayer());
    VerifyCrossThreadRequestWake(impl, SocketEventFlags::kRead);
}

TEST_F(TestSystemEventSource, RequestCallbackOnPendingWriteWakesSelect)
{
    auto & impl = static_cast<LayerImplSelect &>(chip::DeviceLayer::SystemLayer());
    VerifyCrossThreadRequestWake(impl, SocketEventFlags::kWrite);
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS && CHIP_SYSTEM_CONFIG_POSIX_LOCKING &&
       // !CHIP_SYSTEM_CONFIG_USE_LIBEV

#endif // !CHIP_DEVICE_LAYER_TARGET_FAKE
#endif // !CHIP_SYSTEM_CONFIG_USE_DISPATCH
