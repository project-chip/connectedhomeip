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

/**
 *    @file
 *      This file implements unit tests for AttributePersistenceProvider
 *
 */

#include <app-common/zap-generated/cluster-objects.h>
#include <app/DefaultAttributePersistenceProvider.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::app;
using namespace chip::TLV;

const ConcreteAttributePath TestConcretePath = ConcreteAttributePath(1, 1, 1);

namespace {

/**
 *  Set up the test suite.
 */
int Test_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    VerifyOrReturnError(error == CHIP_NO_ERROR, FAILURE);
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int Test_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

/**
 * Tests the storage and retrival of data from the KVS as ByteSpan
 */
void TestStorageAndRetrivalByteSpans(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    ChipError err = persistenceProvider.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Store ByteSpan of size 1
    uint8_t valueArray[1] = { 0x42 };
    ByteSpan value(valueArray);
    err = persistenceProvider.SafeWriteValue(TestConcretePath, value);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    uint8_t getArray[1];
    MutableByteSpan valueReadBack(getArray);
    err = persistenceProvider.SafeReadValue(TestConcretePath, valueReadBack);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, std::equal(valueReadBack.begin(), valueReadBack.end(), value.begin(), value.end()));

    // Finishing
    persistenceProvider.Shutdown();
}

/**
 * Helper to test the storage and retrival of various types from persistent storage.
 * @tparam T The type of the value to store and retrieve
 * @param testValue The test value to store and retrieve
 */
template <typename T>
void testHelperStorageAndRetrivalScalarValues(nlTestSuite * inSuite, DefaultAttributePersistenceProvider & persistenceProvider,
                                              T testValue)
{
    CHIP_ERROR err = persistenceProvider.WriteScalarValue(TestConcretePath, testValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    T valueReadBack = 0;
    err             = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, valueReadBack == testValue);
}

/**
 * Helper to test the storage and retrival of various nullable types from persistent storage.
 * @tparam T The type of the value to store and retrieve
 * @param testValue The test value to store and retrieve
 */
template <typename T>
void testHelperStorageAndRetrivalScalarValues(nlTestSuite * inSuite, DefaultAttributePersistenceProvider & persistenceProvider,
                                              DataModel::Nullable<T> testValue)
{
    CHIP_ERROR err = persistenceProvider.WriteScalarValue(TestConcretePath, testValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    DataModel::Nullable<T> valueReadBack(0);
    err = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, valueReadBack == testValue);
}

/**
 * Tests the storage and retrival of data from the KVS of types  bool, uint8_t, uint16_t, uint32_t, uint64_t.
 */
void TestStorageAndRetrivalScalarValues(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    CHIP_ERROR err = persistenceProvider.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Test bool
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, bool(true));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, bool(false));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, bool(true));

    // Test uint8_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint8_t(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint8_t(42));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint8_t(0xff));

    // Test uint16_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint16_t(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint16_t(0x0101));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint16_t(0xffff));

    // Test uint32_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint32_t(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint32_t(0x01ffff));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint32_t(0xffffffff));

    // Test uint64_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint64_t(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint64_t(0x0100000001));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, uint64_t(0xffffffffffffffff));

    // Finishing
    persistenceProvider.Shutdown();
}

/**
 * Tests the storage and retrival of data from the KVS of types  int8_t, int16_t, int32_t, int64_t.
 */
void TestStorageAndRetrivalSignedScalarValues(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    CHIP_ERROR err = persistenceProvider.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Test int8_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int8_t(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int8_t(42));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int8_t(-127));

    // Test int16_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int16_t(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int16_t(0x7fff));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int16_t(0x8000));

    // Test int32_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int32_t(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int32_t(0x7fffffff));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int32_t(0x80000000));

    // Test int64_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int64_t(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int64_t(0x7fffffffffffffff));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, int64_t(0x8000000000000000));

    // Finishing
    persistenceProvider.Shutdown();
}

/**
 * Tests the storage and retrival of data from the KVS of DataModel::Nullable types bool, uint8_t, uint16_t, uint32_t, uint64_t.
 */
void TestStorageAndRetrivalNullableScalarValues(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    CHIP_ERROR err = persistenceProvider.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Test bool
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<bool>(true));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<bool>(false));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<bool>(true));
    auto nullValBool = DataModel::Nullable<bool>();
    nullValBool.SetNull();
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, nullValBool);

    // Test uint8_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint8_t>(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint8_t>(42));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint8_t>(0xfe));
    auto nullVal8 = DataModel::Nullable<uint8_t>();
    nullVal8.SetNull();
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, nullVal8);

    // Test uint16_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint16_t>(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint16_t>(0x0101));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint16_t>(0xfffe));
    auto nullVal16 = DataModel::Nullable<uint16_t>();
    nullVal16.SetNull();
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, nullVal16);

    // Test uint32_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint32_t>(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint32_t>(0x01ffff));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint32_t>(0xfffffffe));
    auto nullVal32 = DataModel::Nullable<uint32_t>();
    nullVal32.SetNull();
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, nullVal32);

    // Test uint64_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint64_t>(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint64_t>(0x0100000001));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<uint64_t>(0xfffffffffffffffe));
    auto nullVal64 = DataModel::Nullable<uint64_t>();
    nullVal64.SetNull();
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, nullVal64);

    // Finishing
    persistenceProvider.Shutdown();
}

/**
 * Tests the storage and retrival of data from the KVS of DataModel::Nullable types int8_t, int16_t, int32_t, int64_t.
 */
void TestStorageAndRetrivalSignedNullableScalarValues(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    CHIP_ERROR err = persistenceProvider.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Test int8_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int8_t>(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int8_t>(42));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int8_t>(-127));
    auto nullVal8 = DataModel::Nullable<int8_t>();
    nullVal8.SetNull();
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, nullVal8);

    // Test int16_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int16_t>(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int16_t>(0x7fff));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int16_t>(-0x7fff));
    auto nullVal16 = DataModel::Nullable<int16_t>();
    nullVal16.SetNull();
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, nullVal16);

    // Test int32_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int32_t>(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int32_t>(0x7fffffff));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int32_t>(-0x7fffffff));
    auto nullVal32 = DataModel::Nullable<int32_t>();
    nullVal32.SetNull();
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, nullVal32);

    // Test int64_t
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int64_t>(0));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int64_t>(0x7fffffffffffffff));
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, DataModel::Nullable<int64_t>(-0x7fffffffffffffff));
    auto nullVal64 = DataModel::Nullable<int64_t>();
    nullVal64.SetNull();
    testHelperStorageAndRetrivalScalarValues(inSuite, persistenceProvider, nullVal64);

    // Finishing
    persistenceProvider.Shutdown();
}

/**
 * Test that the correct error is given when trying to read a value with a buffer that's too small.
 */
void TestBufferTooSmallErrors(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    CHIP_ERROR err = persistenceProvider.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Store large data
    uint8_t valueArray[9] = { 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42 };
    ByteSpan value(valueArray);
    err = persistenceProvider.SafeWriteValue(TestConcretePath, value);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Confirm the daya is there
    uint8_t getArray[9];
    MutableByteSpan valueReadBack(getArray);
    err = persistenceProvider.SafeReadValue(TestConcretePath, valueReadBack);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, std::equal(valueReadBack.begin(), valueReadBack.end(), value.begin(), value.end()));

    // Fail to get data as ByteSpace of size 0
    uint8_t getArray0[0];
    MutableByteSpan valueReadBackByteSpan0(getArray0, 0);
    err = persistenceProvider.SafeReadValue(TestConcretePath, valueReadBackByteSpan0);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Fail to get data as ByteSpace of size > 0 but < required
    uint8_t getArray8[8];
    MutableByteSpan valueReadBackByteSpan8(getArray8, sizeof(getArray8));
    err = persistenceProvider.SafeReadValue(TestConcretePath, valueReadBackByteSpan8);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Fail to get value as uint8_t
    uint8_t valueReadBack8;
    err = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack8);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Fail to get value as uint16_t
    uint16_t valueReadBack16;
    err = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack16);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Fail to get value as uint32_t
    uint32_t valueReadBack32;
    err = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack32);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Fail to get value as uint64_t
    uint64_t valueReadBack64;
    err = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack64);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Finishing
    persistenceProvider.Shutdown();
}

} // anonymous namespace

namespace {
const nlTest sTests[] = {
    NL_TEST_DEF("Storage and retrival of ByteSpans", TestStorageAndRetrivalByteSpans),
    NL_TEST_DEF("Storage and retrival of unsigned scalar values", TestStorageAndRetrivalScalarValues),
    NL_TEST_DEF("Storage and retrival of signed scalar values", TestStorageAndRetrivalSignedScalarValues),
    NL_TEST_DEF("Storage and retrival of unsigned nullable scalar values", TestStorageAndRetrivalNullableScalarValues),
    NL_TEST_DEF("Storage and retrival of signed nullable scalar values", TestStorageAndRetrivalSignedNullableScalarValues),
    NL_TEST_DEF("Small buffer errors", TestBufferTooSmallErrors),
    NL_TEST_SENTINEL()
};
}

int TestAttributePersistenceProvider()
{
    nlTestSuite theSuite = { "AttributePersistenceProvider", &sTests[0], Test_Setup, Test_Teardown };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestAttributePersistenceProvider)
