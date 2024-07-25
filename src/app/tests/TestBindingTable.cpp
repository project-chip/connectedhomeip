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

#include <app/util/binding-table.h>
#include <app/util/config.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>

using chip::BindingTable;

namespace {

void VerifyTableSame(BindingTable & table, const std::vector<EmberBindingTableEntry> & expected)
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

void VerifyRestored(chip::TestPersistentStorageDelegate & storage, const std::vector<EmberBindingTableEntry> & expected)
{
    BindingTable restoredTable;
    restoredTable.SetPersistentStorage(&storage);
    EXPECT_EQ(restoredTable.LoadFromStorage(), CHIP_NO_ERROR);
    VerifyTableSame(restoredTable, expected);
}

TEST(TestBindingTable, TestEmptyBindingTable)
{
    BindingTable table;
    chip::TestPersistentStorageDelegate testStorage;
    table.SetPersistentStorage(&testStorage);
    EXPECT_EQ(table.Size(), 0u);
    EXPECT_EQ(table.begin(), table.end());
}

TEST(TestBindingTable, TestAdd)
{
    BindingTable table;
    chip::TestPersistentStorageDelegate testStorage;
    table.SetPersistentStorage(&testStorage);
    EmberBindingTableEntry unusedEntry;
    unusedEntry.type = MATTER_UNUSED_BINDING;
    EXPECT_EQ(table.Add(unusedEntry), CHIP_ERROR_INVALID_ARGUMENT);
    for (uint8_t i = 0; i < MATTER_BINDING_TABLE_SIZE; i++)
    {
        EXPECT_EQ(table.Add(EmberBindingTableEntry::ForNode(0, i, 0, 0, std::nullopt)), CHIP_NO_ERROR);
    }
    EXPECT_EQ(table.Add(EmberBindingTableEntry::ForNode(0, 0, 0, 0, std::nullopt)), CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(table.Size(), MATTER_BINDING_TABLE_SIZE);

    auto iter = table.begin();
    for (uint8_t i = 0; i < MATTER_BINDING_TABLE_SIZE; i++)
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
    BindingTable table;
    chip::TestPersistentStorageDelegate testStorage;
    table.SetPersistentStorage(&testStorage);
    EXPECT_EQ(table.Add(EmberBindingTableEntry::ForNode(0, 0, 0, 0, std::nullopt)), CHIP_NO_ERROR);
    auto iter = table.begin();
    EXPECT_EQ(table.RemoveAt(iter), CHIP_NO_ERROR);
    EXPECT_EQ(iter, table.end());
    EXPECT_EQ(table.Size(), 0u);
    EXPECT_EQ(table.begin(), table.end());
    for (uint8_t i = 0; i < MATTER_BINDING_TABLE_SIZE; i++)
    {
        EXPECT_EQ(table.Add(EmberBindingTableEntry::ForNode(0, i, 0, 0, std::nullopt)), CHIP_NO_ERROR);
    }
    iter = table.begin();
    ++iter;
    EXPECT_EQ(table.RemoveAt(iter), CHIP_NO_ERROR);
    EXPECT_EQ(table.Size(), MATTER_BINDING_TABLE_SIZE - 1);
    EXPECT_EQ(iter->nodeId, 2u);
    EXPECT_EQ(iter.GetIndex(), 2u);
    auto iterCheck = table.begin();
    ++iterCheck;
    EXPECT_EQ(iter, iterCheck);

    EXPECT_EQ(table.Add(EmberBindingTableEntry::ForNode(0, 1, 0, 0, std::nullopt)), CHIP_NO_ERROR);
    EXPECT_EQ(table.Size(), MATTER_BINDING_TABLE_SIZE);
    iter = table.begin();
    for (uint8_t i = 0; i < MATTER_BINDING_TABLE_SIZE - 1; i++)
    {
        ++iter;
    }
    EXPECT_EQ(iter->nodeId, 1u);
    EXPECT_EQ(iter.GetIndex(), 1u);
    ++iter;
    EXPECT_EQ(iter, table.end());
    iter = table.begin();
    EXPECT_EQ(table.RemoveAt(iter), CHIP_NO_ERROR);
    EXPECT_EQ(table.Size(), MATTER_BINDING_TABLE_SIZE - 1);
    EXPECT_EQ(iter, table.begin());
    EXPECT_EQ(iter.GetIndex(), 2u);
    EXPECT_EQ(iter->nodeId, 2u);
    EXPECT_EQ(table.GetAt(0).type, MATTER_UNUSED_BINDING);
}

TEST(TestBindingTable, TestPersistentStorage)
{
    chip::TestPersistentStorageDelegate testStorage;
    BindingTable table;
    chip::Optional<chip::ClusterId> cluster = chip::MakeOptional<chip::ClusterId>(static_cast<chip::ClusterId>(UINT16_MAX + 6));
    std::vector<EmberBindingTableEntry> expected = {
        EmberBindingTableEntry::ForNode(0, 0, 0, 0, std::nullopt),
        EmberBindingTableEntry::ForNode(1, 1, 0, 0, cluster.std_optional()),
        EmberBindingTableEntry::ForGroup(2, 2, 0, std::nullopt),
        EmberBindingTableEntry::ForGroup(3, 3, 0, cluster.std_optional()),
    };
    table.SetPersistentStorage(&testStorage);
    EXPECT_EQ(table.Add(expected[0]), CHIP_NO_ERROR);
    EXPECT_EQ(table.Add(expected[1]), CHIP_NO_ERROR);
    EXPECT_EQ(table.Add(expected[2]), CHIP_NO_ERROR);
    EXPECT_EQ(table.Add(expected[3]), CHIP_NO_ERROR);
    VerifyRestored(testStorage, expected);

    // Verify storage untouched if add fails
    testStorage.AddPoisonKey(chip::DefaultStorageKeyAllocator::BindingTableEntry(4).KeyName());
    EXPECT_NE(table.Add(EmberBindingTableEntry::ForNode(4, 4, 0, 0, std::nullopt)), CHIP_NO_ERROR);
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

} // namespace
