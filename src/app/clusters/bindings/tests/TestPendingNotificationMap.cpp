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

#include <app/clusters/bindings/PendingNotificationMap.h>
#include <app/clusters/bindings/binding-table.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Binding;

namespace {

class TestPendingNotificationMap : public ::testing::Test
{
public:
    void SetUp() override { ASSERT_SUCCESS(Platform::MemoryInit()); }
    void TearDown() override { Platform::MemoryShutdown(); }
    static void SetUpTestSuite()
    {
        static chip::TestPersistentStorageDelegate storage;
        Binding::Table::GetInstance().SetPersistentStorage(&storage);
    }
};

void ClearBindingTable(Binding::Table & table)
{
    auto iter = table.begin();
    while (iter != table.end())
    {
        EXPECT_SUCCESS(table.RemoveAt(iter));
    }
}

void CreateDefaultFullBindingTable(Binding::Table & table)
{
    for (uint8_t i = 0; i < Binding::Table::kMaxBindingEntries; i++)
    {
        EXPECT_SUCCESS(table.Add(Binding::TableEntry(i / 10, i % 5, 0, 0, std::make_optional<ClusterId>(i))));
    }
}

struct PendingRecord
{
    int releases     = 0;
    int failures     = 0;
    CHIP_ERROR error = CHIP_NO_ERROR;
    static void Release(void * context) { ++static_cast<PendingRecord *>(context)->releases; }
    static void Failed(const TableEntry &, void * context, CHIP_ERROR error)
    {
        auto & record = *static_cast<PendingRecord *>(context);
        ++record.failures;
        record.error = error;
    }
};

TEST_F(TestPendingNotificationMap, CommandsCannotReplaceOrBeReplaced)
{
    TestPersistentStorageDelegate storage;
    Table table;
    table.SetPersistentStorage(&storage);
    ASSERT_SUCCESS(table.Add(TableEntry::ForNode(1, 123, 1, 2, 6)));
    PendingRecord first;
    PendingRecord rejected;
    {
        PendingNotificationMap map(table);
        map.RegisterPendingNotificationContextReleaseHandler(PendingRecord::Release);
        auto * command = map.NewPendingNotificationContext(&first, PendingRecord::Failed);
        ASSERT_NE(command, nullptr);
        ASSERT_SUCCESS(map.AddPendingNotification(0, command));
        auto * other = map.NewPendingNotificationContext(&rejected, PendingRecord::Failed);
        ASSERT_NE(other, nullptr);
        EXPECT_EQ(map.AddPendingNotification(0, other), CHIP_ERROR_BUSY);
        EXPECT_EQ(map.AddPendingNotification(0, nullptr), CHIP_ERROR_BUSY);
        EXPECT_EQ(first.releases, 0);
        EXPECT_EQ(first.failures, 0);
        Platform::Delete(other);
        ScopedNodeId peer;
        EXPECT_EQ(map.FindLRUConnectPeer(peer), CHIP_ERROR_NOT_FOUND);
    }
    EXPECT_EQ(first.releases, 1);
    EXPECT_EQ(first.failures, 1);
    EXPECT_EQ(first.error, CHIP_ERROR_CANCELLED);
    EXPECT_EQ(rejected.releases, 0);
    EXPECT_EQ(rejected.failures, 0);
}

TEST_F(TestPendingNotificationMap, NotificationsStillCoalesceAndCommandsCannotReplaceThem)
{
    TestPersistentStorageDelegate storage;
    Table table;
    table.SetPersistentStorage(&storage);
    ASSERT_SUCCESS(table.Add(TableEntry::ForNode(1, 123, 1, 2, 6)));
    PendingRecord record;
    PendingNotificationMap map(table);
    map.RegisterPendingNotificationContextReleaseHandler(PendingRecord::Release);
    auto * notification = map.NewPendingNotificationContext(&record);
    ASSERT_NE(notification, nullptr);
    ASSERT_SUCCESS(map.AddPendingNotification(0, notification));
    auto * command = map.NewPendingNotificationContext(&record, PendingRecord::Failed);
    ASSERT_NE(command, nullptr);
    EXPECT_EQ(map.AddPendingNotification(0, command), CHIP_ERROR_BUSY);
    Platform::Delete(command);
    EXPECT_EQ(record.releases, 0);
    ASSERT_SUCCESS(map.AddPendingNotification(0, nullptr));
    EXPECT_EQ(record.releases, 1);
    EXPECT_EQ(record.failures, 0);
}

TEST_F(TestPendingNotificationMap, LruExcludesAllBindingsForPeerWithPendingCommand)
{
    TestPersistentStorageDelegate storage;
    Table table;
    table.SetPersistentStorage(&storage);
    ASSERT_SUCCESS(table.Add(TableEntry::ForNode(1, 123, 1, 2, 6)));
    ASSERT_SUCCESS(table.Add(TableEntry::ForNode(1, 123, 1, 3, 6)));
    ASSERT_SUCCESS(table.Add(TableEntry::ForNode(1, 456, 1, 4, 6)));
    PendingRecord record;
    PendingNotificationMap map(table);
    auto * command = map.NewPendingNotificationContext(&record, PendingRecord::Failed);
    ASSERT_NE(command, nullptr);
    ASSERT_SUCCESS(map.AddPendingNotification(0, command));
    ASSERT_SUCCESS(map.AddPendingNotification(1, nullptr));
    ASSERT_SUCCESS(map.AddPendingNotification(2, nullptr));
    ScopedNodeId peer;
    EXPECT_SUCCESS(map.FindLRUConnectPeer(peer));
    EXPECT_EQ(peer, ScopedNodeId(456, 1));
    map.RemoveAllEntriesForNode(peer);
    EXPECT_TRUE(map.HasPendingNotification(0, true));
    EXPECT_EQ(map.FindLRUConnectPeer(peer), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestPendingNotificationMap, TestEmptyMap)
{
    PendingNotificationMap pendingMap(Binding::Table::GetInstance());
    EXPECT_EQ(pendingMap.begin(), pendingMap.end());
    chip::ScopedNodeId peer;
    EXPECT_EQ(pendingMap.FindLRUConnectPeer(peer), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestPendingNotificationMap, TestAddRemove)
{
    PendingNotificationMap pendingMap(Binding::Table::GetInstance());
    ClearBindingTable(Binding::Table::GetInstance());
    CreateDefaultFullBindingTable(Binding::Table::GetInstance());
    for (uint8_t i = 0; i < Binding::Table::kMaxBindingEntries; i++)
    {
        EXPECT_EQ(pendingMap.AddPendingNotification(i, nullptr), CHIP_NO_ERROR);
    }
    // Confirm adding in one more element fails
    EXPECT_EQ(pendingMap.AddPendingNotification(Binding::Table::kMaxBindingEntries, nullptr), CHIP_ERROR_NO_MEMORY);

    auto iter = pendingMap.begin();
    for (uint8_t i = 0; i < Binding::Table::kMaxBindingEntries; i++)
    {
        PendingNotificationEntry entry = *iter;
        EXPECT_EQ(entry.mBindingEntryId, i);
        ++iter;
    }
    EXPECT_EQ(iter, pendingMap.end());
    pendingMap.RemoveAllEntriesForNode(chip::ScopedNodeId());
    size_t pendingMapCount = 0;
    for (iter = pendingMap.begin(); iter != pendingMap.end(); ++iter)
    {
        PendingNotificationEntry entry   = *iter;
        Binding::TableEntry bindingEntry = Binding::Table::GetInstance().GetAt(entry.mBindingEntryId);
        pendingMapCount++;
        EXPECT_NE(chip::ScopedNodeId(bindingEntry.nodeId, bindingEntry.fabricIndex), chip::ScopedNodeId());
    }
    EXPECT_EQ(pendingMapCount, Binding::Table::kMaxBindingEntries - 2);
    pendingMap.RemoveAllEntriesForFabric(0);
    pendingMapCount = 0;
    for (iter = pendingMap.begin(); iter != pendingMap.end(); ++iter)
    {
        PendingNotificationEntry entry   = *iter;
        Binding::TableEntry bindingEntry = Binding::Table::GetInstance().GetAt(entry.mBindingEntryId);
        pendingMapCount++;
        EXPECT_NE(bindingEntry.fabricIndex, 0);
    }
    EXPECT_EQ(pendingMapCount, Binding::Table::kMaxBindingEntries - 10);
    const FabricIndex maxFabricIndex = static_cast<FabricIndex>(Binding::Table::kMaxBindingEntries / 10);
    for (FabricIndex index = 1; index <= maxFabricIndex; index++)
    {
        pendingMap.RemoveAllEntriesForFabric(index);
    }
    EXPECT_EQ(pendingMap.begin(), pendingMap.end());
}

TEST_F(TestPendingNotificationMap, TestLRUEntry)
{
    PendingNotificationMap pendingMap(Binding::Table::GetInstance());
    ClearBindingTable(Binding::Table::GetInstance());
    CreateDefaultFullBindingTable(Binding::Table::GetInstance());
    EXPECT_EQ(pendingMap.AddPendingNotification(0, nullptr), CHIP_NO_ERROR);
    EXPECT_EQ(pendingMap.AddPendingNotification(1, nullptr), CHIP_NO_ERROR);
    EXPECT_EQ(pendingMap.AddPendingNotification(5, nullptr), CHIP_NO_ERROR);
    EXPECT_EQ(pendingMap.AddPendingNotification(7, nullptr), CHIP_NO_ERROR);
    EXPECT_EQ(pendingMap.AddPendingNotification(11, nullptr), CHIP_NO_ERROR);

    chip::ScopedNodeId node;

    EXPECT_EQ(pendingMap.FindLRUConnectPeer(node), CHIP_NO_ERROR);
    EXPECT_EQ(node.GetFabricIndex(), 0u);
    EXPECT_EQ(node.GetNodeId(), 1u);

    pendingMap.RemoveEntry(1);
    EXPECT_EQ(pendingMap.FindLRUConnectPeer(node), CHIP_NO_ERROR);
    EXPECT_EQ(node.GetFabricIndex(), 0u);
    EXPECT_EQ(node.GetNodeId(), 0u);

    pendingMap.RemoveAllEntriesForFabric(0);
    EXPECT_EQ(pendingMap.FindLRUConnectPeer(node), CHIP_NO_ERROR);
    EXPECT_EQ(node.GetFabricIndex(), 1u);
    EXPECT_EQ(node.GetNodeId(), 1u);
}

} // namespace
