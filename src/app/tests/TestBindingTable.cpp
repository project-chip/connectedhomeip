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

#include <app/util/binding-table.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using chip::BindingTable;
using chip::NullOptional;

namespace {

void TestEmptyBindingTable(nlTestSuite * aSuite, void * aContext)
{
    BindingTable table;
    chip::TestPersistentStorageDelegate testStorage;
    table.SetPersistentStorage(&testStorage);
    NL_TEST_ASSERT(aSuite, table.Size() == 0);
    NL_TEST_ASSERT(aSuite, table.begin() == table.end());
}

void TestAdd(nlTestSuite * aSuite, void * aContext)
{
    BindingTable table;
    chip::TestPersistentStorageDelegate testStorage;
    table.SetPersistentStorage(&testStorage);
    EmberBindingTableEntry unusedEntry;
    unusedEntry.type = EMBER_UNUSED_BINDING;
    NL_TEST_ASSERT(aSuite, table.Add(unusedEntry) == CHIP_ERROR_INVALID_ARGUMENT);
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        NL_TEST_ASSERT(aSuite, table.Add(EmberBindingTableEntry::ForNode(0, i, 0, 0, NullOptional)) == CHIP_NO_ERROR);
    }
    NL_TEST_ASSERT(aSuite, table.Add(EmberBindingTableEntry::ForNode(0, 0, 0, 0, NullOptional)) == CHIP_ERROR_NO_MEMORY);
    NL_TEST_ASSERT(aSuite, table.Size() == EMBER_BINDING_TABLE_SIZE);

    auto iter = table.begin();
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        NL_TEST_ASSERT(aSuite, iter != table.end());
        NL_TEST_ASSERT(aSuite, iter->nodeId == i);
        NL_TEST_ASSERT(aSuite, iter.GetIndex() == i);
        ++iter;
    }
    NL_TEST_ASSERT(aSuite, iter == table.end());
}

void TestRemoveThenAdd(nlTestSuite * aSuite, void * aContext)
{
    BindingTable table;
    chip::TestPersistentStorageDelegate testStorage;
    table.SetPersistentStorage(&testStorage);
    NL_TEST_ASSERT(aSuite, table.Add(EmberBindingTableEntry::ForNode(0, 0, 0, 0, NullOptional)) == CHIP_NO_ERROR);
    auto iter = table.begin();
    NL_TEST_ASSERT(aSuite, table.RemoveAt(iter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, iter == table.end());
    NL_TEST_ASSERT(aSuite, table.Size() == 0);
    NL_TEST_ASSERT(aSuite, table.begin() == table.end());
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        NL_TEST_ASSERT(aSuite, table.Add(EmberBindingTableEntry::ForNode(0, i, 0, 0, NullOptional)) == CHIP_NO_ERROR);
    }
    iter = table.begin();
    ++iter;
    NL_TEST_ASSERT(aSuite, table.RemoveAt(iter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, table.Size() == EMBER_BINDING_TABLE_SIZE - 1);
    NL_TEST_ASSERT(aSuite, iter->nodeId == 2);
    NL_TEST_ASSERT(aSuite, iter.GetIndex() == 2);
    auto iterCheck = table.begin();
    ++iterCheck;
    NL_TEST_ASSERT(aSuite, iter == iterCheck);

    NL_TEST_ASSERT(aSuite, table.Add(EmberBindingTableEntry::ForNode(0, 1, 0, 0, NullOptional)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, table.Size() == EMBER_BINDING_TABLE_SIZE);
    iter = table.begin();
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE - 1; i++)
    {
        ++iter;
    }
    NL_TEST_ASSERT(aSuite, iter->nodeId == 1);
    NL_TEST_ASSERT(aSuite, iter.GetIndex() == 1);
    ++iter;
    NL_TEST_ASSERT(aSuite, iter == table.end());
    iter = table.begin();
    NL_TEST_ASSERT(aSuite, table.RemoveAt(iter) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, table.Size() == EMBER_BINDING_TABLE_SIZE - 1);
    NL_TEST_ASSERT(aSuite, iter == table.begin());
    NL_TEST_ASSERT(aSuite, iter.GetIndex() == 2);
    NL_TEST_ASSERT(aSuite, iter->nodeId == 2);
    NL_TEST_ASSERT(aSuite, table.GetAt(0).type == EMBER_UNUSED_BINDING);
}

void VerifyTableSame(nlTestSuite * aSuite, BindingTable & table, const std::vector<EmberBindingTableEntry> & expected)
{
    NL_TEST_ASSERT(aSuite, table.Size() == expected.size());
    auto iter1 = table.begin();
    auto iter2 = expected.begin();
    while (iter2 != expected.end())
    {
        NL_TEST_ASSERT(aSuite, iter1 != table.end());
        NL_TEST_ASSERT(aSuite, *iter1 == *iter2);
        ++iter1;
        ++iter2;
    }
    NL_TEST_ASSERT(aSuite, iter1 == table.end());
}

void VerifyRestored(nlTestSuite * aSuite, chip::TestPersistentStorageDelegate & storage,
                    const std::vector<EmberBindingTableEntry> & expected)
{
    BindingTable restoredTable;
    restoredTable.SetPersistentStorage(&storage);
    NL_TEST_ASSERT(aSuite, restoredTable.LoadFromStorage() == CHIP_NO_ERROR);
    VerifyTableSame(aSuite, restoredTable, expected);
}

void TestPersistentStorage(nlTestSuite * aSuite, void * aContext)
{
    chip::TestPersistentStorageDelegate testStorage;
    BindingTable table;
    chip::DefaultStorageKeyAllocator key;
    chip::Optional<chip::ClusterId> cluster = chip::MakeOptional<chip::ClusterId>(static_cast<chip::ClusterId>(UINT16_MAX + 6));
    std::vector<EmberBindingTableEntry> expected = {
        EmberBindingTableEntry::ForNode(0, 0, 0, 0, NullOptional),
        EmberBindingTableEntry::ForNode(1, 1, 0, 0, cluster),
        EmberBindingTableEntry::ForGroup(2, 2, 0, NullOptional),
        EmberBindingTableEntry::ForGroup(3, 3, 0, cluster),
    };
    table.SetPersistentStorage(&testStorage);
    NL_TEST_ASSERT(aSuite, table.Add(expected[0]) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, table.Add(expected[1]) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, table.Add(expected[2]) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, table.Add(expected[3]) == CHIP_NO_ERROR);
    VerifyRestored(aSuite, testStorage, expected);

    // Verify storage untouched if add fails
    testStorage.AddPoisonKey(key.BindingTableEntry(4));
    NL_TEST_ASSERT(aSuite, table.Add(EmberBindingTableEntry::ForNode(4, 4, 0, 0, NullOptional)) != CHIP_NO_ERROR);
    VerifyRestored(aSuite, testStorage, expected);
    testStorage.ClearPoisonKeys();

    // Verify storage untouched if removing head fails
    testStorage.AddPoisonKey(key.BindingTable());
    auto iter = table.begin();
    NL_TEST_ASSERT(aSuite, table.RemoveAt(iter) != CHIP_NO_ERROR);
    VerifyTableSame(aSuite, table, expected);
    testStorage.ClearPoisonKeys();
    VerifyRestored(aSuite, testStorage, expected);

    // Verify storage untouched if removing other nodes fails
    testStorage.AddPoisonKey(key.BindingTableEntry(0));
    iter = table.begin();
    ++iter;
    NL_TEST_ASSERT(aSuite, table.RemoveAt(iter) != CHIP_NO_ERROR);
    VerifyTableSame(aSuite, table, expected);
    testStorage.ClearPoisonKeys();
    VerifyRestored(aSuite, testStorage, expected);

    // Verify removing head
    iter = table.begin();
    NL_TEST_ASSERT(aSuite, table.RemoveAt(iter) == CHIP_NO_ERROR);
    VerifyTableSame(aSuite, table, { expected[1], expected[2], expected[3] });
    VerifyRestored(aSuite, testStorage, { expected[1], expected[2], expected[3] });

    // Verify removing other nodes
    ++iter;
    NL_TEST_ASSERT(aSuite, table.RemoveAt(iter) == CHIP_NO_ERROR);
    VerifyTableSame(aSuite, table, { expected[1], expected[3] });
    VerifyRestored(aSuite, testStorage, { expected[1], expected[3] });
}

} // namespace

int TestBindingTable()
{
    static nlTest sTests[] = {
        NL_TEST_DEF("TestEmptyBindingTable", TestEmptyBindingTable),
        NL_TEST_DEF("TestAdd", TestAdd),
        NL_TEST_DEF("TestRemoveThenAdd", TestRemoveThenAdd),
        NL_TEST_DEF("TestPersistentStorage", TestPersistentStorage),
        NL_TEST_SENTINEL(),
    };

    nlTestSuite theSuite = {
        "BindingTable",
        &sTests[0],
        nullptr,
        nullptr,
    };
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestBindingTable)
