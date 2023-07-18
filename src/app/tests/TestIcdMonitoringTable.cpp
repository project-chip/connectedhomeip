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

#include <app/icd/IcdMonitoringTable.h>
#include <lib/core/CHIPError.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;

namespace {

constexpr uint16_t kMaxTestClients1     = 2;
constexpr uint16_t kMaxTestClients2     = 1;
constexpr FabricIndex kTestFabricIndex1 = 1;
constexpr FabricIndex kTestFabricIndex2 = kMaxValidFabricIndex;
constexpr uint64_t kClientNodeId11      = 0x100001;
constexpr uint64_t kClientNodeId12      = 0x100002;
constexpr uint64_t kClientNodeId13      = 0x100003;
constexpr uint64_t kClientNodeId21      = 0x200001;
constexpr uint64_t kClientNodeId22      = 0x200002;

constexpr uint8_t kKeyBuffer0a[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
constexpr uint8_t kKeyBuffer0b[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

constexpr uint8_t kKeyBuffer1a[] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};
constexpr uint8_t kKeyBuffer1b[] = {
    0xf1, 0xe1, 0xd1, 0xc1, 0xb1, 0xa1, 0x91, 0x81, 0x71, 0x61, 0x51, 0x14, 0x31, 0x21, 0x11, 0x01
};
constexpr uint8_t kKeyBuffer2a[] = {
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};
constexpr uint8_t kKeyBuffer2b[] = {
    0xf2, 0xe2, 0xd2, 0xc2, 0xb2, 0xa2, 0x92, 0x82, 0x72, 0x62, 0x52, 0x42, 0x32, 0x22, 0x12, 0x02
};
constexpr uint8_t kKeyBuffer3a[] = {
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};
// constexpr uint8_t kKeyBuffer3b[] = { 0xf3, 0xe3, 0xd3, 0xc3, 0xb3, 0xa3, 0x93, 0x83, 0x73, 0x63, 0x53, 0x14, 0x33, 0x23, 0x13,
// 0x03 };

void TestSaveAndLoadRegistrationValue(nlTestSuite * aSuite, void * aContext)
{
    TestPersistentStorageDelegate storage;
    IcdMonitoringTable saving(storage, kTestFabricIndex1, kMaxTestClients1);
    IcdMonitoringTable loading(storage, kTestFabricIndex1, kMaxTestClients1);
    IcdMonitoringEntry entry;
    CHIP_ERROR err;

    // Insert first entry
    entry.checkInNodeID    = kClientNodeId11;
    entry.monitoredSubject = kClientNodeId12;
    entry.key              = ByteSpan(kKeyBuffer1a);
    err                    = saving.Set(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert second entry
    entry.checkInNodeID    = kClientNodeId12;
    entry.monitoredSubject = kClientNodeId11;
    entry.key              = ByteSpan(kKeyBuffer2a);
    err                    = saving.Set(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert one too many
    entry.checkInNodeID    = kClientNodeId13;
    entry.monitoredSubject = kClientNodeId13;
    entry.key              = ByteSpan(kKeyBuffer3a);
    err                    = saving.Set(2, entry);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    // Retrieve first entry
    err = loading.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer1a) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer1a, entry.key.size()));

    // Retrieve second entry
    err = loading.Get(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer2a) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer2a, entry.key.size()));

    // No more entries
    err = loading.Get(2, entry);
    NL_TEST_ASSERT(aSuite, 2 == loading.Limit());
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == err);

    // Overwrite first entry
    entry.checkInNodeID    = kClientNodeId13;
    entry.monitoredSubject = kClientNodeId11;
    entry.key              = ByteSpan(kKeyBuffer1b);
    err                    = saving.Set(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Retrieve first entry (modified)
    err = loading.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId13 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer1b) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer1b, entry.key.size()));

    // Retrieve second entry (not modified)
    err = loading.Get(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer2a) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer2a, entry.key.size()));
}

void TestSaveAllInvalidRegistrationValues(nlTestSuite * aSuite, void * aContext)
{
    TestPersistentStorageDelegate storage;
    IcdMonitoringTable table(storage, kTestFabricIndex1, kMaxTestClients1);
    IcdMonitoringEntry entry;
    CHIP_ERROR err;

    // Invalid checkInNodeID
    entry.checkInNodeID    = kUndefinedNodeId;
    entry.monitoredSubject = kClientNodeId12;
    entry.key              = ByteSpan(kKeyBuffer1a);
    err                    = table.Set(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    // Invalid monitoredSubject
    entry.checkInNodeID    = kClientNodeId11;
    entry.monitoredSubject = kUndefinedNodeId;
    entry.key              = ByteSpan(kKeyBuffer1a);
    err                    = table.Set(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    // Invalid key (empty)
    entry.checkInNodeID    = kClientNodeId11;
    entry.monitoredSubject = kClientNodeId12;
    entry.key              = ByteSpan();
    err                    = table.Set(0, entry);

    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);
    // Invalid key (too short)
    entry.checkInNodeID    = kClientNodeId11;
    entry.monitoredSubject = kClientNodeId12;
    entry.key              = ByteSpan(kKeyBuffer0a);
    err                    = table.Set(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    // Invalid key (too long)
    entry.checkInNodeID    = kClientNodeId11;
    entry.monitoredSubject = kClientNodeId12;
    entry.key              = ByteSpan(kKeyBuffer0b);
    err                    = table.Set(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);
}

void TestSaveLoadRegistrationValueForMultipleFabrics(nlTestSuite * aSuite, void * aContext)
{
    TestPersistentStorageDelegate storage;
    IcdMonitoringTable table1(storage, kTestFabricIndex1, kMaxTestClients1);
    IcdMonitoringTable table2(storage, kTestFabricIndex2, kMaxTestClients2);
    IcdMonitoringEntry entry;
    CHIP_ERROR err;

    // Insert in first fabric
    entry.checkInNodeID    = kClientNodeId11;
    entry.monitoredSubject = kClientNodeId12;
    entry.key              = ByteSpan(kKeyBuffer1a);
    err                    = table1.Set(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert in first fabric
    entry.checkInNodeID    = kClientNodeId12;
    entry.monitoredSubject = kClientNodeId11;
    entry.key              = ByteSpan(kKeyBuffer1b);
    err                    = table1.Set(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert in second fabric
    entry.checkInNodeID    = kClientNodeId21;
    entry.monitoredSubject = kClientNodeId22;
    entry.key              = ByteSpan(kKeyBuffer2a);
    err                    = table2.Set(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert in second fabric (one too many)
    entry.checkInNodeID    = kClientNodeId22;
    entry.monitoredSubject = kClientNodeId21;
    entry.key              = ByteSpan(kKeyBuffer2b);
    err                    = table2.Set(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    // Retrieve fabric1, first entry
    err = table1.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer1a) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer1a, entry.key.size()));

    // Retrieve fabric2, second entry
    err = table1.Get(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer1b) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer1b, entry.key.size()));

    // Retrieve fabric2, first entry
    err = table2.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex2 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId21 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId22 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer2a) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer2a, entry.key.size()));
}

void TestDeleteValidEntryFromStorage(nlTestSuite * aSuite, void * context)
{
    TestPersistentStorageDelegate storage;
    IcdMonitoringTable table1(storage, kTestFabricIndex1, kMaxTestClients1);
    IcdMonitoringTable table2(storage, kTestFabricIndex2, kMaxTestClients2);
    IcdMonitoringEntry entry;
    CHIP_ERROR err;

    // Insert first entry
    entry.checkInNodeID    = kClientNodeId11;
    entry.monitoredSubject = kClientNodeId12;
    entry.key              = ByteSpan(kKeyBuffer1a);
    err                    = table1.Set(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert second entry
    entry.checkInNodeID    = kClientNodeId12;
    entry.monitoredSubject = kClientNodeId11;
    entry.key              = ByteSpan(kKeyBuffer2a);
    err                    = table1.Set(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert in second fabric
    entry.checkInNodeID    = kClientNodeId21;
    entry.monitoredSubject = kClientNodeId22;
    entry.key              = ByteSpan(kKeyBuffer2a);
    err                    = table2.Set(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Remove (invalid)
    err = table1.Remove(2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR != err);

    // Retrieve fabric1
    err = table1.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer1a) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer1a, entry.key.size()));

    // Retrieve second entry (not modified)
    err = table1.Get(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer2a) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer2a, entry.key.size()));

    // Remove (existing)
    err = table1.Remove(0);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Retrieve second entry (shifted down)
    err = table1.Get(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == err);

    err = table1.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer2a) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer2a, entry.key.size()));

    // Retrieve fabric2, first entry
    err = table2.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex2 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId21 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId22 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer2a) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer2a, entry.key.size()));

    // Remove all (fabric 1)
    err = table1.RemoveAll();
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    err = table1.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == err);

    // Check fabric 2
    err = table2.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex2 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId21 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId22 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, sizeof(kKeyBuffer2a) == entry.key.size());
    NL_TEST_ASSERT(aSuite, 0 == memcmp(entry.key.data(), kKeyBuffer2a, entry.key.size()));

    // Remove all (fabric 2)
    err = table2.RemoveAll();
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    err = table2.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == err);
}

} // namespace

/**
 *  Set up the test suite.
 */
int Test_Setup(void * inContext)
{

    return SUCCESS;
}

int TestClientMonitoringRegistrationTable()
{
    static nlTest sTests[] = {
        NL_TEST_DEF("TestSaveAndLoadRegistrationValue", TestSaveAndLoadRegistrationValue),
        NL_TEST_DEF("TestSaveAllInvalidRegistrationValues", TestSaveAllInvalidRegistrationValues),
        NL_TEST_DEF("TestSaveLoadRegistrationValueForMultipleFabrics", TestSaveLoadRegistrationValueForMultipleFabrics),
        NL_TEST_DEF("TestDeleteValidEntryFromStorage", TestDeleteValidEntryFromStorage), NL_TEST_SENTINEL()
    };

    nlTestSuite cmSuite = { "TestClientMonitoringRegistrationTable", &sTests[0], &Test_Setup, nullptr };

    nlTestRunner(&cmSuite, nullptr);
    return (nlTestRunnerStats(&cmSuite));
}

CHIP_REGISTER_TEST_SUITE(TestClientMonitoringRegistrationTable)
