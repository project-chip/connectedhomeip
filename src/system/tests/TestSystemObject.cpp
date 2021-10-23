/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This is a unit test suite for <tt>chip::System::Object</tt>, *
 *      the part of the CHIP System Layer that implements objects and
 *      their static allocation pools.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

// Install a sleep in the high water mark function, to force
// collisions between the threads that call it.

// clang-format off
#include <lib/support/UnitTestUtils.h>
#define SYSTEM_OBJECT_HWM_TEST_HOOK() do { chip::test_utils::SleepMillis(1); } while(0)
// clang-format on

#include <system/SystemLayer.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/init.h>
#include <lwip/tcpip.h>
#include <lwip/sys.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <pthread.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <type_traits>

// Test context
using namespace chip::System;

namespace chip {
namespace System {

static int Initialize(void * aContext);
static int Finalize(void * aContext);

class TestObject : public Object
{
public:
    CHIP_ERROR Init();

    static void CheckIteration(nlTestSuite * inSuite, void * aContext);
    static void CheckRetention(nlTestSuite * inSuite, void * aContext);
    static void CheckConcurrency(nlTestSuite * inSuite, void * aContext);
#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    static void CheckHighWatermark(nlTestSuite * inSuite, void * aContext);
    static void CheckHighWatermarkConcurrency(nlTestSuite * inSuite, void * aContext);
#endif

private:
    static constexpr int kNumThreads         = 16;
    static constexpr int kLoopIterations     = 1000;
    static constexpr int kMaxDelayIterations = 3;
    static constexpr int kPoolSize           = (kNumThreads * 7) + 1;

    static_assert(kNumThreads > 1, "kNumThreads should be more than 1");
    static_assert(kPoolSize < CHIP_SYS_STATS_COUNT_MAX, "kPoolSize is not less than CHIP_SYS_STATS_COUNT_MAX");

    static ObjectPool<TestObject, kPoolSize> sPool;

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    unsigned int mDelay;

    void Delay(volatile unsigned int & aAccumulator);
    static void * CheckConcurrencyThread(void * aContext);
#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    static void * CheckHighWatermarkThread(void * aContext);
#endif
    static void MultithreadedTest(nlTestSuite * inSuite, void * aContext, void * (*aStartRoutine)(void *) );
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    // Not defined
    TestObject & operator=(const TestObject &) = delete;
};

ObjectPool<TestObject, TestObject::kPoolSize> TestObject::sPool;

CHIP_ERROR TestObject::Init()
{
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    this->mDelay = kMaxDelayIterations > 0 ? 1 : 0;
    if (kMaxDelayIterations > 1)
    {
        this->mDelay += static_cast<unsigned int>(rand() % kMaxDelayIterations);
    }
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    return CHIP_NO_ERROR;
}

namespace {
struct TestContext
{
    nlTestSuite * mTestSuite;
    volatile unsigned int mAccumulator;
};

struct TestContext sContext;
} // namespace

// Test Object Iteration

void TestObject::CheckIteration(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    unsigned int i;

    sPool.Reset();

    for (i = 0; i < kPoolSize; ++i)
    {
        TestObject * lCreated = sPool.TryCreate();

        NL_TEST_ASSERT(lContext.mTestSuite, lCreated != nullptr);
        lCreated->Init();
    }

    i = 0;
    sPool.ForEachActiveObject([&](TestObject * lCreated) {
        NL_TEST_ASSERT(lContext.mTestSuite, lCreated->IsRetained());
        i++;
        return true;
    });
    NL_TEST_ASSERT(lContext.mTestSuite, i == kPoolSize);

    i = 0;
    sPool.ForEachActiveObject([&](TestObject * lCreated) {
        i++;
        if (i == kPoolSize / 2)
            return false;

        return true;
    });
    NL_TEST_ASSERT(lContext.mTestSuite, i == kPoolSize / 2);
}

// Test Object retention

void TestObject::CheckRetention(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    unsigned int i;

    sPool.Reset();

    for (i = 0; i < kPoolSize; ++i)
    {
        TestObject * lCreated = sPool.TryCreate();

        NL_TEST_ASSERT(lContext.mTestSuite, lCreated != nullptr);

        lCreated->Init();
    }

    i = 0;
    TestObject::sPool.ForEachActiveObject([&](TestObject * lGotten) {
        lGotten->Retain();
        i++;
        return true;
    });
    NL_TEST_ASSERT(lContext.mTestSuite, i == kPoolSize);

    i = 0;
    TestObject::sPool.ForEachActiveObject([&](TestObject * lGotten) {
        lGotten->Release();
        i++;
        return true;
    });
    NL_TEST_ASSERT(lContext.mTestSuite, i == kPoolSize);
}

// Test Object concurrency

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
void TestObject::Delay(volatile unsigned int & aAccumulator)
{
    unsigned int lSum = 0;

    if (kMaxDelayIterations > 0)
    {
        for (unsigned int z = 0; z < this->mDelay; ++z)
        {
            lSum += static_cast<unsigned int>(rand());
        }

        lSum = lSum / this->mDelay;
    }

    aAccumulator = lSum;
}

void * TestObject::CheckConcurrencyThread(void * aContext)
{
    const unsigned int kNumObjects = kPoolSize / kNumThreads;
    TestObject * lObject           = nullptr;
    TestContext & lContext         = *static_cast<TestContext *>(aContext);
    unsigned int i;

    // Take this thread's share of objects

    for (i = 0; i < kNumObjects; ++i)
    {
        lObject = nullptr;
        while (lObject == nullptr)
        {
            lObject = sPool.TryCreate();
        }

        NL_TEST_ASSERT(lContext.mTestSuite, lObject->IsRetained());

        lObject->Init();
        lObject->Delay(lContext.mAccumulator);
    }

    // For each iteration, take one more object, and free it form the pool

    for (i = 0; i < kLoopIterations; ++i)
    {
        lObject = nullptr;
        while (lObject == nullptr)
        {
            lObject = sPool.TryCreate();
        }

        NL_TEST_ASSERT(lContext.mTestSuite, lObject->IsRetained());

        lObject->Init();
        lObject->Delay(lContext.mAccumulator);

        lObject->Release();
    }

    return aContext;
}

#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
void * TestObject::CheckHighWatermarkThread(void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    int i;
    chip::System::Stats::count_t lNumInUse;
    chip::System::Stats::count_t lHighWatermark;

    i = (rand() % CHIP_SYS_STATS_COUNT_MAX);

    sPool.UpdateHighWatermark(static_cast<unsigned int>(i));

    sPool.GetStatistics(lNumInUse, lHighWatermark);

    NL_TEST_ASSERT(lContext.mTestSuite, lHighWatermark >= i);
    if (lHighWatermark < i)
    {
        printf("hwm: %d, i: %d\n", lHighWatermark, i);
    }

    return aContext;
}
#endif

void TestObject::MultithreadedTest(nlTestSuite * inSuite, void * aContext, void * (*aStartRoutine)(void *) )
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    pthread_t lThread[kNumThreads];

    sPool.Reset();

    for (unsigned int i = 0; i < kNumThreads; ++i)
    {
        int lError = pthread_create(&lThread[i], nullptr, aStartRoutine, &lContext);

        NL_TEST_ASSERT(lContext.mTestSuite, lError == 0);
    }

    for (pthread_t thread : lThread)
    {
        int lError = pthread_join(thread, nullptr);

        NL_TEST_ASSERT(lContext.mTestSuite, lError == 0);
    }
}
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

void TestObject::CheckConcurrency(nlTestSuite * inSuite, void * aContext)
{
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    MultithreadedTest(inSuite, aContext, CheckConcurrencyThread);
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
}

#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
void TestObject::CheckHighWatermarkConcurrency(nlTestSuite * inSuite, void * aContext)
{
    sPool.Reset();

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    for (unsigned int i = 0; i < 100; i++)
    {
        MultithreadedTest(inSuite, aContext, CheckHighWatermarkThread);
    }
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
}

void TestObject::CheckHighWatermark(nlTestSuite * inSuite, void * aContext)
{
    sPool.Reset();

    const int kNumObjects  = kPoolSize;
    TestObject * lObject   = nullptr;
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    chip::System::Stats::count_t lNumInUse;
    chip::System::Stats::count_t lHighWatermark;

    // Take all objects one at a time and check the watermark
    // increases monotonically
    for (int i = 0; i < kNumObjects; ++i)
    {
        lObject = sPool.TryCreate();

        NL_TEST_ASSERT(lContext.mTestSuite, lObject->IsRetained());

        sPool.GetStatistics(lNumInUse, lHighWatermark);
        NL_TEST_ASSERT(lContext.mTestSuite, lNumInUse == (i + 1));
        NL_TEST_ASSERT(lContext.mTestSuite, lHighWatermark == lNumInUse);

        lObject->Init();
    }

    // Fail an allocation and check that both stats don't change
    NL_TEST_ASSERT(lContext.mTestSuite, sPool.TryCreate() == nullptr);

    sPool.GetStatistics(lNumInUse, lHighWatermark);
    NL_TEST_ASSERT(lContext.mTestSuite, lNumInUse == kNumObjects);
    NL_TEST_ASSERT(lContext.mTestSuite, lHighWatermark == kNumObjects);

    // Free the last object and check that the watermark does not
    // change.
    lObject->Release();
    NL_TEST_ASSERT(lContext.mTestSuite, !lObject->IsRetained());

    sPool.GetStatistics(lNumInUse, lHighWatermark);
    NL_TEST_ASSERT(lContext.mTestSuite, lNumInUse == (kNumObjects - 1));
    NL_TEST_ASSERT(lContext.mTestSuite, lHighWatermark == kNumObjects);
}
#endif // !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

// Test Suite

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Iteration",                TestObject::CheckIteration),
    NL_TEST_DEF("Retention",                TestObject::CheckRetention),
    NL_TEST_DEF("Concurrency",              TestObject::CheckConcurrency),
#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    NL_TEST_DEF("HighWatermark",            TestObject::CheckHighWatermark),
    NL_TEST_DEF("HighWatermarkConcurrency", TestObject::CheckHighWatermarkConcurrency),
#endif
	NL_TEST_SENTINEL()
};

static nlTestSuite sTestSuite =
{
    "chip-system-object",
    &sTests[0],
    Initialize,
    Finalize
};
// clang-format on

/**
 *  Initialize the test suite.
 */
static int Initialize(void * aContext)
{
    TestContext & lContext = *reinterpret_cast<TestContext *>(aContext);

#if CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_VERSION_MAJOR <= 2 && LWIP_VERSION_MINOR < 1
    static sys_mbox_t * sLwIPEventQueue = NULL;

    if (sLwIPEventQueue == NULL)
    {
        sys_mbox_new(sLwIPEventQueue, 100);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    lContext.mTestSuite   = &sTestSuite;
    lContext.mAccumulator = 0;

    return SUCCESS;
}

/**
 *  Finalize the test suite.
 */
static int Finalize(void * aContext)
{
    TestContext & lContext = *reinterpret_cast<TestContext *>(aContext);

    lContext.mTestSuite = nullptr;

    return SUCCESS;
}

} // namespace System
} // namespace chip

int TestSystemObject(void)
{
    // Initialize standard pseudo-random number generator
    srand(0);

    // Run test suit againt one lContext.
    nlTestRunner(&sTestSuite, &chip::System::sContext);

    return nlTestRunnerStats(&sTestSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSystemObject)
