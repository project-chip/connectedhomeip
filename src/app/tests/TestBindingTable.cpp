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
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using chip::BindingTable;
using chip::NullOptional;

namespace {

void TestEmptyBindingTable(nlTestSuite * aSuite, void * aContext)
{
    BindingTable table;
    NL_TEST_ASSERT(aSuite, table.Size() == 0);
    NL_TEST_ASSERT(aSuite, table.begin() == table.end());
}

void TestAdd(nlTestSuite * aSuite, void * aContext)
{
    BindingTable table;

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
    NL_TEST_ASSERT(aSuite, table.Add(EmberBindingTableEntry::ForNode(0, 0, 0, 0, NullOptional)) == CHIP_NO_ERROR);
    auto iter = table.begin();
    NL_TEST_ASSERT(aSuite, table.RemoveAt(iter) == table.end());
    NL_TEST_ASSERT(aSuite, table.Size() == 0);
    NL_TEST_ASSERT(aSuite, table.begin() == table.end());
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        NL_TEST_ASSERT(aSuite, table.Add(EmberBindingTableEntry::ForNode(0, i, 0, 0, NullOptional)) == CHIP_NO_ERROR);
    }
    iter = table.begin();
    ++iter;
    iter = table.RemoveAt(iter);
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
    iter = table.RemoveAt(iter);
    NL_TEST_ASSERT(aSuite, table.Size() == EMBER_BINDING_TABLE_SIZE - 1);
    NL_TEST_ASSERT(aSuite, iter == table.begin());
    NL_TEST_ASSERT(aSuite, iter.GetIndex() == 2);
    NL_TEST_ASSERT(aSuite, iter->nodeId == 2);
    NL_TEST_ASSERT(aSuite, table.GetAt(0).type == EMBER_UNUSED_BINDING);
}

} // namespace

int TestBindingTable()
{
    static nlTest sTests[] = {
        NL_TEST_DEF("TestEmptyBindingTable", TestEmptyBindingTable),
        NL_TEST_DEF("TestAdd", TestAdd),
        NL_TEST_DEF("TestRemoveThenAdd", TestRemoveThenAdd),
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
