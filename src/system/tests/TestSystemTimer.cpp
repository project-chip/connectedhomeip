/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This is a unit test suite for <tt>chip::System::Timer</tt>,
 *      the part of the CHIP System Layer that implements timers.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <system/SystemConfig.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <system/SystemError.h>
#include <system/SystemLayerImpl.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/init.h>
#include <lwip/sys.h>
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include <errno.h>
#include <stdint.h>
#include <string.h>

using chip::ErrorStr;
using namespace chip::System;

template <class LayerImpl, typename Enable = void>
class LayerEvents
{
public:
    static bool HasServiceEvents() { return false; }
    static void ServiceEvents(Layer & aLayer) {}
};

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

template <class LayerImpl>
class LayerEvents<LayerImpl, typename std::enable_if<std::is_base_of<LayerSocketsLoop, LayerImpl>::value>::type>
{
public:
    static bool HasServiceEvents() { return true; }
    static void ServiceEvents(Layer & aLayer)
    {
        LayerSocketsLoop & layer = static_cast<LayerSocketsLoop &>(aLayer);
        layer.PrepareEvents();
        layer.WaitForEvents();
        layer.HandleEvents();
    }
};

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_LWIP

template <class LayerImpl>
class LayerEvents<LayerImpl, typename std::enable_if<std::is_base_of<LayerImplFreeRTOS, LayerImpl>::value>::type>
{
public:
    static bool HasServiceEvents() { return true; }
    static void ServiceEvents(Layer & aLayer)
    {
        LayerImplFreeRTOS & layer = static_cast<LayerImplFreeRTOS &>(aLayer);
        if (layer.IsInitialized())
        {
            layer.HandlePlatformTimer();
        }
    }
};

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

// Test input vector format.
static const uint32_t MAX_NUM_TIMERS = 1000;

class TestContext
{
public:
    Layer * mLayer;
    nlTestSuite * mTestSuite;
    chip::Callback::Callback<> mGreedyTimer; // for greedy timer
    uint32_t mNumTimersHandled;

    void GreedyTimer()
    {
        NL_TEST_ASSERT(mTestSuite, mNumTimersHandled < MAX_NUM_TIMERS);

        if (mNumTimersHandled >= MAX_NUM_TIMERS)
        {
            return;
        }

        mNumTimersHandled++;
    }
    static void GreedyTimer(void * p)
    {
        TestContext * lContext = static_cast<TestContext *>(p);
        lContext->GreedyTimer();
    }

    TestContext() : mGreedyTimer(GreedyTimer, this), mNumTimersHandled(0) {}
};

static TestContext * gCurrentTestContext = nullptr;

class ScopedGlobalTestContext
{
public:
    ScopedGlobalTestContext(TestContext * ctx) { gCurrentTestContext = ctx; }
    ~ScopedGlobalTestContext() { gCurrentTestContext = nullptr; }
};

// Test input data.

static volatile bool sOverflowTestDone;

void TimerFailed(void * aState)
{
    TestContext * lContext = static_cast<TestContext *>(aState);
    NL_TEST_ASSERT(lContext->mTestSuite, false);
    sOverflowTestDone = true;
}

void HandleTimerFailed(Layer * systemLayer, void * aState)
{
    (void) systemLayer;
    TimerFailed(aState);
}

void HandleTimer10Success(Layer * systemLayer, void * aState)
{
    TestContext & lContext = *static_cast<TestContext *>(aState);
    NL_TEST_ASSERT(lContext.mTestSuite, true);
    sOverflowTestDone = true;
}

static void CheckOverflow(nlTestSuite * inSuite, void * aContext)
{
    if (!LayerEvents<LayerImpl>::HasServiceEvents())
        return;

    chip::System::Clock::Milliseconds32 timeout_overflow_0ms = chip::System::Clock::Milliseconds32(652835029);
    chip::System::Clock::Milliseconds32 timeout_10ms         = chip::System::Clock::Milliseconds32(10);

    TestContext & lContext = *static_cast<TestContext *>(aContext);
    Layer & lSys           = *lContext.mLayer;

    sOverflowTestDone = false;

    lSys.StartTimer(timeout_overflow_0ms, HandleTimerFailed, aContext);
    lSys.StartTimer(timeout_10ms, HandleTimer10Success, aContext);

    while (!sOverflowTestDone)
    {
        LayerEvents<LayerImpl>::ServiceEvents(lSys);
    }

    lSys.CancelTimer(HandleTimerFailed, aContext);
    // cb  timer is cancelled by destructor
    lSys.CancelTimer(HandleTimer10Success, aContext);
}

void HandleGreedyTimer(Layer * aLayer, void * aState)
{
    static uint32_t sNumTimersHandled = 0;
    TestContext & lContext            = *static_cast<TestContext *>(aState);
    NL_TEST_ASSERT(lContext.mTestSuite, sNumTimersHandled < MAX_NUM_TIMERS);

    if (sNumTimersHandled >= MAX_NUM_TIMERS)
    {
        return;
    }

    aLayer->StartTimer(chip::System::Clock::kZero, HandleGreedyTimer, aState);
    sNumTimersHandled++;
}

static void CheckStarvation(nlTestSuite * inSuite, void * aContext)
{
    if (!LayerEvents<LayerImpl>::HasServiceEvents())
        return;

    TestContext & lContext = *static_cast<TestContext *>(aContext);
    Layer & lSys           = *lContext.mLayer;

    lSys.StartTimer(chip::System::Clock::kZero, HandleGreedyTimer, aContext);

    LayerEvents<LayerImpl>::ServiceEvents(lSys);
}

static void CheckOrder(nlTestSuite * inSuite, void * aContext)
{
    if (!LayerEvents<LayerImpl>::HasServiceEvents())
        return;

    TestContext & testContext = *static_cast<TestContext *>(aContext);
    Layer & systemLayer       = *testContext.mLayer;
    nlTestSuite * const suite = testContext.mTestSuite;

    struct TestState
    {
        void Record(char c)
        {
            size_t n = strlen(record);
            if (n + 1 < sizeof(record))
            {
                record[n++] = c;
                record[n]   = 0;
            }
        }
        static void A(Layer * layer, void * state) { static_cast<TestState *>(state)->Record('A'); }
        static void B(Layer * layer, void * state) { static_cast<TestState *>(state)->Record('B'); }
        static void C(Layer * layer, void * state) { static_cast<TestState *>(state)->Record('C'); }
        static void D(Layer * layer, void * state) { static_cast<TestState *>(state)->Record('D'); }
        char record[5] = { 0 };
    };
    TestState testState;
    NL_TEST_ASSERT(suite, testState.record[0] == 0);

    Clock::ClockBase * const savedClock = &SystemClock();
    Clock::Internal::MockClock mockClock;
    Clock::Internal::SetSystemClockForTesting(&mockClock);

    using namespace Clock::Literals;
    systemLayer.StartTimer(300_ms, TestState::D, &testState);
    systemLayer.StartTimer(100_ms, TestState::B, &testState);
    systemLayer.StartTimer(200_ms, TestState::C, &testState);
    systemLayer.StartTimer(0_ms, TestState::A, &testState);

    LayerEvents<LayerImpl>::ServiceEvents(systemLayer);
    NL_TEST_ASSERT(suite, strcmp(testState.record, "A") == 0);

    mockClock.AdvanceMonotonic(100_ms);
    LayerEvents<LayerImpl>::ServiceEvents(systemLayer);
    NL_TEST_ASSERT(suite, strcmp(testState.record, "AB") == 0);

    mockClock.AdvanceMonotonic(200_ms);
    LayerEvents<LayerImpl>::ServiceEvents(systemLayer);
    NL_TEST_ASSERT(suite, strcmp(testState.record, "ABCD") == 0);

    Clock::Internal::SetSystemClockForTesting(savedClock);
}

static void CheckCancellation(nlTestSuite * inSuite, void * aContext)
{
    if (!LayerEvents<LayerImpl>::HasServiceEvents())
        return;

    TestContext & testContext = *static_cast<TestContext *>(aContext);
    Layer & systemLayer       = *testContext.mLayer;
    nlTestSuite * const suite = testContext.mTestSuite;

    struct TestState
    {
        TestState(Layer & aSystemLayer) : mSystemLayer(aSystemLayer) {}

        void Record(char c)
        {
            size_t n = strlen(record);
            if (n + 1 < sizeof(record))
            {
                record[n++] = c;
                record[n]   = 0;
            }
        }
        static void A(Layer * layer, void * state)
        {
            auto self = static_cast<TestState *>(state);
            self->Record('A');
            self->mSystemLayer.CancelTimer(B, state);
            self->mSystemLayer.CancelTimer(D, state);
        }
        static void B(Layer * layer, void * state) { static_cast<TestState *>(state)->Record('B'); }
        static void C(Layer * layer, void * state)
        {
            auto self = static_cast<TestState *>(state);
            self->Record('C');
            self->mSystemLayer.CancelTimer(E, state);
        }
        static void D(Layer * layer, void * state) { static_cast<TestState *>(state)->Record('D'); }
        static void E(Layer * layer, void * state) { static_cast<TestState *>(state)->Record('E'); }
        char record[6] = { 0 };

        Layer & mSystemLayer;
    };
    TestState testState(systemLayer);
    NL_TEST_ASSERT(suite, testState.record[0] == 0);

    Clock::ClockBase * const savedClock = &SystemClock();
    Clock::Internal::MockClock mockClock;
    Clock::Internal::SetSystemClockForTesting(&mockClock);

    using namespace Clock::Literals;
    systemLayer.StartTimer(0_ms, TestState::A, &testState);
    systemLayer.StartTimer(0_ms, TestState::B, &testState);
    systemLayer.StartTimer(20_ms, TestState::C, &testState);
    systemLayer.StartTimer(30_ms, TestState::D, &testState);
    systemLayer.StartTimer(50_ms, TestState::E, &testState);

    mockClock.AdvanceMonotonic(100_ms);
    LayerEvents<LayerImpl>::ServiceEvents(systemLayer);
    NL_TEST_ASSERT(suite, strcmp(testState.record, "AC") == 0);

    Clock::Internal::SetSystemClockForTesting(savedClock);
}

namespace {

namespace CancelTimerTest {

// A bit lower than maximum system timers just in case, for systems that
// have some form of limit
constexpr unsigned kCancelTimerCount = CHIP_SYSTEM_CONFIG_NUM_TIMERS - 4;
int gCallbackProcessed[kCancelTimerCount];

/// Validates that gCallbackProcessed has valid values (0 or 1)
void ValidateExecutedTimerCounts(nlTestSuite * suite)
{
    for (unsigned i = 0; i < kCancelTimerCount; i++)
    {
        NL_TEST_ASSERT(suite, (gCallbackProcessed[i] == 0) || (gCallbackProcessed[i] == 1));
    }
}

unsigned ExecutedTimerCount()
{
    unsigned count = 0;
    for (unsigned i = 0; i < kCancelTimerCount; i++)
    {
        if (gCallbackProcessed[i] != 0)
        {
            count++;
        }
    }
    return count;
}

void Callback(Layer * layer, void * state)
{
    unsigned idx = static_cast<unsigned>(reinterpret_cast<uintptr_t>(state));
    if (gCallbackProcessed[idx] != 0)
    {
        ChipLogError(Test, "UNEXPECTED EXECUTION at index %u", idx);
    }

    gCallbackProcessed[idx]++;

    if (ExecutedTimerCount() == kCancelTimerCount / 2)
    {
        ChipLogProgress(Test, "Cancelling timers");
        for (unsigned i = 0; i < kCancelTimerCount; i++)
        {
            if (gCallbackProcessed[i] != 0)
            {
                continue;
            }
            ChipLogProgress(Test, "Timer %u is being cancelled", i);
            gCurrentTestContext->mLayer->CancelTimer(Callback, reinterpret_cast<void *>(static_cast<uintptr_t>(i)));
            gCallbackProcessed[i]++; // pretend executed.
        }
    }
}

void Test(nlTestSuite * inSuite, void * aContext)
{
    // Validates that timers can cancel other timers. Generally the test will
    // do the following:
    //   - schedule several timers to start at the same time
    //   - within each timers, after half of them have run, make one timer
    //     cancel all the other ones
    //   - assert that:
    //       - timers will run if scheduled
    //       - once cancelled, timers will NOT run (i.e. a timer can cancel
    //         other timers, even if they are expiring at the same time)
    memset(gCallbackProcessed, 0, sizeof(gCallbackProcessed));

    TestContext & testContext = *static_cast<TestContext *>(aContext);
    ScopedGlobalTestContext testScope(&testContext);

    Layer & systemLayer       = *testContext.mLayer;
    nlTestSuite * const suite = testContext.mTestSuite;

    Clock::ClockBase * const savedClock = &SystemClock();
    Clock::Internal::MockClock mockClock;
    Clock::Internal::SetSystemClockForTesting(&mockClock);
    using namespace Clock::Literals;

    for (unsigned i = 0; i < kCancelTimerCount; i++)
    {
        NL_TEST_ASSERT(
            suite, systemLayer.StartTimer(10_ms, Callback, reinterpret_cast<void *>(static_cast<uintptr_t>(i))) == CHIP_NO_ERROR);
    }

    LayerEvents<LayerImpl>::ServiceEvents(systemLayer);
    ValidateExecutedTimerCounts(suite);
    NL_TEST_ASSERT(suite, ExecutedTimerCount() == 0);

    mockClock.AdvanceMonotonic(20_ms);
    LayerEvents<LayerImpl>::ServiceEvents(systemLayer);

    ValidateExecutedTimerCounts(suite);
    NL_TEST_ASSERT(suite, ExecutedTimerCount() == kCancelTimerCount);

    Clock::Internal::SetSystemClockForTesting(savedClock);
}

} // namespace CancelTimerTest
} // namespace

// Test the implementation helper classes TimerPool, TimerList, and TimerData.
namespace chip {
namespace System {
class TestTimer
{
public:
    static void CheckTimerPool(nlTestSuite * inSuite, void * aContext);
};
} // namespace System
} // namespace chip

void chip::System::TestTimer::CheckTimerPool(nlTestSuite * inSuite, void * aContext)
{
    TestContext & testContext = *static_cast<TestContext *>(aContext);
    Layer & systemLayer       = *testContext.mLayer;
    nlTestSuite * const suite = testContext.mTestSuite;

    using Timer = TimerList::Node;
    struct TestState
    {
        int count = 0;
        static void Increment(Layer * layer, void * state) { ++static_cast<TestState *>(state)->count; }
        static void Reset(Layer * layer, void * state) { static_cast<TestState *>(state)->count = 0; }
    };
    TestState testState;

    using namespace Clock::Literals;
    struct
    {
        Clock::Timestamp awakenTime;
        TimerCompleteCallback onComplete;
        Timer * timer;
    } testTimer[] = {
        { 111_ms, TestState::Increment }, // 0
        { 100_ms, TestState::Increment }, // 1
        { 202_ms, TestState::Reset },     // 2
        { 303_ms, TestState::Increment }, // 3
    };

    TimerPool<Timer> pool;
    NL_TEST_ASSERT(suite, pool.mTimerPool.Allocated() == 0);
    SYSTEM_STATS_RESET(Stats::kSystemLayer_NumTimers);
    SYSTEM_STATS_RESET_HIGH_WATER_MARK_FOR_TESTING(Stats::kSystemLayer_NumTimers);
    NL_TEST_ASSERT(suite, SYSTEM_STATS_TEST_IN_USE(Stats::kSystemLayer_NumTimers, 0));
    NL_TEST_ASSERT(suite, SYSTEM_STATS_TEST_HIGH_WATER_MARK(Stats::kSystemLayer_NumTimers, 0));

    // Test TimerPool::Create() and TimerData accessors.

    for (auto & timer : testTimer)
    {
        timer.timer = pool.Create(systemLayer, timer.awakenTime, timer.onComplete, &testState);
    }
    NL_TEST_ASSERT(suite, SYSTEM_STATS_TEST_IN_USE(Stats::kSystemLayer_NumTimers, 4));

    for (auto & timer : testTimer)
    {
        NL_TEST_ASSERT(suite, timer.timer != nullptr);
        NL_TEST_ASSERT(suite, timer.timer->AwakenTime() == timer.awakenTime);
        NL_TEST_ASSERT(suite, timer.timer->GetCallback().GetOnComplete() == timer.onComplete);
        NL_TEST_ASSERT(suite, timer.timer->GetCallback().GetAppState() == &testState);
        NL_TEST_ASSERT(suite, timer.timer->GetCallback().GetSystemLayer() == &systemLayer);
    }

    // Test TimerList operations.

    TimerList list;
    NL_TEST_ASSERT(suite, list.Remove(nullptr) == nullptr);
    NL_TEST_ASSERT(suite, list.Remove(nullptr, nullptr) == nullptr);
    NL_TEST_ASSERT(suite, list.PopEarliest() == nullptr);
    NL_TEST_ASSERT(suite, list.PopIfEarlier(500_ms) == nullptr);
    NL_TEST_ASSERT(suite, list.Earliest() == nullptr);
    NL_TEST_ASSERT(suite, list.Empty());

    Timer * earliest = list.Add(testTimer[0].timer); // list: () → (0) returns: 0
    NL_TEST_ASSERT(suite, earliest == testTimer[0].timer);
    NL_TEST_ASSERT(suite, list.PopIfEarlier(10_ms) == nullptr);
    NL_TEST_ASSERT(suite, list.Earliest() == testTimer[0].timer);
    NL_TEST_ASSERT(suite, !list.Empty());

    earliest = list.Add(testTimer[1].timer); // list: (0) → (1 0) returns: 1
    NL_TEST_ASSERT(suite, earliest == testTimer[1].timer);
    NL_TEST_ASSERT(suite, list.Earliest() == testTimer[1].timer);

    earliest = list.Add(testTimer[2].timer); // list: (1 0) → (1 0 2) returns: 1
    NL_TEST_ASSERT(suite, earliest == testTimer[1].timer);
    NL_TEST_ASSERT(suite, list.Earliest() == testTimer[1].timer);

    earliest = list.Add(testTimer[3].timer); // list: (1 0 2) → (1 0 2 3) returns: 1
    NL_TEST_ASSERT(suite, earliest == testTimer[1].timer);
    NL_TEST_ASSERT(suite, list.Earliest() == testTimer[1].timer);

    earliest = list.Remove(earliest); // list: (1 0 2 3) → (0 2 3) returns: 0
    NL_TEST_ASSERT(suite, earliest == testTimer[0].timer);
    NL_TEST_ASSERT(suite, list.Earliest() == testTimer[0].timer);

    earliest = list.Remove(TestState::Reset, &testState); // list: (0 2 3) → (0 3) returns: 2
    NL_TEST_ASSERT(suite, earliest == testTimer[2].timer);
    NL_TEST_ASSERT(suite, list.Earliest() == testTimer[0].timer);

    earliest = list.PopEarliest(); // list: (0 3) → (3) returns: 0
    NL_TEST_ASSERT(suite, earliest == testTimer[0].timer);
    NL_TEST_ASSERT(suite, list.Earliest() == testTimer[3].timer);

    earliest = list.PopIfEarlier(10_ms); // list: (3) → (3) returns: nullptr
    NL_TEST_ASSERT(suite, earliest == nullptr);

    earliest = list.PopIfEarlier(500_ms); // list: (3) → () returns: 3
    NL_TEST_ASSERT(suite, earliest == testTimer[3].timer);
    NL_TEST_ASSERT(suite, list.Empty());

    earliest = list.Add(testTimer[3].timer); // list: () → (3) returns: 3
    list.Clear();                            // list: (3) → ()
    NL_TEST_ASSERT(suite, earliest == testTimer[3].timer);
    NL_TEST_ASSERT(suite, list.Empty());

    for (auto & timer : testTimer)
    {
        list.Add(timer.timer);
    }
    TimerList early = list.ExtractEarlier(200_ms); // list: (1 0 2 3) → (2 3) returns: (1 0)
    NL_TEST_ASSERT(suite, list.PopEarliest() == testTimer[2].timer);
    NL_TEST_ASSERT(suite, list.PopEarliest() == testTimer[3].timer);
    NL_TEST_ASSERT(suite, list.PopEarliest() == nullptr);
    NL_TEST_ASSERT(suite, early.PopEarliest() == testTimer[1].timer);
    NL_TEST_ASSERT(suite, early.PopEarliest() == testTimer[0].timer);
    NL_TEST_ASSERT(suite, early.PopEarliest() == nullptr);

    // Test TimerPool::Invoke()
    NL_TEST_ASSERT(suite, testState.count == 0);
    pool.Invoke(testTimer[0].timer);
    testTimer[0].timer = nullptr;
    NL_TEST_ASSERT(suite, testState.count == 1);
    NL_TEST_ASSERT(suite, pool.mTimerPool.Allocated() == 3);
    NL_TEST_ASSERT(suite, SYSTEM_STATS_TEST_IN_USE(Stats::kSystemLayer_NumTimers, 3));

    // Test TimerPool::Release()
    pool.Release(testTimer[1].timer);
    testTimer[1].timer = nullptr;
    NL_TEST_ASSERT(suite, testState.count == 1);
    NL_TEST_ASSERT(suite, pool.mTimerPool.Allocated() == 2);
    NL_TEST_ASSERT(suite, SYSTEM_STATS_TEST_IN_USE(Stats::kSystemLayer_NumTimers, 2));

    pool.ReleaseAll();
    NL_TEST_ASSERT(suite, pool.mTimerPool.Allocated() == 0);
    NL_TEST_ASSERT(suite, SYSTEM_STATS_TEST_IN_USE(Stats::kSystemLayer_NumTimers, 0));
    NL_TEST_ASSERT(suite, SYSTEM_STATS_TEST_HIGH_WATER_MARK(Stats::kSystemLayer_NumTimers, 4));
}

// Test Suite

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Timer::TestOverflow",             CheckOverflow),
    NL_TEST_DEF("Timer::TestTimerStarvation",      CheckStarvation),
    NL_TEST_DEF("Timer::TestTimerOrder",           CheckOrder),
    NL_TEST_DEF("Timer::TestTimerCancellation",    CheckCancellation),
    NL_TEST_DEF("Timer::TestTimerPool",            chip::System::TestTimer::CheckTimerPool),
    NL_TEST_DEF("Timer::TestCancelTimer",          CancelTimerTest::Test),
    NL_TEST_SENTINEL()
};
// clang-format on

static int TestSetup(void * aContext);
static int TestTeardown(void * aContext);

// clang-format off
static nlTestSuite kTheSuite =
{
    "chip-system-timer",
    &sTests[0],
    TestSetup,
    TestTeardown
};
// clang-format on

static LayerImpl sLayer;

/**
 *  Set up the test suite.
 */
static int TestSetup(void * aContext)
{
    TestContext & lContext = *reinterpret_cast<TestContext *>(aContext);

    if (::chip::Platform::MemoryInit() != CHIP_NO_ERROR)
    {
        return FAILURE;
    }

#if CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_VERSION_MAJOR == 2 && LWIP_VERSION_MINOR == 0
    static sys_mbox_t * sLwIPEventQueue = NULL;

    sys_mbox_new(sLwIPEventQueue, 100);
    tcpip_init(NULL, NULL);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_VERSION_MAJOR == 2 && LWIP_VERSION_MINOR == 0

    sLayer.Init();

    lContext.mLayer     = &sLayer;
    lContext.mTestSuite = &kTheSuite;

    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 *  Free memory reserved at TestSetup.
 */
static int TestTeardown(void * aContext)
{
    TestContext & lContext = *reinterpret_cast<TestContext *>(aContext);

    lContext.mLayer->Shutdown();

#if CHIP_SYSTEM_CONFIG_USE_LWIP && (LWIP_VERSION_MAJOR == 2) && (LWIP_VERSION_MINOR == 0)
    tcpip_finish(NULL, NULL);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && (LWIP_VERSION_MAJOR == 2) && (LWIP_VERSION_MINOR == 0)

    ::chip::Platform::MemoryShutdown();
    return (SUCCESS);
}

int TestSystemTimer(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSystemTimer)
