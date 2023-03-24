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

#include <app/util/ClientMonitoringRegistrationTable.h>
#include <lib/core/CHIPError.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using chip::ClientMonitoringRegistrationTable;
using chip::NullOptional;

namespace {

constexpr chip::FabricIndex kTestFabricIndex = 1;
constexpr uint64_t kTestICid                 = 10;
constexpr chip::NodeId kTestClientNodeId     = 11;

constexpr chip::FabricIndex kTestFabricIndex_2 = 2;
constexpr uint64_t kTestICid_2                 = 20;
constexpr chip::NodeId kTestClientNodeId_2     = 21;

void TestDefaultClientValues(nlTestSuite * aSuite, void * aContext)
{
    chip::TestPersistentStorageDelegate testStorage;
    ClientMonitoringRegistrationTable registration(testStorage);

    NL_TEST_ASSERT(aSuite, !registration.GetClientRegistrationEntry().IsValid());
    NL_TEST_ASSERT(aSuite, registration.GetClientRegistrationEntry().clientNodeId == chip::kUndefinedFabricIndex);
    NL_TEST_ASSERT(aSuite, registration.GetClientRegistrationEntry().ICid == chip::kUndefinedNodeId);
    NL_TEST_ASSERT(aSuite, registration.GetClientRegistrationEntry().fabricIndex == chip::kInvalidIcId);
}

void TestLoadFromStorageEmptyValue(nlTestSuite * aSuite, void * aContext)
{
    chip::TestPersistentStorageDelegate testStorage;
    ClientMonitoringRegistrationTable registration(testStorage);

    CHIP_ERROR err = registration.LoadFromStorage(kTestFabricIndex);
    NL_TEST_ASSERT(aSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

void TestSaveAndLoadRegistrationValue(nlTestSuite * aSuite, void * aContext)
{
    chip::TestPersistentStorageDelegate testStorage;
    ClientMonitoringRegistrationTable savedRegistration(testStorage);
    ClientMonitoringRegistrationTable loadedRegistration(testStorage);

    savedRegistration.GetClientRegistrationEntry().clientNodeId = kTestClientNodeId;
    savedRegistration.GetClientRegistrationEntry().ICid         = kTestICid;
    savedRegistration.GetClientRegistrationEntry().fabricIndex  = kTestFabricIndex;

    CHIP_ERROR err = savedRegistration.SaveToStorage();
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    err = loadedRegistration.LoadFromStorage(kTestFabricIndex);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, loadedRegistration.GetClientRegistrationEntry().clientNodeId == kTestClientNodeId);
    NL_TEST_ASSERT(aSuite, loadedRegistration.GetClientRegistrationEntry().ICid == kTestICid);
    NL_TEST_ASSERT(aSuite, loadedRegistration.GetClientRegistrationEntry().fabricIndex == kTestFabricIndex);
}

void TestSaveLoadRegistrationValueForMultipleFabrics(nlTestSuite * aSuite, void * aContexT)
{
    chip::TestPersistentStorageDelegate testStorage;
    ClientMonitoringRegistrationTable savedRegistration(testStorage);
    ClientMonitoringRegistrationTable loadedRegistration(testStorage);

    savedRegistration.GetClientRegistrationEntry().clientNodeId = kTestClientNodeId;
    savedRegistration.GetClientRegistrationEntry().ICid         = kTestICid;
    savedRegistration.GetClientRegistrationEntry().fabricIndex  = kTestFabricIndex;

    CHIP_ERROR err = savedRegistration.SaveToStorage();
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    savedRegistration.GetClientRegistrationEntry().clientNodeId = kTestClientNodeId_2;
    savedRegistration.GetClientRegistrationEntry().ICid         = kTestICid_2;
    savedRegistration.GetClientRegistrationEntry().fabricIndex  = kTestFabricIndex_2;

    err = savedRegistration.SaveToStorage();
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    err = loadedRegistration.LoadFromStorage(kTestFabricIndex);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, loadedRegistration.GetClientRegistrationEntry().clientNodeId == kTestClientNodeId);
    NL_TEST_ASSERT(aSuite, loadedRegistration.GetClientRegistrationEntry().ICid == kTestICid);
    NL_TEST_ASSERT(aSuite, loadedRegistration.GetClientRegistrationEntry().fabricIndex == kTestFabricIndex);

    err = loadedRegistration.LoadFromStorage(kTestFabricIndex_2);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, loadedRegistration.GetClientRegistrationEntry().clientNodeId == kTestClientNodeId_2);
    NL_TEST_ASSERT(aSuite, loadedRegistration.GetClientRegistrationEntry().ICid == kTestICid_2);
    NL_TEST_ASSERT(aSuite, loadedRegistration.GetClientRegistrationEntry().fabricIndex == kTestFabricIndex_2);
}

void TestSaveAllInvalidRegistrationValues(nlTestSuite * aSuite, void * context)
{
    chip::TestPersistentStorageDelegate testStorage;
    ClientMonitoringRegistrationTable registration(testStorage);

    CHIP_ERROR err = registration.SaveToStorage();
    NL_TEST_ASSERT(aSuite, err == CHIP_ERROR_INCORRECT_STATE);
}

void TestDeleteValidEntryFromStorage(nlTestSuite * aSuite, void * context)
{
    chip::TestPersistentStorageDelegate testStorage;
    ClientMonitoringRegistrationTable table(testStorage);

    table.GetClientRegistrationEntry().clientNodeId = kTestClientNodeId;
    table.GetClientRegistrationEntry().ICid         = kTestICid;
    table.GetClientRegistrationEntry().fabricIndex  = kTestFabricIndex;

    CHIP_ERROR err = table.SaveToStorage();
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, table.HasValueForFabric(kTestFabricIndex));

    err = table.DeleteFromStorage(kTestFabricIndex);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(aSuite, !table.HasValueForFabric(kTestFabricIndex));
}

} // namespace

int TestClientMonitoringRegistrationTable()
{
    static nlTest sTests[] = { NL_TEST_DEF("TestDefaultClientValues", TestDefaultClientValues),
                               NL_TEST_DEF("TestLoadFromStorageEmptyValue", TestLoadFromStorageEmptyValue),
                               NL_TEST_DEF("TestSaveAndLoadRegistrationValue", TestSaveAndLoadRegistrationValue),
                               NL_TEST_DEF("TestSaveAllInvalidRegistrationValues", TestSaveAllInvalidRegistrationValues),
                               NL_TEST_DEF("TestSaveLoadRegistrationValueForMultipleFabrics",
                                           TestSaveLoadRegistrationValueForMultipleFabrics),
                               NL_TEST_DEF("TestDeleteValidEntryFromStorage", TestDeleteValidEntryFromStorage),
                               NL_TEST_SENTINEL() };

    nlTestSuite cmSuite = { "TestClientMonitoringRegistrationTable", &sTests[0], nullptr, nullptr };

    nlTestRunner(&cmSuite, nullptr);
    return (nlTestRunnerStats(&cmSuite));
}

CHIP_REGISTER_TEST_SUITE(TestClientMonitoringRegistrationTable)
