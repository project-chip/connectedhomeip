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
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using chip::BindingTable;
using chip::ClusterId;
using chip::FabricIndex;
using chip::MakeOptional;
using chip::NodeId;
using chip::NullOptional;
using chip::PendingNotificationEntry;
using chip::PendingNotificationMap;

namespace {

void ClearBindingTable(BindingTable & table)
{
    auto iter = table.begin();
    while (iter != table.end())
    {
        iter = table.RemoveAt(iter);
    }
}

void CreateDefaultFullBindingTable(BindingTable & table)
{
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        table.Add(EmberBindingTableEntry::ForNode(i / 10, i % 5, 0, 0, MakeOptional<ClusterId>(i)));
    }
}

void TestEmptyMap(nlTestSuite * aSuite, void * aContext)
{
    PendingNotificationMap pendingMap;
    NL_TEST_ASSERT(aSuite, pendingMap.begin() == pendingMap.end());
    FabricIndex fabricIndex;
    NodeId node;
    NL_TEST_ASSERT(aSuite, pendingMap.FindLRUConnectPeer(&fabricIndex, &node) == CHIP_ERROR_NOT_FOUND);
}

void TestAddRemove(nlTestSuite * aSuite, void * aContext)
{
    PendingNotificationMap pendingMap;
    ClearBindingTable(BindingTable::GetInstance());
    CreateDefaultFullBindingTable(BindingTable::GetInstance());
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        pendingMap.AddPendingNotification(i, nullptr);
    }
    auto iter = pendingMap.begin();
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        PendingNotificationEntry entry = *iter;
        NL_TEST_ASSERT(aSuite, entry.mBindingEntryId == i);
        ++iter;
    }
    NL_TEST_ASSERT(aSuite, iter == pendingMap.end());
    pendingMap.RemoveAllEntriesForNode(0, 0);
    uint8_t expectedEntryIndecies[] = { 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
    iter                            = pendingMap.begin();
    for (uint8_t i = 0; i < sizeof(expectedEntryIndecies); i++)
    {
        PendingNotificationEntry entry = *iter;
        NL_TEST_ASSERT(aSuite, entry.mBindingEntryId == expectedEntryIndecies[i]);
        ++iter;
    }
    NL_TEST_ASSERT(aSuite, iter == pendingMap.end());
    pendingMap.RemoveAllEntriesForFabric(0);
    iter = pendingMap.begin();
    for (uint8_t i = 0; i < 10; i++)
    {
        PendingNotificationEntry entry = *iter;
        NL_TEST_ASSERT(aSuite, entry.mBindingEntryId == 10 + i);
        ++iter;
    }
    NL_TEST_ASSERT(aSuite, iter == pendingMap.end());
    pendingMap.RemoveAllEntriesForFabric(1);
    NL_TEST_ASSERT(aSuite, pendingMap.begin() == pendingMap.end());
}

void TestLRUEntry(nlTestSuite * aSuite, void * aContext)
{
    PendingNotificationMap pendingMap;
    ClearBindingTable(BindingTable::GetInstance());
    CreateDefaultFullBindingTable(BindingTable::GetInstance());
    pendingMap.AddPendingNotification(0, nullptr);
    pendingMap.AddPendingNotification(1, nullptr);
    pendingMap.AddPendingNotification(5, nullptr);
    pendingMap.AddPendingNotification(7, nullptr);
    pendingMap.AddPendingNotification(11, nullptr);

    FabricIndex fabricIndex;
    NodeId node;

    NL_TEST_ASSERT(aSuite, pendingMap.FindLRUConnectPeer(&fabricIndex, &node) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, fabricIndex == 0 && node == 1);

    pendingMap.RemoveEntry(1);
    NL_TEST_ASSERT(aSuite, pendingMap.FindLRUConnectPeer(&fabricIndex, &node) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, fabricIndex == 0 && node == 0);

    pendingMap.RemoveAllEntriesForFabric(0);
    NL_TEST_ASSERT(aSuite, pendingMap.FindLRUConnectPeer(&fabricIndex, &node) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(aSuite, fabricIndex == 1 && node == 1);
}

} // namespace

int TestPeindingNotificationMap()
{
    static nlTest sTests[] = {
        NL_TEST_DEF("TestEmptyMap", TestEmptyMap),
        NL_TEST_DEF("TestAddRemove", TestAddRemove),
        NL_TEST_DEF("TestLRUEntry", TestLRUEntry),
        NL_TEST_SENTINEL(),
    };

    nlTestSuite theSuite = {
        "PendingNotificationMap",
        &sTests[0],
        nullptr,
        nullptr,
    };
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestPeindingNotificationMap)
