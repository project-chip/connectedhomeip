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
#include <app/util/binding-table.h>
#include <app/util/config.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <pw_unit_test/framework.h>

using chip::BindingTable;
using chip::ClusterId;
using chip::FabricIndex;
using chip::MakeOptional;
using chip::NodeId;
using chip::NullOptional;
using chip::PendingNotificationEntry;
using chip::PendingNotificationMap;

namespace {

class TestPendingNotificationMap : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        static chip::TestPersistentStorageDelegate storage;
        BindingTable::GetInstance().SetPersistentStorage(&storage);
    }
};

void ClearBindingTable(BindingTable & table)
{
    auto iter = table.begin();
    while (iter != table.end())
    {
        table.RemoveAt(iter);
    }
}

void CreateDefaultFullBindingTable(BindingTable & table)
{
    for (uint8_t i = 0; i < MATTER_BINDING_TABLE_SIZE; i++)
    {
        table.Add(EmberBindingTableEntry::ForNode(i / 10, i % 5, 0, 0, std::make_optional<ClusterId>(i)));
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
    ClearBindingTable(BindingTable::GetInstance());
    CreateDefaultFullBindingTable(BindingTable::GetInstance());
    for (uint8_t i = 0; i < MATTER_BINDING_TABLE_SIZE; i++)
    {
        EXPECT_EQ(pendingMap.AddPendingNotification(i, nullptr), CHIP_NO_ERROR);
    }
    // Confirm adding in one more element fails
    EXPECT_EQ(pendingMap.AddPendingNotification(MATTER_BINDING_TABLE_SIZE, nullptr), CHIP_ERROR_NO_MEMORY);

    auto iter = pendingMap.begin();
    for (uint8_t i = 0; i < MATTER_BINDING_TABLE_SIZE; i++)
    {
        PendingNotificationEntry entry = *iter;
        EXPECT_EQ(entry.mBindingEntryId, i);
        ++iter;
    }
    EXPECT_EQ(iter, pendingMap.end());
    pendingMap.RemoveAllEntriesForNode(chip::ScopedNodeId());
    uint8_t expectedEntryIndecies[] = { 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
    iter                            = pendingMap.begin();
    for (uint8_t ch : expectedEntryIndecies)
    {
        PendingNotificationEntry entry = *iter;
        EXPECT_EQ(entry.mBindingEntryId, ch);
        ++iter;
    }
    EXPECT_EQ(iter, pendingMap.end());
    pendingMap.RemoveAllEntriesForFabric(0);
    iter = pendingMap.begin();
    for (uint8_t i = 0; i < 10; i++)
    {
        PendingNotificationEntry entry = *iter;
        EXPECT_EQ(entry.mBindingEntryId, 10u + i);
        ++iter;
    }
    EXPECT_EQ(iter, pendingMap.end());
    pendingMap.RemoveAllEntriesForFabric(1);
    EXPECT_EQ(pendingMap.begin(), pendingMap.end());
}

TEST_F(TestPendingNotificationMap, TestLRUEntry)
{
    PendingNotificationMap pendingMap;
    ClearBindingTable(BindingTable::GetInstance());
    CreateDefaultFullBindingTable(BindingTable::GetInstance());
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
