/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      Unit tests for the Chip Persisted Storage API.
 *
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nlunit-test.h>

#include <CHIPVersion.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/PersistedCounter.h>
#include <lib/support/UnitTestRegistration.h>
#include <platform/PersistedStorage.h>

#include "TestPersistedStorageImplementation.h"

struct TestPersistedCounterContext
{
    TestPersistedCounterContext();
    bool mVerbose;
};

TestPersistedCounterContext::TestPersistedCounterContext() : mVerbose(false) {}

static void InitializePersistedStorage(TestPersistedCounterContext * context)
{
    sPersistentStore.clear();
}

static int TestSetup(void * inContext)
{
    return SUCCESS;
}

static int TestTeardown(void * inContext)
{
    sPersistentStore.clear();
    return SUCCESS;
}

static void CheckOOB(nlTestSuite * inSuite, void * inContext)
{
    TestPersistedCounterContext * context = static_cast<TestPersistedCounterContext *>(inContext);
    CHIP_ERROR err                        = CHIP_NO_ERROR;
    chip::PersistedCounter counter;
    const char * testKey = "testcounter";
    char testValue[CHIP_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH];
    uint64_t value = 0;

    memset(testValue, 0, sizeof(testValue));

    InitializePersistedStorage(context);

    // When initializing the first time out of the box, we should have
    // a count of 0 and a value of 0x10000 for the next starting value
    // in persistent storage.

    err = counter.Init(testKey, 0x10000);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    value = counter.GetValue();
    NL_TEST_ASSERT(inSuite, value == 0);
}

static void CheckReboot(nlTestSuite * inSuite, void * inContext)
{
    TestPersistedCounterContext * context = static_cast<TestPersistedCounterContext *>(inContext);
    CHIP_ERROR err                        = CHIP_NO_ERROR;
    chip::PersistedCounter counter, counter2;
    const char * testKey = "testcounter";
    char testValue[CHIP_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH];
    uint64_t value = 0;

    memset(testValue, 0, sizeof(testValue));

    InitializePersistedStorage(context);

    // When initializing the first time out of the box, we should have
    // a count of 0.

    err = counter.Init(testKey, 0x10000);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    value = counter.GetValue();
    NL_TEST_ASSERT(inSuite, value == 0);

    // Now we "reboot", and we should get a count of 0x10000.

    err = counter2.Init(testKey, 0x10000);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    value = counter2.GetValue();
    NL_TEST_ASSERT(inSuite, value == 0x10000);
}

static void CheckWriteNextCounterStart(nlTestSuite * inSuite, void * inContext)
{
    TestPersistedCounterContext * context = static_cast<TestPersistedCounterContext *>(inContext);
    CHIP_ERROR err                        = CHIP_NO_ERROR;
    chip::PersistedCounter counter;
    const char * testKey = "testcounter";
    char testValue[CHIP_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH];
    uint64_t value = 0;

    memset(testValue, 0, sizeof(testValue));

    InitializePersistedStorage(context);

    // When initializing the first time out of the box, we should have
    // a count of 0.

    err = counter.Init(testKey, 0x10000);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    value = counter.GetValue();
    NL_TEST_ASSERT(inSuite, value == 0);

    // Verify that we write out the next starting counter value after
    // we've exhausted the counter's range.

    for (int32_t i = 0; i < 0x10000; i++)
    {
        err = counter.Advance();
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    value = counter.GetValue();
    NL_TEST_ASSERT(inSuite, value == 0x10000);

    for (int32_t i = 0; i < 0x10000; i++)
    {
        err = counter.Advance();
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    value = counter.GetValue();
    NL_TEST_ASSERT(inSuite, value == 0x20000);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("Out of box Test", CheckOOB),                                 //
    NL_TEST_DEF("Reboot Test", CheckReboot),                                  //
    NL_TEST_DEF("Write Next Counter Start Test", CheckWriteNextCounterStart), //
    NL_TEST_SENTINEL()                                                        //
};

int TestPersistedCounter()
{
    TestPersistedCounterContext context;

    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
    {
        return EXIT_FAILURE;
    }

    nlTestSuite theSuite = { "chip-persisted-storage", &sTests[0], TestSetup, TestTeardown };

    // Run test suite against one context
    nlTestRunner(&theSuite, &context);

    int r = nlTestRunnerStats(&theSuite);

    chip::Platform::MemoryShutdown();

    return r;
}

CHIP_REGISTER_TEST_SUITE(TestPersistedCounter);
