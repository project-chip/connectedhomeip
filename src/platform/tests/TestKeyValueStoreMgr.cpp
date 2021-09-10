/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements a unit test suite for the Key Value Store Manager
 *      code functionality.
 *
 */

#include <nlunit-test.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::DeviceLayer::PersistedStorage;

static void TestKeyValueStoreMgr_EmptyStringKey(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    const char * kTestKey   = "str_key";
    const char kTestValue[] = "";
    char read_value[sizeof(kTestValue)];
    size_t read_size;
    err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = KeyValueStoreMgr().Get(kTestKey, read_value, sizeof(read_value), &read_size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Verify if read value is the same as wrote one
    NL_TEST_ASSERT(inSuite, strcmp(kTestValue, read_value) == 0);
    NL_TEST_ASSERT(inSuite, read_size == sizeof(kTestValue));
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Try to get deleted key and verify if CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND is returned
    err = KeyValueStoreMgr().Get(kTestKey, read_value, sizeof(read_value), &read_size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static void TestKeyValueStoreMgr_StringKey(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    const char * kTestKey   = "str_key";
    const char kTestValue[] = "test_value";
    char read_value[sizeof(kTestValue)];
    size_t read_size;
    err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = KeyValueStoreMgr().Get(kTestKey, read_value, sizeof(read_value), &read_size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Verify if read value is the same as wrote one
    NL_TEST_ASSERT(inSuite, strcmp(kTestValue, read_value) == 0);
    NL_TEST_ASSERT(inSuite, read_size == sizeof(kTestValue));
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Try to get deleted key and verify if CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND is returned
    err = KeyValueStoreMgr().Get(kTestKey, read_value, sizeof(read_value), &read_size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static void TestKeyValueStoreMgr_Uint32Key(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    const char * kTestKey = "uint32_key";
    const char kTestValue = 5;
    uint32_t read_value;
    err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = KeyValueStoreMgr().Get(kTestKey, &read_value);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Verify if read value is the same as wrote one
    NL_TEST_ASSERT(inSuite, kTestValue == read_value);
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Try to get deleted key and verify if CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND is returned
    err = KeyValueStoreMgr().Get(kTestKey, &read_value);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static void TestKeyValueStoreMgr_ArrayKey(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    const char * kTestKey  = "array_key";
    uint32_t kTestValue[5] = { 1, 2, 3, 4, 5 };
    uint32_t read_value[5];
    size_t read_size;
    err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = KeyValueStoreMgr().Get(kTestKey, read_value, sizeof(read_value), &read_size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Verify if read value is the same as wrote one
    NL_TEST_ASSERT(inSuite, memcmp(kTestValue, read_value, sizeof(kTestValue)) == 0);
    NL_TEST_ASSERT(inSuite, read_size == sizeof(kTestValue));
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Try to get deleted key and verify if CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND is returned
    err = KeyValueStoreMgr().Get(kTestKey, read_value, sizeof(read_value), &read_size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static void TestKeyValueStoreMgr_StructKey(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    struct TestStruct
    {
        uint8_t value1;
        uint32_t value2;
    };
    const char * kTestKey = "struct_key";
    TestStruct kTestValue{ 1, 2 };
    TestStruct read_value;
    size_t read_size;
    err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = KeyValueStoreMgr().Get(kTestKey, &read_value, sizeof(read_value), &read_size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Verify if read value is the same as wrote one
    NL_TEST_ASSERT(inSuite, kTestValue.value1 == read_value.value1);
    NL_TEST_ASSERT(inSuite, kTestValue.value2 == read_value.value2);
    NL_TEST_ASSERT(inSuite, read_size == sizeof(kTestValue));
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Try to get deleted key and verify if CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND is returned
    err = KeyValueStoreMgr().Get(kTestKey, &read_value, sizeof(read_value), &read_size);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static void TestKeyValueStoreMgr_UpdateKeyValue(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    const char * kTestKey = "update_key";
    uint32_t read_value;
    for (uint32_t i = 0; i < 10; i++)
    {
        err = KeyValueStoreMgr().Put(kTestKey, i);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        err = KeyValueStoreMgr().Get(kTestKey, &read_value);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, i == read_value);
    }
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestKeyValueStoreMgr_TooSmallBufferRead(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    const char * kTestKey  = "too_small_buffer_read_key";
    uint32_t kTestValue[5] = { 1, 2, 3, 4, 5 };
    uint32_t read_value;
    size_t read_size;
    err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Returns buffer too small and should read as many bytes as possible
    err = KeyValueStoreMgr().Get(kTestKey, &read_value, sizeof(read_value), &read_size, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);
    NL_TEST_ASSERT(inSuite, read_size == sizeof(read_value));
    NL_TEST_ASSERT(inSuite, kTestValue[0] == read_value);
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestKeyValueStoreMgr_MultiReadKey(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    const char * kTestKey  = "multi_key";
    uint32_t kTestValue[5] = { 1, 2, 3, 4, 5 };
    err                    = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    for (uint32_t i = 0; i < 5; i++)
    {
        uint32_t read_value;
        size_t read_size;
        // Returns buffer too small for all but the last read.
        err = KeyValueStoreMgr().Get(kTestKey, &read_value, sizeof(read_value), &read_size, i * sizeof(uint32_t));
        NL_TEST_ASSERT(inSuite, err == (i < 4 ? CHIP_ERROR_BUFFER_TOO_SMALL : CHIP_NO_ERROR));
        NL_TEST_ASSERT(inSuite, read_size == sizeof(read_value));
        NL_TEST_ASSERT(inSuite, kTestValue[i] == read_value);
    }
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("Test KeyValueStoreMgr_EmptyStringKey", TestKeyValueStoreMgr_EmptyStringKey),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_StringKey", TestKeyValueStoreMgr_StringKey),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_Uint32Key", TestKeyValueStoreMgr_Uint32Key),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_ArrayKey", TestKeyValueStoreMgr_ArrayKey),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_StructKey", TestKeyValueStoreMgr_StructKey),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_UpdateKeyValue", TestKeyValueStoreMgr_UpdateKeyValue),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_TooSmallBufferRead", TestKeyValueStoreMgr_TooSmallBufferRead),
#ifndef __ZEPHYR__
                                 // Zephyr platform does not support partial or offset reads yet.
                                 NL_TEST_DEF("Test KeyValueStoreMgr_MultiReadKey", TestKeyValueStoreMgr_MultiReadKey),
#endif
                                 NL_TEST_SENTINEL() };

/**
 *  Set up the test suite.
 */
int TestKeyValueStoreMgr_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestKeyValueStoreMgr_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestKeyValueStoreMgr()
{
    nlTestSuite theSuite = { "KeyValueStoreMgr tests", &sTests[0], TestKeyValueStoreMgr_Setup, TestKeyValueStoreMgr_Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestKeyValueStoreMgr);
