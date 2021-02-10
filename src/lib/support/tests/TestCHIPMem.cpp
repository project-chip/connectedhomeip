/* See Project CHIP LICENSE file for licensing information. */

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

#include <nlunit-test.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

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
    p1 = static_cast<char *>(MemoryAlloc(64, true));
    NL_TEST_ASSERT(inSuite, p1 != nullptr);

    p2 = static_cast<char *>(MemoryAlloc(256, true));
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
    char * pa = static_cast<char *>(MemoryAlloc(128, true));
    NL_TEST_ASSERT(inSuite, pa != nullptr);

    char * pb = static_cast<char *>(MemoryRealloc(pa, 256));
    NL_TEST_ASSERT(inSuite, pb != nullptr);

    chip::Platform::MemoryFree(pb);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("Test MemAlloc::Malloc", TestMemAlloc_Malloc),
                                 NL_TEST_DEF("Test MemAlloc::Calloc", TestMemAlloc_Calloc),
                                 NL_TEST_DEF("Test MemAlloc::Realloc", TestMemAlloc_Realloc), NL_TEST_SENTINEL() };

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
