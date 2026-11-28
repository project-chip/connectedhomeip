/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/bindings/binding-table.h>
#include <lib/core/TLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

#include <vector>

using namespace chip::app::Clusters;

namespace {

void VerifyTableSame(Binding::Table & table, const std::vector<Binding::TableEntry> & expected)
{
    ASSERT_EQ(table.Size(), expected.size());
    auto iter1 = table.begin();
    auto iter2 = expected.begin();
    while (iter2 != expected.end())
    {
        EXPECT_EQ(*iter1, *iter2);
        ++iter1;
        ++iter2;
    }
    EXPECT_EQ(iter1, table.end());
}

void VerifyRestored(chip::TestPersistentStorageDelegate & storage, const std::vector<Binding::TableEntry> & expected)
{
    Binding::Table restoredTable;
    restoredTable.SetPersistentStorage(&storage);
    EXPECT_EQ(restoredTable.LoadFromStorage(), CHIP_NO_ERROR);
    VerifyTableSame(restoredTable, expected);
}

TEST(TestBindingTable, TestEmptyBindingTable)
{
    Binding::Table table;
    chip::TestPersistentStorageDelegate testStorage;
    table.SetPersistentStorage(&testStorage);
    EXPECT_EQ(table.Size(), 0u);
    EXPECT_EQ(table.begin(), table.end());
}

TEST(TestBindingTable, TestAdd)
{
    Binding::Table table;
    chip::TestPersistentStorageDelegate testStorage;
    table.SetPersistentStorage(&testStorage);
    Binding::TableEntry unusedEntry;
    unusedEntry.type = Binding::MATTER_UNUSED_BINDING;
    EXPECT_EQ(table.Add(unusedEntry), CHIP_ERROR_INVALID_ARGUMENT);
    for (uint8_t i = 0; i < Binding::Table::kMaxBindingEntries; i++)
    {
        EXPECT_EQ(table.Add(Binding::TableEntry::ForNode(0, i, 0, 0, std::nullopt)), CHIP_NO_ERROR);
    }
    EXPECT_EQ(table.Add(Binding::TableEntry::ForNode(0, 0, 0, 0, std::nullopt)), CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(table.Size(), Binding::Table::kMaxBindingEntries);

    auto iter = table.begin();
    for (uint8_t i = 0; i < Binding::Table::kMaxBindingEntries; i++)
    {
        EXPECT_NE(iter, table.end());
        EXPECT_EQ(iter->nodeId, i);
        EXPECT_EQ(iter.GetIndex(), i);
        ++iter;
    }
    EXPECT_EQ(iter, table.end());
}

TEST(TestBindingTable, TestRemoveThenAdd)
{
    Binding::Table table;
    chip::TestPersistentStorageDelegate testStorage;
    table.SetPersistentStorage(&testStorage);
    EXPECT_EQ(table.Add(Binding::TableEntry::ForNode(0, 0, 0, 0, std::nullopt)), CHIP_NO_ERROR);
    auto iter = table.begin();
    EXPECT_EQ(table.RemoveAt(iter), CHIP_NO_ERROR);
    EXPECT_EQ(iter, table.end());
    EXPECT_EQ(table.Size(), 0u);
    EXPECT_EQ(table.begin(), table.end());
    for (uint8_t i = 0; i < Binding::Table::kMaxBindingEntries; i++)
    {
        EXPECT_EQ(table.Add(Binding::TableEntry::ForNode(0, i, 0, 0, std::nullopt)), CHIP_NO_ERROR);
    }
    iter = table.begin();
    ++iter;
    EXPECT_EQ(table.RemoveAt(iter), CHIP_NO_ERROR);
    EXPECT_EQ(table.Size(), Binding::Table::kMaxBindingEntries - 1);
    EXPECT_EQ(iter->nodeId, 2u);
    EXPECT_EQ(iter.GetIndex(), 2u);
    auto iterCheck = table.begin();
    ++iterCheck;
    EXPECT_EQ(iter, iterCheck);

    EXPECT_EQ(table.Add(Binding::TableEntry::ForNode(0, 1, 0, 0, std::nullopt)), CHIP_NO_ERROR);
    EXPECT_EQ(table.Size(), Binding::Table::kMaxBindingEntries);
    iter = table.begin();
    for (uint8_t i = 0; i < Binding::Table::kMaxBindingEntries - 1; i++)
    {
        ++iter;
    }
    EXPECT_EQ(iter->nodeId, 1u);
    EXPECT_EQ(iter.GetIndex(), 1u);
    ++iter;
    EXPECT_EQ(iter, table.end());
    iter = table.begin();
    EXPECT_EQ(table.RemoveAt(iter), CHIP_NO_ERROR);
    EXPECT_EQ(table.Size(), Binding::Table::kMaxBindingEntries - 1);
    EXPECT_EQ(iter, table.begin());
    EXPECT_EQ(iter.GetIndex(), 2u);
    EXPECT_EQ(iter->nodeId, 2u);
    EXPECT_EQ(table.GetAt(0).type, Binding::MATTER_UNUSED_BINDING);
}

TEST(TestBindingTable, TestPersistentStorage)
{
    chip::TestPersistentStorageDelegate testStorage;
    Binding::Table table;
    chip::Optional<chip::ClusterId> cluster   = chip::MakeOptional<chip::ClusterId>(static_cast<chip::ClusterId>(UINT16_MAX + 6));
    std::vector<Binding::TableEntry> expected = {
        Binding::TableEntry::ForNode(0, 0, 0, 0, std::nullopt),
        Binding::TableEntry::ForNode(1, 1, 0, 0, cluster.std_optional()),
        Binding::TableEntry::ForGroup(2, 2, 0, std::nullopt),
        Binding::TableEntry::ForGroup(3, 3, 0, cluster.std_optional()),
    };
    table.SetPersistentStorage(&testStorage);
    EXPECT_EQ(table.Add(expected[0]), CHIP_NO_ERROR);
    EXPECT_EQ(table.Add(expected[1]), CHIP_NO_ERROR);
    EXPECT_EQ(table.Add(expected[2]), CHIP_NO_ERROR);
    EXPECT_EQ(table.Add(expected[3]), CHIP_NO_ERROR);
    VerifyRestored(testStorage, expected);

    // Verify storage untouched if add fails
    testStorage.AddPoisonKey(chip::DefaultStorageKeyAllocator::BindingTableEntry(4).KeyName());
    EXPECT_NE(table.Add(Binding::TableEntry::ForNode(4, 4, 0, 0, std::nullopt)), CHIP_NO_ERROR);
    VerifyRestored(testStorage, expected);
    testStorage.ClearPoisonKeys();

    // Verify storage untouched if removing head fails
    testStorage.AddPoisonKey(chip::DefaultStorageKeyAllocator::BindingTable().KeyName());
    auto iter = table.begin();
    EXPECT_NE(table.RemoveAt(iter), CHIP_NO_ERROR);
    VerifyTableSame(table, expected);
    testStorage.ClearPoisonKeys();
    VerifyRestored(testStorage, expected);

    // Verify storage untouched if removing other nodes fails
    testStorage.AddPoisonKey(chip::DefaultStorageKeyAllocator::BindingTableEntry(0).KeyName());
    iter = table.begin();
    ++iter;
    EXPECT_NE(table.RemoveAt(iter), CHIP_NO_ERROR);
    VerifyTableSame(table, expected);
    testStorage.ClearPoisonKeys();
    VerifyRestored(testStorage, expected);

    // Verify removing head
    iter = table.begin();
    EXPECT_EQ(table.RemoveAt(iter), CHIP_NO_ERROR);
    VerifyTableSame(table, { expected[1], expected[2], expected[3] });
    VerifyRestored(testStorage, { expected[1], expected[2], expected[3] });

    // Verify removing other nodes
    ++iter;
    EXPECT_EQ(table.RemoveAt(iter), CHIP_NO_ERROR);
    VerifyTableSame(table, { expected[1], expected[3] });
    VerifyRestored(testStorage, { expected[1], expected[3] });
}

TEST(TestBindingTable, TestLoadRejectsOutOfRangeIndex)
{
    chip::TestPersistentStorageDelegate testStorage;

    // Populate one valid entry so a well-formed serialized entry blob exists in storage.
    {
        Binding::Table table;
        table.SetPersistentStorage(&testStorage);
        EXPECT_EQ(table.Add(Binding::TableEntry::ForNode(0, 0, 0, 0, std::nullopt)), CHIP_NO_ERROR);
    }

    // One past the last valid slot; mBindingTable / mNextIndex are sized kMaxBindingEntries.
    constexpr uint8_t kOutOfRangeIndex = static_cast<uint8_t>(Binding::Table::kMaxBindingEntries);

    // Copy the serialized entry to the out-of-range index key so a full entry parses and the load reaches
    // the mBindingTable[index] write.
    {
        uint8_t entryBuffer[128];
        uint16_t entrySize = sizeof(entryBuffer);
        EXPECT_EQ(
            testStorage.SyncGetKeyValue(chip::DefaultStorageKeyAllocator::BindingTableEntry(0).KeyName(), entryBuffer, entrySize),
            CHIP_NO_ERROR);
        EXPECT_EQ(testStorage.SyncSetKeyValue(chip::DefaultStorageKeyAllocator::BindingTableEntry(kOutOfRangeIndex).KeyName(),
                                              entryBuffer, entrySize),
                  CHIP_NO_ERROR);
    }

    // Point the stored list head at the out-of-range index. Layout mirrors Table::SaveListInfo(): an
    // anonymous structure of { storage version (context tag 1) = 1, head (context tag 2) }.
    {
        uint8_t listInfoBuffer[32];
        chip::TLV::TLVWriter writer;
        writer.Init(listInfoBuffer);
        chip::TLV::TLVType container;
        EXPECT_EQ(writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, container), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Put(chip::TLV::ContextTag(1), static_cast<uint32_t>(1)), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Put(chip::TLV::ContextTag(2), kOutOfRangeIndex), CHIP_NO_ERROR);
        EXPECT_EQ(writer.EndContainer(container), CHIP_NO_ERROR);
        EXPECT_EQ(testStorage.SyncSetKeyValue(chip::DefaultStorageKeyAllocator::BindingTable().KeyName(), listInfoBuffer,
                                              static_cast<uint16_t>(writer.GetLengthWritten())),
                  CHIP_NO_ERROR);
    }

    // The load must fail cleanly instead of writing past the end of the table arrays.
    Binding::Table table;
    table.SetPersistentStorage(&testStorage);
    EXPECT_NE(table.LoadFromStorage(), CHIP_NO_ERROR);
    EXPECT_EQ(table.Size(), 0u);
}

class RecordingListener : public Binding::Table::Listener
{
public:
    void OnBindingsChanged(chip::EndpointId endpointId) override { endpoints.push_back(endpointId); }

    std::vector<chip::EndpointId> endpoints;
};

class SelfRemovingListener : public Binding::Table::Listener
{
public:
    explicit SelfRemovingListener(Binding::Table & table) : mTable(table) {}

    void OnBindingsChanged(chip::EndpointId endpointId) override
    {
        endpoints.push_back(endpointId);
        mTable.RemoveListener(*this);
    }

    std::vector<chip::EndpointId> endpoints;

private:
    Binding::Table & mTable;
};

TEST(TestBindingTable, TestListenerNotifications)
{
    chip::TestPersistentStorageDelegate storage;
    Binding::Table table;
    table.SetPersistentStorage(&storage);

    RecordingListener listener;
    table.AddListener(listener);

    // Invalid entry must not notify.
    Binding::TableEntry unusedEntry;
    unusedEntry.type = Binding::MATTER_UNUSED_BINDING;
    EXPECT_EQ(table.Add(unusedEntry), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_TRUE(listener.endpoints.empty());

    // Successful Add notifies with the entry's local endpoint.
    ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForNode(1, 0x10, /* localEndpoint = */ 3, 1, std::nullopt)));
    ASSERT_EQ(listener.endpoints.size(), 1u);
    EXPECT_EQ(listener.endpoints[0], 3u);
    listener.endpoints.clear();

    // Storage failure on Add must not notify.
    storage.SetRejectWrites(true);
    EXPECT_NE(table.Add(Binding::TableEntry::ForNode(1, 0x20, /* localEndpoint = */ 4, 1, std::nullopt)), CHIP_NO_ERROR);
    EXPECT_TRUE(listener.endpoints.empty());

    // Storage failure on RemoveAt must not notify.
    auto iter = table.begin();
    EXPECT_NE(table.RemoveAt(iter), CHIP_NO_ERROR);
    EXPECT_TRUE(listener.endpoints.empty());
    storage.SetRejectWrites(false);

    // Successful RemoveAt notifies with the removed entry's local endpoint.
    iter = table.begin();
    ASSERT_SUCCESS(table.RemoveAt(iter));
    ASSERT_EQ(listener.endpoints.size(), 1u);
    EXPECT_EQ(listener.endpoints[0], 3u);
    listener.endpoints.clear();

    // Full table Add must not notify.
    for (uint8_t i = 0; i < Binding::Table::kMaxBindingEntries; i++)
    {
        ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForNode(1, i, /* localEndpoint = */ 5, 1, std::nullopt)));
    }
    listener.endpoints.clear();
    EXPECT_EQ(table.Add(Binding::TableEntry::ForNode(1, 99, /* localEndpoint = */ 6, 1, std::nullopt)), CHIP_ERROR_NO_MEMORY);
    EXPECT_TRUE(listener.endpoints.empty());

    // LoadFromStorage restores persisted entries without firing mutation notifications.
    Binding::Table restoredTable;
    restoredTable.SetPersistentStorage(&storage);
    RecordingListener restoredListener;
    restoredTable.AddListener(restoredListener);
    ASSERT_SUCCESS(restoredTable.LoadFromStorage());
    EXPECT_EQ(restoredTable.Size(), Binding::Table::kMaxBindingEntries);
    EXPECT_TRUE(restoredListener.endpoints.empty());
}

TEST(TestBindingTable, TestListenerLifecycle)
{
    chip::TestPersistentStorageDelegate storage;
    Binding::Table table;
    table.SetPersistentStorage(&storage);

    RecordingListener listenerA;
    RecordingListener listenerB;

    // Duplicate AddListener is a no-op; removing an unregistered listener is a no-op.
    table.RemoveListener(listenerA);
    table.AddListener(listenerA);
    table.AddListener(listenerA);
    table.AddListener(listenerB);

    ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForGroup(1, 0x0101, /* localEndpoint = */ 7, std::nullopt)));
    ASSERT_EQ(listenerA.endpoints.size(), 1u);
    EXPECT_EQ(listenerA.endpoints[0], 7u);
    ASSERT_EQ(listenerB.endpoints.size(), 1u);
    EXPECT_EQ(listenerB.endpoints[0], 7u);
    listenerA.endpoints.clear();
    listenerB.endpoints.clear();

    // RemoveListener is idempotent and stops further notifications for that listener.
    table.RemoveListener(listenerA);
    table.RemoveListener(listenerA);
    ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForGroup(1, 0x0102, /* localEndpoint = */ 8, std::nullopt)));
    EXPECT_TRUE(listenerA.endpoints.empty());
    ASSERT_EQ(listenerB.endpoints.size(), 1u);
    EXPECT_EQ(listenerB.endpoints[0], 8u);
    listenerB.endpoints.clear();

    // A listener that removes itself inside OnBindingsChanged does not skip subsequent listeners.
    table.RemoveListener(listenerB);
    SelfRemovingListener selfRemover(table);
    table.AddListener(selfRemover);
    table.AddListener(listenerB);
    ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForGroup(1, 0x0103, /* localEndpoint = */ 9, std::nullopt)));
    ASSERT_EQ(selfRemover.endpoints.size(), 1u);
    EXPECT_EQ(selfRemover.endpoints[0], 9u);
    ASSERT_EQ(listenerB.endpoints.size(), 1u);
    EXPECT_EQ(listenerB.endpoints[0], 9u);
    selfRemover.endpoints.clear();
    listenerB.endpoints.clear();

    // Listener destroyed while still registered auto-unlinks cleanly.
    {
        RecordingListener scopedListener;
        table.AddListener(scopedListener);
        ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForGroup(1, 0x0104, /* localEndpoint = */ 10, std::nullopt)));
        ASSERT_EQ(scopedListener.endpoints.size(), 1u);
    }
    listenerB.endpoints.clear();
    ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForGroup(1, 0x0105, /* localEndpoint = */ 11, std::nullopt)));
    ASSERT_EQ(listenerB.endpoints.size(), 1u);
    EXPECT_EQ(listenerB.endpoints[0], 11u);

    // Table destroyed while a listener is still registered clears the list cleanly.
    RecordingListener longLivedListener;
    {
        Binding::Table shortLivedTable;
        shortLivedTable.SetPersistentStorage(&storage);
        shortLivedTable.AddListener(longLivedListener);
    }
    EXPECT_TRUE(longLivedListener.endpoints.empty());
}

} // namespace
