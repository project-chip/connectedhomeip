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

#include <pw_unit_test/framework.h>

#include <app/icd/server/ICDMonitoringTable.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ClusterEnums.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>

#if CHIP_CRYPTO_PSA
#include <crypto/CHIPCryptoPALPSA.h>
#endif

using namespace chip;
using namespace chip::app::Clusters::IcdManagement;

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

constexpr uint64_t kClientNodeMaxValue = std::numeric_limits<uint64_t>::max();

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

struct TestICDMonitoringTable : public ::testing::Test
{
    void SetUp() override
    {
#if CHIP_CRYPTO_PSA
        ASSERT_EQ(psa_crypto_init(), PSA_SUCCESS);
#endif
    }

    void ValidateHmac128(const Crypto::Hmac128KeyHandle & saved, const Crypto::Hmac128KeyHandle & loaded)
    {
#if CHIP_CRYPTO_PSA
        EXPECT_NE(saved.As<psa_key_id_t>(), loaded.As<psa_key_id_t>());
        EXPECT_GE(loaded.As<psa_key_id_t>(), to_underlying(Crypto::KeyIdBase::ICDKeyRangeStart));
        EXPECT_LE(loaded.As<psa_key_id_t>(), to_underlying(Crypto::KeyIdBase::Maximum));
#else
        EXPECT_EQ(memcmp(saved.As<Crypto::Symmetric128BitsKeyByteArray>(), loaded.As<Crypto::Symmetric128BitsKeyByteArray>(),
                         sizeof(Crypto::Symmetric128BitsKeyByteArray)),
                  0);
#endif
    }
};

TEST_F(TestICDMonitoringTable, TestEntryAssignationOverload)
{
    TestSessionKeystoreImpl keystore;
    ICDMonitoringEntry entry(&keystore);

    // Test Setting Key
    EXPECT_EQ(entry.SetKey(ByteSpan(kKeyBuffer1a)), CHIP_NO_ERROR);

    entry.fabricIndex = 2;

    EXPECT_FALSE(entry.IsValid());

    entry.checkInNodeID    = 34;
    entry.monitoredSubject = 32;
    entry.clientType       = ClientTypeEnum::kEphemeral;

    // Entry should be valid now
    EXPECT_TRUE(entry.IsValid());

    ICDMonitoringEntry entry2;

    EXPECT_FALSE(entry2.IsValid());

    entry2 = entry;

    EXPECT_TRUE(entry2.IsValid());

    EXPECT_EQ(entry.fabricIndex, entry2.fabricIndex);
    EXPECT_EQ(entry.checkInNodeID, entry2.checkInNodeID);
    EXPECT_EQ(entry.monitoredSubject, entry2.monitoredSubject);
    EXPECT_EQ(entry.clientType, entry2.clientType);

    EXPECT_TRUE(entry2.IsKeyEquivalent(ByteSpan(kKeyBuffer1a)));
}

TEST_F(TestICDMonitoringTable, TestEntryMaximumSize)
{
    TestPersistentStorageDelegate storage;
    TestSessionKeystoreImpl keystore;
    ICDMonitoringTable table(storage, kTestFabricIndex1, kMaxTestClients1, &keystore);

    ICDMonitoringEntry entry(&keystore);
    entry.checkInNodeID    = kClientNodeMaxValue;
    entry.monitoredSubject = kClientNodeMaxValue;
    entry.clientType       = ClientTypeEnum::kPermanent;
    EXPECT_EQ(CHIP_NO_ERROR, entry.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_NO_ERROR, table.Set(0, entry));
}

TEST_F(TestICDMonitoringTable, TestEntryKeyFunctions)
{
    TestSessionKeystoreImpl keystore;
    ICDMonitoringEntry entry(&keystore);

    // Test Setting Key
    EXPECT_EQ(entry.SetKey(ByteSpan(kKeyBuffer1a)), CHIP_NO_ERROR);

    // Test Setting Key again
    EXPECT_EQ(entry.SetKey(ByteSpan(kKeyBuffer1b)), CHIP_ERROR_INTERNAL);

    // Test Key Deletion
    EXPECT_EQ(entry.DeleteKey(), CHIP_NO_ERROR);

    // Test Setting Key again
    EXPECT_EQ(entry.SetKey(ByteSpan(kKeyBuffer1b)), CHIP_NO_ERROR);

    // Test Comparing Key
    EXPECT_FALSE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1a)));

    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1b)));

    // Test Deleting Key
    EXPECT_EQ(entry.DeleteKey(), CHIP_NO_ERROR);
}

TEST_F(TestICDMonitoringTable, TestSaveAndLoadRegistrationValue)
{
    TestPersistentStorageDelegate storage;
    TestSessionKeystoreImpl keystore;
    ICDMonitoringTable saving(storage, kTestFabricIndex1, kMaxTestClients1, &keystore);
    ICDMonitoringTable loading(storage, kTestFabricIndex1, kMaxTestClients1, &keystore);
    ICDMonitoringEntry entry(&keystore);

    // Insert first entry
    ICDMonitoringEntry entry1(&keystore);
    entry1.checkInNodeID    = kClientNodeId11;
    entry1.monitoredSubject = kClientNodeId12;
    entry1.clientType       = ClientTypeEnum::kPermanent;
    EXPECT_EQ(CHIP_NO_ERROR, entry1.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_NO_ERROR, saving.Set(0, entry1));

    // Insert second entry
    ICDMonitoringEntry entry2(&keystore);
    entry2.checkInNodeID    = kClientNodeId12;
    entry2.monitoredSubject = kClientNodeId11;
    entry2.clientType       = ClientTypeEnum::kEphemeral;
    EXPECT_EQ(CHIP_NO_ERROR, entry2.SetKey(ByteSpan(kKeyBuffer2a)));
    EXPECT_EQ(CHIP_NO_ERROR, saving.Set(1, entry2));

    // Insert one too many
    ICDMonitoringEntry entry3(&keystore);
    entry3.checkInNodeID    = kClientNodeId13;
    entry3.monitoredSubject = kClientNodeId13;
    EXPECT_EQ(CHIP_NO_ERROR, entry3.SetKey(ByteSpan(kKeyBuffer3a)));
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, saving.Set(2, entry3));

    // Retrieve first entry
    EXPECT_EQ(CHIP_NO_ERROR, loading.Get(0, entry));
    EXPECT_EQ(kTestFabricIndex1, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId11, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId12, entry.monitoredSubject);
    EXPECT_EQ(ClientTypeEnum::kPermanent, entry.clientType);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1a)));
    ValidateHmac128(entry1.hmacKeyHandle, entry.hmacKeyHandle);

    // Retrieve second entry
    EXPECT_EQ(CHIP_NO_ERROR, loading.Get(1, entry));
    EXPECT_EQ(kTestFabricIndex1, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId12, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId11, entry.monitoredSubject);
    EXPECT_EQ(ClientTypeEnum::kEphemeral, entry.clientType);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer2a)));
    ValidateHmac128(entry2.hmacKeyHandle, entry.hmacKeyHandle);

    // No more entries
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, loading.Get(2, entry));
    EXPECT_EQ(2, loading.Limit());

    // Remove first entry
    saving.Remove(0);

    ICDMonitoringEntry entry4(&keystore);
    entry4.checkInNodeID    = kClientNodeId13;
    entry4.monitoredSubject = kClientNodeId11;
    EXPECT_EQ(CHIP_NO_ERROR, entry4.SetKey(ByteSpan(kKeyBuffer1b)));
    EXPECT_EQ(CHIP_NO_ERROR, saving.Set(1, entry4));

    // Retrieve first entry (not modified but shifted)
    EXPECT_EQ(CHIP_NO_ERROR, loading.Get(0, entry));
    EXPECT_EQ(kTestFabricIndex1, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId12, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId11, entry.monitoredSubject);
    EXPECT_EQ(ClientTypeEnum::kEphemeral, entry.clientType);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer2a)));
    ValidateHmac128(entry2.hmacKeyHandle, entry.hmacKeyHandle);

    // Retrieve second entry
    EXPECT_EQ(CHIP_NO_ERROR, loading.Get(1, entry));
    EXPECT_EQ(kTestFabricIndex1, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId13, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId11, entry.monitoredSubject);
    EXPECT_EQ(ClientTypeEnum::kPermanent, entry.clientType);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1b)));
    ValidateHmac128(entry4.hmacKeyHandle, entry.hmacKeyHandle);
}

TEST_F(TestICDMonitoringTable, TestSaveAllInvalidRegistrationValues)
{
    TestPersistentStorageDelegate storage;
    TestSessionKeystoreImpl keystore;
    ICDMonitoringTable table(storage, kTestFabricIndex1, kMaxTestClients1, &keystore);

    // Invalid checkInNodeID
    ICDMonitoringEntry entry1(&keystore);
    entry1.checkInNodeID    = kUndefinedNodeId;
    entry1.monitoredSubject = kClientNodeId12;
    EXPECT_EQ(CHIP_NO_ERROR, entry1.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, table.Set(0, entry1));

    // Invalid monitoredSubject
    ICDMonitoringEntry entry2(&keystore);
    entry2.checkInNodeID    = kClientNodeId11;
    entry2.monitoredSubject = kUndefinedNodeId;
    EXPECT_EQ(CHIP_NO_ERROR, entry2.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, table.Set(0, entry2));

    // Invalid key (empty)
    ICDMonitoringEntry entry3(&keystore);
    entry3.checkInNodeID    = kClientNodeId11;
    entry3.monitoredSubject = kClientNodeId12;
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, entry3.SetKey(ByteSpan()));
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, table.Set(0, entry3));

    // Invalid key (too short)
    ICDMonitoringEntry entry4(&keystore);
    entry4.checkInNodeID    = kClientNodeId11;
    entry4.monitoredSubject = kClientNodeId12;
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, entry4.SetKey(ByteSpan(kKeyBuffer0a)));
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, table.Set(0, entry4));

    // Invalid key (too long)
    ICDMonitoringEntry entry5(&keystore);
    entry5.checkInNodeID    = kClientNodeId11;
    entry5.monitoredSubject = kClientNodeId12;
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, entry5.SetKey(ByteSpan(kKeyBuffer0b)));
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, table.Set(0, entry5));
}

TEST_F(TestICDMonitoringTable, TestSaveLoadRegistrationValueForMultipleFabrics)
{
    TestPersistentStorageDelegate storage;
    TestSessionKeystoreImpl keystore;
    ICDMonitoringTable table1(storage, kTestFabricIndex1, kMaxTestClients1, &keystore);
    ICDMonitoringTable table2(storage, kTestFabricIndex2, kMaxTestClients2, &keystore);
    ICDMonitoringEntry entry(&keystore);

    // Insert in first fabric
    ICDMonitoringEntry entry1(&keystore);
    entry1.checkInNodeID    = kClientNodeId11;
    entry1.monitoredSubject = kClientNodeId12;
    EXPECT_EQ(CHIP_NO_ERROR, entry1.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_NO_ERROR, table1.Set(0, entry1));

    // Insert in first fabric
    ICDMonitoringEntry entry2(&keystore);
    entry2.checkInNodeID    = kClientNodeId12;
    entry2.monitoredSubject = kClientNodeId11;
    EXPECT_EQ(CHIP_NO_ERROR, entry2.SetKey(ByteSpan(kKeyBuffer1b)));
    EXPECT_EQ(CHIP_NO_ERROR, table1.Set(1, entry2));

    // Insert in second fabric
    ICDMonitoringEntry entry3(&keystore);
    entry3.checkInNodeID    = kClientNodeId21;
    entry3.monitoredSubject = kClientNodeId22;
    EXPECT_EQ(CHIP_NO_ERROR, entry3.SetKey(ByteSpan(kKeyBuffer2a)));
    EXPECT_EQ(CHIP_NO_ERROR, table2.Set(0, entry3));

    // Insert in second fabric (one too many)
    ICDMonitoringEntry entry4(&keystore);
    entry4.checkInNodeID    = kClientNodeId22;
    entry4.monitoredSubject = kClientNodeId21;
    EXPECT_EQ(CHIP_NO_ERROR, entry4.SetKey(ByteSpan(kKeyBuffer2b)));
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, table2.Set(1, entry4));

    // Retrieve fabric1, first entry

    EXPECT_EQ(CHIP_NO_ERROR, table1.Get(0, entry));
    EXPECT_EQ(kTestFabricIndex1, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId11, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId12, entry.monitoredSubject);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1a)));
    ValidateHmac128(entry1.hmacKeyHandle, entry.hmacKeyHandle);

    // Retrieve fabric1, second entry
    EXPECT_EQ(CHIP_NO_ERROR, table1.Get(1, entry));
    EXPECT_EQ(kTestFabricIndex1, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId12, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId11, entry.monitoredSubject);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1b)));
    ValidateHmac128(entry2.hmacKeyHandle, entry.hmacKeyHandle);

    // Retrieve fabric2, first entry
    EXPECT_EQ(CHIP_NO_ERROR, table2.Get(0, entry));
    EXPECT_EQ(kTestFabricIndex2, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId21, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId22, entry.monitoredSubject);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer2a)));
    ValidateHmac128(entry3.hmacKeyHandle, entry.hmacKeyHandle);
}

TEST_F(TestICDMonitoringTable, TestDeleteValidEntryFromStorage)
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
    EXPECT_EQ(CHIP_NO_ERROR, entry1.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_NO_ERROR, table1.Set(0, entry1));

    // Insert in first fabric
    ICDMonitoringEntry entry2(&keystore);
    entry2.checkInNodeID    = kClientNodeId12;
    entry2.monitoredSubject = kClientNodeId11;
    EXPECT_EQ(CHIP_NO_ERROR, entry2.SetKey(ByteSpan(kKeyBuffer2a)));
    err = table1.Set(1, entry2);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // Insert in second fabric
    ICDMonitoringEntry entry3(&keystore);
    entry3.checkInNodeID    = kClientNodeId21;
    entry3.monitoredSubject = kClientNodeId22;
    EXPECT_EQ(CHIP_NO_ERROR, entry3.SetKey(ByteSpan(kKeyBuffer1b)));
    EXPECT_EQ(CHIP_NO_ERROR, table2.Set(0, entry3));

    // Remove (invalid)
    EXPECT_NE(CHIP_NO_ERROR, table1.Remove(2));

    // Retrieve fabric1
    EXPECT_EQ(CHIP_NO_ERROR, table1.Get(0, entry));
    EXPECT_EQ(kTestFabricIndex1, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId11, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId12, entry.monitoredSubject);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1a)));
    ValidateHmac128(entry1.hmacKeyHandle, entry.hmacKeyHandle);

    // Retrieve second entry (not modified)
    EXPECT_EQ(CHIP_NO_ERROR, table1.Get(1, entry));
    EXPECT_EQ(kTestFabricIndex1, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId12, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId11, entry.monitoredSubject);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer2a)));
    ValidateHmac128(entry2.hmacKeyHandle, entry.hmacKeyHandle);

    // Remove (existing)
    EXPECT_EQ(CHIP_NO_ERROR, table1.Remove(0));

    // Retrieve second entry (shifted down)
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, table1.Get(1, entry));

    err = table1.Get(0, entry);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_EQ(kTestFabricIndex1, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId12, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId11, entry.monitoredSubject);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer2a)));
    ValidateHmac128(entry2.hmacKeyHandle, entry.hmacKeyHandle);

    // Retrieve fabric2, first entry
    EXPECT_EQ(CHIP_NO_ERROR, table2.Get(0, entry));
    EXPECT_EQ(kTestFabricIndex2, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId21, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId22, entry.monitoredSubject);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1b)));
    ValidateHmac128(entry3.hmacKeyHandle, entry.hmacKeyHandle);

    // Remove all (fabric 1)
    EXPECT_EQ(CHIP_NO_ERROR, table1.RemoveAll());
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, table1.Get(0, entry));

    // Check fabric 2
    EXPECT_EQ(CHIP_NO_ERROR, table2.Get(0, entry));
    EXPECT_EQ(kTestFabricIndex2, entry.fabricIndex);
    EXPECT_EQ(kClientNodeId21, entry.checkInNodeID);
    EXPECT_EQ(kClientNodeId22, entry.monitoredSubject);
    EXPECT_TRUE(entry.IsKeyEquivalent(ByteSpan(kKeyBuffer1b)));
    ValidateHmac128(entry3.hmacKeyHandle, entry.hmacKeyHandle);

    // Remove all (fabric 2)
    EXPECT_EQ(CHIP_NO_ERROR, table2.RemoveAll());
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, table2.Get(0, entry));
}

} // namespace
