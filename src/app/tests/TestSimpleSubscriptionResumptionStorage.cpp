/*
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

#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

#include <app/SimpleSubscriptionResumptionStorage.h>
#include <lib/support/TestPersistentStorageDelegate.h>

#include <lib/support/DefaultStorageKeyAllocator.h>

class SimpleSubscriptionResumptionStorageTest : public chip::app::SimpleSubscriptionResumptionStorage
{
public:
    CHIP_ERROR TestSave(chip::TLV::TLVWriter & writer,
                        chip::app::SubscriptionResumptionStorage::SubscriptionInfo & subscriptionInfo)
    {
        return Save(writer, subscriptionInfo);
    }
    static constexpr size_t TestMaxSubscriptionSize() { return MaxSubscriptionSize(); }
};

struct TestSubscriptionInfo : public chip::app::SubscriptionResumptionStorage::SubscriptionInfo
{
    bool operator==(const SubscriptionInfo & that) const
    {
        if ((mNodeId != that.mNodeId) || (mFabricIndex != that.mFabricIndex) || (mSubscriptionId != that.mSubscriptionId) ||
            (mMinInterval != that.mMinInterval) || (mMaxInterval != that.mMaxInterval) || (mFabricFiltered != that.mFabricFiltered))
        {
            return false;
        }
        if ((mAttributePaths.AllocatedSize() != that.mAttributePaths.AllocatedSize()) ||
            (mEventPaths.AllocatedSize() != that.mEventPaths.AllocatedSize()))
        {
            return false;
        }
        for (size_t i = 0; i < mAttributePaths.AllocatedSize(); i++)
        {
            if ((mAttributePaths[i].mEndpointId != that.mAttributePaths[i].mEndpointId) ||
                (mAttributePaths[i].mClusterId != that.mAttributePaths[i].mClusterId) ||
                (mAttributePaths[i].mAttributeId != that.mAttributePaths[i].mAttributeId))
            {
                return false;
            }
        }
        for (size_t i = 0; i < mEventPaths.AllocatedSize(); i++)
        {
            if ((mEventPaths[i].mEndpointId != that.mEventPaths[i].mEndpointId) ||
                (mEventPaths[i].mClusterId != that.mEventPaths[i].mClusterId) ||
                (mEventPaths[i].mEventId != that.mEventPaths[i].mEventId) ||
                (mEventPaths[i].mIsUrgentEvent != that.mEventPaths[i].mIsUrgentEvent))
            {
                return false;
            }
        }
        return true;
    }
};

void TestSubscriptionCount(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    SimpleSubscriptionResumptionStorageTest subscriptionStorage;
    subscriptionStorage.Init(&storage);

    // Write some subscriptions and see the counts are correct
    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo = { .mNodeId = 6666, .mFabricIndex = 46 };

    for (size_t i = 0; i < (CHIP_IM_MAX_NUM_SUBSCRIPTIONS / 2); i++)
    {
        subscriptionInfo.mSubscriptionId = static_cast<chip::SubscriptionId>(i);
        subscriptionStorage.Save(subscriptionInfo);
    }

    // Make sure iterator counts correctly
    auto * iterator = subscriptionStorage.IterateSubscriptions();
    NL_TEST_ASSERT(inSuite, iterator->Count() == (CHIP_IM_MAX_NUM_SUBSCRIPTIONS / 2));

    // Verify subscriptions manually count correctly
    size_t count = 0;
    while (iterator->Next(subscriptionInfo))
    {
        count++;
    }
    iterator->Release();
    NL_TEST_ASSERT(inSuite, count == (CHIP_IM_MAX_NUM_SUBSCRIPTIONS / 2));

    // Delete all and verify iterator counts 0
    CHIP_ERROR err = subscriptionStorage.DeleteAll(46);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    iterator = subscriptionStorage.IterateSubscriptions();
    NL_TEST_ASSERT(inSuite, iterator->Count() == 0);

    // Verify subscriptions manually count correctly
    count = 0;
    while (iterator->Next(subscriptionInfo))
    {
        count++;
    }
    iterator->Release();
    NL_TEST_ASSERT(inSuite, count == 0);
}

void TestSubscriptionMaxCount(nlTestSuite * inSuite, void * inContext)
{
    // Force large MacCount value and check that Init resets it properly, and deletes extra subs:

    chip::TestPersistentStorageDelegate storage;
    SimpleSubscriptionResumptionStorageTest subscriptionStorage;

    // First set a large MaxCount before Init
    uint16_t countMaxToSave = 2 * CHIP_IM_MAX_NUM_SUBSCRIPTIONS;
    CHIP_ERROR err          = storage.SyncSetKeyValue(chip::DefaultStorageKeyAllocator::SubscriptionResumptionMaxCount().KeyName(),
                                                      &countMaxToSave, sizeof(uint16_t));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Then write something beyond CHIP_IM_MAX_NUM_SUBSCRIPTIONS
    chip::Platform::ScopedMemoryBuffer<uint8_t> junkBytes;
    junkBytes.Calloc(subscriptionStorage.TestMaxSubscriptionSize() / 2);
    NL_TEST_ASSERT(inSuite, junkBytes.Get() != nullptr);
    NL_TEST_ASSERT(inSuite,
                   storage.SyncSetKeyValue(
                       chip::DefaultStorageKeyAllocator::SubscriptionResumption(CHIP_IM_MAX_NUM_SUBSCRIPTIONS + 1).KeyName(),
                       junkBytes.Get(), static_cast<uint16_t>(subscriptionStorage.TestMaxSubscriptionSize() / 2)) == CHIP_NO_ERROR);

    subscriptionStorage.Init(&storage);

    // First check the MaxCount is reset to CHIP_IM_MAX_NUM_SUBSCRIPTIONS
    uint16_t countMax = 0;
    uint16_t len      = sizeof(countMax);
    err = storage.SyncGetKeyValue(chip::DefaultStorageKeyAllocator::SubscriptionResumptionMaxCount().KeyName(), &countMax, len);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, countMax == CHIP_IM_MAX_NUM_SUBSCRIPTIONS);

    // Then check the fake sub is no more
    NL_TEST_ASSERT(inSuite,
                   !storage.SyncDoesKeyExist(
                       chip::DefaultStorageKeyAllocator::SubscriptionResumption(CHIP_IM_MAX_NUM_SUBSCRIPTIONS + 1).KeyName()));
}

void TestSubscriptionState(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    SimpleSubscriptionResumptionStorageTest subscriptionStorage;
    subscriptionStorage.Init(&storage);

    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo1 = {
        .mNodeId         = 1111,
        .mFabricIndex    = 41,
        .mSubscriptionId = 1,
        .mMinInterval    = 1,
        .mMaxInterval    = 11,
        .mFabricFiltered = true,
    };
    subscriptionInfo1.mAttributePaths.Calloc(2);
    subscriptionInfo1.mAttributePaths[0].mEndpointId  = 1;
    subscriptionInfo1.mAttributePaths[0].mClusterId   = 1;
    subscriptionInfo1.mAttributePaths[0].mAttributeId = 1;
    subscriptionInfo1.mAttributePaths[1].mEndpointId  = 2;
    subscriptionInfo1.mAttributePaths[1].mClusterId   = 2;
    subscriptionInfo1.mAttributePaths[1].mAttributeId = 2;

    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo2 = {
        .mNodeId         = 2222,
        .mFabricIndex    = 42,
        .mSubscriptionId = 2,
        .mMinInterval    = 2,
        .mMaxInterval    = 12,
        .mFabricFiltered = false,
    };
    subscriptionInfo2.mEventPaths.Calloc(2);
    subscriptionInfo2.mEventPaths[0].mEndpointId    = 3;
    subscriptionInfo2.mEventPaths[0].mClusterId     = 3;
    subscriptionInfo2.mEventPaths[0].mEventId       = 3;
    subscriptionInfo2.mEventPaths[0].mIsUrgentEvent = false;
    subscriptionInfo2.mEventPaths[1].mEndpointId    = 4;
    subscriptionInfo2.mEventPaths[1].mClusterId     = 4;
    subscriptionInfo2.mEventPaths[1].mEventId       = 4;
    subscriptionInfo2.mEventPaths[1].mIsUrgentEvent = true;

    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo3 = {
        .mNodeId         = 3333,
        .mFabricIndex    = 43,
        .mSubscriptionId = 3,
        .mMinInterval    = 3,
        .mMaxInterval    = 13,
        .mFabricFiltered = true,
    };
    subscriptionInfo3.mAttributePaths.Calloc(2);
    subscriptionInfo3.mAttributePaths[0].mEndpointId  = 5;
    subscriptionInfo3.mAttributePaths[0].mClusterId   = 5;
    subscriptionInfo3.mAttributePaths[0].mAttributeId = 5;
    subscriptionInfo3.mAttributePaths[1].mEndpointId  = 6;
    subscriptionInfo3.mAttributePaths[1].mClusterId   = 6;
    subscriptionInfo3.mAttributePaths[1].mAttributeId = 6;
    subscriptionInfo3.mEventPaths.Calloc(2);
    subscriptionInfo3.mEventPaths[0].mEndpointId    = 7;
    subscriptionInfo3.mEventPaths[0].mClusterId     = 7;
    subscriptionInfo3.mEventPaths[0].mEventId       = 7;
    subscriptionInfo2.mEventPaths[0].mIsUrgentEvent = true;
    subscriptionInfo3.mEventPaths[1].mEndpointId    = 8;
    subscriptionInfo3.mEventPaths[1].mClusterId     = 8;
    subscriptionInfo3.mEventPaths[1].mEventId       = 8;
    subscriptionInfo2.mEventPaths[1].mIsUrgentEvent = false;

    CHIP_ERROR err;
    err = subscriptionStorage.Save(subscriptionInfo1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = subscriptionStorage.Save(subscriptionInfo2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = subscriptionStorage.Save(subscriptionInfo3);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    auto * iterator = subscriptionStorage.IterateSubscriptions();
    NL_TEST_ASSERT(inSuite, iterator->Count() == 3);

    // Verify subscriptions manually count correctly
    TestSubscriptionInfo subscriptionInfo;
    NL_TEST_ASSERT(inSuite, iterator->Next(subscriptionInfo));
    NL_TEST_ASSERT(inSuite, subscriptionInfo == subscriptionInfo1);
    NL_TEST_ASSERT(inSuite, iterator->Next(subscriptionInfo));
    NL_TEST_ASSERT(inSuite, subscriptionInfo == subscriptionInfo2);
    NL_TEST_ASSERT(inSuite, iterator->Next(subscriptionInfo));
    NL_TEST_ASSERT(inSuite, subscriptionInfo == subscriptionInfo3);
    // Verify at end of list
    NL_TEST_ASSERT(inSuite, !iterator->Next(subscriptionInfo));
    iterator->Release();

    // Delete fabric 1 and subscription 2 and check only 3 remains.
    subscriptionStorage.Delete(subscriptionInfo1.mNodeId, subscriptionInfo1.mFabricIndex, subscriptionInfo1.mSubscriptionId);
    subscriptionStorage.DeleteAll(subscriptionInfo2.mFabricIndex);

    iterator = subscriptionStorage.IterateSubscriptions();
    NL_TEST_ASSERT(inSuite, iterator->Count() == 1);
    NL_TEST_ASSERT(inSuite, iterator->Next(subscriptionInfo));
    NL_TEST_ASSERT(inSuite, subscriptionInfo == subscriptionInfo3);
    // Verify at end of list
    NL_TEST_ASSERT(inSuite, !iterator->Next(subscriptionInfo));
    iterator->Release();

    // Delete 3 also, and see that both count is 0 and MaxCount is removed from storage
    subscriptionStorage.DeleteAll(subscriptionInfo3.mFabricIndex);
    iterator = subscriptionStorage.IterateSubscriptions();
    NL_TEST_ASSERT(inSuite, iterator->Count() == 0);
    NL_TEST_ASSERT(inSuite, !iterator->Next(subscriptionInfo));
    iterator->Release();

    uint16_t countMax = 0;
    uint16_t len      = sizeof(countMax);
    err = storage.SyncGetKeyValue(chip::DefaultStorageKeyAllocator::SubscriptionResumptionMaxCount().KeyName(), &countMax, len);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static constexpr chip::TLV::Tag kTestValue1Tag = chip::TLV::ContextTag(30);
static constexpr chip::TLV::Tag kTestValue2Tag = chip::TLV::ContextTag(31);

void TestSubscriptionStateUnexpectedFields(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    SimpleSubscriptionResumptionStorageTest subscriptionStorage;
    subscriptionStorage.Init(&storage);

    // Write additional entries at the end of TLV and see it still loads correctly
    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo1 = {
        .mNodeId         = 4444,
        .mFabricIndex    = 44,
        .mSubscriptionId = 4,
        .mMinInterval    = 4,
        .mMaxInterval    = 14,
        .mFabricFiltered = true,
    };
    subscriptionInfo1.mAttributePaths.Calloc(2);
    subscriptionInfo1.mAttributePaths[0].mEndpointId  = 9;
    subscriptionInfo1.mAttributePaths[0].mClusterId   = 9;
    subscriptionInfo1.mAttributePaths[0].mAttributeId = 9;
    subscriptionInfo1.mAttributePaths[1].mEndpointId  = 10;
    subscriptionInfo1.mAttributePaths[1].mClusterId   = 10;
    subscriptionInfo1.mAttributePaths[1].mAttributeId = 10;

    chip::Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(subscriptionStorage.TestMaxSubscriptionSize());
    NL_TEST_ASSERT(inSuite, backingBuffer.Get() != nullptr);
    chip::TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), subscriptionStorage.TestMaxSubscriptionSize());

    NL_TEST_ASSERT(inSuite, subscriptionStorage.TestSave(writer, subscriptionInfo1) == CHIP_NO_ERROR);

    // Additional stuff
    chip::TLV::TLVType containerType;
    NL_TEST_ASSERT(inSuite,
                   writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, containerType) == CHIP_NO_ERROR);
    uint32_t value1 = 1;
    uint32_t value2 = 2;
    NL_TEST_ASSERT(inSuite, writer.Put(kTestValue1Tag, value1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, writer.Put(kTestValue2Tag, value2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, writer.EndContainer(containerType) == CHIP_NO_ERROR);

    const auto len = writer.GetLengthWritten();

    writer.Finalize(backingBuffer);

    NL_TEST_ASSERT(inSuite,
                   storage.SyncSetKeyValue(chip::DefaultStorageKeyAllocator::SubscriptionResumption(0).KeyName(),
                                           backingBuffer.Get(), static_cast<uint16_t>(len)) == CHIP_NO_ERROR);

    // Now read back and verify
    auto * iterator = subscriptionStorage.IterateSubscriptions();
    NL_TEST_ASSERT(inSuite, iterator->Count() == 1);
    TestSubscriptionInfo subscriptionInfo;
    NL_TEST_ASSERT(inSuite, iterator->Next(subscriptionInfo));
    NL_TEST_ASSERT(inSuite, subscriptionInfo == subscriptionInfo1);
    iterator->Release();
}

void TestSubscriptionStateTooBigToLoad(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    SimpleSubscriptionResumptionStorageTest subscriptionStorage;
    subscriptionStorage.Init(&storage);

    // Write additional too-big data at the end of TLV and see it fails to loads and entry deleted
    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo1 = {
        .mNodeId         = 5555,
        .mFabricIndex    = 45,
        .mSubscriptionId = 5,
        .mMinInterval    = 5,
        .mMaxInterval    = 15,
        .mFabricFiltered = false,
    };
    subscriptionInfo1.mAttributePaths.Calloc(2);
    subscriptionInfo1.mAttributePaths[0].mEndpointId  = 11;
    subscriptionInfo1.mAttributePaths[0].mClusterId   = 11;
    subscriptionInfo1.mAttributePaths[0].mAttributeId = 11;
    subscriptionInfo1.mAttributePaths[1].mEndpointId  = 12;
    subscriptionInfo1.mAttributePaths[1].mClusterId   = 12;
    subscriptionInfo1.mAttributePaths[1].mAttributeId = 12;

    chip::Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(subscriptionStorage.TestMaxSubscriptionSize() * 2);
    NL_TEST_ASSERT(inSuite, backingBuffer.Get() != nullptr);
    chip::TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), subscriptionStorage.TestMaxSubscriptionSize() * 2);

    NL_TEST_ASSERT(inSuite, subscriptionStorage.TestSave(writer, subscriptionInfo1) == CHIP_NO_ERROR);

    // Additional too-many bytes
    chip::TLV::TLVType containerType;
    NL_TEST_ASSERT(inSuite,
                   writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, containerType) == CHIP_NO_ERROR);
    // Write MaxSubscriptionSize() to guarantee Load failure
    chip::Platform::ScopedMemoryBuffer<uint8_t> additionalBytes;
    additionalBytes.Calloc(subscriptionStorage.TestMaxSubscriptionSize());
    NL_TEST_ASSERT(inSuite, additionalBytes.Get() != nullptr);
    NL_TEST_ASSERT(inSuite,
                   writer.PutBytes(kTestValue1Tag, additionalBytes.Get(),
                                   static_cast<uint32_t>(subscriptionStorage.TestMaxSubscriptionSize())) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, writer.EndContainer(containerType) == CHIP_NO_ERROR);

    const auto len = writer.GetLengthWritten();

    writer.Finalize(backingBuffer);

    NL_TEST_ASSERT(inSuite,
                   storage.SyncSetKeyValue(chip::DefaultStorageKeyAllocator::SubscriptionResumption(0).KeyName(),
                                           backingBuffer.Get(), static_cast<uint16_t>(len)) == CHIP_NO_ERROR);

    // Now read back and verify
    auto * iterator = subscriptionStorage.IterateSubscriptions();
    NL_TEST_ASSERT(inSuite, iterator->Count() == 1);
    TestSubscriptionInfo subscriptionInfo;
    NL_TEST_ASSERT(inSuite, !iterator->Next(subscriptionInfo));
    NL_TEST_ASSERT(inSuite, iterator->Count() == 0);
    iterator->Release();
}

void TestSubscriptionStateJunkData(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    SimpleSubscriptionResumptionStorageTest subscriptionStorage;
    subscriptionStorage.Init(&storage);

    chip::Platform::ScopedMemoryBuffer<uint8_t> junkBytes;
    junkBytes.Calloc(subscriptionStorage.TestMaxSubscriptionSize() / 2);
    NL_TEST_ASSERT(inSuite, junkBytes.Get() != nullptr);
    NL_TEST_ASSERT(inSuite,
                   storage.SyncSetKeyValue(chip::DefaultStorageKeyAllocator::SubscriptionResumption(0).KeyName(), junkBytes.Get(),
                                           static_cast<uint16_t>(subscriptionStorage.TestMaxSubscriptionSize() / 2)) ==
                       CHIP_NO_ERROR);

    // Now read back and verify
    auto * iterator = subscriptionStorage.IterateSubscriptions();
    NL_TEST_ASSERT(inSuite, iterator->Count() == 1);
    TestSubscriptionInfo subscriptionInfo;
    NL_TEST_ASSERT(inSuite, !iterator->Next(subscriptionInfo));
    NL_TEST_ASSERT(inSuite, iterator->Count() == 0);
    iterator->Release();
}
/**
 *  Set up the test suite.
 */
int TestSubscription_Setup(void * inContext)
{
    VerifyOrReturnError(CHIP_NO_ERROR == chip::Platform::MemoryInit(), FAILURE);

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestSubscription_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestSubscriptionCount", TestSubscriptionCount),
    NL_TEST_DEF("TestSubscriptionMaxCount", TestSubscriptionMaxCount),
    NL_TEST_DEF("TestSubscriptionState", TestSubscriptionState),
    NL_TEST_DEF("TestSubscriptionStateUnexpectedFields", TestSubscriptionStateUnexpectedFields),
    NL_TEST_DEF("TestSubscriptionStateTooBigToLoad", TestSubscriptionStateTooBigToLoad),
    NL_TEST_DEF("TestSubscriptionStateJunkData", TestSubscriptionStateJunkData),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SimpleSubscriptionResumptionStorage",
    &sTests[0],
    &TestSubscription_Setup, &TestSubscription_Teardown
};
// clang-format on

/**
 *  Main
 */
int TestSimpleSubscriptionResumptionStorage()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestSimpleSubscriptionResumptionStorage)
