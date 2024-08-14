/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/thread-network-directory-server/DefaultThreadNetworkDirectoryStorage.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>

#include <cstdint>
#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;

namespace {

class TestDefaultThreadNetworkDirectoryStorage : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    TestPersistentStorageDelegate persistentStorageDelegate;
    DefaultThreadNetworkDirectoryStorage storageImpl{ persistentStorageDelegate };
    ThreadNetworkDirectoryStorage & storage = storageImpl;
};

TEST_F(TestDefaultThreadNetworkDirectoryStorage, TestEmptyStorage)
{
    EXPECT_GE(storage.Capacity(), 2); // can't meaningfully test with less and spec requires 2 per fabric

    {
        ThreadNetworkDirectoryStorage::ExtendedPanId exPanId;
        auto * iterator = storage.IterateNetworkIds();
        EXPECT_TRUE(iterator != nullptr);
        EXPECT_EQ(iterator->Count(), 0u);
        EXPECT_FALSE(iterator->Next(exPanId));
        iterator->Release();
    }

    ThreadNetworkDirectoryStorage::ExtendedPanId exPanId(UINT64_C(0x1122334455667788));
    EXPECT_FALSE(storage.ContainsNetwork(exPanId));
    EXPECT_EQ(storage.RemoveNetwork(exPanId), CHIP_ERROR_NOT_FOUND);
    MutableByteSpan mutableSpan;
    EXPECT_EQ(storage.GetNetworkDataset(exPanId, mutableSpan), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDefaultThreadNetworkDirectoryStorage, TestAddEmptyDataset)
{
    ThreadNetworkDirectoryStorage::ExtendedPanId exPanId(UINT64_C(0xd00fd00fdeadbeef));
    EXPECT_EQ(storage.AddOrUpdateNetwork(exPanId, ByteSpan()), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestDefaultThreadNetworkDirectoryStorage, TestAddDatasetTooLong)
{
    ThreadNetworkDirectoryStorage::ExtendedPanId exPanId(UINT64_C(0xd00fd00fdeadbeef));
    uint8_t dataset[ThreadNetworkDirectoryStorage::kMaxThreadDatasetLen + 1] = { 1, 2, 3 };
    EXPECT_EQ(storage.AddOrUpdateNetwork(exPanId, ByteSpan(dataset)), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestDefaultThreadNetworkDirectoryStorage, TestAddRemoveNetworks)
{
    const ThreadNetworkDirectoryStorage::ExtendedPanId panA(UINT64_C(0xd00fd00fdeadbeef));
    const uint8_t datasetA[]        = { 1 };
    const uint8_t updatedDatasetA[] = { 0x11, 0x11 };
    const ThreadNetworkDirectoryStorage::ExtendedPanId panB(UINT64_C(0xe475cafef00df00d));
    const uint8_t datasetB[] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
    uint8_t mutableBytes[ThreadNetworkDirectoryStorage::kMaxThreadDatasetLen];

    // Add Pan A only
    EXPECT_EQ(storage.AddOrUpdateNetwork(panA, ByteSpan(datasetA)), CHIP_NO_ERROR);
    EXPECT_TRUE(storage.ContainsNetwork(panA));
    EXPECT_FALSE(storage.ContainsNetwork(panB));

    {
        MutableByteSpan retrievedDatasetA(mutableBytes);
        EXPECT_EQ(storage.GetNetworkDataset(panA, retrievedDatasetA), CHIP_NO_ERROR);
        EXPECT_TRUE(retrievedDatasetA.data_equal(ByteSpan(datasetA)));
    }
    {
        MutableByteSpan emptyMutableSpan;
        EXPECT_EQ(storage.GetNetworkDataset(panA, emptyMutableSpan), CHIP_ERROR_BUFFER_TOO_SMALL);
    }
    {
        ThreadNetworkDirectoryStorage::ExtendedPanId exPanId;
        auto * iterator = storage.IterateNetworkIds();
        EXPECT_TRUE(iterator != nullptr);
        EXPECT_EQ(iterator->Count(), 1u);
        EXPECT_TRUE(iterator->Next(exPanId));
        EXPECT_EQ(exPanId, panA);
        EXPECT_NE(exPanId, panB); // just to ensure operator== is sane
        EXPECT_FALSE(iterator->Next(exPanId));
        EXPECT_EQ(iterator->Count(), 1u);
        iterator->Release();
    }

    // Update Pan A
    EXPECT_EQ(storage.AddOrUpdateNetwork(panA, ByteSpan(updatedDatasetA)), CHIP_NO_ERROR);
    EXPECT_TRUE(storage.ContainsNetwork(panA));
    EXPECT_FALSE(storage.ContainsNetwork(panB));

    {
        MutableByteSpan retrievedUpdatedDatasetA(mutableBytes);
        EXPECT_EQ(storage.GetNetworkDataset(panA, retrievedUpdatedDatasetA), CHIP_NO_ERROR);
        EXPECT_TRUE(retrievedUpdatedDatasetA.data_equal(ByteSpan(updatedDatasetA)));
    }
    {
        auto * iterator = storage.IterateNetworkIds();
        EXPECT_TRUE(iterator != nullptr);
        EXPECT_EQ(iterator->Count(), 1u); // still 1
        iterator->Release();
    }

    // Add Pan B
    EXPECT_EQ(storage.AddOrUpdateNetwork(panB, ByteSpan(datasetB)), CHIP_NO_ERROR);
    EXPECT_TRUE(storage.ContainsNetwork(panA));
    EXPECT_TRUE(storage.ContainsNetwork(panB));
    {
        ThreadNetworkDirectoryStorage::ExtendedPanId exPanId1;
        ThreadNetworkDirectoryStorage::ExtendedPanId exPanId2;
        ThreadNetworkDirectoryStorage::ExtendedPanId unused;
        auto * iterator = storage.IterateNetworkIds();
        EXPECT_TRUE(iterator != nullptr);
        EXPECT_EQ(iterator->Count(), 2u);
        EXPECT_TRUE(iterator->Next(exPanId1));
        EXPECT_TRUE(iterator->Next(exPanId2));
        EXPECT_TRUE((exPanId1 == panA && exPanId2 == panB) || (exPanId1 == panB && exPanId2 == panA));
        EXPECT_FALSE(iterator->Next(unused));
        EXPECT_EQ(iterator->Count(), 2u);
        iterator->Release();
    }
    {
        MutableByteSpan retrievedDatasetB(mutableBytes);
        EXPECT_EQ(storage.GetNetworkDataset(panB, retrievedDatasetB), CHIP_NO_ERROR);
        EXPECT_TRUE(retrievedDatasetB.data_equal(ByteSpan(datasetB)));
    }

    // Remove Pan A
    EXPECT_EQ(storage.RemoveNetwork(panA), CHIP_NO_ERROR);
    EXPECT_FALSE(storage.ContainsNetwork(panA));
    EXPECT_TRUE(storage.ContainsNetwork(panB));

    {
        MutableByteSpan mutableSpan;
        EXPECT_EQ(storage.GetNetworkDataset(panA, mutableSpan), CHIP_ERROR_NOT_FOUND);
    }
    {
        ThreadNetworkDirectoryStorage::ExtendedPanId exPanId;
        auto * iterator = storage.IterateNetworkIds();
        EXPECT_TRUE(iterator != nullptr);
        EXPECT_EQ(iterator->Count(), 1u);
        EXPECT_TRUE(iterator->Next(exPanId));
        EXPECT_EQ(exPanId, panB);
        EXPECT_FALSE(iterator->Next(exPanId));
        iterator->Release();
    }

    // Remove Pan B
    EXPECT_EQ(storage.RemoveNetwork(panB), CHIP_NO_ERROR);
    EXPECT_FALSE(storage.ContainsNetwork(panA));
    EXPECT_FALSE(storage.ContainsNetwork(panB));

    {
        MutableByteSpan mutableSpan;
        EXPECT_EQ(storage.GetNetworkDataset(panB, mutableSpan), CHIP_ERROR_NOT_FOUND);
    }
    {
        ThreadNetworkDirectoryStorage::ExtendedPanId exPanId;
        auto * iterator = storage.IterateNetworkIds();
        EXPECT_TRUE(iterator != nullptr);
        EXPECT_EQ(iterator->Count(), 0u);
        EXPECT_FALSE(iterator->Next(exPanId));
        iterator->Release();
    }
}

TEST_F(TestDefaultThreadNetworkDirectoryStorage, TestStorageOverflow)
{
    // Fill up the storage to capacity
    uint8_t dataset[] = { 42 };
    for (uint64_t id = 0; id < storage.Capacity(); id++)
    {
        EXPECT_EQ(storage.AddOrUpdateNetwork(ThreadNetworkDirectoryStorage::ExtendedPanId(id), ByteSpan(dataset)), CHIP_NO_ERROR);
    }

    // Capacity limit should be enforced
    const ThreadNetworkDirectoryStorage::ExtendedPanId exPanId(UINT64_C(0xd00fd00fdeadbeef));
    EXPECT_EQ(storage.AddOrUpdateNetwork(exPanId, ByteSpan(dataset)), CHIP_ERROR_NO_MEMORY);

    // Updating an existing network is still possible
    dataset[0] = 88;
    EXPECT_EQ(storage.AddOrUpdateNetwork(ThreadNetworkDirectoryStorage::ExtendedPanId(0u), ByteSpan(dataset)), CHIP_NO_ERROR);
}

TEST_F(TestDefaultThreadNetworkDirectoryStorage, TestAddNetworkStorageFailure)
{
    const ThreadNetworkDirectoryStorage::ExtendedPanId panA(UINT64_C(0xd00fd00fdeadbeef));
    const uint8_t datasetA[] = { 1 };
    const ThreadNetworkDirectoryStorage::ExtendedPanId panB(UINT64_C(0xe475cafef00df00d));
    uint8_t datasetB[] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };

    EXPECT_EQ(storage.AddOrUpdateNetwork(panA, ByteSpan(datasetA)), CHIP_NO_ERROR);

    // Make the storage delegate return CHIP_ERROR_PERSISTED_STORAGE_FAILED for writes.
    // Attempting to add a network should fail and state should be unaffected.
    persistentStorageDelegate.SetRejectWrites(true);
    EXPECT_EQ(storage.AddOrUpdateNetwork(panB, ByteSpan(datasetB)), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    EXPECT_TRUE(storage.ContainsNetwork(panA));
    EXPECT_FALSE(storage.ContainsNetwork(panB));
    {
        auto * iterator = storage.IterateNetworkIds();
        EXPECT_EQ(iterator->Count(), 1u);
        iterator->Release();
    }
}

TEST_F(TestDefaultThreadNetworkDirectoryStorage, TestAddNetworkStorageWriteIndexFailure)
{
    const ThreadNetworkDirectoryStorage::ExtendedPanId panA(UINT64_C(0xd00fd00fdeadbeef));
    const uint8_t datasetA[] = { 1 };
    const ThreadNetworkDirectoryStorage::ExtendedPanId panB(UINT64_C(0xe475cafef00df00d));
    uint8_t datasetB[] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };

    EXPECT_EQ(storage.AddOrUpdateNetwork(panA, ByteSpan(datasetA)), CHIP_NO_ERROR);

    // White box test: Poison the index key specifically, so the dataset
    // can be written but the subsequent update of the index fails.
    // Attempting to add a network should fail and state should be unaffected.
    StorageKeyName indexKey = DefaultStorageKeyAllocator::ThreadNetworkDirectoryIndex();
    persistentStorageDelegate.AddPoisonKey(indexKey.KeyName());
    EXPECT_EQ(storage.AddOrUpdateNetwork(panB, ByteSpan(datasetB)), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    EXPECT_TRUE(storage.ContainsNetwork(panA));
    EXPECT_FALSE(storage.ContainsNetwork(panB));
    {
        auto * iterator = storage.IterateNetworkIds();
        EXPECT_EQ(iterator->Count(), 1u);
        iterator->Release();
    }
}

TEST_F(TestDefaultThreadNetworkDirectoryStorage, TestRemoveNetworkStorageFailure)
{
    const ThreadNetworkDirectoryStorage::ExtendedPanId panA(UINT64_C(0xd00f0001));
    const ThreadNetworkDirectoryStorage::ExtendedPanId panB(UINT64_C(0xd00f0002));
    const ThreadNetworkDirectoryStorage::ExtendedPanId panC(UINT64_C(0xd00f0003));
    const uint8_t dataset[] = { 1 };

    // Just in case the test is compiled with CHIP_CONFIG_MAX_FABRICS == 1
    bool canStorePanC = (storage.Capacity() >= 3);

    // Add Pans A, B, and C
    EXPECT_EQ(storage.AddOrUpdateNetwork(panA, ByteSpan(dataset)), CHIP_NO_ERROR);
    EXPECT_EQ(storage.AddOrUpdateNetwork(panB, ByteSpan(dataset)), CHIP_NO_ERROR);
    if (canStorePanC)
    {
        EXPECT_EQ(storage.AddOrUpdateNetwork(panC, ByteSpan(dataset)), CHIP_NO_ERROR);
    }

    // Make the storage delegate return CHIP_ERROR_PERSISTED_STORAGE_FAILED for writes.
    // Attempting to remove a network should fail and state should be unaffected.
    persistentStorageDelegate.SetRejectWrites(true);
    EXPECT_EQ(storage.RemoveNetwork(panA), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    EXPECT_TRUE(storage.ContainsNetwork(panA));
    EXPECT_TRUE(storage.ContainsNetwork(panB));
    if (canStorePanC)
    {
        EXPECT_TRUE(storage.ContainsNetwork(panC));
    }
    {
        auto * iterator = storage.IterateNetworkIds();
        EXPECT_EQ(iterator->Count(), canStorePanC ? 3u : 2u);
        iterator->Release();
    }
}

} // namespace
