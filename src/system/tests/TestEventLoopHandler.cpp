/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <cstdlib>
#include <functional>
#include <string>

#include <pw_unit_test/framework.h>
#include <system/SystemConfig.h>

// EventLoopHandlers are only supported by a select-based LayerSocketsLoop
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && !CHIP_SYSTEM_CONFIG_USE_DISPATCH
// The fake PlatformManagerImpl does not drive the system layer event loop
#if !CHIP_DEVICE_LAYER_TARGET_FAKE

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::System::Clock;
using namespace chip::System::Clock::Literals;

class TestEventLoopHandler : public ::testing::Test
{
public:
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

    static System::LayerSocketsLoop & SystemLayer() { return static_cast<System::LayerSocketsLoop &>(DeviceLayer::SystemLayer()); }

    // Schedules a call to the provided lambda and returns a cancel function.
    template <class Lambda>
    static std::function<void()> Schedule(Timeout delay, Lambda lambda)
    {
        System::TimerCompleteCallback callback = [](System::Layer * layer, void * ctx) {
            auto * function = static_cast<std::function<void()> *>(ctx);
            (*function)();
            delete function;
        };
        auto * function = new std::function<void()>(lambda);
        SystemLayer().StartTimer(delay, callback, function);
        return [=] {
            SystemLayer().CancelTimer(callback, function);
            delete function;
        };
    }

    template <class Lambda>
    static void ScheduleNextTick(Lambda lambda)
    {
        // ScheduleLambda is based on device events, which are greedily processed until the
        // queue is empty, so we can't use it to wait for the next event loop tick. Just use
        // a timer with a very short delay.
        Schedule(1_ms, lambda);
    }
};

TEST_F(TestEventLoopHandler, EventLoopHandlerSequence)
{
    struct : public System::EventLoopHandler
    {
        std::string trace;
        Timestamp PrepareEvents(Timestamp now) override
        {
            trace.append("P");
            return Timestamp::max();
        }
        void HandleEvents() override { trace.append("H"); }
    } loopHandler;

    ScheduleNextTick([&] {
        loopHandler.trace.append("1");
        SystemLayer().AddLoopHandler(loopHandler);
        loopHandler.trace.append("A");
        ScheduleNextTick([&] { // "P"
            loopHandler.trace.append("2");
            ScheduleNextTick([&] { // "H", "P"
                loopHandler.trace.append("3");
                SystemLayer().RemoveLoopHandler(loopHandler);
                loopHandler.trace.append("R");
                ScheduleNextTick([&] {
                    loopHandler.trace.append("4");
                    DeviceLayer::PlatformMgr().StopEventLoopTask();
                });
            });
        });
    });

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    EXPECT_EQ(loopHandler.trace, std::string("1AP2HP3R4"));
#else
    EXPECT_EQ(loopHandler.trace, std::string("1APHP2HPHP3R4"));
#endif
}

TEST_F(TestEventLoopHandler, EventLoopHandlerWake)
{
    struct : public System::EventLoopHandler
    {
        Timestamp startTimestamp = System::SystemClock().GetMonotonicTimestamp();
        Timestamp wakeTimestamp  = Timestamp::max();

        Timestamp PrepareEvents(Timestamp now) override { return now + 400_ms; }
        void HandleEvents() override
        {
            // StartTimer() (called by Schedule()) is liable to causes an immediate
            // wakeup via Signal(), so ignore this call if it's only been a few ms.
            auto now = System::SystemClock().GetMonotonicTimestamp();
            if (now - startTimestamp >= 100_ms)
            {
                wakeTimestamp = now;
                DeviceLayer::PlatformMgr().StopEventLoopTask();
            }
        }
    } loopHandler;

    // Schedule a fallback timer to ensure the test stops
    auto cancelFallback = Schedule(1000_ms, [] { DeviceLayer::PlatformMgr().StopEventLoopTask(); });
    SystemLayer().AddLoopHandler(loopHandler);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    SystemLayer().RemoveLoopHandler(loopHandler);
    cancelFallback(); // avoid leaking the fallback timer

    // Get the upper bound of the sleep duration from the environment, so we can
    // adjust it if the test machine is under heavy load, e.g. in CI or on a slow VM.
    // By default, we expect the sleep duration to be close to the requested 400ms.
    unsigned int expectedMaxDuration = 500u; // allow some slack for test machine load
    const char * maxDurationEnv      = std::getenv("CHIP_TEST_EVENT_LOOP_HANDLER_MAX_DURATION_MS");
    if (maxDurationEnv != nullptr)
    {
        ChipLogDetail(Test, "CHIP_TEST_EVENT_LOOP_HANDLER_MAX_DURATION_MS=%s", maxDurationEnv);
        expectedMaxDuration = static_cast<unsigned int>(std::stoul(maxDurationEnv));
    }

    Timestamp sleepDuration = loopHandler.wakeTimestamp - loopHandler.startTimestamp;
    EXPECT_GE(sleepDuration.count(), 400u); // loopHandler requested wake-up after 400ms
    EXPECT_LE(sleepDuration.count(), expectedMaxDuration);
}

#endif // !CHIP_DEVICE_LAYER_TARGET_FAKE
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && !CHIP_SYSTEM_CONFIG_USE_DISPATCH
