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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/DefaultAttributePersistenceProvider.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::TLV;

const ConcreteAttributePath TestConcretePath = ConcreteAttributePath(1, 1, 1);

namespace {

class TestAttributePersistenceProvider : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

/**
 * Tests the storage and retrival of data from the KVS as ByteSpan
 */
TEST_F(TestAttributePersistenceProvider, TestStorageAndRetrivalByteSpans)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    ChipError err = persistenceProvider.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Store ByteSpan of size 1
    uint8_t valueArray[1] = { 0x42 };
    ByteSpan value(valueArray);
    err = persistenceProvider.SafeWriteValue(TestConcretePath, value);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint8_t getArray[1];
    MutableByteSpan valueReadBack(getArray);
    err = persistenceProvider.SafeReadValue(TestConcretePath, valueReadBack);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(valueReadBack.data_equal(value));

    uint8_t getArrayThatIsLongerThanNeeded[10];
    MutableByteSpan valueReadBack2(getArrayThatIsLongerThanNeeded);
    err = persistenceProvider.SafeReadValue(TestConcretePath, valueReadBack2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(valueReadBack2.data_equal(value));

    // Finishing
    persistenceProvider.Shutdown();
}

/**
 * Helper to test the storage and retrival of various types from persistent storage.
 * @tparam T The type of the value to store and retrieve
 * @param testValue The test value to store and retrieve
 */
template <typename T>
void testHelperStorageAndRetrivalScalarValues(DefaultAttributePersistenceProvider & persistenceProvider, T testValue)
{
    CHIP_ERROR err = persistenceProvider.WriteScalarValue(TestConcretePath, testValue);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    T valueReadBack = 0;
    err             = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(valueReadBack, testValue);
}

/**
 * Helper to test the storage and retrival of various nullable types from persistent storage.
 * @tparam T The type of the value to store and retrieve
 * @param testValue The test value to store and retrieve
 */
template <typename T>
void testHelperStorageAndRetrivalScalarValues(DefaultAttributePersistenceProvider & persistenceProvider,
                                              DataModel::Nullable<T> testValue)
{
    CHIP_ERROR err = persistenceProvider.WriteScalarValue(TestConcretePath, testValue);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DataModel::Nullable<T> valueReadBack(0);
    err = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(valueReadBack, testValue);
}

/**
 * Tests the storage and retrival of data from the KVS of types  bool, uint8_t, uint16_t, uint32_t, uint64_t.
 */
TEST_F(TestAttributePersistenceProvider, TestStorageAndRetrivalScalarValues)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    CHIP_ERROR err = persistenceProvider.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Test bool
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, bool(true));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, bool(false));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, bool(true));

    // Test uint8_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint8_t(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint8_t(42));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint8_t(0xff));

    // Test uint16_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint16_t(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint16_t(0x0101));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint16_t(0xffff));

    // Test uint32_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint32_t(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint32_t(0x01ffff));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint32_t(0xffffffff));

    // Test uint64_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint64_t(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint64_t(0x0100000001));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, uint64_t(0xffffffffffffffff));

    // Finishing
    persistenceProvider.Shutdown();
}

/**
 * Tests the storage and retrival of data from the KVS of types  int8_t, int16_t, int32_t, int64_t.
 */
TEST_F(TestAttributePersistenceProvider, TestStorageAndRetrivalSignedScalarValues)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    CHIP_ERROR err = persistenceProvider.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Test int8_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int8_t(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int8_t(42));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int8_t(-127));

    // Test int16_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int16_t(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int16_t(0x7fff));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int16_t(0x8000));

    // Test int32_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int32_t(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int32_t(0x7fffffff));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int32_t(0x80000000));

    // Test int64_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int64_t(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int64_t(0x7fffffffffffffff));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, int64_t(0x8000000000000000));

    // Finishing
    persistenceProvider.Shutdown();
}

/**
 * Tests the storage and retrival of data from the KVS of DataModel::Nullable types bool, uint8_t, uint16_t, uint32_t, uint64_t.
 */
TEST_F(TestAttributePersistenceProvider, TestStorageAndRetrivalNullableScalarValues)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    CHIP_ERROR err = persistenceProvider.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Test bool
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<bool>(true));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<bool>(false));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<bool>(true));
    auto nullValBool = DataModel::Nullable<bool>();
    nullValBool.SetNull();
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, nullValBool);

    // Test uint8_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint8_t>(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint8_t>(42));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint8_t>(0xfe));
    auto nullVal8 = DataModel::Nullable<uint8_t>();
    nullVal8.SetNull();
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, nullVal8);

    // Test uint16_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint16_t>(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint16_t>(0x0101));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint16_t>(0xfffe));
    auto nullVal16 = DataModel::Nullable<uint16_t>();
    nullVal16.SetNull();
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, nullVal16);

    // Test uint32_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint32_t>(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint32_t>(0x01ffff));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint32_t>(0xfffffffe));
    auto nullVal32 = DataModel::Nullable<uint32_t>();
    nullVal32.SetNull();
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, nullVal32);

    // Test uint64_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint64_t>(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint64_t>(0x0100000001));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<uint64_t>(0xfffffffffffffffe));
    auto nullVal64 = DataModel::Nullable<uint64_t>();
    nullVal64.SetNull();
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, nullVal64);

    // Finishing
    persistenceProvider.Shutdown();
}

/**
 * Tests the storage and retrival of data from the KVS of DataModel::Nullable types int8_t, int16_t, int32_t, int64_t.
 */
TEST_F(TestAttributePersistenceProvider, TestStorageAndRetrivalSignedNullableScalarValues)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    CHIP_ERROR err = persistenceProvider.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Test int8_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int8_t>(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int8_t>(42));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int8_t>(-127));
    auto nullVal8 = DataModel::Nullable<int8_t>();
    nullVal8.SetNull();
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, nullVal8);

    // Test int16_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int16_t>(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int16_t>(0x7fff));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int16_t>(-0x7fff));
    auto nullVal16 = DataModel::Nullable<int16_t>();
    nullVal16.SetNull();
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, nullVal16);

    // Test int32_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int32_t>(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int32_t>(0x7fffffff));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int32_t>(-0x7fffffff));
    auto nullVal32 = DataModel::Nullable<int32_t>();
    nullVal32.SetNull();
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, nullVal32);

    // Test int64_t
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int64_t>(0));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int64_t>(0x7fffffffffffffff));
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, DataModel::Nullable<int64_t>(-0x7fffffffffffffff));
    auto nullVal64 = DataModel::Nullable<int64_t>();
    nullVal64.SetNull();
    testHelperStorageAndRetrivalScalarValues(persistenceProvider, nullVal64);

    // Finishing
    persistenceProvider.Shutdown();
}

/**
 * Test that the correct error is given when trying to read a value with a buffer that's too small.
 */
TEST_F(TestAttributePersistenceProvider, TestBufferTooSmallErrors)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // Init
    CHIP_ERROR err = persistenceProvider.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Store large data
    uint8_t valueArray[9] = { 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42 };
    ByteSpan value(valueArray);
    err = persistenceProvider.SafeWriteValue(TestConcretePath, value);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Confirm the daya is there
    uint8_t getArray[9];
    MutableByteSpan valueReadBack(getArray);
    err = persistenceProvider.SafeReadValue(TestConcretePath, valueReadBack);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(std::equal(valueReadBack.begin(), valueReadBack.end(), value.begin(), value.end()));

    // Fail to get data as ByteSpace of size 0
    uint8_t getArray0[0];
    MutableByteSpan valueReadBackByteSpan0(getArray0, 0);
    err = persistenceProvider.SafeReadValue(TestConcretePath, valueReadBackByteSpan0);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Fail to get data as ByteSpace of size > 0 but < required
    uint8_t getArray8[8];
    MutableByteSpan valueReadBackByteSpan8(getArray8, sizeof(getArray8));
    err = persistenceProvider.SafeReadValue(TestConcretePath, valueReadBackByteSpan8);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    // TODO: ReadScalarValue() does not take a buffer, so expecting CHIP_ERROR_BUFFER_TOO_SMALL is bad API
    // Fail to get value as uint8_t
    uint8_t valueReadBack8;
    err = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack8);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Fail to get value as uint16_t
    uint16_t valueReadBack16;
    err = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack16);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Fail to get value as uint32_t
    uint32_t valueReadBack32;
    err = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack32);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Fail to get value as uint64_t
    uint64_t valueReadBack64;
    err = persistenceProvider.ReadScalarValue(TestConcretePath, valueReadBack64);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Finishing
    persistenceProvider.Shutdown();
}

} // anonymous namespace
