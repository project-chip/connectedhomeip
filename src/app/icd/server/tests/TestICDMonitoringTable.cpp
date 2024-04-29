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

#include <app/icd/server/ICDMonitoringTable.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

#include <crypto/DefaultSessionKeystore.h>

using namespace chip;

using TestSessionKeystoreImpl = Crypto::DefaultSessionKeystore;

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

void TestEntryAssignationOverload(nlTestSuite * aSuite, void * aContext)
{
    TestSessionKeystoreImpl keystore;
    ICDMonitoringEntry entry(&keystore);

    // Test Setting Key
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry.SetKey(ByteSpan(kKeyBuffer1a)));

    entry.fabricIndex = 2;

    NL_TEST_ASSERT(aSuite, !entry.IsValid());

    entry.checkInNodeID    = 34;
    entry.monitoredSubject = 32;

    // Entry should be valid now
    NL_TEST_ASSERT(aSuite, entry.IsValid());

    ICDMonitoringEntry entry2;

    NL_TEST_ASSERT(aSuite, !entry2.IsValid());

    entry2 = entry;

    NL_TEST_ASSERT(aSuite, entry2.IsValid());

    NL_TEST_ASSERT(aSuite, entry2.fabricIndex == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, entry2.checkInNodeID == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, entry2.monitoredSubject == entry.monitoredSubject);

    NL_TEST_ASSERT(aSuite, entry2.IsKeyEquivalent(ByteSpan(kKeyBuffer1a)));
}

void TestEntryKeyFunctions(nlTestSuite * aSuite, void * aContext)
{
    TestSessionKeystoreImpl keystore;
    ICDMonitoringEntry entry(&keystore);

    // Test Setting Key
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry.SetKey(ByteSpan(kKeyBuffer1a)));

    // Test Setting Key again
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INTERNAL == entry.SetKey(ByteSpan(kKeyBuffer1b)));

    // Test Key Deletion
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry.DeleteKey());

    // Test Setting Key again
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry.SetKey(ByteSpan(kKeyBuffer1b)));

    // Test Comparing Key
    NL_TEST_ASSERT(aSuite, !entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1a)));

    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1b)));

    // Test Deleting Key
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry.DeleteKey());
}

void TestSaveAndLoadRegistrationValue(nlTestSuite * aSuite, void * aContext)
{
    TestPersistentStorageDelegate storage;
    TestSessionKeystoreImpl keystore;
    ICDMonitoringTable saving(storage, kTestFabricIndex1, kMaxTestClients1, &keystore);
    ICDMonitoringTable loading(storage, kTestFabricIndex1, kMaxTestClients1, &keystore);
    ICDMonitoringEntry entry(&keystore);
    CHIP_ERROR err;

    // Insert first entry
    ICDMonitoringEntry entry1(&keystore);
    entry1.checkInNodeID    = kClientNodeId11;
    entry1.monitoredSubject = kClientNodeId12;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry1.SetKey(ByteSpan(kKeyBuffer1a)));
    err = saving.Set(0, entry1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert second entry
    ICDMonitoringEntry entry2(&keystore);
    entry2.checkInNodeID    = kClientNodeId12;
    entry2.monitoredSubject = kClientNodeId11;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry2.SetKey(ByteSpan(kKeyBuffer2a)));
    err = saving.Set(1, entry2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert one too many
    ICDMonitoringEntry entry3(&keystore);
    entry3.checkInNodeID    = kClientNodeId13;
    entry3.monitoredSubject = kClientNodeId13;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry3.SetKey(ByteSpan(kKeyBuffer3a)));
    err = saving.Set(2, entry3);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    // Retrieve first entry
    err = loading.Get(0, entry);

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1a)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry1.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);

    // Retrieve second entry
    err = loading.Get(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer2a)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry2.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);

    // No more entries
    err = loading.Get(2, entry);
    NL_TEST_ASSERT(aSuite, 2 == loading.Limit());
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == err);

    // Remove first entry
    saving.Remove(0);

    ICDMonitoringEntry entry4(&keystore);
    entry4.checkInNodeID    = kClientNodeId13;
    entry4.monitoredSubject = kClientNodeId11;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry4.SetKey(ByteSpan(kKeyBuffer1b)));
    err = saving.Set(1, entry4);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Retrieve first entry (not modified but shifted)
    err = loading.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer2a)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry2.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);

    // Retrieve second entry
    err = loading.Get(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId13 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1b)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry4.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);
}

void TestSaveAllInvalidRegistrationValues(nlTestSuite * aSuite, void * aContext)
{
    TestPersistentStorageDelegate storage;
    TestSessionKeystoreImpl keystore;
    ICDMonitoringTable table(storage, kTestFabricIndex1, kMaxTestClients1, &keystore);
    CHIP_ERROR err;

    // Invalid checkInNodeID
    ICDMonitoringEntry entry1(&keystore);
    entry1.checkInNodeID    = kUndefinedNodeId;
    entry1.monitoredSubject = kClientNodeId12;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry1.SetKey(ByteSpan(kKeyBuffer1a)));
    err = table.Set(0, entry1);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    // Invalid monitoredSubject
    ICDMonitoringEntry entry2(&keystore);
    entry2.checkInNodeID    = kClientNodeId11;
    entry2.monitoredSubject = kUndefinedNodeId;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry2.SetKey(ByteSpan(kKeyBuffer1a)));
    err = table.Set(0, entry2);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    // Invalid key (empty)
    ICDMonitoringEntry entry3(&keystore);
    entry3.checkInNodeID    = kClientNodeId11;
    entry3.monitoredSubject = kClientNodeId12;
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == entry3.SetKey(ByteSpan()));
    err = table.Set(0, entry3);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    // Invalid key (too short)
    ICDMonitoringEntry entry4(&keystore);
    entry4.checkInNodeID    = kClientNodeId11;
    entry4.monitoredSubject = kClientNodeId12;
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == entry4.SetKey(ByteSpan(kKeyBuffer0a)));
    err = table.Set(0, entry4);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    // Invalid key (too long)
    ICDMonitoringEntry entry5(&keystore);
    entry5.checkInNodeID    = kClientNodeId11;
    entry5.monitoredSubject = kClientNodeId12;
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == entry5.SetKey(ByteSpan(kKeyBuffer0b)));
    err = table.Set(0, entry5);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);
}

void TestSaveLoadRegistrationValueForMultipleFabrics(nlTestSuite * aSuite, void * aContext)
{
    TestPersistentStorageDelegate storage;
    TestSessionKeystoreImpl keystore;
    ICDMonitoringTable table1(storage, kTestFabricIndex1, kMaxTestClients1, &keystore);
    ICDMonitoringTable table2(storage, kTestFabricIndex2, kMaxTestClients2, &keystore);
    ICDMonitoringEntry entry(&keystore);
    CHIP_ERROR err;

    // Insert in first fabric
    ICDMonitoringEntry entry1(&keystore);
    entry1.checkInNodeID    = kClientNodeId11;
    entry1.monitoredSubject = kClientNodeId12;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry1.SetKey(ByteSpan(kKeyBuffer1a)));
    err = table1.Set(0, entry1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert in first fabric
    ICDMonitoringEntry entry2(&keystore);
    entry2.checkInNodeID    = kClientNodeId12;
    entry2.monitoredSubject = kClientNodeId11;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry2.SetKey(ByteSpan(kKeyBuffer1b)));
    err = table1.Set(1, entry2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert in second fabric
    ICDMonitoringEntry entry3(&keystore);
    entry3.checkInNodeID    = kClientNodeId21;
    entry3.monitoredSubject = kClientNodeId22;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry3.SetKey(ByteSpan(kKeyBuffer2a)));
    err = table2.Set(0, entry3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert in second fabric (one too many)
    ICDMonitoringEntry entry4(&keystore);
    entry4.checkInNodeID    = kClientNodeId22;
    entry4.monitoredSubject = kClientNodeId21;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry4.SetKey(ByteSpan(kKeyBuffer2b)));
    err = table2.Set(1, entry4);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    // Retrieve fabric1, first entry
    err = table1.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1a)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry1.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);

    // Retrieve fabric1, second entry
    err = table1.Get(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1b)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry2.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);

    // Retrieve fabric2, first entry
    err = table2.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex2 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId21 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId22 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer2a)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry3.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);
}

void TestDeleteValidEntryFromStorage(nlTestSuite * aSuite, void * context)
{
    TestPersistentStorageDelegate storage;
    TestSessionKeystoreImpl keystore;
    ICDMonitoringTable table1(storage, kTestFabricIndex1, kMaxTestClients1, &keystore);
    ICDMonitoringTable table2(storage, kTestFabricIndex2, kMaxTestClients2, &keystore);
    ICDMonitoringEntry entry(&keystore);
    CHIP_ERROR err;

    // Insert in first fabric
    ICDMonitoringEntry entry1(&keystore);
    entry1.checkInNodeID    = kClientNodeId11;
    entry1.monitoredSubject = kClientNodeId12;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry1.SetKey(ByteSpan(kKeyBuffer1a)));
    err = table1.Set(0, entry1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert in first fabric
    ICDMonitoringEntry entry2(&keystore);
    entry2.checkInNodeID    = kClientNodeId12;
    entry2.monitoredSubject = kClientNodeId11;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry2.SetKey(ByteSpan(kKeyBuffer2a)));
    err = table1.Set(1, entry2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);

    // Insert in second fabric
    ICDMonitoringEntry entry3(&keystore);
    entry3.checkInNodeID    = kClientNodeId21;
    entry3.monitoredSubject = kClientNodeId22;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry3.SetKey(ByteSpan(kKeyBuffer1b)));
    err = table2.Set(0, entry3);
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
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1a)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry1.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);

    // Retrieve second entry (not modified)
    err = table1.Get(1, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex1 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId12 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId11 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer2a)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry2.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);

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
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer2a)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry2.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);

    // Retrieve fabric2, first entry
    err = table2.Get(0, entry);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(aSuite, kTestFabricIndex2 == entry.fabricIndex);
    NL_TEST_ASSERT(aSuite, kClientNodeId21 == entry.checkInNodeID);
    NL_TEST_ASSERT(aSuite, kClientNodeId22 == entry.monitoredSubject);
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1b)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry3.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);

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
    NL_TEST_ASSERT(aSuite, entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1b)));
    NL_TEST_ASSERT(aSuite,
                   memcmp(entry3.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                          sizeof(Crypto::Symmetric128BitsKeyByteArray)) == 0);

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
    static nlTest sTests[] = { NL_TEST_DEF("TestEntryKeyFunctions", TestEntryKeyFunctions),
                               NL_TEST_DEF("TestEntryAssignationOverload", TestEntryAssignationOverload),
                               NL_TEST_DEF("TestSaveAndLoadRegistrationValue", TestSaveAndLoadRegistrationValue),
                               NL_TEST_DEF("TestSaveAllInvalidRegistrationValues", TestSaveAllInvalidRegistrationValues),
                               NL_TEST_DEF("TestSaveLoadRegistrationValueForMultipleFabrics",
                                           TestSaveLoadRegistrationValueForMultipleFabrics),
                               NL_TEST_DEF("TestDeleteValidEntryFromStorage", TestDeleteValidEntryFromStorage),
                               NL_TEST_SENTINEL() };

    nlTestSuite cmSuite = { "TestClientMonitoringRegistrationTable", &sTests[0], &Test_Setup, nullptr };

    nlTestRunner(&cmSuite, nullptr);
    return (nlTestRunnerStats(&cmSuite));
}

CHIP_REGISTER_TEST_SUITE(TestClientMonitoringRegistrationTable)
