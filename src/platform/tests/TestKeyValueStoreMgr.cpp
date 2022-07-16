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

static void TestKeyValueStoreMgr_EmptyString(nlTestSuite * inSuite, void * inContext)
{
    constexpr const char * kTestKey   = "str_key";
    constexpr const char kTestValue[] = "";
    constexpr size_t kTestValueLen    = 0;

    char readValue[sizeof(kTestValue)];
    size_t readSize;

    CHIP_ERROR err = KeyValueStoreMgr().Put(kTestKey, kTestValue, kTestValueLen);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify if read value is the same as wrote one
    err = KeyValueStoreMgr().Get(kTestKey, readValue, sizeof(readValue), &readSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, readSize == kTestValueLen);

    // Verify that read succeeds even if 0-length buffer is provided
    err = KeyValueStoreMgr().Get(kTestKey, readValue, 0, &readSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, readSize == kTestValueLen);

    err = KeyValueStoreMgr().Get(kTestKey, nullptr, 0, &readSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, readSize == kTestValueLen);

    // Verify deletion
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Try to get deleted key and verify if CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND is returned
    err = KeyValueStoreMgr().Get(kTestKey, readValue, sizeof(readValue), &readSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static void TestKeyValueStoreMgr_String(nlTestSuite * inSuite, void * inContext)
{
    constexpr const char * kTestKey   = "str_key";
    constexpr const char kTestValue[] = "test_value";

    char readValue[sizeof(kTestValue)];
    size_t readSize;

    CHIP_ERROR err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify if read value is the same as wrote one
    err = KeyValueStoreMgr().Get(kTestKey, readValue, sizeof(readValue), &readSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(kTestValue, readValue) == 0);
    NL_TEST_ASSERT(inSuite, readSize == sizeof(kTestValue));

    // Verify deletion
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Try to get deleted key and verify if CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND is returned
    err = KeyValueStoreMgr().Get(kTestKey, readValue, sizeof(readValue), &readSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static void TestKeyValueStoreMgr_Uint32(nlTestSuite * inSuite, void * inContext)
{
    constexpr const char * kTestKey     = "uint32_key";
    constexpr const uint32_t kTestValue = 5;

    uint32_t readValue = UINT32_MAX;

    CHIP_ERROR err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify if read value is the same as wrote one
    err = KeyValueStoreMgr().Get(kTestKey, &readValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, kTestValue == readValue);

    // Verify deletion
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Try to get deleted key and verify if CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND is returned
    err = KeyValueStoreMgr().Get(kTestKey, &readValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static void TestKeyValueStoreMgr_Array(nlTestSuite * inSuite, void * inContext)
{
    constexpr const char * kTestKey  = "array_key";
    constexpr uint32_t kTestValue[5] = { 1, 2, 3, 4, 5 };

    uint32_t readValue[5];
    size_t readSize;

    CHIP_ERROR err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify if read value is the same as wrote one
    err = KeyValueStoreMgr().Get(kTestKey, readValue, sizeof(readValue), &readSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(kTestValue, readValue, sizeof(kTestValue)) == 0);
    NL_TEST_ASSERT(inSuite, readSize == sizeof(kTestValue));

    // Verify deletion
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Try to get deleted key and verify if CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND is returned
    err = KeyValueStoreMgr().Get(kTestKey, readValue, sizeof(readValue), &readSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static void TestKeyValueStoreMgr_Struct(nlTestSuite * inSuite, void * inContext)
{
    struct TestStruct
    {
        uint8_t value1;
        uint32_t value2;
    };

    constexpr const char * kTestKey = "struct_key";
    constexpr TestStruct kTestValue{ 1, 2 };

    TestStruct readValue;
    size_t readSize;

    CHIP_ERROR err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify if read value is the same as wrote one
    err = KeyValueStoreMgr().Get(kTestKey, &readValue, sizeof(readValue), &readSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, kTestValue.value1 == readValue.value1);
    NL_TEST_ASSERT(inSuite, kTestValue.value2 == readValue.value2);
    NL_TEST_ASSERT(inSuite, readSize == sizeof(kTestValue));

    // Verify deletion
    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Try to get deleted key and verify if CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND is returned
    err = KeyValueStoreMgr().Get(kTestKey, &readValue, sizeof(readValue), &readSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static void TestKeyValueStoreMgr_UpdateValue(nlTestSuite * inSuite, void * inContext)
{
    constexpr const char * kTestKey = "update_key";

    CHIP_ERROR err;
    uint32_t readValue;

    for (uint32_t i = 0; i < 10; i++)
    {
        err = KeyValueStoreMgr().Put(kTestKey, i);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = KeyValueStoreMgr().Get(kTestKey, &readValue);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, i == readValue);
    }

    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestKeyValueStoreMgr_TooSmallBufferRead(nlTestSuite * inSuite, void * inContext)
{
    constexpr const char * kTestKey = "too_small_buffer_read_key";
    constexpr uint8_t kTestValue[]  = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    uint8_t readValue[9];
    size_t readSize;

    CHIP_ERROR err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Returns buffer too small and should read as many bytes as possible
    err = KeyValueStoreMgr().Get(kTestKey, &readValue, sizeof(readValue), &readSize, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);
    NL_TEST_ASSERT(inSuite, readSize == sizeof(readValue));
    NL_TEST_ASSERT(inSuite, memcmp(kTestValue, readValue, readSize) == 0);

    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestKeyValueStoreMgr_AllCharactersKey(nlTestSuite * inSuite, void * inContext)
{
    // Test that all printable characters [0x20 - 0x7f) can be part of the key
    constexpr size_t kKeyLength   = 32;
    constexpr char kCharBegin     = 0x20;
    constexpr char kCharEnd       = 0x7f;
    constexpr uint32_t kTestValue = 5;

    char allChars[kCharEnd - kCharBegin];

    for (char character = kCharBegin; character < kCharEnd; character++)
    {
        allChars[character - kCharBegin] = character;
    }

    for (size_t charId = 0; charId < sizeof(allChars); charId += kKeyLength)
    {
        char testKey[kKeyLength + 1] = {};
        memcpy(testKey, &allChars[charId], chip::min(sizeof(allChars) - charId, kKeyLength));

        CHIP_ERROR err = KeyValueStoreMgr().Put(testKey, kTestValue);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        uint32_t readValue = UINT32_MAX;
        err                = KeyValueStoreMgr().Get(testKey, &readValue);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = KeyValueStoreMgr().Delete(testKey);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }
}

static void TestKeyValueStoreMgr_NonExistentDelete(nlTestSuite * inSuite, void * inContext)
{
    constexpr const char * kTestKey = "non_existent";

    CHIP_ERROR err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static void TestKeyValueStoreMgr_MultiRead(nlTestSuite * inSuite, void * inContext)
{
    constexpr const char * kTestKey  = "multi_key";
    constexpr uint32_t kTestValue[5] = { 1, 2, 3, 4, 5 };

    CHIP_ERROR err = KeyValueStoreMgr().Put(kTestKey, kTestValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    for (uint32_t i = 0; i < 5; i++)
    {
        uint32_t readValue;
        size_t readSize;

        // Returns buffer too small for all but the last read.
        err = KeyValueStoreMgr().Get(kTestKey, &readValue, sizeof(readValue), &readSize, i * sizeof(uint32_t));
        NL_TEST_ASSERT(inSuite, err == (i < 4 ? CHIP_ERROR_BUFFER_TOO_SMALL : CHIP_NO_ERROR));
        NL_TEST_ASSERT(inSuite, readSize == sizeof(readValue));
        NL_TEST_ASSERT(inSuite, kTestValue[i] == readValue);
    }

    err = KeyValueStoreMgr().Delete(kTestKey);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

#ifdef __ZEPHYR__
static void TestKeyValueStoreMgr_DoFactoryReset(nlTestSuite * inSuite, void * inContext)
{
    constexpr const char * kStrKey  = "string_with_weird_chars\\=_key";
    constexpr const char * kUintKey = "some_uint_key";

    NL_TEST_ASSERT(inSuite, KeyValueStoreMgr().Put(kStrKey, "some_string") == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, KeyValueStoreMgr().Put(kUintKey, uint32_t(1234)) == CHIP_NO_ERROR);

    char readString[16];
    uint32_t readValue;

    NL_TEST_ASSERT(inSuite, KeyValueStoreMgr().Get(kStrKey, readString, sizeof(readString)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, KeyValueStoreMgr().Get(kUintKey, &readValue) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, KeyValueStoreMgrImpl().DoFactoryReset() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   KeyValueStoreMgr().Get(kStrKey, readString, sizeof(readString)) == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, KeyValueStoreMgr().Get(kUintKey, &readValue) == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}
#endif
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("Test KeyValueStoreMgr_EmptyString", TestKeyValueStoreMgr_EmptyString),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_String", TestKeyValueStoreMgr_String),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_Uint32", TestKeyValueStoreMgr_Uint32),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_Array", TestKeyValueStoreMgr_Array),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_Struct", TestKeyValueStoreMgr_Struct),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_UpdateValue", TestKeyValueStoreMgr_UpdateValue),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_TooSmallBufferRead", TestKeyValueStoreMgr_TooSmallBufferRead),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_AllCharactersKey", TestKeyValueStoreMgr_AllCharactersKey),
                                 NL_TEST_DEF("Test KeyValueStoreMgr_NonExistentDelete", TestKeyValueStoreMgr_NonExistentDelete),
#if !defined(__ZEPHYR__) && !defined(__MBED__)
                                 // Zephyr and Mbed platforms do not support partial or offset reads yet.
                                 NL_TEST_DEF("Test KeyValueStoreMgr_MultiRead", TestKeyValueStoreMgr_MultiRead),
#endif
#ifdef __ZEPHYR__
                                 NL_TEST_DEF("Test TestKeyValueStoreMgr_DoFactoryReset", TestKeyValueStoreMgr_DoFactoryReset),
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
