/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <protocols/secure_channel/SessionIDAllocator.h>

#include <nlunit-test.h>

using namespace chip;

void TestSessionIDAllocator_Allocate(nlTestSuite * inSuite, void * inContext)
{
    SessionIDAllocator allocator;

    NL_TEST_ASSERT(inSuite, allocator.Peek() == 0);

    uint16_t id;

    for (uint16_t i = 0; i < 16; i++)
    {
        CHIP_ERROR err = allocator.Allocate(id);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, id == i);
        NL_TEST_ASSERT(inSuite, allocator.Peek() == i + 1);
    }
}

void TestSessionIDAllocator_Free(nlTestSuite * inSuite, void * inContext)
{
    SessionIDAllocator allocator;

    NL_TEST_ASSERT(inSuite, allocator.Peek() == 0);

    uint16_t id;

    for (uint16_t i = 0; i < 16; i++)
    {
        CHIP_ERROR err = allocator.Allocate(id);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, id == i);
        NL_TEST_ASSERT(inSuite, allocator.Peek() == i + 1);
    }

    // Free an intermediate ID
    allocator.Free(10);
    NL_TEST_ASSERT(inSuite, allocator.Peek() == 16);

    // Free the last allocated ID
    allocator.Free(15);
    NL_TEST_ASSERT(inSuite, allocator.Peek() == 15);

    // Free some random unallocated ID
    allocator.Free(100);
    NL_TEST_ASSERT(inSuite, allocator.Peek() == 15);
}

void TestSessionIDAllocator_Reserve(nlTestSuite * inSuite, void * inContext)
{
    SessionIDAllocator allocator;

    uint16_t id;

    for (uint16_t i = 0; i < 16; i++)
    {
        CHIP_ERROR err = allocator.Allocate(id);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, id == i);
        NL_TEST_ASSERT(inSuite, allocator.Peek() == i + 1);
    }

    allocator.Reserve(100);
    NL_TEST_ASSERT(inSuite, allocator.Peek() == 101);
}

void TestSessionIDAllocator_ReserveUpTo(nlTestSuite * inSuite, void * inContext)
{
    SessionIDAllocator allocator;

    allocator.ReserveUpTo(100);
    NL_TEST_ASSERT(inSuite, allocator.Peek() == 101);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("SessionIDAllocator_Allocate", TestSessionIDAllocator_Allocate),
    NL_TEST_DEF("SessionIDAllocator_Free", TestSessionIDAllocator_Free),
    NL_TEST_DEF("SessionIDAllocator_Reserve", TestSessionIDAllocator_Reserve),
    NL_TEST_DEF("SessionIDAllocator_ReserveUpTo", TestSessionIDAllocator_ReserveUpTo),

    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Set up the test suite.
 */
static int TestSetup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SessionIDAllocator",
    &sTests[0],
    TestSetup,
    TestTeardown,
};
// clang-format on

/**
 *  Main
 */
int TestSessionIDAllocator()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestSessionIDAllocator)
