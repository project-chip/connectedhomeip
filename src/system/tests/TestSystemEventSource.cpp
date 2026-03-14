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

using namespace chip;
using namespace chip::System;
using namespace chip::System::Clock::Literals;

// EventSources are only supported by a select-based LayerSelectLoop
#if !CHIP_SYSTEM_CONFIG_USE_DISPATCH
// The fake PlatformManagerImpl does not drive the system layer event loop
#if !CHIP_DEVICE_LAYER_TARGET_FAKE

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
        if (FD_ISSET(wakeEvent.GetReadFD(), &readfds))
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
#endif // !CHIP_DEVICE_LAYER_TARGET_FAKE
#endif // !CHIP_SYSTEM_CONFIG_USE_DISPATCH
