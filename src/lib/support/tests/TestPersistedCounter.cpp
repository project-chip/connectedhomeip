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

#include <map>
#include <string>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nlunit-test.h>

#include <CHIPVersion.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/PersistedCounter.h>
#include <lib/support/UnitTestRegistration.h>
#include <platform/ConfigurationManager.h>
#include <platform/PersistedStorage.h>

namespace {

constexpr size_t kMaxPersistedValueLengthSupported = 256;

std::map<std::string, std::string> sPersistentStore;

FILE * sPersistentStoreFile = nullptr;

} // namespace

namespace chip {
namespace Platform {
namespace PersistedStorage {

static void RemoveEndOfLineSymbol(char * str)
{
    size_t len = strlen(str) - 1;
    if (str[len] == '\n')
        str[len] = '\0';
}

static CHIP_ERROR GetCounterValueFromFile(const char * aKey, uint32_t & aValue)
{
    char key[CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH];
    char value[kMaxPersistedValueLengthSupported];

    rewind(sPersistentStoreFile);

    while (fgets(key, sizeof(key), sPersistentStoreFile) != nullptr)
    {
        RemoveEndOfLineSymbol(key);

        if (strcmp(key, aKey) == 0)
        {
            if (fgets(value, sizeof(value), sPersistentStoreFile) == nullptr)
            {
                return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
            }
            RemoveEndOfLineSymbol(value);

            if (!chip::ArgParser::ParseInt(value, aValue, 0))
                return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
}

static CHIP_ERROR SaveCounterValueToFile(const char * aKey, uint32_t aValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int res;
    char key[CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH];
    char value[kMaxPersistedValueLengthSupported];

    snprintf(value, sizeof(value), "0x%08X\n", aValue);

    rewind(sPersistentStoreFile);

    // Find the stored counter value location in the file.
    while (fgets(key, sizeof(key), sPersistentStoreFile) != nullptr)
    {
        RemoveEndOfLineSymbol(key);

        // If value is found in the file then override it.
        if (strcmp(key, aKey) == 0)
        {
            res = fputs(value, sPersistentStoreFile);
            VerifyOrExit(res != EOF, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

            ExitNow();
        }
    }

    // If value not found in the file then write the counter key and
    // the counter value to the end of the file.
    res = fputs(aKey, sPersistentStoreFile);
    VerifyOrExit(res != EOF, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    res = fputs("\n", sPersistentStoreFile);
    VerifyOrExit(res != EOF, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    res = fputs(value, sPersistentStoreFile);
    VerifyOrExit(res != EOF, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

exit:
    fflush(sPersistentStoreFile);
    return err;
}

CHIP_ERROR Read(const char * aKey, uint32_t & aValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::map<std::string, std::string>::iterator it;

    VerifyOrReturnError(aKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(strlen(aKey) <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH, CHIP_ERROR_INVALID_STRING_LENGTH);

    if (sPersistentStoreFile)
    {
        err = GetCounterValueFromFile(aKey, aValue);
    }
    else
    {
        it = sPersistentStore.find(aKey);
        VerifyOrReturnError(it != sPersistentStore.end(), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

        size_t aValueLength =
            Base64Decode(it->second.c_str(), static_cast<uint16_t>(it->second.length()), reinterpret_cast<uint8_t *>(&aValue));
        VerifyOrReturnError(aValueLength == sizeof(uint32_t), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    }

    return err;
}

CHIP_ERROR Write(const char * aKey, uint32_t aValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(aKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(strlen(aKey) <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH, CHIP_ERROR_INVALID_STRING_LENGTH);

    if (sPersistentStoreFile)
    {
        err = SaveCounterValueToFile(aKey, aValue);
    }
    else
    {
        char encodedValue[BASE64_ENCODED_LEN(sizeof(uint32_t)) + 1];

        memset(encodedValue, 0, sizeof(encodedValue));
        Base64Encode(reinterpret_cast<uint8_t *>(&aValue), sizeof(aValue), encodedValue);

        sPersistentStore[aKey] = encodedValue;
    }

    return err;
}

} // namespace PersistedStorage
} // namespace Platform
} // namespace chip

struct TestPersistedCounterContext
{
    TestPersistedCounterContext();
    bool mVerbose;
};

TestPersistedCounterContext::TestPersistedCounterContext() : mVerbose(false) {}

static void InitializePersistedStorage(TestPersistedCounterContext * context)
{
    sPersistentStore.clear();
    chip::DeviceLayer::SetConfigurationMgr(&chip::DeviceLayer::ConfigurationManagerImpl::GetDefaultInstance());
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
    char testValue[kMaxPersistedValueLengthSupported];
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
    char testValue[kMaxPersistedValueLengthSupported];
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
    char testValue[kMaxPersistedValueLengthSupported];
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
