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

#include "TestSupport.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nlunit-test.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Platform;

// =================================
//      Unit tests
// =================================

static void TestMemAlloc_Malloc(nlTestSuite * inSuite, void * inContext)
{
    char * p1 = NULL;
    char * p2 = NULL;
    char * p3 = NULL;

    // Verify long-term allocation
    p1 = (char *) MemoryAlloc(64, true);
    NL_TEST_ASSERT(inSuite, p1 != NULL);

    p2 = (char *) MemoryAlloc(256, true);
    NL_TEST_ASSERT(inSuite, p2 != NULL);

    chip::Platform::MemoryFree(p1);
    chip::Platform::MemoryFree(p2);

    // Verify short-term allocation
    p1 = (char *) MemoryAlloc(256);
    NL_TEST_ASSERT(inSuite, p1 != NULL);

    p2 = (char *) MemoryAlloc(256);
    NL_TEST_ASSERT(inSuite, p2 != NULL);

    p3 = (char *) MemoryAlloc(256);
    NL_TEST_ASSERT(inSuite, p3 != NULL);

    chip::Platform::MemoryFree(p1);
    chip::Platform::MemoryFree(p2);
    chip::Platform::MemoryFree(p3);
}

static void TestMemAlloc_Calloc(nlTestSuite * inSuite, void * inContext)
{
    char * p = (char *) MemoryCalloc(128, true);
    NL_TEST_ASSERT(inSuite, p != NULL);

    for (int i = 0; i < 128; i++)
        NL_TEST_ASSERT(inSuite, p[i] == 0);

    chip::Platform::MemoryFree(p);
}

static void TestMemAlloc_Realloc(nlTestSuite * inSuite, void * inContext)
{
    char * pa = (char *) MemoryAlloc(128, true);
    NL_TEST_ASSERT(inSuite, pa != NULL);

    char * pb = (char *) MemoryRealloc(pa, 256);
    NL_TEST_ASSERT(inSuite, pb != NULL);

    chip::Platform::MemoryFree(pb);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("Test MemAlloc::Malloc", TestMemAlloc_Malloc),
                                 NL_TEST_DEF("Test MemAlloc::Calloc", TestMemAlloc_Calloc),
                                 NL_TEST_DEF("Test MemAlloc::Realloc", TestMemAlloc_Realloc), NL_TEST_SENTINEL() };

int TestMemAlloc(void)
{
    nlTestSuite theSuite = { "CHIP Memory Allocation tests", &sTests[0], NULL, NULL };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
