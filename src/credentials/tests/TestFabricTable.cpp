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
#include <nlunit-test.h>

#include <lib/core/CHIPCore.h>

#include <credentials/FabricTable.h>

#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/TestOnlyLocalCertificateAuthority.h>
#include <credentials/tests/CHIPCert_test_vectors.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/asn1/ASN1.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestExtendedAssertions.h>
#include <lib/support/UnitTestRegistration.h>
#include <platform/ConfigurationManager.h>

#include <lib/support/BytesToHex.h>

#include <stdarg.h>

using namespace chip;
using namespace chip::Credentials;

namespace {

Crypto::P256Keypair gFabric1OpKey;

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
static CHIP_ERROR LoadTestFabric(nlTestSuite * inSuite, FabricTable & fabricTable, bool doCommit)
{
    Crypto::P256SerializedKeypair opKeysSerialized;
    Crypto::P256Keypair opKey;
    FabricInfo fabricInfo;
    FabricIndex fabricIndex;
    memcpy((uint8_t *) (opKeysSerialized), TestCerts::sTestCert_Node01_01_PublicKey, TestCerts::sTestCert_Node01_01_PublicKey_Len);
    memcpy((uint8_t *) (opKeysSerialized) + TestCerts::sTestCert_Node01_01_PublicKey_Len, TestCerts::sTestCert_Node01_01_PrivateKey,
           TestCerts::sTestCert_Node01_01_PrivateKey_Len);

    ByteSpan rcacSpan(TestCerts::sTestCert_Root01_Chip, TestCerts::sTestCert_Root01_Chip_Len);
    ByteSpan icacSpan(TestCerts::sTestCert_ICA01_Chip, TestCerts::sTestCert_ICA01_Chip_Len);
    ByteSpan nocSpan(TestCerts::sTestCert_Node01_01_Chip, TestCerts::sTestCert_Node01_01_Chip_Len);

    NL_TEST_ASSERT(inSuite,
                   opKeysSerialized.SetLength(TestCerts::sTestCert_Node01_02_PublicKey_Len +
                                              TestCerts::sTestCert_Node01_02_PrivateKey_Len) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gFabric1OpKey.Deserialize(opKeysSerialized) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcacSpan) == CHIP_NO_ERROR);

    CHIP_ERROR err = fabricTable.AddNewPendingFabricWithProvidedOpKey(nocSpan, icacSpan, VendorId::TestVendor1, &gFabric1OpKey,
                                                                      /*isExistingOpKeyExternallyOwned =*/true, &fabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    if (doCommit)
    {
        err = fabricTable.CommitPendingFabricData();
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    return err;
}

void TestLastKnownGoodTimeInit(nlTestSuite * inSuite, void * inContext)
{
    // Fabric table init should init Last Known Good Time to the firmware build time.
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;

    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
    System::Clock::Seconds32 lastKnownGoodChipEpochTime;

    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();
    NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime) == CHIP_NO_ERROR);
    System::Clock::Seconds32 firmwareBuildTime;
    NL_TEST_ASSERT(inSuite, DeviceLayer::ConfigurationMgr().GetFirmwareBuildChipEpochTime(firmwareBuildTime) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, lastKnownGoodChipEpochTime == firmwareBuildTime);
}

void TestUpdateLastKnownGoodTime(nlTestSuite * inSuite, void * inContext)
{
    // Adding a fabric should advance Last Known Good Time if any certificate's
    // NotBefore time is later than the build time, and else should leave it
    // to the initial build time value.

    // Test certs all have this NotBefore: Oct 15 14:23:43 2020 GMT
    const ASN1::ASN1UniversalTime asn1Expected = { 2020, 10, 15, 14, 23, 43 };
    uint32_t testCertNotBeforeSeconds;
    NL_TEST_ASSERT(inSuite, Credentials::ASN1ToChipEpochTime(asn1Expected, testCertNotBeforeSeconds) == CHIP_NO_ERROR);
    System::Clock::Seconds32 testCertNotBeforeTime = System::Clock::Seconds32(testCertNotBeforeSeconds);

    // Test that certificate NotBefore times that are before the Firmware build time
    // do not advance Last Known Good Time.
    System::Clock::Seconds32 afterNotBeforeBuildTimes[] = { System::Clock::Seconds32(testCertNotBeforeTime.count() + 1),
                                                            System::Clock::Seconds32(testCertNotBeforeTime.count() + 1000),
                                                            System::Clock::Seconds32(testCertNotBeforeTime.count() + 1000000) };
    for (auto buildTime : afterNotBeforeBuildTimes)
    {
        // Set build time to the desired value.
        NL_TEST_ASSERT(inSuite, DeviceLayer::ConfigurationMgr().SetFirmwareBuildChipEpochTime(buildTime) == CHIP_NO_ERROR);
        chip::TestPersistentStorageDelegate testStorage;

        {
            // Initialize a fabric table.
            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Read back Last Known Good Time, which will have been initialized to firmware build time.
            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);

            // Load a test fabric, but do not commit.
            NL_TEST_ASSERT(inSuite, LoadTestFabric(inSuite, fabricTable, /* doCommit = */ false) == CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it hasn't moved forward.
            // This test case was written after the test certs' NotBefore time and we
            // are using a configuration manager that should reflect a real build time.
            // Therefore, we expect that build time is after NotBefore and so Last
            // Known Good Time will be set to the later of these, build time, even
            // after installing the new fabric.
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);

            // Verify that calling the fail-safe roll back interface does change
            // last known good time, as it hadn't been updated in the first place.
            fabricTable.RevertPendingFabricData();
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);

            // Now reload the test fabric and commit this time.
            NL_TEST_ASSERT(inSuite, LoadTestFabric(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it hasn't moved forward.
            // This test case was written after the test certs' NotBefore time and we
            // are using a configuration manager that should reflect a real build time.
            // Therefore, we expect that build time is after NotBefore and so Last
            // Known Good Time will be set to the later of these, build time, even
            // after installing the new fabric.
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);

            // Call revert again.  Since we've committed, this is a no-op.
            // Last known good time should again be unchanged.
            fabricTable.RevertPendingFabricData();
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);
        }
        {
            // Test reloading last known good time from persistence.
            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Verify that last known good time was retained.
            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);
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
        NL_TEST_ASSERT(inSuite, DeviceLayer::ConfigurationMgr().SetFirmwareBuildChipEpochTime(buildTime) == CHIP_NO_ERROR);
        chip::TestPersistentStorageDelegate testStorage;
        {
            // Initialize a fabric table.
            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Load a test fabric, but do not commit.
            NL_TEST_ASSERT(inSuite, LoadTestFabric(inSuite, fabricTable, /* doCommit = */ false) == CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it is now set to the certificate
            // NotBefore time, as this should be at or after firmware build time.
            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == testCertNotBeforeTime);

            // Now test revert.  Last known good time should change back to the
            // previous value.
            fabricTable.RevertPendingFabricData();
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);
        }
        {
            // Test reloading last known good time from persistence.
            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Verify that the original last known good time was retained, since
            // we reverted before.
            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
        }
        {
            // Now test loading a fabric and committing.
            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();
            NL_TEST_ASSERT(inSuite, LoadTestFabric(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it is now set to the certificate
            // NotBefore time, as this should be at or after firmware build time.
            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == testCertNotBeforeTime);

            // Now test revert, which will be a no-op because we already
            // committed.  Verify that Last Known Good time is retained.
            fabricTable.RevertPendingFabricData();
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == testCertNotBeforeTime);
        }
        {
            // Test reloading last known good time from persistence.
            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Verify that the new last known good time was retained, since
            // we committed.
            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == testCertNotBeforeTime);
        }
    }
}

void TestSetLastKnownGoodTime(nlTestSuite * inSuite, void * inContext)
{
    // It is desirable for nodes to set Last Known Good Time whenever a good
    // time source is available, including cases where this would set the time
    // backward.  However, it is impermissible to set last known good time to
    // any time before the Firmware Build time or the latest NotBefore of any
    // installed certificate.

    // Test certs all have this NotBefore: Oct 15 14:23:43 2020 GMT
    const ASN1::ASN1UniversalTime asn1Expected = { 2020, 10, 15, 14, 23, 43 };
    uint32_t testCertNotBeforeSeconds;
    NL_TEST_ASSERT(inSuite, Credentials::ASN1ToChipEpochTime(asn1Expected, testCertNotBeforeSeconds) == CHIP_NO_ERROR);
    System::Clock::Seconds32 testCertNotBeforeTime = System::Clock::Seconds32(testCertNotBeforeSeconds);

    // Iterate over two cases: one with build time prior to our certificates' NotBefore, one with build time after.
    System::Clock::Seconds32 testCaseFirmwareBuildTimes[] = { System::Clock::Seconds32(testCertNotBeforeTime.count() - 100000),
                                                              System::Clock::Seconds32(testCertNotBeforeTime.count() + 100000) };

    for (auto buildTime : testCaseFirmwareBuildTimes)
    {
        // Set build time to the desired value.
        NL_TEST_ASSERT(inSuite, DeviceLayer::ConfigurationMgr().SetFirmwareBuildChipEpochTime(buildTime) == CHIP_NO_ERROR);
        chip::TestPersistentStorageDelegate testStorage;
        System::Clock::Seconds32 newTime;
        {
            // Initialize a fabric table.
            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Load a test fabric
            NL_TEST_ASSERT(inSuite, LoadTestFabric(inSuite, fabricTable, /* doCommit= */ true) == CHIP_NO_ERROR);

            // Verify the Last Known Good Time matches our expected initial value.
            System::Clock::Seconds32 initialLastKnownGoodTime =
                buildTime > testCertNotBeforeTime ? buildTime : testCertNotBeforeTime;
            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == initialLastKnownGoodTime);

            // Read Last Known Good Time and verify that it hasn't moved forward, since
            // build time is later than the test certs' NotBefore times.
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == initialLastKnownGoodTime);

            // Attempt to set a Last Known Good Time that is before the firmware build time.  This should fail.
            newTime = System::Clock::Seconds32(buildTime.count() - 1000);
            NL_TEST_ASSERT(inSuite, fabricTable.SetLastKnownGoodChipEpochTime(newTime) != CHIP_NO_ERROR);

            // Verify Last Known Good Time is unchanged.
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == initialLastKnownGoodTime);

            // Attempt to set a Last Known Good Time that is before our certificates' NotBefore times.  This should fail.
            newTime = System::Clock::Seconds32(testCertNotBeforeTime.count() - 1000);
            NL_TEST_ASSERT(inSuite, fabricTable.SetLastKnownGoodChipEpochTime(newTime) != CHIP_NO_ERROR);

            // Verify Last Known Good Time is unchanged.
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == initialLastKnownGoodTime);

            // Attempt to set a Last Known Good Time that at our current value.
            NL_TEST_ASSERT(inSuite, fabricTable.SetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);

            // Verify Last Known Good Time is unchanged.
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == initialLastKnownGoodTime);

            // Attempt to set Last Known Good Times that is after our current value.
            newTime = System::Clock::Seconds32(initialLastKnownGoodTime.count() + 1000);
            NL_TEST_ASSERT(inSuite, fabricTable.SetLastKnownGoodChipEpochTime(newTime) == CHIP_NO_ERROR);

            // Verify Last Known Good Time is updated.
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == newTime);
        }
        {
            // Verify that Last Known Good Time was persisted.

            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == newTime);
        }
    }
}

// Test adding 2 fabrics, updating 1, removing 1
void TestBasicAddNocUpdateNocFlow(nlTestSuite * inSuite, void * inContext)
{
    Credentials::TestOnlyLocalCertificateAuthority fabric11CertAuthority;
    Credentials::TestOnlyLocalCertificateAuthority fabric44CertAuthority;

    chip::TestPersistentStorageDelegate storage;

    // Uncomment the next line for superior debugging powers if you blow-up this test
    // storage.SetLoggingLevel(chip::TestPersistentStorageDelegate::LoggingLevel::kLogMutation);

    // Initialize test CA and a Fabric 11 externally owned key
    NL_TEST_ASSERT(inSuite, fabric11CertAuthority.Init().IsSuccess());
    NL_TEST_ASSERT(inSuite, fabric44CertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    chip::Crypto::P256Keypair fabric11Node55Keypair; // Fabric ID 11,
    NL_TEST_ASSERT(inSuite, fabric11Node55Keypair.Initialize() == CHIP_NO_ERROR);

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);

    size_t numFabricsIterated = 0;

    size_t numStorageKeysAtStart = storage.GetNumKeys();

    // Sequence 1: Add node ID 55 on fabric 11, using externally owned key and no ICAC --> Yield fabricIndex 1
    {
        FabricId fabricId = 11;
        NodeId nodeId     = 55;
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabric11CertAuthority.SetIncludeIcac(false)
                                   .GenerateNocChain(fabricId, nodeId, fabric11Node55Keypair.Pubkey())
                                   .GetStatus());
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

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 0);
            NL_TEST_ASSERT(inSuite, saw1 == false);
        }

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        bool keyIsExternallyOwned  = true;

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithProvidedOpKey(noc, ByteSpan{}, kVendorId, &fabric11Node55Keypair,
                                                                                keyIsExternallyOwned, &newFabricIndex));
        NL_TEST_ASSERT(inSuite, newFabricIndex == 1);
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);

        // No storage yet
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageKeysAtStart);

        // Validate iterator sees pending
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == nodeId);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == fabricId);
                    saw1 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 1);
            NL_TEST_ASSERT(inSuite, saw1 == true);
        }

        // Commit, now storage should have keys
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());

        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == (numStorageKeysAtStart + 4)); // 2 opcerts + fabric metadata + index

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
        if (fabricInfo != nullptr)
        {
            Credentials::ChipCertificateSet certificates;
            NL_TEST_ASSERT_SUCCESS(inSuite, certificates.Init(1));
            NL_TEST_ASSERT_SUCCESS(inSuite,
                                   certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));
            Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == newFabricIndex);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == nodeId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == fabricId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric));
            NL_TEST_ASSERT(inSuite, rootPublicKeyOfFabric.Matches(rcacPublicKey));
        }

        // Validate that fabric has the correct operational key by verifying a signature
        Crypto::P256ECDSASignature sig;
        uint8_t message[] = { 'm', 's', 'g' };
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig));
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabric11Node55Keypair.Pubkey().ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));

        // Validate iterator sees committed
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == nodeId);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == fabricId);
                    saw1 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 1);
            NL_TEST_ASSERT(inSuite, saw1 == true);
        }
    }

    size_t numStorageAfterFirstAdd = storage.GetNumKeys();

    // Sequence 2: Add node ID 999 on fabric 44, using operational keystore and ICAC --> Yield fabricIndex 2
    {
        FabricId fabricId = 44;
        NodeId nodeId     = 999;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabric44CertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan rcac = fabric44CertAuthority.GetRcac();
        ByteSpan icac = fabric44CertAuthority.GetIcac();
        ByteSpan noc  = fabric44CertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        FabricIndex newFabricIndex = kUndefinedFabricIndex;

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);
        NL_TEST_ASSERT(inSuite, newFabricIndex == 2);
        // No storage yet
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageAfterFirstAdd);

        // Commit, now storage should have keys
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);

        NL_TEST_ASSERT_EQUALS(inSuite, storage.GetNumKeys(),
                              (numStorageAfterFirstAdd + 5)); // 3 opcerts + fabric metadata + 1 operational key

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
        if (fabricInfo != nullptr)
        {
            Credentials::ChipCertificateSet certificates;
            NL_TEST_ASSERT_SUCCESS(inSuite, certificates.Init(1));
            NL_TEST_ASSERT_SUCCESS(inSuite,
                                   certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));
            Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == newFabricIndex);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == nodeId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == fabricId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric));
            NL_TEST_ASSERT(inSuite, rootPublicKeyOfFabric.Matches(rcacPublicKey));
        }

        // Validate that fabric has the correct operational key by verifying a signature
        {
            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            Crypto::P256PublicKey nocPubKey;
            NL_TEST_ASSERT_SUCCESS(inSuite, VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), nocPubKey));

            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig));
            NL_TEST_ASSERT_SUCCESS(inSuite, nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
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
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 55);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 11);
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 999);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 44);
                    saw2 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 2);
            NL_TEST_ASSERT(inSuite, saw1 == true);
            NL_TEST_ASSERT(inSuite, saw2 == true);
        }
    }

    size_t numStorageAfterSecondAdd = storage.GetNumKeys();

    // Sequence 3: Update node ID 999 to 1000 on fabric 44, using operational keystore and no ICAC --> Stays fabricIndex 2
    {
        FabricId fabricId       = 44;
        NodeId nodeId           = 1000;
        FabricIndex fabricIndex = 2;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };

        // Make sure to tag fabric index to pending opkey: otherwise the UpdateNOC fails
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AllocatePendingOperationalKey(chip::MakeOptional(static_cast<FabricIndex>(2)), csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabric44CertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan rcac = fabric44CertAuthority.GetRcac();
        ByteSpan noc  = fabric44CertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.UpdatePendingFabricWithOperationalKeystore(2, noc, ByteSpan{}));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);

        // No storage yet
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageAfterSecondAdd);

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
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 55);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 11);
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 1000);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 44);
                    saw2 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 2);
            NL_TEST_ASSERT(inSuite, saw1 == true);
            NL_TEST_ASSERT(inSuite, saw2 == true);
        }

        // Commit, now storage should have keys
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);

        NL_TEST_ASSERT_EQUALS(inSuite, storage.GetNumKeys(), (numStorageAfterSecondAdd - 1)); // ICAC got deleted

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
        if (fabricInfo != nullptr)
        {
            Credentials::ChipCertificateSet certificates;
            NL_TEST_ASSERT_SUCCESS(inSuite, certificates.Init(1));
            NL_TEST_ASSERT_SUCCESS(inSuite,
                                   certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));
            Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == fabricIndex);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == nodeId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == fabricId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(fabricIndex, rootPublicKeyOfFabric));
            NL_TEST_ASSERT(inSuite, rootPublicKeyOfFabric.Matches(rcacPublicKey));
        }

        // Validate that fabric has the correct operational key by verifying a signature
        {
            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            Crypto::P256PublicKey nocPubKey;
            NL_TEST_ASSERT_SUCCESS(inSuite, VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), nocPubKey));

            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(fabricIndex, ByteSpan{ message }, sig));
            NL_TEST_ASSERT_SUCCESS(inSuite, nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
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
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 55);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 11);
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 1000);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 44);
                    saw2 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 2);
            NL_TEST_ASSERT(inSuite, saw1 == true);
            NL_TEST_ASSERT(inSuite, saw2 == true);
        }
    }

    size_t numStorageAfterUpdate = storage.GetNumKeys();

    // Sequence 4: Rename fabric index 2, applies immediately when nothing pending
    {
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SetFabricLabel(2, CharSpan("roboto")));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);

        NL_TEST_ASSERT_EQUALS(inSuite, storage.GetNumKeys(), numStorageAfterUpdate); // Number of keys unchanged

        // Validate basic contents
        {
            const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
            NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
            if (fabricInfo != nullptr)
            {
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 2);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 1000);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 44);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().data_equal(CharSpan{ "roboto", strlen("roboto") }));
            }
        }
    }

    // Sequence 5: Remove FabricIndex 1 (FabricId 11, NodeId 55), make sure FabricIndex 2 (FabricId 44, NodeId 1000) still exists
    {
        // Remove the fabric: no commit needed
        {
            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.Delete(1));
            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);

            NL_TEST_ASSERT_EQUALS(inSuite, storage.GetNumKeys(), (numStorageAfterUpdate - 3)); // Deleted NOC, RCAC, Metadata
        }

        // Validate contents of Fabric Index 2 is still OK
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
        if (fabricInfo != nullptr)
        {
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 2);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 1000);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 44);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().data_equal(CharSpan{ "roboto", strlen("roboto") }));

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(2, rootPublicKeyOfFabric));
        }

        // Validate that fabric has the correct operational key by verifying a signature
        {
            uint8_t nocBuf[Credentials::kMaxCHIPCertLength];
            MutableByteSpan nocSpan{ nocBuf };
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchNOCCert(2, nocSpan));

            Credentials::ChipCertificateSet certificates;
            NL_TEST_ASSERT_SUCCESS(inSuite, certificates.Init(1));
            NL_TEST_ASSERT_SUCCESS(inSuite,
                                   certificates.LoadCert(nocSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));
            Crypto::P256PublicKey nocPubKey(certificates.GetCertSet()[0].mPublicKey);

            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(2, ByteSpan{ message }, sig));
            NL_TEST_ASSERT_SUCCESS(inSuite, nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
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
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 1000);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 44);
                    saw2 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 1);
            NL_TEST_ASSERT(inSuite, saw1 == false);
            NL_TEST_ASSERT(inSuite, saw2 == true);
        }
    }
}

void TestAddMultipleSameRootDifferentFabricId(nlTestSuite * inSuite, void * inContext)
{
    // TODO: Write test
}

void TestPersistence(nlTestSuite * inSuite, void * inContext)
{
    // TODO: Write test
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
}

void TestAddNocFailSafe(nlTestSuite * inSuite, void * inContext)
{
    // TODO: Write test
}

void TestUpdateNocFailSafe(nlTestSuite * inSuite, void * inContext)
{
    // TODO: Write test
}

void TestSequenceErrors(nlTestSuite * inSuite, void * inContext)
{
    // TODO: Write test
}

void TestFabricLabelChange(nlTestSuite * inSuite, void * inContext)
{
    // TODO: Write test
}

void TestCompressedFabricId(nlTestSuite * inSuite, void * inContext)
{
    // TODO: Write test
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Last Known Good Time Init", TestLastKnownGoodTimeInit),
    NL_TEST_DEF("Update Last Known Good Time", TestUpdateLastKnownGoodTime),
    NL_TEST_DEF("Set Last Known Good Time", TestSetLastKnownGoodTime),
    NL_TEST_DEF("Test basic AddNOC flow", TestBasicAddNocUpdateNocFlow),
    NL_TEST_DEF("Test adding multiple fabrics that chain to same root, different fabric ID", TestAddMultipleSameRootDifferentFabricId),
    NL_TEST_DEF("Validate fabrics are loaded from persistence at FabricTable::init", TestPersistence),
    NL_TEST_DEF("Test fail-safe handling during AddNOC", TestAddNocFailSafe),
    NL_TEST_DEF("Test fail-safe handling during UpdateNoc", TestUpdateNocFailSafe),
    NL_TEST_DEF("Test interlock sequencing errors", TestSequenceErrors),
    NL_TEST_DEF("Test fabric label changes", TestFabricLabelChange),
    NL_TEST_DEF("Test compressed fabric ID is properly generated", TestCompressedFabricId),

    NL_TEST_SENTINEL()
};

// clang-format on

int TestFabricTable_Setup(void * inContext);
int TestFabricTable_Teardown(void * inContext);

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-FabricTable",
    &sTests[0],
    TestFabricTable_Setup,
    TestFabricTable_Teardown,
};
// clang-format on

/**
 *  Set up the test suite.
 */
int TestFabricTable_Setup(void * inContext)
{
    DeviceLayer::SetConfigurationMgr(&DeviceLayer::ConfigurationManagerImpl::GetDefaultInstance());
    return chip::Platform::MemoryInit() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
}

/**
 *  Tear down the test suite.
 */
int TestFabricTable_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

/**
 *  Main
 */
int TestFabricTable()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestFabricTable)
