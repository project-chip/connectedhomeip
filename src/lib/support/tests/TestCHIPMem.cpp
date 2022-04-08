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

/**
 *    @file
 *      This file implements a unit test suite for CHIP Memory Management
 *      code functionality.
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Platform;

// =================================
//      Unit tests
// =================================

static void TestMemAlloc_Malloc(nlTestSuite * inSuite, void * inContext)
{
    char * p1 = nullptr;
    char * p2 = nullptr;
    char * p3 = nullptr;

    // Verify long-term allocation
    p1 = static_cast<char *>(MemoryAlloc(64));
    NL_TEST_ASSERT(inSuite, p1 != nullptr);

    p2 = static_cast<char *>(MemoryAlloc(256));
    NL_TEST_ASSERT(inSuite, p2 != nullptr);

    chip::Platform::MemoryFree(p1);
    chip::Platform::MemoryFree(p2);

    // Verify short-term allocation
    p1 = static_cast<char *>(MemoryAlloc(256));
    NL_TEST_ASSERT(inSuite, p1 != nullptr);

    p2 = static_cast<char *>(MemoryAlloc(256));
    NL_TEST_ASSERT(inSuite, p2 != nullptr);

    p3 = static_cast<char *>(MemoryAlloc(256));
    NL_TEST_ASSERT(inSuite, p3 != nullptr);

    chip::Platform::MemoryFree(p1);
    chip::Platform::MemoryFree(p2);
    chip::Platform::MemoryFree(p3);
}

static void TestMemAlloc_Calloc(nlTestSuite * inSuite, void * inContext)
{
    char * p = static_cast<char *>(MemoryCalloc(128, true));
    NL_TEST_ASSERT(inSuite, p != nullptr);

    for (int i = 0; i < 128; i++)
        NL_TEST_ASSERT(inSuite, p[i] == 0);

    chip::Platform::MemoryFree(p);
}

static void TestMemAlloc_Realloc(nlTestSuite * inSuite, void * inContext)
{
    char * pa = static_cast<char *>(MemoryAlloc(128));
    NL_TEST_ASSERT(inSuite, pa != nullptr);

    char * pb = static_cast<char *>(MemoryRealloc(pa, 256));
    NL_TEST_ASSERT(inSuite, pb != nullptr);

    chip::Platform::MemoryFree(pb);
}

static void TestMemAlloc_UniquePtr(nlTestSuite * inSuite, void * inContext)
{
    // UniquePtr is a wrapper of std::unique_ptr for platform allocators, we just check if we created a correct wrapper here.
    int constructorCalled = 0;
    int destructorCalled  = 0;

    class Cls
    {
    public:
        Cls(int * constructCtr, int * desctructorCtr) : mpDestructorCtr(desctructorCtr) { (*constructCtr)++; }
        ~Cls() { (*mpDestructorCtr)++; }

    private:
        int * mpDestructorCtr;
    };

    {
        auto ptr = MakeUnique<Cls>(&constructorCalled, &destructorCalled);
        NL_TEST_ASSERT(inSuite, constructorCalled == 1);
        NL_TEST_ASSERT(inSuite, destructorCalled == 0);
        IgnoreUnusedVariable(ptr);
    }

    NL_TEST_ASSERT(inSuite, destructorCalled == 1);
}

static void TestMemAlloc_SharedPtr(nlTestSuite * inSuite, void * inContext)
{
    // SharedPtr is a wrapper of std::shared_ptr for platform allocators.
    int instanceConstructorCalled      = 0;
    int instanceDestructorCalled       = 0;
    int otherInstanceConstructorCalled = 0;
    int otherInstanceDestructorCalled  = 0;

    class Cls
    {
    public:
        Cls(int * constructCtr, int * desctructorCtr) : mpDestructorCtr(desctructorCtr) { (*constructCtr)++; }
        ~Cls() { (*mpDestructorCtr)++; }

    private:
        int * mpDestructorCtr;
    };

    // Check constructor call for a block-scoped variable and share our
    // reference to a function-scoped variable.
    SharedPtr<Cls> otherReference;
    {
        auto ptr = MakeShared<Cls>(&instanceConstructorCalled, &instanceDestructorCalled);
        NL_TEST_ASSERT(inSuite, instanceConstructorCalled == 1);
        // Capture a shared reference so we aren't destructed when we leave this scope.
        otherReference = ptr;
    }

    // Verify that by sharing to otherReference, we weren't destructed.
    NL_TEST_ASSERT(inSuite, instanceDestructorCalled == 0);

    // Now drop our reference.
    otherReference = MakeShared<Cls>(&otherInstanceConstructorCalled, &otherInstanceDestructorCalled);

    // Verify that the new instance was constructed and the first instance was
    // destructed, and that we retain a reference to the new instance.
    NL_TEST_ASSERT(inSuite, instanceConstructorCalled == 1);
    NL_TEST_ASSERT(inSuite, instanceDestructorCalled == 1);
    NL_TEST_ASSERT(inSuite, otherInstanceConstructorCalled == 1);
    NL_TEST_ASSERT(inSuite, otherInstanceDestructorCalled == 0);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("Test MemAlloc::Malloc", TestMemAlloc_Malloc),
                                 NL_TEST_DEF("Test MemAlloc::Calloc", TestMemAlloc_Calloc),
                                 NL_TEST_DEF("Test MemAlloc::Realloc", TestMemAlloc_Realloc),
                                 NL_TEST_DEF("Test MemAlloc::UniquePtr", TestMemAlloc_UniquePtr),
                                 NL_TEST_DEF("Test MemAlloc::SharedPtr", TestMemAlloc_SharedPtr),
                                 NL_TEST_SENTINEL() };

/**
 *  Set up the test suite.
 */
int TestMemAlloc_Setup(void * inContext)
{
    CHIP_ERROR error = MemoryInit();
    if (error != CHIP_NO_ERROR)
        return (FAILURE);
    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 */
int TestMemAlloc_Teardown(void * inContext)
{
    MemoryShutdown();
    return (SUCCESS);
}

int TestMemAlloc()
{
    nlTestSuite theSuite = { "CHIP Memory Allocation tests", &sTests[0], TestMemAlloc_Setup, TestMemAlloc_Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestMemAlloc)
