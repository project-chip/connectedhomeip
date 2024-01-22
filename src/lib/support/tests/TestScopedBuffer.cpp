/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/support/ScopedBuffer.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

class TestCounterMemoryManagement
{
public:
    static int Counter() { return mAllocCount; }

    static void MemoryFree(void * p)
    {
        mAllocCount--;
        chip::Platform::MemoryFree(p);
    }
    static void * MemoryAlloc(size_t size)
    {
        mAllocCount++;
        return chip::Platform::MemoryAlloc(size);
    }
    static void * MemoryCalloc(size_t num, size_t size)
    {

        mAllocCount++;
        return chip::Platform::MemoryCalloc(num, size);
    }

private:
    static int mAllocCount;
};
int TestCounterMemoryManagement::mAllocCount = 0;

using TestCounterScopedBuffer = chip::Platform::ScopedMemoryBuffer<char, TestCounterMemoryManagement>;

void TestAutoFree(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 0);

    {
        TestCounterScopedBuffer buffer;

        NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 0);
        NL_TEST_ASSERT(inSuite, buffer.Alloc(128));
        NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 1);
    }
    NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 0);
}

void TestFreeDuringAllocs(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 0);

    {
        TestCounterScopedBuffer buffer;

        NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 0);
        NL_TEST_ASSERT(inSuite, buffer.Alloc(128));
        NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 1);
        NL_TEST_ASSERT(inSuite, buffer.Alloc(64));
        NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 1);
        NL_TEST_ASSERT(inSuite, buffer.Calloc(10));
        NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 1);
    }
    NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 0);
}

void TestRelease(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 0);
    void * ptr = nullptr;

    {
        TestCounterScopedBuffer buffer;

        NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 0);
        NL_TEST_ASSERT(inSuite, buffer.Alloc(128));
        NL_TEST_ASSERT(inSuite, buffer.Get() != nullptr);

        ptr = buffer.Release();
        NL_TEST_ASSERT(inSuite, ptr != nullptr);
        NL_TEST_ASSERT(inSuite, buffer.Get() == nullptr);
    }

    NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 1);

    {
        TestCounterScopedBuffer buffer;
        NL_TEST_ASSERT(inSuite, buffer.Alloc(128));
        NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 2);
        TestCounterMemoryManagement::MemoryFree(ptr);
        NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 1);
    }

    NL_TEST_ASSERT(inSuite, TestCounterMemoryManagement::Counter() == 0);
}

int Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

int Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF_FN(TestAutoFree),         //
    NL_TEST_DEF_FN(TestFreeDuringAllocs), //
    NL_TEST_DEF_FN(TestRelease),          //
    NL_TEST_SENTINEL()                    //
};

int TestScopedBuffer()
{
    nlTestSuite theSuite = { "CHIP ScopedBuffer tests", &sTests[0], Setup, Teardown };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestScopedBuffer)
