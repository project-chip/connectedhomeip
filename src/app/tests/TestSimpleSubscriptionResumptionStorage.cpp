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
    size_t TestMaxCount() { return MaxCount(); }
    CHIP_ERROR TestSave(chip::TLV::TLVWriter & writer,
                        chip::app::SubscriptionResumptionStorage::SubscriptionInfo & subscriptionInfo)
    {
        return Save(writer, subscriptionInfo);
    }
    static constexpr size_t TestMaxSubscriptionSize() { return MaxSubscriptionSize(); }
};

// Fictitious subsccription 1
constexpr chip::FabricIndex fabric1            = 10;
constexpr chip::NodeId node1                   = 12344321;
constexpr chip::SubscriptionId subscriptionId1 = 1;
constexpr uint16_t minInterval1                = 1;
constexpr uint16_t maxInterval1                = 10;
constexpr bool fabricFiltered1                 = false;
// has only attribute subs
constexpr chip::EndpointId attributeEndpointId1_1 = 1;
constexpr chip::ClusterId attributeClusterId1_1   = 1;
constexpr chip::AttributeId attributeId1_1        = 1;
constexpr chip::EndpointId attributeEndpointId1_2 = 2;
constexpr chip::ClusterId attributeClusterId1_2   = 2;
constexpr chip::AttributeId attributeId1_2        = 2;

// Fictitious subsccription 2
constexpr chip::FabricIndex fabric2            = 14;
constexpr chip::NodeId node2                   = 11223344;
constexpr chip::SubscriptionId subscriptionId2 = 2;
constexpr uint16_t minInterval2                = 2;
constexpr uint16_t maxInterval2                = 20;
constexpr bool fabricFiltered2                 = true;
// has only event subs
constexpr chip::EndpointId eventEndpointId2_1 = 1;
constexpr chip::ClusterId eventClusterId2_1   = 1;
constexpr chip::AttributeId eventId2_1        = 1;
constexpr bool isUrgentEvent2_1               = false;
constexpr chip::EndpointId eventEndpointId2_2 = 2;
constexpr chip::ClusterId eventClusterId2_2   = 2;
constexpr chip::AttributeId eventId2_2        = 2;
constexpr bool isUrgentEvent2_2               = true;

// Fictitious subsccription 3
constexpr chip::FabricIndex fabric3            = 18;
constexpr chip::NodeId node3                   = 44332211;
constexpr chip::SubscriptionId subscriptionId3 = 3;
constexpr uint16_t minInterval3                = 3;
constexpr uint16_t maxInterval3                = 30;
constexpr bool fabricFiltered3                 = true;
// has both attributes and events
constexpr chip::EndpointId attributeEndpointId3_1 = 1;
constexpr chip::ClusterId attributeClusterId3_1   = 1;
constexpr chip::AttributeId attributeId3_1        = 1;
constexpr chip::EndpointId attributeEndpointId3_2 = 2;
constexpr chip::ClusterId attributeClusterId3_2   = 2;
constexpr chip::AttributeId attributeId3_2        = 2;
constexpr chip::EndpointId eventEndpointId3_1     = 1;
constexpr chip::ClusterId eventClusterId3_1       = 1;
constexpr chip::AttributeId eventId3_1            = 1;
constexpr bool isUrgentEvent3_1                   = false;
constexpr chip::EndpointId eventEndpointId3_2     = 2;
constexpr chip::ClusterId eventClusterId3_2       = 2;
constexpr chip::AttributeId eventId3_2            = 2;
constexpr bool isUrgentEvent3_2                   = true;

struct TestSubscriptionInfo : public chip::app::SubscriptionResumptionStorage::SubscriptionInfo
{
    bool operator==(const SubscriptionInfo & that) const
    {
        if ((mNodeId != that.mNodeId) || (mFabricIndex != that.mFabricIndex) || (mSubscriptionId != that.mSubscriptionId) ||
            (mMinInterval != that.mMinInterval) || (mMaxInterval != that.mMaxInterval) || (mFabricFiltered != that.mFabricFiltered))
        {
            return false;
        }
        if ((mAttributePaths.AllocatedCount() != that.mAttributePaths.AllocatedCount()) ||
            (mEventPaths.AllocatedCount() != that.mEventPaths.AllocatedCount()))
        {
            return false;
        }
        for (size_t i = 0; i < mAttributePaths.AllocatedCount(); i++)
        {
            if ((mAttributePaths[i].mEndpointId != that.mAttributePaths[i].mEndpointId) ||
                (mAttributePaths[i].mClusterId != that.mAttributePaths[i].mClusterId) ||
                (mAttributePaths[i].mAttributeId != that.mAttributePaths[i].mAttributeId))
            {
                return false;
            }
        }
        for (size_t i = 0; i < mEventPaths.AllocatedCount(); i++)
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

void TestCount(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    SimpleSubscriptionResumptionStorageTest subscriptionStorage;
    subscriptionStorage.Init(&storage);

    // Check by default it returns correct value
    NL_TEST_ASSERT(inSuite, subscriptionStorage.TestMaxCount() == CHIP_IM_MAX_NUM_SUBSCRIPTIONS);

    // Force larger value and check that it returns the larger value
    uint32_t countMaxToSave = 2 * CHIP_IM_MAX_NUM_SUBSCRIPTIONS;
    storage.SyncSetKeyValue(chip::DefaultStorageKeyAllocator::SubscriptionResumptionMaxCount().KeyName(), &countMaxToSave,
                            static_cast<uint16_t>(sizeof(uint32_t)));
    NL_TEST_ASSERT(inSuite, subscriptionStorage.TestMaxCount() == (2 * CHIP_IM_MAX_NUM_SUBSCRIPTIONS));

    // Reset
    storage.SyncDeleteKeyValue(chip::DefaultStorageKeyAllocator::SubscriptionResumptionMaxCount().KeyName());

    // Write some subscriptions and see the counts are correct
    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo = { .mNodeId = node1, .mFabricIndex = fabric1 };

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
    CHIP_ERROR err = subscriptionStorage.DeleteAll(fabric1);
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

    NL_TEST_ASSERT(inSuite, false);
}

void TestState(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    SimpleSubscriptionResumptionStorageTest subscriptionStorage;
    subscriptionStorage.Init(&storage);

    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo1 = {
        .mNodeId         = node1,
        .mFabricIndex    = fabric1,
        .mSubscriptionId = subscriptionId1,
        .mMinInterval    = minInterval1,
        .mMaxInterval    = maxInterval1,
        .mFabricFiltered = fabricFiltered1,
    };
    subscriptionInfo1.mAttributePaths.Calloc(2);
    subscriptionInfo1.mAttributePaths[0].mEndpointId  = attributeEndpointId1_1;
    subscriptionInfo1.mAttributePaths[0].mClusterId   = attributeClusterId1_1;
    subscriptionInfo1.mAttributePaths[0].mAttributeId = attributeId1_1;
    subscriptionInfo1.mAttributePaths[1].mEndpointId  = attributeEndpointId1_2;
    subscriptionInfo1.mAttributePaths[1].mClusterId   = attributeClusterId1_2;
    subscriptionInfo1.mAttributePaths[1].mAttributeId = attributeId1_2;

    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo2 = {
        .mNodeId         = node2,
        .mFabricIndex    = fabric2,
        .mSubscriptionId = subscriptionId2,
        .mMinInterval    = minInterval2,
        .mMaxInterval    = maxInterval2,
        .mFabricFiltered = fabricFiltered2,
    };
    subscriptionInfo2.mEventPaths.Calloc(2);
    subscriptionInfo2.mEventPaths[0].mEndpointId    = eventEndpointId2_1;
    subscriptionInfo2.mEventPaths[0].mClusterId     = eventClusterId2_1;
    subscriptionInfo2.mEventPaths[0].mEventId       = eventId2_1;
    subscriptionInfo2.mEventPaths[0].mIsUrgentEvent = isUrgentEvent2_1;
    subscriptionInfo2.mEventPaths[1].mEndpointId    = eventEndpointId2_2;
    subscriptionInfo2.mEventPaths[1].mClusterId     = eventClusterId2_2;
    subscriptionInfo2.mEventPaths[1].mEventId       = eventId2_2;
    subscriptionInfo2.mEventPaths[1].mIsUrgentEvent = isUrgentEvent2_2;

    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo3 = {
        .mNodeId         = node3,
        .mFabricIndex    = fabric3,
        .mSubscriptionId = subscriptionId3,
        .mMinInterval    = minInterval3,
        .mMaxInterval    = maxInterval3,
        .mFabricFiltered = fabricFiltered3,
    };
    subscriptionInfo3.mAttributePaths.Calloc(2);
    subscriptionInfo3.mAttributePaths[0].mEndpointId  = attributeEndpointId3_1;
    subscriptionInfo3.mAttributePaths[0].mClusterId   = attributeClusterId3_1;
    subscriptionInfo3.mAttributePaths[0].mAttributeId = attributeId3_1;
    subscriptionInfo3.mAttributePaths[1].mEndpointId  = attributeEndpointId3_2;
    subscriptionInfo3.mAttributePaths[1].mClusterId   = attributeClusterId3_2;
    subscriptionInfo3.mAttributePaths[1].mAttributeId = attributeId3_2;
    subscriptionInfo3.mEventPaths.Calloc(2);
    subscriptionInfo3.mEventPaths[0].mEndpointId    = eventEndpointId3_1;
    subscriptionInfo3.mEventPaths[0].mClusterId     = eventClusterId3_1;
    subscriptionInfo3.mEventPaths[0].mEventId       = eventId3_1;
    subscriptionInfo2.mEventPaths[0].mIsUrgentEvent = isUrgentEvent3_1;
    subscriptionInfo3.mEventPaths[1].mEndpointId    = eventEndpointId3_2;
    subscriptionInfo3.mEventPaths[1].mClusterId     = eventClusterId3_2;
    subscriptionInfo3.mEventPaths[1].mEventId       = eventId3_2;
    subscriptionInfo2.mEventPaths[1].mIsUrgentEvent = isUrgentEvent3_2;

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

    uint32_t countMax = 0;
    uint16_t len      = sizeof(countMax);
    err = storage.SyncGetKeyValue(chip::DefaultStorageKeyAllocator::SubscriptionResumptionMaxCount().KeyName(), &countMax, len);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

static constexpr chip::TLV::Tag kTestValue1Tag = chip::TLV::ContextTag(30);
static constexpr chip::TLV::Tag kTestValue2Tag = chip::TLV::ContextTag(31);

void TestStateUnexpectedFields(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    SimpleSubscriptionResumptionStorageTest subscriptionStorage;
    subscriptionStorage.Init(&storage);

    // Write additional entries at the end of TLV and see it still loads correctly
    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo1 = {
        .mNodeId         = node1,
        .mFabricIndex    = fabric1,
        .mSubscriptionId = subscriptionId1,
        .mMinInterval    = minInterval1,
        .mMaxInterval    = maxInterval1,
        .mFabricFiltered = fabricFiltered1,
    };
    subscriptionInfo1.mAttributePaths.Calloc(2);
    subscriptionInfo1.mAttributePaths[0].mEndpointId  = attributeEndpointId1_1;
    subscriptionInfo1.mAttributePaths[0].mClusterId   = attributeClusterId1_1;
    subscriptionInfo1.mAttributePaths[0].mAttributeId = attributeId1_1;
    subscriptionInfo1.mAttributePaths[1].mEndpointId  = attributeEndpointId1_2;
    subscriptionInfo1.mAttributePaths[1].mClusterId   = attributeClusterId1_2;
    subscriptionInfo1.mAttributePaths[1].mAttributeId = attributeId1_2;

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

void TestStateTooBigToLoad(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    SimpleSubscriptionResumptionStorageTest subscriptionStorage;
    subscriptionStorage.Init(&storage);

    // Write additional too-big data at the end of TLV and see it fails to loads and entry deleted
    chip::app::SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo1 = {
        .mNodeId         = node1,
        .mFabricIndex    = fabric1,
        .mSubscriptionId = subscriptionId1,
        .mMinInterval    = minInterval1,
        .mMaxInterval    = maxInterval1,
        .mFabricFiltered = fabricFiltered1,
    };
    subscriptionInfo1.mAttributePaths.Calloc(2);
    subscriptionInfo1.mAttributePaths[0].mEndpointId  = attributeEndpointId1_1;
    subscriptionInfo1.mAttributePaths[0].mClusterId   = attributeClusterId1_1;
    subscriptionInfo1.mAttributePaths[0].mAttributeId = attributeId1_1;
    subscriptionInfo1.mAttributePaths[1].mEndpointId  = attributeEndpointId1_2;
    subscriptionInfo1.mAttributePaths[1].mClusterId   = attributeClusterId1_2;
    subscriptionInfo1.mAttributePaths[1].mAttributeId = attributeId1_2;

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

void TestStateJunkData(nlTestSuite * inSuite, void * inContext)
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
int Test_Setup(void * inContext)
{
    VerifyOrReturnError(CHIP_NO_ERROR == chip::Platform::MemoryInit(), FAILURE);

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

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestCount", TestCount),
    NL_TEST_DEF("TestState", TestState),
    NL_TEST_DEF("TestStateUnexpectedFields", TestStateUnexpectedFields),
    NL_TEST_DEF("TestStateTooBigToLoad", TestStateTooBigToLoad),
    NL_TEST_DEF("TestStateJunkData", TestStateJunkData),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SimpleSubscriptionResumptionStorage",
    &sTests[0],
    &Test_Setup, &Test_Teardown
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
