/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *      This file implements unit tests for FabricTable implementation.
 */

#include <errno.h>
#include <stdarg.h>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>

#include <credentials/FabricTable.h>

#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/TestOnlyLocalCertificateAuthority.h>
#include <credentials/tests/CHIPCert_test_vectors.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/asn1/ASN1.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>

#include <platform/ConfigurationManager.h>

#include <lib/support/BytesToHex.h>

using namespace chip;
using namespace chip::Credentials;

namespace {

class ScopedFabricTable
{
public:
    ScopedFabricTable() {}
    ~ScopedFabricTable()
    {
        mFabricTable.Shutdown();
        mOpCertStore.Finish();
        mOpKeyStore.Finish();
    }

    CHIP_ERROR Init(chip::TestPersistentStorageDelegate * storage)
    {
        chip::FabricTable::InitParams initParams;
        initParams.storage             = storage;
        initParams.operationalKeystore = &mOpKeyStore;
        initParams.opCertStore         = &mOpCertStore;

        ReturnErrorOnFailure(mOpKeyStore.Init(storage));
        ReturnErrorOnFailure(mOpCertStore.Init(storage));
        return mFabricTable.Init(initParams);
    }

    FabricTable & GetFabricTable() { return mFabricTable; }

private:
    chip::FabricTable mFabricTable;
    chip::PersistentStorageOperationalKeystore mOpKeyStore;
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
};

/**
 * Load a single test fabric with with the Root01:ICA01:Node01_01 identity.
 */
static CHIP_ERROR LoadTestFabric_Node01_01(FabricTable & fabricTable, bool doCommit)
{
    Crypto::P256SerializedKeypair opKeysSerialized;
    static Crypto::P256Keypair opKey_Node01_01;

    FabricIndex fabricIndex;
    memcpy(opKeysSerialized.Bytes(), TestCerts::sTestCert_Node01_01_PublicKey.data(),
           TestCerts::sTestCert_Node01_01_PublicKey.size());
    memcpy(opKeysSerialized.Bytes() + TestCerts::sTestCert_Node01_01_PublicKey.size(),
           TestCerts::sTestCert_Node01_01_PrivateKey.data(), TestCerts::sTestCert_Node01_01_PrivateKey.size());

    ByteSpan rcacSpan(TestCerts::sTestCert_Root01_Chip);
    ByteSpan icacSpan(TestCerts::sTestCert_ICA01_Chip);
    ByteSpan nocSpan(TestCerts::sTestCert_Node01_01_Chip);

    ReturnErrorOnFailure(opKeysSerialized.SetLength(TestCerts::sTestCert_Node01_01_PublicKey.size() +
                                                    TestCerts::sTestCert_Node01_01_PrivateKey.size()));
    ReturnErrorOnFailure(opKey_Node01_01.Deserialize(opKeysSerialized));
    ReturnErrorOnFailure(fabricTable.AddNewPendingTrustedRootCert(rcacSpan));

    ReturnErrorOnFailure(fabricTable.AddNewPendingFabricWithProvidedOpKey(nocSpan, icacSpan, VendorId::TestVendor1,
                                                                          &opKey_Node01_01,
                                                                          /*isExistingOpKeyExternallyOwned =*/true, &fabricIndex));
    if (doCommit)
    {
        ReturnErrorOnFailure(fabricTable.CommitPendingFabricData());
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR LoadTestFabric_Node01_02(FabricTable & fabricTable, bool doCommit)
{
    Crypto::P256SerializedKeypair opKeysSerialized;
    FabricIndex fabricIndex;
    static Crypto::P256Keypair opKey_Node01_02;

    memcpy(opKeysSerialized.Bytes(), TestCerts::sTestCert_Node01_02_PublicKey.data(),
           TestCerts::sTestCert_Node01_02_PublicKey.size());
    memcpy(opKeysSerialized.Bytes() + TestCerts::sTestCert_Node01_02_PublicKey.size(),
           TestCerts::sTestCert_Node01_02_PrivateKey.data(), TestCerts::sTestCert_Node01_02_PrivateKey.size());

    ByteSpan rcacSpan(TestCerts::sTestCert_Root01_Chip);
    ByteSpan nocSpan(TestCerts::sTestCert_Node01_02_Chip);

    ReturnErrorOnFailure(opKeysSerialized.SetLength(TestCerts::sTestCert_Node01_02_PublicKey.size() +
                                                    TestCerts::sTestCert_Node01_02_PrivateKey.size()));
    ReturnErrorOnFailure(opKey_Node01_02.Deserialize(opKeysSerialized));

    ReturnErrorOnFailure(fabricTable.AddNewPendingTrustedRootCert(rcacSpan));

    ReturnErrorOnFailure(fabricTable.AddNewPendingFabricWithProvidedOpKey(nocSpan, {}, VendorId::TestVendor1, &opKey_Node01_02,
                                                                          /*isExistingOpKeyExternallyOwned =*/true, &fabricIndex));
    if (doCommit)
    {
        ReturnErrorOnFailure(fabricTable.CommitPendingFabricData());
    }

    return CHIP_NO_ERROR;
}

/**
 * Load a single test fabric with with the Root02:ICA02:Node02_01 identity.
 */
static CHIP_ERROR LoadTestFabric_Node02_01(FabricTable & fabricTable, bool doCommit,
                                           FabricTable::AdvertiseIdentity advertiseIdentity = FabricTable::AdvertiseIdentity::Yes)
{
    Crypto::P256SerializedKeypair opKeysSerialized;
    FabricIndex fabricIndex;
    static Crypto::P256Keypair opKey_Node02_01;

    memcpy(opKeysSerialized.Bytes(), TestCerts::sTestCert_Node02_01_PublicKey.data(),
           TestCerts::sTestCert_Node02_01_PublicKey.size());
    memcpy(opKeysSerialized.Bytes() + TestCerts::sTestCert_Node02_01_PublicKey.size(),
           TestCerts::sTestCert_Node02_01_PrivateKey.data(), TestCerts::sTestCert_Node02_01_PrivateKey.size());

    ByteSpan rcacSpan(TestCerts::sTestCert_Root02_Chip);
    ByteSpan icacSpan(TestCerts::sTestCert_ICA02_Chip);
    ByteSpan nocSpan(TestCerts::sTestCert_Node02_01_Chip);

    EXPECT_EQ(opKeysSerialized.SetLength(TestCerts::sTestCert_Node02_01_PublicKey.size() +
                                         TestCerts::sTestCert_Node02_01_PrivateKey.size()),
              CHIP_NO_ERROR);
    EXPECT_EQ(opKey_Node02_01.Deserialize(opKeysSerialized), CHIP_NO_ERROR);

    EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcacSpan), CHIP_NO_ERROR);

    CHIP_ERROR err =
        fabricTable.AddNewPendingFabricWithProvidedOpKey(nocSpan, icacSpan, VendorId::TestVendor1, &opKey_Node02_01,
                                                         /*isExistingOpKeyExternallyOwned =*/true, &fabricIndex, advertiseIdentity);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    if (doCommit)
    {
        err = fabricTable.CommitPendingFabricData();
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    return err;
}

const FabricInfo * FindFabric(FabricTable & fabricTable, ByteSpan rootPublicKey, FabricId fabricId)
{
    Crypto::P256PublicKey key;
    EXPECT_GE(key.Length(), rootPublicKey.size());
    if (key.Length() < rootPublicKey.size())
    {
        return nullptr;
    }
    memcpy(key.Bytes(), rootPublicKey.data(), rootPublicKey.size());
    return fabricTable.FindFabric(key, fabricId);
}

struct TestFabricTable : public ::testing::Test
{

    static void SetUpTestSuite()
    {
        DeviceLayer::SetConfigurationMgr(&DeviceLayer::ConfigurationManagerImpl::GetDefaultInstance());
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
#if CHIP_CRYPTO_PSA
        ASSERT_EQ(psa_crypto_init(), PSA_SUCCESS);
#endif
    }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestFabricTable, TestLastKnownGoodTimeInit)
{
    // Fabric table init should init Last Known Good Time to the firmware build time.
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;

    EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
    System::Clock::Seconds32 lastKnownGoodChipEpochTime;

    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();
    EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime), CHIP_NO_ERROR);
    System::Clock::Seconds32 firmwareBuildTime;
    EXPECT_EQ(DeviceLayer::ConfigurationMgr().GetFirmwareBuildChipEpochTime(firmwareBuildTime), CHIP_NO_ERROR);
    EXPECT_EQ(lastKnownGoodChipEpochTime, firmwareBuildTime);
}

TEST_F(TestFabricTable, TestCollidingFabrics)
{
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    //
    // Start by loading NOCs for two nodes on the same fabric. The second one should fail since the FabricTable by default
    // doesn't permit colliding fabrics.
    //
    EXPECT_EQ(LoadTestFabric_Node01_01(fabricTable, /* doCommit = */ true), CHIP_NO_ERROR);
    EXPECT_NE(LoadTestFabric_Node01_02(fabricTable, /* doCommit = */ true), CHIP_NO_ERROR);

    //
    // Revert the partially added NOC from the last call, permit colliding fabrics in the FabricTable and try again.
    // This time, it should succeed
    //
    fabricTable.RevertPendingFabricData();
    fabricTable.PermitCollidingFabrics();
    EXPECT_EQ(LoadTestFabric_Node01_02(fabricTable, /* doCommit = */ true), CHIP_NO_ERROR);

    ByteSpan rcacSpan(TestCerts::sTestCert_Root01_Chip);
    Credentials::P256PublicKeySpan rootPublicKeySpan;

    EXPECT_EQ(Credentials::ExtractPublicKeyFromChipCert(rcacSpan, rootPublicKeySpan), CHIP_NO_ERROR);

    //
    // Ensure we can find both node identities in the FabricTable.
    //
    {
        chip::Platform::ScopedMemoryBuffer<uint8_t> nocBuf;
        ByteSpan origNocSpan(TestCerts::sTestCert_Node01_01_Chip);
        NodeId nodeId;
        FabricId fabricId;

        EXPECT_EQ(ExtractNodeIdFabricIdFromOpCert(origNocSpan, &nodeId, &fabricId), CHIP_NO_ERROR);
        EXPECT_NE(fabricTable.FindIdentity(rootPublicKeySpan, fabricId, nodeId), nullptr);
    }

    {
        chip::Platform::ScopedMemoryBuffer<uint8_t> nocBuf;
        ByteSpan origNocSpan(TestCerts::sTestCert_Node01_02_Chip);
        NodeId nodeId;
        FabricId fabricId;

        EXPECT_EQ(ExtractNodeIdFabricIdFromOpCert(origNocSpan, &nodeId, &fabricId), CHIP_NO_ERROR);
        EXPECT_NE(fabricTable.FindIdentity(rootPublicKeySpan, fabricId, nodeId), nullptr);
    }
}

TEST_F(TestFabricTable, TestUpdateLastKnownGoodTime)
{
    // Adding a fabric should advance Last Known Good Time if any certificate's
    // NotBefore time is later than the build time, and else should leave it
    // to the initial build time value.

    // Test certs all have this NotBefore: Oct 15 14:23:43 2020 GMT
    const ASN1::ASN1UniversalTime asn1Expected = { 2020, 10, 15, 14, 23, 43 };
    uint32_t testCertNotBeforeSeconds;
    EXPECT_EQ(Credentials::ASN1ToChipEpochTime(asn1Expected, testCertNotBeforeSeconds), CHIP_NO_ERROR);
    System::Clock::Seconds32 testCertNotBeforeTime = System::Clock::Seconds32(testCertNotBeforeSeconds);

    // Test that certificate NotBefore times that are before the Firmware build time
    // do not advance Last Known Good Time.
    System::Clock::Seconds32 afterNotBeforeBuildTimes[] = { System::Clock::Seconds32(testCertNotBeforeTime.count() + 1),
                                                            System::Clock::Seconds32(testCertNotBeforeTime.count() + 1000),
                                                            System::Clock::Seconds32(testCertNotBeforeTime.count() + 1000000) };
    for (auto buildTime : afterNotBeforeBuildTimes)
    {
        // Set build time to the desired value.
        EXPECT_EQ(DeviceLayer::ConfigurationMgr().SetFirmwareBuildChipEpochTime(buildTime), CHIP_NO_ERROR);
        chip::TestPersistentStorageDelegate testStorage;

        {
            // Initialize a fabric table.
            ScopedFabricTable fabricTableHolder;
            EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Read back Last Known Good Time, which will have been initialized to firmware build time.
            System::Clock::Seconds32 lastKnownGoodTime;
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, buildTime);

            // Load a test fabric, but do not commit.
            EXPECT_EQ(LoadTestFabric_Node01_01(fabricTable, /* doCommit = */ false), CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it hasn't moved forward.
            // This test case was written after the test certs' NotBefore time and we
            // are using a configuration manager that should reflect a real build time.
            // Therefore, we expect that build time is after NotBefore and so Last
            // Known Good Time will be set to the later of these, build time, even
            // after installing the new fabric.
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, buildTime);

            // Verify that calling the fail-safe roll back interface does not change
            // last known good time, as it hadn't been updated in the first place.
            fabricTable.RevertPendingFabricData();
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, buildTime);

            // Now reload the test fabric and commit this time.
            EXPECT_EQ(LoadTestFabric_Node01_01(fabricTable, /* doCommit = */ true), CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it hasn't moved forward.
            // This test case was written after the test certs' NotBefore time and we
            // are using a configuration manager that should reflect a real build time.
            // Therefore, we expect that build time is after NotBefore and so Last
            // Known Good Time will be set to the later of these, build time, even
            // after installing the new fabric.
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, buildTime);

            // Call revert again.  Since we've committed, this is a no-op.
            // Last known good time should again be unchanged.
            fabricTable.RevertPendingFabricData();
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, buildTime);
        }
        {
            // Test reloading last known good time from persistence.
            ScopedFabricTable fabricTableHolder;
            EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Verify that last known good time was retained.
            System::Clock::Seconds32 lastKnownGoodTime;
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, buildTime);
        }
    }

    System::Clock::Seconds32 beforeNotBeforeBuildTimes[] = { testCertNotBeforeTime,
                                                             System::Clock::Seconds32(testCertNotBeforeTime.count() - 1),
                                                             System::Clock::Seconds32(testCertNotBeforeTime.count() - 1000),
                                                             System::Clock::Seconds32(testCertNotBeforeTime.count() - 1000000) };
    // Test that certificate NotBefore times that are at or after the Firmware
    // build time do result in Last Known Good Times set to these.
    // Verify behavior both for fail-safe roll back and commit scenarios.
    for (auto buildTime : beforeNotBeforeBuildTimes)
    {
        // Set build time to the desired value.
        EXPECT_EQ(DeviceLayer::ConfigurationMgr().SetFirmwareBuildChipEpochTime(buildTime), CHIP_NO_ERROR);
        chip::TestPersistentStorageDelegate testStorage;
        {
            // Initialize a fabric table.
            ScopedFabricTable fabricTableHolder;
            EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Load a test fabric, but do not commit.
            EXPECT_EQ(LoadTestFabric_Node01_01(fabricTable, /* doCommit = */ false), CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it is now set to the certificate
            // NotBefore time, as this should be at or after firmware build time.
            System::Clock::Seconds32 lastKnownGoodTime;
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, testCertNotBeforeTime);

            // Now test revert.  Last known good time should change back to the
            // previous value.
            fabricTable.RevertPendingFabricData();
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, buildTime);
        }
        {
            // Test reloading last known good time from persistence.
            ScopedFabricTable fabricTableHolder;
            EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Verify that the original last known good time was retained, since
            // we reverted before.
            System::Clock::Seconds32 lastKnownGoodTime;
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
        }
        {
            // Now test loading a fabric and committing.
            ScopedFabricTable fabricTableHolder;
            EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();
            EXPECT_EQ(LoadTestFabric_Node01_01(fabricTable, /* doCommit = */ true), CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it is now set to the certificate
            // NotBefore time, as this should be at or after firmware build time.
            System::Clock::Seconds32 lastKnownGoodTime;
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, testCertNotBeforeTime);

            // Now test revert, which will be a no-op because we already
            // committed.  Verify that Last Known Good time is retained.
            fabricTable.RevertPendingFabricData();
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, testCertNotBeforeTime);
        }
        {
            // Test reloading last known good time from persistence.
            ScopedFabricTable fabricTableHolder;
            EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Verify that the new last known good time was retained, since
            // we committed.
            System::Clock::Seconds32 lastKnownGoodTime;
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, testCertNotBeforeTime);
        }
    }
}

TEST_F(TestFabricTable, TestSetLastKnownGoodTime)
{
    // It is desirable for nodes to set Last Known Good Time whenever a good
    // time source is available, including cases where this would set the time
    // backward.  However, it is impermissible to set last known good time to
    // any time before the Firmware Build time or the latest NotBefore of any
    // installed certificate.

    // Test certs all have this NotBefore: Oct 15 14:23:43 2020 GMT
    const ASN1::ASN1UniversalTime asn1Expected = { 2020, 10, 15, 14, 23, 43 };
    uint32_t testCertNotBeforeSeconds;
    EXPECT_EQ(Credentials::ASN1ToChipEpochTime(asn1Expected, testCertNotBeforeSeconds), CHIP_NO_ERROR);
    System::Clock::Seconds32 testCertNotBeforeTime = System::Clock::Seconds32(testCertNotBeforeSeconds);

    // Iterate over two cases: one with build time prior to our certificates' NotBefore, one with build time after.
    System::Clock::Seconds32 testCaseFirmwareBuildTimes[] = { System::Clock::Seconds32(testCertNotBeforeTime.count() - 100000),
                                                              System::Clock::Seconds32(testCertNotBeforeTime.count() + 100000) };

    for (auto buildTime : testCaseFirmwareBuildTimes)
    {
        // Set build time to the desired value.
        EXPECT_EQ(DeviceLayer::ConfigurationMgr().SetFirmwareBuildChipEpochTime(buildTime), CHIP_NO_ERROR);
        chip::TestPersistentStorageDelegate testStorage;
        System::Clock::Seconds32 newTime;
        {
            // Initialize a fabric table.
            ScopedFabricTable fabricTableHolder;
            EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Load a test fabric
            EXPECT_EQ(LoadTestFabric_Node01_01(fabricTable, /* doCommit= */ true), CHIP_NO_ERROR);

            // Verify the Last Known Good Time matches our expected initial value.
            System::Clock::Seconds32 initialLastKnownGoodTime =
                buildTime > testCertNotBeforeTime ? buildTime : testCertNotBeforeTime;
            System::Clock::Seconds32 lastKnownGoodTime;
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, initialLastKnownGoodTime);

            // Read Last Known Good Time and verify that it hasn't moved forward, since
            // build time is later than the test certs' NotBefore times.
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, initialLastKnownGoodTime);

            // Attempt to set a Last Known Good Time that is before the firmware build time.  This should fail.
            newTime = System::Clock::Seconds32(buildTime.count() - 1000);
            EXPECT_NE(fabricTable.SetLastKnownGoodChipEpochTime(newTime), CHIP_NO_ERROR);

            // Verify Last Known Good Time is unchanged.
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, initialLastKnownGoodTime);

            // Attempt to set a Last Known Good Time that is before our certificates' NotBefore times.  This should fail.
            newTime = System::Clock::Seconds32(testCertNotBeforeTime.count() - 1000);
            EXPECT_NE(fabricTable.SetLastKnownGoodChipEpochTime(newTime), CHIP_NO_ERROR);

            // Verify Last Known Good Time is unchanged.
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, initialLastKnownGoodTime);

            // Attempt to set a Last Known Good Time that at our current value.
            EXPECT_EQ(fabricTable.SetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);

            // Verify Last Known Good Time is unchanged.
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, initialLastKnownGoodTime);

            // Attempt to set Last Known Good Times that is after our current value.
            newTime = System::Clock::Seconds32(initialLastKnownGoodTime.count() + 1000);
            EXPECT_EQ(fabricTable.SetLastKnownGoodChipEpochTime(newTime), CHIP_NO_ERROR);

            // Verify Last Known Good Time is updated.
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, newTime);
        }
        {
            // Verify that Last Known Good Time was persisted.

            ScopedFabricTable fabricTableHolder;
            EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            System::Clock::Seconds32 lastKnownGoodTime;
            EXPECT_EQ(fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime), CHIP_NO_ERROR);
            EXPECT_EQ(lastKnownGoodTime, newTime);
        }
    }
}

// Test adding 2 fabrics, updating 1, removing 1
TEST_F(TestFabricTable, TestBasicAddNocUpdateNocFlow)
{
    Credentials::TestOnlyLocalCertificateAuthority fabric11CertAuthority;
    Credentials::TestOnlyLocalCertificateAuthority fabric44CertAuthority;

    chip::TestPersistentStorageDelegate storage;

    // Uncomment the next line for superior debugging powers if you blow-up this test
    // storage.SetLoggingLevel(chip::TestPersistentStorageDelegate::LoggingLevel::kLogMutation);

    // Initialize test CA and a Fabric 11 externally owned key
    EXPECT_TRUE(fabric11CertAuthority.Init().IsSuccess());
    EXPECT_TRUE(fabric44CertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    chip::Crypto::P256Keypair fabric11Node55Keypair; // Fabric ID 11,
    EXPECT_EQ(fabric11Node55Keypair.Initialize(Crypto::ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.FabricCount(), 0);
    EXPECT_EQ(fabricTable.GetPendingNewFabricIndex(), kUndefinedFabricIndex);

    {
        FabricIndex nextFabricIndex = kUndefinedFabricIndex;
        EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
        EXPECT_EQ(nextFabricIndex, 1);
    }

    size_t numFabricsIterated = 0;

    size_t numStorageKeysAtStart = storage.GetNumKeys();

    // Sequence 1: Add node ID 55 on fabric 11, using externally owned key and no ICAC --> Yield fabricIndex 1
    {
        FabricId fabricId = 11;
        NodeId nodeId     = 55;
        EXPECT_EQ(fabric11CertAuthority.SetIncludeIcac(false)
                      .GenerateNocChain(fabricId, nodeId, fabric11Node55Keypair.Pubkey())
                      .GetStatus(),
                  CHIP_NO_ERROR);
        ByteSpan rcac = fabric11CertAuthority.GetRcac();
        ByteSpan noc  = fabric11CertAuthority.GetNoc();

        // Validate iterator sees nothing yet
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 0u);
            EXPECT_FALSE(saw1);
        }

        uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
        {
            // No pending root cert yet.
            MutableByteSpan fetchedSpan{ rcacBuf };
            EXPECT_EQ(fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan), CHIP_ERROR_NOT_FOUND);
        }

        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        {
            // Now have a pending root cert.
            MutableByteSpan fetchedSpan{ rcacBuf };
            EXPECT_EQ(fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan), CHIP_NO_ERROR);
            EXPECT_TRUE(fetchedSpan.data_equal(rcac));
        }
        EXPECT_EQ(fabricTable.GetPendingNewFabricIndex(), kUndefinedFabricIndex);

        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        bool keyIsExternallyOwned  = true;

        EXPECT_EQ(fabricTable.FabricCount(), 0);
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithProvidedOpKey(noc, ByteSpan{}, kVendorId, &fabric11Node55Keypair,
                                                                   keyIsExternallyOwned, &newFabricIndex),
                  CHIP_NO_ERROR);
        EXPECT_EQ(newFabricIndex, 1);
        EXPECT_EQ(fabricTable.FabricCount(), 1);

        // After adding the pending new fabric (equivalent of AddNOC processing), the new
        // fabric must be pending.
        EXPECT_EQ(fabricTable.GetPendingNewFabricIndex(), 1);

        {
            // No more pending root cert; it's associated with a fabric now.
            MutableByteSpan fetchedSpan{ rcacBuf };
            EXPECT_EQ(fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan), CHIP_ERROR_NOT_FOUND);
        }

        // No storage yet
        EXPECT_EQ(storage.GetNumKeys(), numStorageKeysAtStart);

        // Next fabric index has not been updated yet.
        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 1);
        }

        // Validate iterator sees pending
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), nodeId);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), fabricId);
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_TRUE(saw1);
        }

        // Commit, now storage should have keys
        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

        EXPECT_EQ(storage.GetNumKeys(), (numStorageKeysAtStart + 4)); // 2 opcerts + fabric metadata + index

        // Next fabric index has been updated.
        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 2);
        }

        // Fabric can't be pending anymore.
        EXPECT_EQ(fabricTable.GetPendingNewFabricIndex(), kUndefinedFabricIndex);

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        ASSERT_NE(fabricInfo, nullptr);
        Credentials::ChipCertificateSet certificates;
        EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
        EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
        Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

        EXPECT_EQ(fabricInfo->GetFabricIndex(), newFabricIndex);
        EXPECT_EQ(fabricInfo->GetNodeId(), nodeId);
        EXPECT_EQ(fabricInfo->GetFabricId(), fabricId);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

        Crypto::P256PublicKey rootPublicKeyOfFabric;
        EXPECT_EQ(fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
        EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

        // Validate that fabric has the correct operational key by verifying a signature
        Crypto::P256ECDSASignature sig;
        uint8_t message[] = { 'm', 's', 'g' };
        EXPECT_EQ(fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig), CHIP_NO_ERROR);
        EXPECT_EQ(fabric11Node55Keypair.Pubkey().ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);

        // Validate iterator sees committed
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), nodeId);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), fabricId);
                    EXPECT_TRUE(iterFabricInfo.ShouldAdvertiseIdentity());
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_TRUE(saw1);
        }
    }

    size_t numStorageAfterFirstAdd = storage.GetNumKeys();

    // Sequence 2: Add node ID 999 on fabric 44, using operational keystore and ICAC --> Yield fabricIndex 2
    {
        FabricId fabricId = 44;
        NodeId nodeId     = 999;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabric44CertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                  CHIP_NO_ERROR);
        ByteSpan rcac = fabric44CertAuthority.GetRcac();
        ByteSpan icac = fabric44CertAuthority.GetIcac();
        ByteSpan noc  = fabric44CertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 1);

        // Next fabric index should still be the same as before.
        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 2);
        }

        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.GetPendingNewFabricIndex(), kUndefinedFabricIndex);

        FabricIndex newFabricIndex = kUndefinedFabricIndex;

        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 2);
        EXPECT_EQ(newFabricIndex, 2);
        EXPECT_EQ(fabricTable.GetPendingNewFabricIndex(), 2);

        // No storage yet
        EXPECT_EQ(storage.GetNumKeys(), numStorageAfterFirstAdd);
        // Next fabric index has not been updated yet.
        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 2);
        }

        // Commit, now storage should have keys
        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 2);

        EXPECT_EQ(storage.GetNumKeys(),
                  (numStorageAfterFirstAdd + 5)); // 3 opcerts + fabric metadata + 1 operational key

        // Next fabric index has been updated.
        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 3);
        }

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
        ASSERT_NE(fabricInfo, nullptr);
        Credentials::ChipCertificateSet certificates;
        EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
        EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
        Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

        EXPECT_EQ(fabricInfo->GetFabricIndex(), newFabricIndex);
        EXPECT_EQ(fabricInfo->GetNodeId(), nodeId);
        EXPECT_EQ(fabricInfo->GetFabricId(), fabricId);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

        Crypto::P256PublicKey rootPublicKeyOfFabric;
        EXPECT_EQ(fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
        EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

        // Validate that fabric has the correct operational key by verifying a signature
        {
            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            Crypto::P256PublicKey nocPubKey;
            EXPECT_EQ(VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), nocPubKey), CHIP_NO_ERROR);

            EXPECT_EQ(fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig), CHIP_NO_ERROR);
            EXPECT_EQ(nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
        }

        // Verify we can now see 2 fabrics with the iterator
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            bool saw2          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 55u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 11u);
                    EXPECT_TRUE(iterFabricInfo.ShouldAdvertiseIdentity());
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 999u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 44u);
                    EXPECT_TRUE(iterFabricInfo.ShouldAdvertiseIdentity());
                    saw2 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 2u);
            EXPECT_TRUE(saw1);
            EXPECT_TRUE(saw2);
        }
    }

    size_t numStorageAfterSecondAdd = storage.GetNumKeys();

    // Sequence 3: Update node ID 999 to 1000 on fabric 44, using operational keystore and no ICAC --> Stays fabricIndex 2
    {
        FabricId fabricId       = 44;
        NodeId nodeId           = 1000;
        FabricIndex fabricIndex = 2;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };

        // Make sure to tag fabric index to pending opkey: otherwise the UpdateNOC fails
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::MakeOptional(static_cast<FabricIndex>(2)), csrSpan),
                  CHIP_NO_ERROR);

        EXPECT_EQ(fabric44CertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                  CHIP_NO_ERROR);
        ByteSpan rcac = fabric44CertAuthority.GetRcac();
        ByteSpan noc  = fabric44CertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 2);
        EXPECT_EQ(fabricTable.UpdatePendingFabricWithOperationalKeystore(2, noc, ByteSpan{}, FabricTable::AdvertiseIdentity::No),
                  CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 2);

        // No storage yet
        EXPECT_EQ(storage.GetNumKeys(), numStorageAfterSecondAdd);

        // Validate iterator sees the pending data
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            bool saw2          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 55u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 11u);
                    EXPECT_TRUE(iterFabricInfo.ShouldAdvertiseIdentity());
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 1000u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 44u);
                    EXPECT_FALSE(iterFabricInfo.ShouldAdvertiseIdentity());
                    saw2 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 2u);
            EXPECT_TRUE(saw1);
            EXPECT_TRUE(saw2);
        }

        // Commit, now storage should have keys
        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 2);

        EXPECT_EQ(storage.GetNumKeys(), (numStorageAfterSecondAdd - 1)); // ICAC got deleted

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
        ASSERT_NE(fabricInfo, nullptr);
        Credentials::ChipCertificateSet certificates;
        EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
        EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
        Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

        EXPECT_EQ(fabricInfo->GetFabricIndex(), fabricIndex);
        EXPECT_EQ(fabricInfo->GetNodeId(), nodeId);
        EXPECT_EQ(fabricInfo->GetFabricId(), fabricId);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

        Crypto::P256PublicKey rootPublicKeyOfFabric;
        EXPECT_EQ(fabricTable.FetchRootPubkey(fabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
        EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

        // Validate that fabric has the correct operational key by verifying a signature
        {
            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            Crypto::P256PublicKey nocPubKey;
            EXPECT_EQ(VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), nocPubKey), CHIP_NO_ERROR);

            EXPECT_EQ(fabricTable.SignWithOpKeypair(fabricIndex, ByteSpan{ message }, sig), CHIP_NO_ERROR);
            EXPECT_EQ(nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
        }

        // Validate iterator sees the committed update
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            bool saw2          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 55u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 11u);
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 1000u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 44u);
                    saw2 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 2u);
            EXPECT_TRUE(saw1);
            EXPECT_TRUE(saw2);
        }

        // Next fabric index has stayed the same.
        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 3);
        }
    }

    size_t numStorageAfterUpdate = storage.GetNumKeys();

    // Sequence 4: Rename fabric index 2, applies immediately when nothing pending
    {
        EXPECT_EQ(fabricTable.FabricCount(), 2);
        EXPECT_EQ(fabricTable.SetFabricLabel(2, "roboto"_span), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 2);

        EXPECT_EQ(storage.GetNumKeys(), numStorageAfterUpdate); // Number of keys unchanged

        // Validate basic contents
        {
            const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
            ASSERT_NE(fabricInfo, nullptr);
            EXPECT_EQ(fabricInfo->GetFabricIndex(), 2);
            EXPECT_EQ(fabricInfo->GetNodeId(), 1000u);
            EXPECT_EQ(fabricInfo->GetFabricId(), 44u);
            EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
            EXPECT_TRUE(fabricInfo->GetFabricLabel().data_equal(CharSpan{ "roboto", strlen("roboto") }));
        }

        // Next fabric index has stayed the same.
        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 3);
        }
    }

    // Sequence 5: Remove FabricIndex 1 (FabricId 11, NodeId 55), make sure FabricIndex 2 (FabricId 44, NodeId 1000) still exists
    {
        // Remove the fabric: no commit needed
        {
            EXPECT_EQ(fabricTable.FabricCount(), 2);
            EXPECT_EQ(fabricTable.Delete(1), CHIP_NO_ERROR);
            EXPECT_EQ(fabricTable.FabricCount(), 1);

            EXPECT_EQ(storage.GetNumKeys(), (numStorageAfterUpdate - 3)); // Deleted NOC, RCAC, Metadata
        }

        // Next fabric index has stayed the same.
        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 3);
        }

        // Validate contents of Fabric Index 2 is still OK
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_EQ(fabricInfo->GetFabricIndex(), 2);
        EXPECT_EQ(fabricInfo->GetNodeId(), 1000u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 44u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_TRUE(fabricInfo->GetFabricLabel().data_equal(CharSpan{ "roboto", strlen("roboto") }));

        Crypto::P256PublicKey rootPublicKeyOfFabric;
        EXPECT_EQ(fabricTable.FetchRootPubkey(2, rootPublicKeyOfFabric), CHIP_NO_ERROR);

        // Validate that fabric has the correct operational key by verifying a signature
        {
            uint8_t nocBuf[Credentials::kMaxCHIPCertLength];
            MutableByteSpan nocSpan{ nocBuf };
            EXPECT_EQ(fabricTable.FetchNOCCert(2, nocSpan), CHIP_NO_ERROR);

            Credentials::ChipCertificateSet certificates;
            EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
            EXPECT_EQ(certificates.LoadCert(nocSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
            Crypto::P256PublicKey nocPubKey(certificates.GetCertSet()[0].mPublicKey);

            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            EXPECT_EQ(fabricTable.SignWithOpKeypair(2, ByteSpan{ message }, sig), CHIP_NO_ERROR);
            EXPECT_EQ(nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
        }

        // Validate iterator only sees the remaining fabric
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            bool saw2          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 1000u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 44u);
                    saw2 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_FALSE(saw1);
            EXPECT_TRUE(saw2);
        }
    }
}

TEST_F(TestFabricTable, TestAddMultipleSameRootDifferentFabricId)
{
    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority;

    chip::TestPersistentStorageDelegate storage;
    EXPECT_TRUE(fabricCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.FabricCount(), 0);

    uint8_t rcac1Buf[kMaxCHIPCertLength];
    MutableByteSpan rcac1Span{ rcac1Buf };

    // First scope: add FabricID 1111, node ID 55
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(), CHIP_NO_ERROR);
        ByteSpan rcac = fabricCertAuthority.GetRcac();
        // Keep a copy for second scope check
        CopySpanToMutableSpan(rcac, rcac1Span);

        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 0);
        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex,
                                                                         FabricTable::AdvertiseIdentity::No),
                  CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(newFabricIndex, 1);

        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
        EXPECT_EQ(fabricInfo->GetNodeId(), 55u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 1111u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);
        EXPECT_FALSE(fabricInfo->ShouldAdvertiseIdentity());

        EXPECT_EQ(fabricTable.SetShouldAdvertiseIdentity(newFabricIndex, FabricTable::AdvertiseIdentity::Yes), CHIP_NO_ERROR);
        EXPECT_TRUE(fabricInfo->ShouldAdvertiseIdentity());

        // Check that for indices we don't have a fabric for, SetShouldAdvertiseIdentity fails.
        EXPECT_EQ(fabricTable.SetShouldAdvertiseIdentity(kUndefinedFabricIndex, FabricTable::AdvertiseIdentity::No),
                  CHIP_ERROR_INVALID_FABRIC_INDEX);
        EXPECT_EQ(fabricTable.SetShouldAdvertiseIdentity(kUndefinedFabricIndex, FabricTable::AdvertiseIdentity::Yes),
                  CHIP_ERROR_INVALID_FABRIC_INDEX);
        EXPECT_EQ(fabricTable.SetShouldAdvertiseIdentity(2, FabricTable::AdvertiseIdentity::Yes), CHIP_ERROR_INVALID_FABRIC_INDEX);
        EXPECT_EQ(fabricTable.SetShouldAdvertiseIdentity(2, FabricTable::AdvertiseIdentity::No), CHIP_ERROR_INVALID_FABRIC_INDEX);

        EXPECT_TRUE(fabricInfo->ShouldAdvertiseIdentity());

        EXPECT_EQ(fabricTable.SetShouldAdvertiseIdentity(newFabricIndex, FabricTable::AdvertiseIdentity::No), CHIP_NO_ERROR);
        EXPECT_FALSE(fabricInfo->ShouldAdvertiseIdentity());
    }
    size_t numStorageKeysAfterFirstAdd = storage.GetNumKeys();
    EXPECT_EQ(numStorageKeysAfterFirstAdd, 7u); // Metadata, index, 3 certs, 1 opkey, last known good time

    // Second scope: add FabricID 2222, node ID 66, same root as first
    {
        FabricId fabricId = 2222;
        NodeId nodeId     = 66;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(), CHIP_NO_ERROR);
        ByteSpan rcac2 = fabricCertAuthority.GetRcac();
        EXPECT_TRUE(rcac2.data_equal(rcac1Span));

        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac2), CHIP_NO_ERROR);
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 2);
        EXPECT_EQ(newFabricIndex, 2);

        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_EQ(fabricInfo->GetFabricIndex(), 2);
        EXPECT_EQ(fabricInfo->GetNodeId(), 66u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 2222u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

        EXPECT_TRUE(fabricInfo->ShouldAdvertiseIdentity());

        EXPECT_EQ(fabricTable.SetShouldAdvertiseIdentity(newFabricIndex, FabricTable::AdvertiseIdentity::No), CHIP_NO_ERROR);
        EXPECT_FALSE(fabricInfo->ShouldAdvertiseIdentity());

        EXPECT_EQ(fabricTable.SetShouldAdvertiseIdentity(newFabricIndex, FabricTable::AdvertiseIdentity::Yes), CHIP_NO_ERROR);
        EXPECT_TRUE(fabricInfo->ShouldAdvertiseIdentity());
    }
    size_t numStorageKeysAfterSecondAdd = storage.GetNumKeys();
    EXPECT_EQ(numStorageKeysAfterSecondAdd, (numStorageKeysAfterFirstAdd + 5)); // Add 3 certs, 1 metadata, 1 opkey
}

TEST_F(TestFabricTable, TestAddMultipleSameFabricIdDifferentRoot)
{
    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority1;
    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority2;

    chip::TestPersistentStorageDelegate storage;
    EXPECT_TRUE(fabricCertAuthority1.Init().IsSuccess());
    EXPECT_TRUE(fabricCertAuthority2.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.FabricCount(), 0);

    uint8_t rcac1Buf[kMaxCHIPCertLength];
    MutableByteSpan rcac1Span{ rcac1Buf };

    // First scope: add FabricID 1111, node ID 55
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabricCertAuthority1.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(), CHIP_NO_ERROR);
        ByteSpan rcac = fabricCertAuthority1.GetRcac();
        // Keep a copy for second scope check
        CopySpanToMutableSpan(rcac, rcac1Span);

        ByteSpan icac = fabricCertAuthority1.GetIcac();
        ByteSpan noc  = fabricCertAuthority1.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 0);
        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(newFabricIndex, 1);

        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
        EXPECT_EQ(fabricInfo->GetNodeId(), 55u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 1111u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);
    }
    size_t numStorageKeysAfterFirstAdd = storage.GetNumKeys();
    EXPECT_EQ(numStorageKeysAfterFirstAdd, 7u); // Metadata, index, 3 certs, 1 opkey, last known good time

    // Second scope: add FabricID 1111, node ID 66, different root from first
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 66;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabricCertAuthority2.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(), CHIP_NO_ERROR);
        ByteSpan rcac2 = fabricCertAuthority2.GetRcac();
        EXPECT_FALSE(rcac2.data_equal(rcac1Span));

        ByteSpan icac = fabricCertAuthority2.GetIcac();
        ByteSpan noc  = fabricCertAuthority2.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac2), CHIP_NO_ERROR);
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 2);
        EXPECT_EQ(newFabricIndex, 2);

        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_EQ(fabricInfo->GetFabricIndex(), 2);
        EXPECT_EQ(fabricInfo->GetNodeId(), 66u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 1111u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);
    }
    size_t numStorageKeysAfterSecondAdd = storage.GetNumKeys();
    EXPECT_EQ(numStorageKeysAfterSecondAdd, (numStorageKeysAfterFirstAdd + 5)); // Add 3 certs, 1 metadata, 1 opkey
}

TEST_F(TestFabricTable, TestPersistence)
{
    /**
     *
     * - Create an outer scope with storage delegate
     *   - Keep buffer slots for the operational public keys of the 2 fabrics added in the next scope
     *
     * - Create a new scope with a ScopedFabricTable
     *   - Add 2 fabrics, fully committed, using OperationalKeystore-based storage (e.g. CSR for opkey)
     *   - Make sure to save public keys in other scope for next step
     *
     * - Create a new scope with a ScopedFabricTable
     *   - Validate that after init, it has 2 fabrics and the 2 fabrics match fabric indices expected,
     *     and that the fabric tables are usable, and that NOC can be extracted for each, and that
     *     its public key matches expectation, and that they match the public keys stored in outer scope
     *     and verify you can sign and verify messages with the opkey
     *
     */

    Crypto::P256PublicKey fIdx1PublicKey;
    Crypto::P256PublicKey fIdx2PublicKey;

    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority;

    chip::TestPersistentStorageDelegate storage;

    EXPECT_TRUE(fabricCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // First scope: add 2 fabrics with same root: (1111, 2222), commit them, keep track of public keys
    {
        // Initialize a FabricTable
        ScopedFabricTable fabricTableHolder;
        EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
        FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

        EXPECT_EQ(fabricTable.FabricCount(), 0);

        // Add Fabric 1111 Node Id 55
        {
            FabricId fabricId = 1111;
            NodeId nodeId     = 55;

            uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
            MutableByteSpan csrSpan{ csrBuf };
            EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

            EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                      CHIP_NO_ERROR);
            ByteSpan rcac = fabricCertAuthority.GetRcac();
            ByteSpan icac = fabricCertAuthority.GetIcac();
            ByteSpan noc  = fabricCertAuthority.GetNoc();

            EXPECT_EQ(fabricTable.FabricCount(), 0);
            EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(fabricTable.FabricCount(), 1);
            EXPECT_EQ(newFabricIndex, 1);

            EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

            // Validate contents
            const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
            ASSERT_NE(fabricInfo, nullptr);
            Credentials::ChipCertificateSet certificates;
            EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
            EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
            Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

            EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
            EXPECT_EQ(fabricInfo->GetNodeId(), 55u);
            EXPECT_EQ(fabricInfo->GetFabricId(), 1111u);
            EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
            EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            EXPECT_EQ(fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
            EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

            // Validate that fabric has the correct operational key by verifying a signature
            {
                Crypto::P256ECDSASignature sig;
                uint8_t message[] = { 'm', 's', 'g' };

                EXPECT_EQ(VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), fIdx1PublicKey), CHIP_NO_ERROR);

                EXPECT_EQ(fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig), CHIP_NO_ERROR);
                EXPECT_EQ(fIdx1PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
            }
        }

        // Add Fabric 2222 Node Id 66, no ICAC
        {
            FabricId fabricId = 2222;
            NodeId nodeId     = 66;

            uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
            MutableByteSpan csrSpan{ csrBuf };
            EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

            EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                      CHIP_NO_ERROR);
            ByteSpan rcac = fabricCertAuthority.GetRcac();
            ByteSpan noc  = fabricCertAuthority.GetNoc();

            EXPECT_EQ(fabricTable.FabricCount(), 1);
            EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, ByteSpan{}, kVendorId, &newFabricIndex),
                      CHIP_NO_ERROR);
            EXPECT_EQ(fabricTable.FabricCount(), 2);
            EXPECT_EQ(newFabricIndex, 2);

            EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

            // Validate contents
            const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
            ASSERT_NE(fabricInfo, nullptr);
            Credentials::ChipCertificateSet certificates;
            EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
            EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
            Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

            EXPECT_EQ(fabricInfo->GetFabricIndex(), 2);
            EXPECT_EQ(fabricInfo->GetNodeId(), 66u);
            EXPECT_EQ(fabricInfo->GetFabricId(), 2222u);
            EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
            EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            EXPECT_EQ(fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
            EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

            // Validate that fabric has the correct operational key by verifying a signature
            {
                Crypto::P256ECDSASignature sig;
                uint8_t message[] = { 'm', 's', 'g' };

                EXPECT_EQ(VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), fIdx2PublicKey), CHIP_NO_ERROR);

                EXPECT_EQ(fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig), CHIP_NO_ERROR);
                EXPECT_EQ(fIdx2PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
            }
        }

        EXPECT_EQ(fabricTable.FabricCount(), 2);

        // Verify we can now see 2 fabrics with the iterator
        {
            size_t numFabricsIterated = 0;
            bool saw1                 = false;
            bool saw2                 = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 55u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 1111u);
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 66u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 2222u);
                    saw2 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 2u);
            EXPECT_TRUE(saw1);
            EXPECT_TRUE(saw2);
        }

        // Next fabric index should now be 3, since we added 1 and 2 above.
        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 3);
        }
    }

    // Global: Last known good time + fabric index = 2
    // Fabric 1111: Metadata, 1 opkey, RCAC/ICAC/NOC = 5
    // Fabric 2222: Metadata, 1 opkey, RCAC/NOC = 4
    EXPECT_EQ(storage.GetNumKeys(), (2u + 5u + 4u));

    // Second scope: Validate that a fresh FabricTable loads the previously committed fabrics on Init.
    {
        // Initialize a FabricTable
        ScopedFabricTable fabricTableHolder;
        EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
        FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

        EXPECT_EQ(fabricTable.FabricCount(), 2);

        // Verify we can see 2 fabrics with the iterator
        {
            size_t numFabricsIterated = 0;
            bool saw1                 = false;
            bool saw2                 = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 55u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 1111u);
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 66u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 2222u);
                    saw2 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 2u);
            EXPECT_TRUE(saw1);
            EXPECT_TRUE(saw2);
        }

        // Validate contents of Fabric 2222
        {
            uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
            MutableByteSpan rcacSpan{ rcacBuf };
            EXPECT_EQ(fabricTable.FetchRootCert(2, rcacSpan), CHIP_NO_ERROR);

            const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
            ASSERT_NE(fabricInfo, nullptr);
            Credentials::ChipCertificateSet certificates;
            EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
            EXPECT_EQ(certificates.LoadCert(rcacSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
            Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

            EXPECT_EQ(fabricInfo->GetFabricIndex(), 2);
            EXPECT_EQ(fabricInfo->GetNodeId(), 66u);
            EXPECT_EQ(fabricInfo->GetFabricId(), 2222u);
            EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
            EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            EXPECT_EQ(fabricTable.FetchRootPubkey(2, rootPublicKeyOfFabric), CHIP_NO_ERROR);
            EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

            // Validate that fabric has the correct operational key by verifying a signature
            {
                Crypto::P256ECDSASignature sig;
                uint8_t message[] = { 'm', 's', 'g' };

                EXPECT_EQ(fabricTable.SignWithOpKeypair(2, ByteSpan{ message }, sig), CHIP_NO_ERROR);
                EXPECT_EQ(fIdx2PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
            }
        }

        // Validate contents of Fabric 1111
        {
            uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
            MutableByteSpan rcacSpan{ rcacBuf };
            EXPECT_EQ(fabricTable.FetchRootCert(1, rcacSpan), CHIP_NO_ERROR);

            const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
            ASSERT_NE(fabricInfo, nullptr);
            Credentials::ChipCertificateSet certificates;
            EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
            EXPECT_EQ(certificates.LoadCert(rcacSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
            Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

            EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
            EXPECT_EQ(fabricInfo->GetNodeId(), 55u);
            EXPECT_EQ(fabricInfo->GetFabricId(), 1111u);
            EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
            EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            EXPECT_EQ(fabricTable.FetchRootPubkey(1, rootPublicKeyOfFabric), CHIP_NO_ERROR);
            EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

            // Validate that fabric has the correct operational key by verifying a signature
            {
                Crypto::P256ECDSASignature sig;
                uint8_t message[] = { 'm', 's', 'g' };

                EXPECT_EQ(fabricTable.SignWithOpKeypair(1, ByteSpan{ message }, sig), CHIP_NO_ERROR);
                EXPECT_EQ(fIdx1PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
            }

            // Validate that signing with Fabric index 2 fails to verify with fabric index 1
            {
                Crypto::P256ECDSASignature sig;
                uint8_t message[] = { 'm', 's', 'g' };

                EXPECT_EQ(fabricTable.SignWithOpKeypair(2, ByteSpan{ message }, sig), CHIP_NO_ERROR);
                EXPECT_EQ(fIdx1PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig),
                          CHIP_ERROR_INVALID_SIGNATURE);
            }
        }

        // Validate that next fabric index is still 3;
        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 3);
        }
    }
}

TEST_F(TestFabricTable, TestAddNocFailSafe)
{
    Credentials::TestOnlyLocalCertificateAuthority fabric11CertAuthority;
    Credentials::TestOnlyLocalCertificateAuthority fabric44CertAuthority;

    chip::TestPersistentStorageDelegate storage;

    EXPECT_TRUE(fabric11CertAuthority.Init().IsSuccess());
    EXPECT_TRUE(fabric44CertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.FabricCount(), 0);

    size_t numFabricsIterated = 0;

    size_t numStorageKeysAtStart = storage.GetNumKeys();

    // Sequence 1: Add node ID 55 on fabric 11, see that pending works, and that revert works
    {
        FabricId fabricId = 11;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabric11CertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                  CHIP_NO_ERROR);
        ByteSpan rcac = fabric11CertAuthority.GetRcac();
        ByteSpan noc  = fabric11CertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        FabricIndex newFabricIndex = kUndefinedFabricIndex;

        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 1);
        }

        EXPECT_EQ(fabricTable.FabricCount(), 0);
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, ByteSpan{}, kVendorId, &newFabricIndex),
                  CHIP_NO_ERROR);
        EXPECT_EQ(newFabricIndex, 1);
        EXPECT_EQ(fabricTable.FabricCount(), 1);

        // No storage yet
        EXPECT_EQ(storage.GetNumKeys(), numStorageKeysAtStart); // Nothing yet

        // Validate iterator sees pending
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), nodeId);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), fabricId);
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_TRUE(saw1);
        }

        // Revert, should see nothing yet
        fabricTable.RevertPendingFabricData();

        EXPECT_EQ(fabricTable.FabricCount(), 0);

        // No started except fabric index metadata
        EXPECT_EQ(storage.GetNumKeys(), (numStorageKeysAtStart + 1));

        // Validate iterator sees nothing
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 0u);
            EXPECT_FALSE(saw1);
        }

        // Validate next fabric index has not changed.
        {
            FabricIndex nextFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.PeekFabricIndexForNextAddition(nextFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(nextFabricIndex, 1);
        }
    }

    size_t numStorageAfterRevert = storage.GetNumKeys();

    // Sequence 2: Add node ID 999 on fabric 44, using operational keystore and ICAC --> Yield fabricIndex 1
    {
        FabricId fabricId = 44;
        NodeId nodeId     = 999;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabric44CertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                  CHIP_NO_ERROR);
        ByteSpan rcac = fabric44CertAuthority.GetRcac();
        ByteSpan icac = fabric44CertAuthority.GetIcac();
        ByteSpan noc  = fabric44CertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 0);
        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        FabricIndex newFabricIndex = kUndefinedFabricIndex;

        EXPECT_EQ(fabricTable.FabricCount(), 0);
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(newFabricIndex, 1);
        // No storage yet
        EXPECT_EQ(storage.GetNumKeys(), numStorageAfterRevert);

        // Commit, now storage should have keys
        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 1);

        EXPECT_EQ(storage.GetNumKeys(),
                  (numStorageAfterRevert + 5)); // 3 opcerts + fabric metadata + 1 operational key

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        ASSERT_NE(fabricInfo, nullptr);
        Credentials::ChipCertificateSet certificates;
        EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
        EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
        Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

        EXPECT_EQ(fabricInfo->GetFabricIndex(), newFabricIndex);
        EXPECT_EQ(fabricInfo->GetNodeId(), nodeId);
        EXPECT_EQ(fabricInfo->GetFabricId(), fabricId);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

        Crypto::P256PublicKey rootPublicKeyOfFabric;
        EXPECT_EQ(fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
        EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

        // Validate that fabric has the correct operational key by verifying a signature
        {
            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            Crypto::P256PublicKey nocPubKey;
            EXPECT_EQ(VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), nocPubKey), CHIP_NO_ERROR);

            EXPECT_EQ(fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig), CHIP_NO_ERROR);
            EXPECT_EQ(nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
        }

        // Verify we can now see the fabric with the iterator
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 999u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 44u);
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_TRUE(saw1);
        }
    }

    size_t numStorageAfterAdd = storage.GetNumKeys();

    // Sequence 3: Do a RevertPendingFabricData() again, see that it doesn't affect existing fabric

    {
        // Revert, should should look like a no-op
        fabricTable.RevertPendingFabricData();

        // No change of storage
        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(storage.GetNumKeys(), numStorageAfterAdd);

        // Verify we can still see the fabric with the iterator
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 999u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 44u);
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_TRUE(saw1);
        }
    }
}

TEST_F(TestFabricTable, TestUpdateNocFailSafe)
{
    Credentials::TestOnlyLocalCertificateAuthority fabric11CertAuthority;
    Credentials::TestOnlyLocalCertificateAuthority fabric44CertAuthority;

    chip::TestPersistentStorageDelegate storage;

    storage.SetLoggingLevel(chip::TestPersistentStorageDelegate::LoggingLevel::kLogMutation);

    EXPECT_TRUE(fabric11CertAuthority.Init().IsSuccess());
    EXPECT_TRUE(fabric44CertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.FabricCount(), 0);

    size_t numFabricsIterated = 0;

    size_t numStorageKeysAtStart = storage.GetNumKeys();

    // Sequence 1: Add node ID 999 on fabric 44, using operational keystore and ICAC --> Yield fabricIndex 1
    {
        FabricId fabricId = 44;
        NodeId nodeId     = 999;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabric44CertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                  CHIP_NO_ERROR);
        ByteSpan rcac = fabric44CertAuthority.GetRcac();
        ByteSpan icac = fabric44CertAuthority.GetIcac();
        ByteSpan noc  = fabric44CertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 0);
        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        FabricIndex newFabricIndex = kUndefinedFabricIndex;

        EXPECT_EQ(fabricTable.FabricCount(), 0);
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(newFabricIndex, 1);
        // No storage yet
        EXPECT_EQ(storage.GetNumKeys(), numStorageKeysAtStart);

        // Commit, now storage should have keys
        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 1);

        EXPECT_EQ(storage.GetNumKeys(),
                  (numStorageKeysAtStart + 6)); // 3 opcerts + fabric metadata + 1 operational key + LKGT + fabric index

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        ASSERT_NE(fabricInfo, nullptr);
        Credentials::ChipCertificateSet certificates;
        EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
        EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
        Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

        EXPECT_EQ(fabricInfo->GetFabricIndex(), newFabricIndex);
        EXPECT_EQ(fabricInfo->GetNodeId(), nodeId);
        EXPECT_EQ(fabricInfo->GetFabricId(), fabricId);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

        Crypto::P256PublicKey rootPublicKeyOfFabric;
        EXPECT_EQ(fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
        EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

        // Validate that fabric has the correct operational key by verifying a signature
        {
            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            Crypto::P256PublicKey nocPubKey;
            EXPECT_EQ(VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), nocPubKey), CHIP_NO_ERROR);

            EXPECT_EQ(fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig), CHIP_NO_ERROR);
            EXPECT_EQ(nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
        }

        // Verify we can now see the fabric with the iterator
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 999u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 44u);
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_TRUE(saw1);
        }
    }

    size_t numStorageAfterAdd = storage.GetNumKeys();

    // Sequence 2: Do an Update to NodeId 1000, with no ICAC, but revert it
    {
        FabricId fabricId       = 44;
        NodeId nodeId           = 1000;
        FabricIndex fabricIndex = 1;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };

        EXPECT_EQ(fabricTable.GetPendingNewFabricIndex(), kUndefinedFabricIndex);

        // Make sure to tag fabric index to pending opkey: otherwise the UpdateNOC fails
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::MakeOptional(static_cast<FabricIndex>(1)), csrSpan),
                  CHIP_NO_ERROR);

        EXPECT_EQ(fabric44CertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                  CHIP_NO_ERROR);
        ByteSpan rcac = fabric44CertAuthority.GetRcac();
        ByteSpan noc  = fabric44CertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(fabricTable.UpdatePendingFabricWithOperationalKeystore(1, noc, ByteSpan{}), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.GetPendingNewFabricIndex(), kUndefinedFabricIndex);
        EXPECT_EQ(fabricTable.FabricCount(), 1);

        // No storage yet
        EXPECT_EQ(storage.GetNumKeys(), numStorageAfterAdd);

        // Validate iterator sees the pending data
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 1000u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 44u);
                    EXPECT_TRUE(iterFabricInfo.ShouldAdvertiseIdentity());
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_TRUE(saw1);
        }

        // Revert, should see Node ID 999 again
        fabricTable.RevertPendingFabricData();
        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(fabricTable.GetPendingNewFabricIndex(), kUndefinedFabricIndex);

        EXPECT_EQ(storage.GetNumKeys(), numStorageAfterAdd);

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        ASSERT_NE(fabricInfo, nullptr);
        {
            Credentials::ChipCertificateSet certificates;
            EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
            EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
            Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

            EXPECT_EQ(fabricInfo->GetFabricIndex(), fabricIndex);
            EXPECT_EQ(fabricInfo->GetNodeId(), 999u);
            EXPECT_EQ(fabricInfo->GetFabricId(), 44u);
            EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
            EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            EXPECT_EQ(fabricTable.FetchRootPubkey(fabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
            EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));
        }

        // Validate that fabric has the correct operational key by verifying a signature
        {
            uint8_t nocBuf[Credentials::kMaxCHIPCertLength];
            MutableByteSpan nocSpan{ nocBuf };
            EXPECT_EQ(fabricTable.FetchNOCCert(1, nocSpan), CHIP_NO_ERROR);
            Credentials::ChipCertificateSet certificates;
            EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
            EXPECT_EQ(certificates.LoadCert(nocSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
            Crypto::P256PublicKey nocPubKey(certificates.GetCertSet()[0].mPublicKey);

            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            EXPECT_EQ(fabricTable.SignWithOpKeypair(fabricIndex, ByteSpan{ message }, sig), CHIP_NO_ERROR);
            EXPECT_EQ(nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
        }

        // Validate iterator sees the previous fabric
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 999u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 44u);
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_TRUE(saw1);
        }
    }

    // Sequence 3: Do an Update to NodeId 1001, with no ICAC, but commit it
    {
        FabricId fabricId       = 44;
        NodeId nodeId           = 1001;
        FabricIndex fabricIndex = 1;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };

        // Make sure to tag fabric index to pending opkey: otherwise the UpdateNOC fails
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::MakeOptional(static_cast<FabricIndex>(1)), csrSpan),
                  CHIP_NO_ERROR);

        EXPECT_EQ(fabric44CertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                  CHIP_NO_ERROR);
        ByteSpan rcac = fabric44CertAuthority.GetRcac();
        ByteSpan noc  = fabric44CertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(fabricTable.UpdatePendingFabricWithOperationalKeystore(1, noc, ByteSpan{}), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 1);

        // No storage yet
        EXPECT_EQ(storage.GetNumKeys(), numStorageAfterAdd);

        // Validate iterator sees the pending data
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 1001u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 44u);
                    EXPECT_TRUE(iterFabricInfo.ShouldAdvertiseIdentity());
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_TRUE(saw1);
        }

        // Commit, should see Node ID 1001, and 1 less cert in the storage
        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(storage.GetNumKeys(), numStorageAfterAdd - 1);

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        ASSERT_NE(fabricInfo, nullptr);
        Credentials::ChipCertificateSet certificates;
        EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
        EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
        Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

        EXPECT_EQ(fabricInfo->GetFabricIndex(), fabricIndex);
        EXPECT_EQ(fabricInfo->GetNodeId(), 1001u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 44u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

        Crypto::P256PublicKey rootPublicKeyOfFabric;
        EXPECT_EQ(fabricTable.FetchRootPubkey(fabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
        EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

        // Validate that fabric has the correct operational key by verifying a signature
        {
            Crypto::P256PublicKey nocPubKey;
            EXPECT_EQ(VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), nocPubKey), CHIP_NO_ERROR);

            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            EXPECT_EQ(fabricTable.SignWithOpKeypair(fabricIndex, ByteSpan{ message }, sig), CHIP_NO_ERROR);
            EXPECT_EQ(nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
        }

        // Validate iterator sees the updated fabric
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 1001u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 44u);
                    saw1 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_TRUE(saw1);
        }
    }
}

TEST_F(TestFabricTable, TestAddRootCertFailSafe)
{
    Credentials::TestOnlyLocalCertificateAuthority fabric11CertAuthority;

    chip::TestPersistentStorageDelegate storage;

    EXPECT_TRUE(fabric11CertAuthority.Init().IsSuccess());

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.FabricCount(), 0);

    // Add a root cert, see that pending works, and that revert works
    {
        ByteSpan rcac = fabric11CertAuthority.GetRcac();

        uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
        {
            // No pending root cert yet.
            MutableByteSpan fetchedSpan{ rcacBuf };
            EXPECT_EQ(fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan), CHIP_ERROR_NOT_FOUND);
        }

        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        {
            // Now have a pending root cert.
            MutableByteSpan fetchedSpan{ rcacBuf };
            EXPECT_EQ(fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan), CHIP_NO_ERROR);
            EXPECT_TRUE(fetchedSpan.data_equal(rcac));
        }

        // Revert
        fabricTable.RevertPendingFabricData();

        {
            // No pending root cert anymore.
            MutableByteSpan fetchedSpan{ rcacBuf };
            EXPECT_EQ(fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan), CHIP_ERROR_NOT_FOUND);
        }
    }
}

TEST_F(TestFabricTable, TestSequenceErrors)
{
    // TODO: Write test
}

TEST_F(TestFabricTable, TestFabricLabelChange)
{
    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority;

    chip::TestPersistentStorageDelegate storage;
    EXPECT_TRUE(fabricCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.FabricCount(), 0);

    // First scope: add FabricID 1111, node ID 55
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(), CHIP_NO_ERROR);
        ByteSpan rcac = fabricCertAuthority.GetRcac();
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 0);
        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(newFabricIndex, 1);

        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
        EXPECT_EQ(fabricInfo->GetNodeId(), 55u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 1111u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);
    }
    size_t numStorageKeysAfterFirstAdd = storage.GetNumKeys();
    EXPECT_EQ(numStorageKeysAfterFirstAdd, 7u); // Metadata, index, 3 certs, 1 opkey, last known good time

    // Second scope: set FabricLabel to "acme fabric", make sure it cannot be reverted
    {
        // Fabric label starts unset from prior scope
        CharSpan fabricLabel = "placeholder"_span;

        EXPECT_EQ(fabricTable.GetFabricLabel(1, fabricLabel), CHIP_NO_ERROR);
        EXPECT_EQ(fabricLabel.size(), 0u);

        // Set a valid name
        EXPECT_EQ(fabricTable.SetFabricLabel(1, "acme fabric"_span), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.GetFabricLabel(1, fabricLabel), CHIP_NO_ERROR);
        EXPECT_TRUE(fabricLabel.data_equal("acme fabric"_span));

        // Revert pending fabric data. Should not revert name since nothing pending.
        fabricTable.RevertPendingFabricData();

        fabricLabel = "placeholder"_span;
        EXPECT_EQ(fabricTable.GetFabricLabel(1, fabricLabel), CHIP_NO_ERROR);
        EXPECT_TRUE(fabricLabel.data_equal("acme fabric"_span));

        // Verify we fail to set too large a label (> kFabricLabelMaxLengthInBytes)
        CharSpan fabricLabelTooBig = "012345678901234567890123456789123456"_span;
        EXPECT_GT(fabricLabelTooBig.size(), chip::kFabricLabelMaxLengthInBytes);

        EXPECT_EQ(fabricTable.SetFabricLabel(1, fabricLabelTooBig), CHIP_ERROR_INVALID_ARGUMENT);

        fabricLabel = "placeholder"_span;
        EXPECT_EQ(fabricTable.GetFabricLabel(1, fabricLabel), CHIP_NO_ERROR);
        EXPECT_TRUE(fabricLabel.data_equal("acme fabric"_span));
    }

    // Third scope: set fabric label after an update, it sticks, but then goes back after revert
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 66; // Update node ID from 55 to 66

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::MakeOptional(static_cast<FabricIndex>(1)), csrSpan),
                  CHIP_NO_ERROR);

        EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(), CHIP_NO_ERROR);
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(fabricTable.UpdatePendingFabricWithOperationalKeystore(1, noc, icac), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 1);

        // Validate contents prior to change/revert
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);

        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
        EXPECT_EQ(fabricInfo->GetNodeId(), 66u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 1111u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);

        CharSpan fabricLabel = fabricInfo->GetFabricLabel();
        EXPECT_TRUE(fabricLabel.data_equal("acme fabric"_span));

        // Update fabric label
        fabricLabel = "placeholder"_span;
        EXPECT_EQ(fabricTable.SetFabricLabel(1, "roboto fabric"_span), CHIP_NO_ERROR);

        fabricLabel = "placeholder"_span;
        EXPECT_EQ(fabricTable.GetFabricLabel(1, fabricLabel), CHIP_NO_ERROR);
        EXPECT_TRUE(fabricLabel.data_equal("roboto fabric"_span));

        // Revert pending fabric data. Should revert name to "acme fabric"
        fabricTable.RevertPendingFabricData();

        fabricLabel = "placeholder"_span;
        EXPECT_EQ(fabricTable.GetFabricLabel(1, fabricLabel), CHIP_NO_ERROR);
        EXPECT_TRUE(fabricLabel.data_equal("acme fabric"_span));
    }
}

TEST_F(TestFabricTable, TestCompressedFabricId)
{
    // TODO: Write test
}

TEST_F(TestFabricTable, TestFabricLookup)
{
    // Initialize a fabric table.
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();
    EXPECT_EQ(LoadTestFabric_Node01_01(fabricTable, /* doCommit = */ true), CHIP_NO_ERROR);
    EXPECT_EQ(LoadTestFabric_Node02_01(fabricTable, /* doCommit = */ true, FabricTable::AdvertiseIdentity::No), CHIP_NO_ERROR);

    // These two NOCs have the same fabric id on purpose; only the trust root is
    // different.
    constexpr FabricId kNode01_01_and_02_01_FabricId = 0xFAB000000000001D;

    // Attempt lookup of the Root01 fabric.
    {
        auto fabricInfo = FindFabric(fabricTable, TestCerts::sTestCert_Root01_PublicKey, kNode01_01_and_02_01_FabricId);
        ASSERT_NE(fabricInfo, nullptr);

        EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
        EXPECT_TRUE(fabricInfo->ShouldAdvertiseIdentity());
    }

    // Attempt lookup of the Root02 fabric.
    {
        auto fabricInfo = FindFabric(fabricTable, TestCerts::sTestCert_Root02_PublicKey, kNode01_01_and_02_01_FabricId);
        ASSERT_NE(fabricInfo, nullptr);

        EXPECT_EQ(fabricInfo->GetFabricIndex(), 2);
        EXPECT_FALSE(fabricInfo->ShouldAdvertiseIdentity());
    }

    // Attempt lookup of FabricIndex 0 --> should always fail.
    {
        EXPECT_EQ(fabricTable.FindFabricWithIndex(0), nullptr);
    }
}

TEST_F(TestFabricTable, ShouldFailSetFabricIndexWithInvalidIndex)
{
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.SetFabricIndexForNextAddition(kUndefinedFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
}

TEST_F(TestFabricTable, ShouldFailSetFabricIndexWithPendingFabric)
{
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(ByteSpan(TestCerts::sTestCert_Root01_Chip)), CHIP_NO_ERROR);

    EXPECT_EQ(fabricTable.SetFabricIndexForNextAddition(1), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestFabricTable, ShouldFailSetFabricIndexWhenInUse)
{
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(LoadTestFabric_Node01_01(fabricTable, /* doCommit = */ true), CHIP_NO_ERROR);
    EXPECT_EQ(fabricTable.SetFabricIndexForNextAddition(1), CHIP_ERROR_FABRIC_EXISTS);
}

TEST_F(TestFabricTable, ShouldAddFabricAtRequestedIndex)
{
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.SetFabricIndexForNextAddition(2), CHIP_NO_ERROR);
    EXPECT_EQ(LoadTestFabric_Node02_01(fabricTable, /* doCommit = */ true), CHIP_NO_ERROR);

    EXPECT_EQ(fabricTable.SetFabricIndexForNextAddition(1), CHIP_NO_ERROR);
    EXPECT_EQ(LoadTestFabric_Node01_01(fabricTable, /* doCommit = */ true), CHIP_NO_ERROR);

    {
        auto fabricInfo = FindFabric(fabricTable, TestCerts::sTestCert_Root01_PublicKey, TestCerts::kTestCert_Node01_01_FabricId);
        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
        EXPECT_EQ(fabricInfo->GetNodeId(), TestCerts::kTestCert_Node01_01_NodeId);
        EXPECT_EQ(fabricInfo->GetFabricId(), TestCerts::kTestCert_Node01_01_FabricId);
    }

    {
        auto fabricInfo = FindFabric(fabricTable, TestCerts::sTestCert_Root02_PublicKey, TestCerts::kTestCert_Node02_01_FabricId);
        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_EQ(fabricInfo->GetFabricIndex(), 2);
        EXPECT_EQ(fabricInfo->GetNodeId(), TestCerts::kTestCert_Node02_01_NodeId);
        EXPECT_EQ(fabricInfo->GetFabricId(), TestCerts::kTestCert_Node02_01_FabricId);
    }
}

TEST_F(TestFabricTable, TestFetchCATs)
{
    // Initialize a fabric table.
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&testStorage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();
    EXPECT_EQ(LoadTestFabric_Node01_01(fabricTable, /* doCommit = */ true), CHIP_NO_ERROR);
    EXPECT_EQ(LoadTestFabric_Node02_01(fabricTable, /* doCommit = */ true), CHIP_NO_ERROR);

    // Attempt Fetching fabric index 1 CATs and verify contents.
    {
        CATValues cats;
        EXPECT_EQ(fabricTable.FetchCATs(1, cats), CHIP_NO_ERROR);
        // Test fabric NOCs don't contain any CATs.
        EXPECT_EQ(cats, kUndefinedCATs);
    }

    // Attempt Fetching fabric index 2 CATs and verify contents.
    {
        CATValues cats;
        EXPECT_EQ(fabricTable.FetchCATs(2, cats), CHIP_NO_ERROR);
        // Test fabric NOCs don't contain any CATs.
        EXPECT_EQ(cats, kUndefinedCATs);
    }

    // TODO(#20335): Add test cases for NOCs that actually embed CATs
}

// Validate that adding the same fabric twice fails (same root, same FabricId)
TEST_F(TestFabricTable, TestAddNocRootCollision)
{
    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority;

    chip::TestPersistentStorageDelegate storage;
    EXPECT_TRUE(fabricCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.FabricCount(), 0);

    // First scope: add FabricID 1111, node ID 55
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(), CHIP_NO_ERROR);
        ByteSpan rcac = fabricCertAuthority.GetRcac();
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 0);
        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex,
                                                                         FabricTable::AdvertiseIdentity::No),
                  CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(newFabricIndex, 1);

        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_FALSE(fabricInfo->ShouldAdvertiseIdentity());

        Credentials::ChipCertificateSet certificates;
        EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
        EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
        Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

        EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
        EXPECT_EQ(fabricInfo->GetNodeId(), 55u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 1111u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

        Crypto::P256PublicKey rootPublicKeyOfFabric;
        EXPECT_EQ(fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
        EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));
    }
    size_t numStorageKeysAfterFirstAdd = storage.GetNumKeys();
    EXPECT_EQ(numStorageKeysAfterFirstAdd, 7u); // Metadata, index, 3 certs, 1 opkey, last known good time

    // Second scope: add FabricID 1111, node ID 55 *again* --> Collision of Root/FabricID with existing
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(), CHIP_NO_ERROR);
        ByteSpan rcac = fabricCertAuthority.GetRcac();
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex),
                  CHIP_ERROR_FABRIC_EXISTS);
        EXPECT_EQ(fabricTable.FabricCount(), 1);

        CHIP_ERROR err = fabricTable.CommitPendingFabricData();
        printf("err = %" CHIP_ERROR_FORMAT "\n", err.Format());
        EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

        // Validate contents of Fabric index 1 still valid
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        ASSERT_NE(fabricInfo, nullptr);
        Credentials::ChipCertificateSet certificates;
        EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
        EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
        Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

        EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
        EXPECT_EQ(fabricInfo->GetNodeId(), 55u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 1111u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

        Crypto::P256PublicKey rootPublicKeyOfFabric;
        EXPECT_EQ(fabricTable.FetchRootPubkey(1, rootPublicKeyOfFabric), CHIP_NO_ERROR);
        EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));
    }

    // Ensure no new persisted keys after failed colliding add
    EXPECT_EQ(storage.GetNumKeys(), numStorageKeysAfterFirstAdd);

    // Third scope: add FabricID 2222, node ID 55 --> Not colliding, should work. The failing commit above]
    // should have been enough of a revert that this scope succeeds without any additional revert.
    {
        FabricId fabricId = 2222;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(), CHIP_NO_ERROR);
        ByteSpan rcac = fabricCertAuthority.GetRcac();
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex), CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 2);
        EXPECT_EQ(newFabricIndex, 2);

        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_TRUE(fabricInfo->ShouldAdvertiseIdentity());

        Credentials::ChipCertificateSet certificates;
        EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
        EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
        Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

        EXPECT_EQ(fabricInfo->GetFabricIndex(), 2);
        EXPECT_EQ(fabricInfo->GetNodeId(), 55u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 2222u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

        Crypto::P256PublicKey rootPublicKeyOfFabric;
        EXPECT_EQ(fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
        EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));
    }
    size_t numStorageKeysAfterSecondAdd = storage.GetNumKeys();

    EXPECT_EQ(numStorageKeysAfterSecondAdd, (numStorageKeysAfterFirstAdd + 5)); // Metadata, 3 certs, 1 opkey
}

TEST_F(TestFabricTable, TestInvalidChaining)
{
    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority;
    Credentials::TestOnlyLocalCertificateAuthority differentCertAuthority;

    chip::TestPersistentStorageDelegate storage;
    EXPECT_TRUE(fabricCertAuthority.Init().IsSuccess());
    EXPECT_TRUE(differentCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    EXPECT_EQ(fabricTable.FabricCount(), 0);

    // Try to add fabric with either the NOC not chaining properly, or ICAC not chaining properly, fail,
    // then succeed with proper chaining
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

        // Generate same cert chain from two different roots
        EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(differentCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                  CHIP_NO_ERROR);

        ByteSpan rcac = fabricCertAuthority.GetRcac();
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        ByteSpan otherIcac = differentCertAuthority.GetIcac();
        ByteSpan otherNoc  = differentCertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.FabricCount(), 0);
        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);

        // Add with NOC not chaining to ICAC: fail
        {
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            CHIP_ERROR err = fabricTable.AddNewPendingFabricWithOperationalKeystore(otherNoc, icac, kVendorId, &newFabricIndex);
            EXPECT_NE(err, CHIP_NO_ERROR);
            EXPECT_EQ(fabricTable.FabricCount(), 0);
        }

        // Add with ICAC not chaining to root: fail
        {
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            CHIP_ERROR err = fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, otherIcac, kVendorId, &newFabricIndex);
            EXPECT_NE(err, CHIP_NO_ERROR);
            EXPECT_EQ(fabricTable.FabricCount(), 0);
        }

        // Add with NOC and ICAC chaining together, but not to root: fail
        {
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            CHIP_ERROR err =
                fabricTable.AddNewPendingFabricWithOperationalKeystore(otherNoc, otherIcac, kVendorId, &newFabricIndex);
            EXPECT_NE(err, CHIP_NO_ERROR);
            EXPECT_EQ(fabricTable.FabricCount(), 0);
        }

        // Revert state, start tests without ICAC
        fabricTable.RevertPendingFabricData();

        // Generate same cert chain from two different roots

        csrSpan = MutableByteSpan{ csrBuf };
        EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);
        EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(differentCertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                  CHIP_NO_ERROR);

        rcac = fabricCertAuthority.GetRcac();
        noc  = fabricCertAuthority.GetNoc();

        otherNoc = differentCertAuthority.GetNoc();

        EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);

        // Add with NOC not chaining to RCAC: fail
        {
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            CHIP_ERROR err =
                fabricTable.AddNewPendingFabricWithOperationalKeystore(otherNoc, ByteSpan{}, kVendorId, &newFabricIndex);
            EXPECT_NE(err, CHIP_NO_ERROR);
            EXPECT_EQ(fabricTable.FabricCount(), 0);
        }

        // Add properly now
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, ByteSpan{}, kVendorId, &newFabricIndex),
                  CHIP_NO_ERROR);
        EXPECT_EQ(fabricTable.FabricCount(), 1);
        EXPECT_EQ(newFabricIndex, 1);

        EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        ASSERT_NE(fabricInfo, nullptr);
        EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
        EXPECT_EQ(fabricInfo->GetNodeId(), 55u);
        EXPECT_EQ(fabricInfo->GetFabricId(), 1111u);
        EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
        EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);
    }
}

TEST_F(TestFabricTable, TestEphemeralKeys)
{
    // Initialize a fabric table with operational keystore
    {
        chip::TestPersistentStorageDelegate storage;

        // Initialize a FabricTable
        ScopedFabricTable fabricTableHolder;
        EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
        FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

        Crypto::P256ECDSASignature sig;
        uint8_t message[] = { 'm', 's', 'g' };

        Crypto::P256Keypair * ephemeralKeypair = fabricTable.AllocateEphemeralKeypairForCASE();
        ASSERT_NE(ephemeralKeypair, nullptr);
        EXPECT_EQ(ephemeralKeypair->Initialize(Crypto::ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

        EXPECT_EQ(ephemeralKeypair->ECDSA_sign_msg(message, sizeof(message), sig), CHIP_NO_ERROR);
        EXPECT_EQ(ephemeralKeypair->Pubkey().ECDSA_validate_msg_signature(message, sizeof(message), sig), CHIP_NO_ERROR);

        fabricTable.ReleaseEphemeralKeypair(ephemeralKeypair);
    }

    // Use a fabric table without an operational keystore: should still work
    {
        chip::TestPersistentStorageDelegate storage;

        chip::Credentials::PersistentStorageOpCertStore opCertStore;
        EXPECT_EQ(opCertStore.Init(&storage), CHIP_NO_ERROR);

        FabricTable fabricTable;
        FabricTable::InitParams initParams;
        initParams.storage     = &storage;
        initParams.opCertStore = &opCertStore;

        EXPECT_EQ(fabricTable.Init(initParams), CHIP_NO_ERROR);

        Crypto::P256ECDSASignature sig;
        uint8_t message[] = { 'm', 's', 'g' };

        Crypto::P256Keypair * ephemeralKeypair = fabricTable.AllocateEphemeralKeypairForCASE();
        ASSERT_NE(ephemeralKeypair, nullptr);
        EXPECT_EQ(ephemeralKeypair->Initialize(Crypto::ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

        EXPECT_EQ(ephemeralKeypair->ECDSA_sign_msg(message, sizeof(message), sig), CHIP_NO_ERROR);
        EXPECT_EQ(ephemeralKeypair->Pubkey().ECDSA_validate_msg_signature(message, sizeof(message), sig), CHIP_NO_ERROR);

        fabricTable.ReleaseEphemeralKeypair(ephemeralKeypair);

        fabricTable.Shutdown();
        opCertStore.Finish();
    }
}

TEST_F(TestFabricTable, TestCommitMarker)
{
    Crypto::P256PublicKey fIdx1PublicKey;
    Crypto::P256PublicKey fIdx2PublicKey;

    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority;

    chip::TestPersistentStorageDelegate storage;

    // Log verbosity on this test helps debug significantly
    storage.SetLoggingLevel(chip::TestPersistentStorageDelegate::LoggingLevel::kLogMutationAndReads);

    EXPECT_TRUE(fabricCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    size_t numStorageKeysAfterFirstAdd = 0;

    // First scope: add 2 fabrics with same root:
    //   - FabricID 1111, Node ID 55
    //   - FabricID 2222, Node ID 66
    //      - Abort commit on second fabric
    {
        // Initialize a fabric table
        ScopedFabricTable fabricTableHolder;
        EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
        FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

        EXPECT_EQ(fabricTable.GetDeletedFabricFromCommitMarker(), kUndefinedFabricIndex);
        EXPECT_EQ(fabricTable.FabricCount(), 0);

        // Add Fabric 1111 Node Id 55
        {
            FabricId fabricId = 1111;
            NodeId nodeId     = 55;

            uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
            MutableByteSpan csrSpan{ csrBuf };
            EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

            EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                      CHIP_NO_ERROR);
            ByteSpan rcac = fabricCertAuthority.GetRcac();
            ByteSpan icac = fabricCertAuthority.GetIcac();
            ByteSpan noc  = fabricCertAuthority.GetNoc();

            EXPECT_EQ(fabricTable.FabricCount(), 0);
            EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(fabricTable.FabricCount(), 1);
            EXPECT_EQ(newFabricIndex, 1);

            EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_NO_ERROR);

            // Validate contents
            const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
            ASSERT_NE(fabricInfo, nullptr);
            Credentials::ChipCertificateSet certificates;
            EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
            EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
            Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

            EXPECT_EQ(fabricInfo->GetFabricIndex(), 1);
            EXPECT_EQ(fabricInfo->GetNodeId(), 55u);
            EXPECT_EQ(fabricInfo->GetFabricId(), 1111u);
            EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
            EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            EXPECT_EQ(fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
            EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

            // Validate that fabric has the correct operational key by verifying a signature
            {
                Crypto::P256ECDSASignature sig;
                uint8_t message[] = { 'm', 's', 'g' };

                EXPECT_EQ(VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), fIdx1PublicKey), CHIP_NO_ERROR);

                EXPECT_EQ(fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig), CHIP_NO_ERROR);
                EXPECT_EQ(fIdx1PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig), CHIP_NO_ERROR);
            }
        }
        numStorageKeysAfterFirstAdd = storage.GetNumKeys();

        EXPECT_EQ(numStorageKeysAfterFirstAdd, 7u); // Metadata, index, 3 certs, 1 opkey, last known good time

        // The following test requires test methods not available on all builds.
        // TODO: Debug why some CI jobs don't set it properly.
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST

        // Add Fabric 2222 Node Id 66, no ICAC *** AND ABORT COMMIT ***
        {
            FabricId fabricId = 2222;
            NodeId nodeId     = 66;

            uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
            MutableByteSpan csrSpan{ csrBuf };
            EXPECT_EQ(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan), CHIP_NO_ERROR);

            EXPECT_EQ(fabricCertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus(),
                      CHIP_NO_ERROR);
            ByteSpan rcac = fabricCertAuthority.GetRcac();
            ByteSpan noc  = fabricCertAuthority.GetNoc();

            EXPECT_EQ(fabricTable.FabricCount(), 1);
            EXPECT_EQ(fabricTable.AddNewPendingTrustedRootCert(rcac), CHIP_NO_ERROR);
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            EXPECT_EQ(fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, ByteSpan{}, kVendorId, &newFabricIndex),
                      CHIP_NO_ERROR);
            EXPECT_EQ(fabricTable.FabricCount(), 2);
            EXPECT_EQ(newFabricIndex, 2);

            // Validate contents of pending
            const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
            ASSERT_NE(fabricInfo, nullptr);
            Credentials::ChipCertificateSet certificates;
            EXPECT_EQ(certificates.Init(1), CHIP_NO_ERROR);
            EXPECT_EQ(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)), CHIP_NO_ERROR);
            Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

            EXPECT_EQ(fabricInfo->GetFabricIndex(), 2);
            EXPECT_EQ(fabricInfo->GetNodeId(), 66u);
            EXPECT_EQ(fabricInfo->GetFabricId(), 2222u);
            EXPECT_EQ(fabricInfo->GetVendorId(), kVendorId);
            EXPECT_EQ(fabricInfo->GetFabricLabel().size(), 0u);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            EXPECT_EQ(fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric), CHIP_NO_ERROR);
            EXPECT_TRUE(rootPublicKeyOfFabric.Matches(rcacPublicKey));

            // Make sure no additional storage yet
            EXPECT_EQ(storage.GetNumKeys(), numStorageKeysAfterFirstAdd);

            // --> FORCE AN ERROR ON COMMIT that will BYPASS commit clean-up (similar to reboot during commit)
            fabricTable.SetForceAbortCommitForTest(true);
            EXPECT_EQ(fabricTable.CommitPendingFabricData(), CHIP_ERROR_INTERNAL);

            // Check that there are more keys now, partially committed: at least a Commit Marker (+1)
            // and some more keys from the aborted process.
            EXPECT_GT(storage.GetNumKeys(), (numStorageKeysAfterFirstAdd + 1));
        }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
    }

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    {
        storage.DumpKeys();

        // Initialize a FabricTable again. Make sure it succeeds in initing.
        ScopedFabricTable fabricTableHolder;

        EXPECT_GT(storage.GetNumKeys(), (numStorageKeysAfterFirstAdd + 1));

        EXPECT_EQ(fabricTableHolder.Init(&storage), CHIP_NO_ERROR);
        FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

        // Make sure that after init, the fabricTable has only 1 fabric
        EXPECT_EQ(fabricTable.FabricCount(), 1);

        // Make sure it caught the last partially committed fabric
        EXPECT_EQ(fabricTable.GetDeletedFabricFromCommitMarker(), 2);

        // Second read must return kUndefinedFabricIndex
        EXPECT_EQ(fabricTable.GetDeletedFabricFromCommitMarker(), kUndefinedFabricIndex);

        {
            // Here we would do other clean-ups (e.g. see Server.cpp that uses the above) and then
            // clear the commit marker after.
            fabricTable.ClearCommitMarker();
        }

        // Make sure that all other pending storage got deleted
        EXPECT_EQ(storage.GetNumKeys(), numStorageKeysAfterFirstAdd);

        // Verify we can only see 1 fabric with the iterator
        {
            size_t numFabricsIterated = 0;
            bool saw1                 = false;
            bool saw2                 = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    EXPECT_EQ(iterFabricInfo.GetNodeId(), 55u);
                    EXPECT_EQ(iterFabricInfo.GetFabricId(), 1111u);
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    saw2 = true;
                }
            }

            EXPECT_EQ(numFabricsIterated, 1u);
            EXPECT_TRUE(saw1);
            EXPECT_FALSE(saw2);
        }
    }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
}

} // namespace
