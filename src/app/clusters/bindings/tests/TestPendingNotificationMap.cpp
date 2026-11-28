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

struct ReentrantRelease
{
    PendingNotificationMap * map = nullptr;
    Platform::UniquePtr<PendingNotificationContext> command;
    int releases         = 0;
    CHIP_ERROR admission = CHIP_ERROR_INCORRECT_STATE;

    static void QueueCommand(void * context)
    {
        auto & record = *static_cast<ReentrantRelease *>(context);
        ++record.releases;
        record.admission = record.map->AddPendingNotification(0, record.command.get());
        if (record.admission == CHIP_NO_ERROR)
        {
            record.command.release();
        }
    }

    static void Shutdown(void * context)
    {
        auto & record = *static_cast<ReentrantRelease *>(context);
        ++record.releases;
        record.map->Shutdown();
    }
};

TEST_F(TestPendingNotificationMap, ReentrantCommandRejectsOuterNotificationAndRemainsQueued)
{
    TestPersistentStorageDelegate storage;
    Table table;
    table.SetPersistentStorage(&storage);
    ASSERT_SUCCESS(table.Add(TableEntry::ForNode(1, 123, 1, 2, 6)));
    PendingRecord accepted;
    PendingRecord rejected;
    ReentrantRelease removed;
    PendingNotificationMap map(table);
    removed.map = &map;
    map.RegisterPendingNotificationContextReleaseHandler(PendingRecord::Release);
    removed.command.reset(map.NewPendingNotificationContext(&accepted, PendingRecord::Failed));
    ASSERT_NE(removed.command, nullptr);
    const auto * command = removed.command.get();
    Platform::UniquePtr<PendingNotificationContext> replacement(map.NewPendingNotificationContext(&rejected));
    ASSERT_NE(replacement, nullptr);
    map.RegisterPendingNotificationContextReleaseHandler(ReentrantRelease::QueueCommand);
    auto * original = map.NewPendingNotificationContext(&removed);
    ASSERT_NE(original, nullptr);
    ASSERT_SUCCESS(map.AddPendingNotification(0, original));

    const auto error = map.AddPendingNotification(0, replacement.get());
    if (error == CHIP_NO_ERROR)
    {
        replacement.release();
    }
    EXPECT_EQ(error, CHIP_ERROR_BUSY);
    EXPECT_EQ(removed.admission, CHIP_NO_ERROR);
    EXPECT_EQ(removed.releases, 1);
    EXPECT_EQ(accepted.releases, 0);
    EXPECT_EQ(rejected.releases, 0);
    auto it = map.begin();
    ASSERT_NE(it, map.end());
    EXPECT_EQ((*it).mBindingEntryId, 0);
    EXPECT_EQ((*it).mContext, command);
    EXPECT_EQ(++it, map.end());
    map.Shutdown();
    EXPECT_EQ(accepted.releases, 1);
    EXPECT_EQ(accepted.failures, 1);
    EXPECT_EQ(accepted.error, CHIP_ERROR_CANCELLED);
    EXPECT_EQ(rejected.releases, 0);
    ASSERT_NE(replacement, nullptr);
    replacement.reset();
    PendingRecord::Release(&rejected);
    EXPECT_EQ(rejected.releases, 1);
    EXPECT_EQ(removed.releases, 1);
}

TEST_F(TestPendingNotificationMap, ReentrantShutdownRejectsOuterNotificationWithoutTakingContext)
{
    TestPersistentStorageDelegate storage;
    Table table;
    table.SetPersistentStorage(&storage);
    ASSERT_SUCCESS(table.Add(TableEntry::ForNode(1, 123, 1, 2, 6)));
    PendingRecord rejected;
    ReentrantRelease removed;
    PendingNotificationMap map(table);
    removed.map = &map;
    map.RegisterPendingNotificationContextReleaseHandler(ReentrantRelease::Shutdown);
    auto * original = map.NewPendingNotificationContext(&removed);
    ASSERT_NE(original, nullptr);
    ASSERT_SUCCESS(map.AddPendingNotification(0, original));
    map.RegisterPendingNotificationContextReleaseHandler(PendingRecord::Release);
    Platform::UniquePtr<PendingNotificationContext> replacement(map.NewPendingNotificationContext(&rejected));
    ASSERT_NE(replacement, nullptr);

    const auto error = map.AddPendingNotification(0, replacement.get());
    if (error == CHIP_NO_ERROR)
    {
        replacement.release();
    }
    EXPECT_EQ(error, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(map.IsShuttingDown());
    EXPECT_EQ(map.begin(), map.end());
    EXPECT_EQ(removed.releases, 1);
    EXPECT_EQ(rejected.releases, 0);
    map.Shutdown();
    EXPECT_EQ(rejected.releases, 0);
    ASSERT_NE(replacement, nullptr);
    replacement.reset();
    PendingRecord::Release(&rejected);
    EXPECT_EQ(rejected.releases, 1);
}

enum class RemovalScope
{
    kContext,
    kNode,
    kFabric,
};

struct RemovalRecord
{
    PendingNotificationContext * reusedContext = nullptr;
    PendingNotificationMap * map               = nullptr;
    int failures                               = 0;
    int releases                               = 0;
    uint32_t endpoints                         = 0;
    CHIP_ERROR error                           = CHIP_NO_ERROR;
    bool requeue                               = false;
    bool evictEarlier                          = false;

    static void Release(void * context) { ++static_cast<RemovalRecord *>(context)->releases; }
    static void Failed(const TableEntry & binding, void * context, CHIP_ERROR error)
    {
        auto & record = *static_cast<RemovalRecord *>(context);
        ++record.failures;
        record.endpoints |= uint32_t{ 1 } << binding.remote;
        record.error = error;
        if (record.evictEarlier && record.failures == 1)
        {
            record.map->RemoveAllEntriesForNode(ScopedNodeId(111, 2));
        }
        // Limit retries so the regression fails instead of hanging with the old removal loop.
        if (record.requeue && record.failures <= 2)
        {
            auto * next =
                record.reusedContext != nullptr ? record.reusedContext : record.map->NewPendingNotificationContext(&record, Failed);
            ASSERT_NE(next, nullptr);
            const auto admission = record.map->AddPendingNotification(0, next);
            EXPECT_SUCCESS(admission);
            if (admission != CHIP_NO_ERROR && record.reusedContext == nullptr)
            {
                Platform::Delete(next);
            }
        }
    }
};

TEST_F(TestPendingNotificationMap, BulkRemovalDoesNotSkipEntriesWhenCallbackEvictsEarlierPeer)
{
    for (const auto scope : { RemovalScope::kContext, RemovalScope::kNode, RemovalScope::kFabric })
    {
        TestPersistentStorageDelegate storage;
        Table table;
        table.SetPersistentStorage(&storage);
        ASSERT_SUCCESS(table.Add(TableEntry::ForNode(2, 111, 1, 1, 6)));
        ASSERT_SUCCESS(table.Add(TableEntry::ForNode(1, 222, 1, 2, 6)));
        ASSERT_SUCCESS(table.Add(TableEntry::ForNode(1, 222, 1, 3, 6)));
        RemovalRecord record;
        PendingNotificationMap map(table);
        record.map          = &map;
        record.evictEarlier = true;
        map.RegisterPendingNotificationContextReleaseHandler(RemovalRecord::Release);
        ASSERT_SUCCESS(map.AddPendingNotification(0, nullptr));
        auto * context = map.NewPendingNotificationContext(&record, RemovalRecord::Failed);
        ASSERT_NE(context, nullptr);
        ASSERT_SUCCESS(map.AddPendingNotification(1, context));
        ASSERT_SUCCESS(map.AddPendingNotification(2, context));

        switch (scope)
        {
        case RemovalScope::kContext:
            map.RemoveContext(context, CHIP_ERROR_TIMEOUT);
            break;
        case RemovalScope::kNode:
            map.RemoveAllEntriesForNode(ScopedNodeId(222, 1));
            break;
        case RemovalScope::kFabric:
            map.RemoveAllEntriesForFabric(1);
            break;
        }
        EXPECT_EQ(map.begin(), map.end());
        EXPECT_EQ(record.failures, 2);
        EXPECT_EQ(record.endpoints, 0xCu);
        EXPECT_EQ(record.error, scope == RemovalScope::kContext ? CHIP_ERROR_TIMEOUT : CHIP_ERROR_CANCELLED);
        EXPECT_EQ(record.releases, 1);
    }
}

TEST_F(TestPendingNotificationMap, BulkRemovalPreservesCommandsAcceptedDuringCallbacks)
{
    for (const auto scope : { RemovalScope::kContext, RemovalScope::kNode, RemovalScope::kFabric })
    {
        TestPersistentStorageDelegate storage;
        Table table;
        table.SetPersistentStorage(&storage);
        ASSERT_SUCCESS(table.Add(TableEntry::ForNode(1, 222, 1, 2, 6)));
        RemovalRecord record;
        PendingNotificationMap map(table);
        record.map     = &map;
        record.requeue = true;
        map.RegisterPendingNotificationContextReleaseHandler(RemovalRecord::Release);
        auto * context = map.NewPendingNotificationContext(&record, RemovalRecord::Failed);
        ASSERT_NE(context, nullptr);
        ASSERT_SUCCESS(map.AddPendingNotification(0, context));
        if (scope == RemovalScope::kContext)
        {
            record.reusedContext = context;
        }

        switch (scope)
        {
        case RemovalScope::kContext:
            map.RemoveContext(context, CHIP_ERROR_TIMEOUT);
            break;
        case RemovalScope::kNode:
            map.RemoveAllEntriesForNode(ScopedNodeId(222, 1));
            break;
        case RemovalScope::kFabric:
            map.RemoveAllEntriesForFabric(1);
            break;
        }
        record.requeue = false;
        EXPECT_EQ(record.failures, 1);
        const int releasesBeforeShutdown = record.reusedContext == nullptr ? 1 : 0;
        EXPECT_EQ(record.releases, releasesBeforeShutdown);
        EXPECT_TRUE(map.HasPendingNotification(0, true));
        auto it = map.begin();
        ASSERT_NE(it, map.end());
        EXPECT_EQ(++it, map.end());
        map.Shutdown();
        EXPECT_EQ(record.failures, 2);
        EXPECT_EQ(record.releases, releasesBeforeShutdown + 1);
    }
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
