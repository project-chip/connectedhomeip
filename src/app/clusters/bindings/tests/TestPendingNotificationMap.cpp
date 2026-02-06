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

TEST_F(TestPendingNotificationMap, TestEmptyMap)
{
    PendingNotificationMap pendingMap;
    EXPECT_EQ(pendingMap.begin(), pendingMap.end());
    chip::ScopedNodeId peer;
    EXPECT_EQ(pendingMap.FindLRUConnectPeer(peer), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestPendingNotificationMap, TestAddRemove)
{
    PendingNotificationMap pendingMap;
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
    PendingNotificationMap pendingMap;
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
