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
static CHIP_ERROR LoadTestFabric_Node01_01(nlTestSuite * inSuite, FabricTable & fabricTable, bool doCommit)
{
    Crypto::P256SerializedKeypair opKeysSerialized;
    static Crypto::P256Keypair opKey_Node01_01;

    FabricIndex fabricIndex;
    memcpy((uint8_t *) (opKeysSerialized), TestCerts::sTestCert_Node01_01_PublicKey, TestCerts::sTestCert_Node01_01_PublicKey_Len);
    memcpy((uint8_t *) (opKeysSerialized) + TestCerts::sTestCert_Node01_01_PublicKey_Len, TestCerts::sTestCert_Node01_01_PrivateKey,
           TestCerts::sTestCert_Node01_01_PrivateKey_Len);

    ByteSpan rcacSpan(TestCerts::sTestCert_Root01_Chip, TestCerts::sTestCert_Root01_Chip_Len);
    ByteSpan icacSpan(TestCerts::sTestCert_ICA01_Chip, TestCerts::sTestCert_ICA01_Chip_Len);
    ByteSpan nocSpan(TestCerts::sTestCert_Node01_01_Chip, TestCerts::sTestCert_Node01_01_Chip_Len);

    ReturnErrorOnFailure(
        opKeysSerialized.SetLength(TestCerts::sTestCert_Node01_01_PublicKey_Len + TestCerts::sTestCert_Node01_01_PrivateKey_Len));
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

static CHIP_ERROR LoadTestFabric_Node01_02(nlTestSuite * inSuite, FabricTable & fabricTable, bool doCommit)
{
    Crypto::P256SerializedKeypair opKeysSerialized;
    FabricIndex fabricIndex;
    static Crypto::P256Keypair opKey_Node01_02;

    memcpy((uint8_t *) (opKeysSerialized), TestCerts::sTestCert_Node01_02_PublicKey, TestCerts::sTestCert_Node01_02_PublicKey_Len);
    memcpy((uint8_t *) (opKeysSerialized) + TestCerts::sTestCert_Node01_02_PublicKey_Len, TestCerts::sTestCert_Node01_02_PrivateKey,
           TestCerts::sTestCert_Node01_02_PrivateKey_Len);

    ByteSpan rcacSpan(TestCerts::sTestCert_Root01_Chip, TestCerts::sTestCert_Root01_Chip_Len);
    ByteSpan nocSpan(TestCerts::sTestCert_Node01_02_Chip, TestCerts::sTestCert_Node01_02_Chip_Len);

    ReturnErrorOnFailure(
        opKeysSerialized.SetLength(TestCerts::sTestCert_Node01_02_PublicKey_Len + TestCerts::sTestCert_Node01_02_PrivateKey_Len));
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
static CHIP_ERROR LoadTestFabric_Node02_01(nlTestSuite * inSuite, FabricTable & fabricTable, bool doCommit)
{
    Crypto::P256SerializedKeypair opKeysSerialized;
    FabricIndex fabricIndex;
    static Crypto::P256Keypair opKey_Node02_01;

    memcpy((uint8_t *) (opKeysSerialized), TestCerts::sTestCert_Node02_01_PublicKey, TestCerts::sTestCert_Node02_01_PublicKey_Len);
    memcpy((uint8_t *) (opKeysSerialized) + TestCerts::sTestCert_Node02_01_PublicKey_Len, TestCerts::sTestCert_Node02_01_PrivateKey,
           TestCerts::sTestCert_Node02_01_PrivateKey_Len);

    ByteSpan rcacSpan(TestCerts::sTestCert_Root02_Chip, TestCerts::sTestCert_Root02_Chip_Len);
    ByteSpan icacSpan(TestCerts::sTestCert_ICA02_Chip, TestCerts::sTestCert_ICA02_Chip_Len);
    ByteSpan nocSpan(TestCerts::sTestCert_Node02_01_Chip, TestCerts::sTestCert_Node02_01_Chip_Len);

    NL_TEST_ASSERT(inSuite,
                   opKeysSerialized.SetLength(TestCerts::sTestCert_Node02_01_PublicKey_Len +
                                              TestCerts::sTestCert_Node02_01_PrivateKey_Len) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opKey_Node02_01.Deserialize(opKeysSerialized) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcacSpan) == CHIP_NO_ERROR);

    CHIP_ERROR err = fabricTable.AddNewPendingFabricWithProvidedOpKey(nocSpan, icacSpan, VendorId::TestVendor1, &opKey_Node02_01,
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

void TestCollidingFabrics(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;
    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    //
    // Start by loading NOCs for two nodes on the same fabric. The second one should fail since the FabricTable by default
    // doesn't permit colliding fabrics.
    //
    NL_TEST_ASSERT(inSuite, LoadTestFabric_Node01_01(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, LoadTestFabric_Node01_02(inSuite, fabricTable, /* doCommit = */ true) != CHIP_NO_ERROR);

    //
    // Revert the partially added NOC from the last call, permit colliding fabrics in the FabricTable and try again.
    // This time, it should succeed
    //
    fabricTable.RevertPendingFabricData();
    fabricTable.PermitCollidingFabrics();
    NL_TEST_ASSERT(inSuite, LoadTestFabric_Node01_02(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);

    ByteSpan rcacSpan(TestCerts::sTestCert_Root01_Chip, TestCerts::sTestCert_Root01_Chip_Len);
    Credentials::P256PublicKeySpan rootPublicKeySpan;

    NL_TEST_ASSERT(inSuite, Credentials::ExtractPublicKeyFromChipCert(rcacSpan, rootPublicKeySpan) == CHIP_NO_ERROR);

    //
    // Ensure we can find both node identities in the FabricTable.
    //
    {
        chip::Platform::ScopedMemoryBuffer<uint8_t> nocBuf;
        ByteSpan origNocSpan(TestCerts::sTestCert_Node01_01_Chip, TestCerts::sTestCert_Node01_01_Chip_Len);
        NodeId nodeId;
        FabricId fabricId;

        NL_TEST_ASSERT(inSuite, ExtractNodeIdFabricIdFromOpCert(origNocSpan, &nodeId, &fabricId) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, fabricTable.FindIdentity(rootPublicKeySpan, fabricId, nodeId) != nullptr);
    }

    {
        chip::Platform::ScopedMemoryBuffer<uint8_t> nocBuf;
        ByteSpan origNocSpan(TestCerts::sTestCert_Node01_02_Chip, TestCerts::sTestCert_Node01_02_Chip_Len);
        NodeId nodeId;
        FabricId fabricId;

        NL_TEST_ASSERT(inSuite, ExtractNodeIdFabricIdFromOpCert(origNocSpan, &nodeId, &fabricId) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, fabricTable.FindIdentity(rootPublicKeySpan, fabricId, nodeId) != nullptr);
    }
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
            NL_TEST_ASSERT(inSuite, LoadTestFabric_Node01_01(inSuite, fabricTable, /* doCommit = */ false) == CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it hasn't moved forward.
            // This test case was written after the test certs' NotBefore time and we
            // are using a configuration manager that should reflect a real build time.
            // Therefore, we expect that build time is after NotBefore and so Last
            // Known Good Time will be set to the later of these, build time, even
            // after installing the new fabric.
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);

            // Verify that calling the fail-safe roll back interface does not change
            // last known good time, as it hadn't been updated in the first place.
            fabricTable.RevertPendingFabricData();
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);

            // Now reload the test fabric and commit this time.
            NL_TEST_ASSERT(inSuite, LoadTestFabric_Node01_01(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);

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
            NL_TEST_ASSERT(inSuite, LoadTestFabric_Node01_01(inSuite, fabricTable, /* doCommit = */ false) == CHIP_NO_ERROR);

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
            NL_TEST_ASSERT(inSuite, LoadTestFabric_Node01_01(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);

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
            NL_TEST_ASSERT(inSuite, LoadTestFabric_Node01_01(inSuite, fabricTable, /* doCommit= */ true) == CHIP_NO_ERROR);

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

        uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
        {
            // No pending root cert yet.
            MutableByteSpan fetchedSpan{ rcacBuf };
            NL_TEST_ASSERT(inSuite, fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan) == CHIP_ERROR_NOT_FOUND);
        }

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        {
            // Now have a pending root cert.
            MutableByteSpan fetchedSpan{ rcacBuf };
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan));
            NL_TEST_ASSERT(inSuite, fetchedSpan.data_equal(rcac));
        }

        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        bool keyIsExternallyOwned  = true;

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithProvidedOpKey(noc, ByteSpan{}, kVendorId, &fabric11Node55Keypair,
                                                                                keyIsExternallyOwned, &newFabricIndex));
        NL_TEST_ASSERT(inSuite, newFabricIndex == 1);
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        {
            // No more pending root cert; it's associated with a fabric now.
            MutableByteSpan fetchedSpan{ rcacBuf };
            NL_TEST_ASSERT(inSuite, fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan) == CHIP_ERROR_NOT_FOUND);
        }

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
    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority;

    chip::TestPersistentStorageDelegate storage;
    NL_TEST_ASSERT(inSuite, fabricCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);

    uint8_t rcac1Buf[kMaxCHIPCertLength];
    MutableByteSpan rcac1Span{ rcac1Buf };

    // First scope: add FabricID 1111, node ID 55
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan rcac = fabricCertAuthority.GetRcac();
        // Keep a copy for second scope check
        CopySpanToMutableSpan(rcac, rcac1Span);

        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT(inSuite, newFabricIndex == 1);

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
        if (fabricInfo != nullptr)
        {
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 1);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 55);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 1111);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);
        }
    }
    size_t numStorageKeysAfterFirstAdd = storage.GetNumKeys();
    NL_TEST_ASSERT(inSuite, numStorageKeysAfterFirstAdd == 7); // Metadata, index, 3 certs, 1 opkey, last known good time

    // Second scope: add FabricID 2222, node ID 66, same root as first
    {
        FabricId fabricId = 2222;
        NodeId nodeId     = 66;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan rcac2 = fabricCertAuthority.GetRcac();
        NL_TEST_ASSERT(inSuite, rcac2.data_equal(rcac1Span));

        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac2));
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);
        NL_TEST_ASSERT(inSuite, newFabricIndex == 2);

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
        if (fabricInfo != nullptr)
        {
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 2);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 66);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 2222);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);
        }
    }
    size_t numStorageKeysAfterSecondAdd = storage.GetNumKeys();
    NL_TEST_ASSERT(inSuite, numStorageKeysAfterSecondAdd == (numStorageKeysAfterFirstAdd + 5)); // Add 3 certs, 1 metadata, 1 opkey
}

void TestPersistence(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, fabricCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // First scope: add 2 fabrics with same root: (1111, 2222), commit them, keep track of public keys
    {
        // Initialize a FabricTable
        ScopedFabricTable fabricTableHolder;
        NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
        FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);

        // Add Fabric 1111 Node Id 55
        {
            FabricId fabricId = 1111;
            NodeId nodeId     = 55;

            uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
            MutableByteSpan csrSpan{ csrBuf };
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

            NL_TEST_ASSERT_SUCCESS(
                inSuite, fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
            ByteSpan rcac = fabricCertAuthority.GetRcac();
            ByteSpan icac = fabricCertAuthority.GetIcac();
            ByteSpan noc  = fabricCertAuthority.GetNoc();

            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            NL_TEST_ASSERT_SUCCESS(inSuite,
                                   fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex));
            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
            NL_TEST_ASSERT(inSuite, newFabricIndex == 1);

            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());

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

                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 1);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 55);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 1111);
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

                NL_TEST_ASSERT_SUCCESS(inSuite, VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), fIdx1PublicKey));

                NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig));
                NL_TEST_ASSERT_SUCCESS(inSuite, fIdx1PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
            }
        }

        // Add Fabric 2222 Node Id 66, no ICAC
        {
            FabricId fabricId = 2222;
            NodeId nodeId     = 66;

            uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
            MutableByteSpan csrSpan{ csrBuf };
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

            NL_TEST_ASSERT_SUCCESS(
                inSuite, fabricCertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
            ByteSpan rcac = fabricCertAuthority.GetRcac();
            ByteSpan noc  = fabricCertAuthority.GetNoc();

            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            NL_TEST_ASSERT_SUCCESS(
                inSuite, fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, ByteSpan{}, kVendorId, &newFabricIndex));
            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);
            NL_TEST_ASSERT(inSuite, newFabricIndex == 2);

            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());

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

                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 2);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 66);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 2222);
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

                NL_TEST_ASSERT_SUCCESS(inSuite, VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), fIdx2PublicKey));

                NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig));
                NL_TEST_ASSERT_SUCCESS(inSuite, fIdx2PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
            }
        }

        NL_TEST_ASSERT(inSuite, fabricTable.FabricCount() == 2);

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
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 55);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 1111);
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 66);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 2222);
                    saw2 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 2);
            NL_TEST_ASSERT(inSuite, saw1 == true);
            NL_TEST_ASSERT(inSuite, saw2 == true);
        }
    }

    // Global: Last known good time + fabric index = 2
    // Fabric 1111: Metadata, 1 opkey, RCAC/ICAC/NOC = 5
    // Fabric 2222: Metadata, 1 opkey, RCAC/NOC = 4
    NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == (2 + 5 + 4));

    // Second scope: Validate that a fresh FabricTable loads the previously committed fabrics on Init.
    {
        // Initialize a FabricTable
        ScopedFabricTable fabricTableHolder;
        NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
        FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);

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
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 55);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 1111);
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 66);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 2222);
                    saw2 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 2);
            NL_TEST_ASSERT(inSuite, saw1 == true);
            NL_TEST_ASSERT(inSuite, saw2 == true);
        }

        // Validate contents of Fabric 2222
        {
            uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
            MutableByteSpan rcacSpan{ rcacBuf };
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootCert(2, rcacSpan));

            const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
            NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
            if (fabricInfo != nullptr)
            {
                Credentials::ChipCertificateSet certificates;
                NL_TEST_ASSERT_SUCCESS(inSuite, certificates.Init(1));
                NL_TEST_ASSERT_SUCCESS(inSuite,
                                       certificates.LoadCert(rcacSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));
                Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 2);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 66);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 2222);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);

                Crypto::P256PublicKey rootPublicKeyOfFabric;
                NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(2, rootPublicKeyOfFabric));
                NL_TEST_ASSERT(inSuite, rootPublicKeyOfFabric.Matches(rcacPublicKey));
            }

            // Validate that fabric has the correct operational key by verifying a signature
            {
                Crypto::P256ECDSASignature sig;
                uint8_t message[] = { 'm', 's', 'g' };

                NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(2, ByteSpan{ message }, sig));
                NL_TEST_ASSERT_SUCCESS(inSuite, fIdx2PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
            }
        }

        // Validate contents of Fabric 1111
        {
            uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
            MutableByteSpan rcacSpan{ rcacBuf };
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootCert(1, rcacSpan));

            const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
            NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
            if (fabricInfo != nullptr)
            {
                Credentials::ChipCertificateSet certificates;
                NL_TEST_ASSERT_SUCCESS(inSuite, certificates.Init(1));
                NL_TEST_ASSERT_SUCCESS(inSuite,
                                       certificates.LoadCert(rcacSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));
                Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 1);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 55);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 1111);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);

                Crypto::P256PublicKey rootPublicKeyOfFabric;
                NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(1, rootPublicKeyOfFabric));
                NL_TEST_ASSERT(inSuite, rootPublicKeyOfFabric.Matches(rcacPublicKey));
            }

            // Validate that fabric has the correct operational key by verifying a signature
            {
                Crypto::P256ECDSASignature sig;
                uint8_t message[] = { 'm', 's', 'g' };

                NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(1, ByteSpan{ message }, sig));
                NL_TEST_ASSERT_SUCCESS(inSuite, fIdx1PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
            }

            // Validate that signing with Fabric index 2 fails to verify with fabric index 1
            {
                Crypto::P256ECDSASignature sig;
                uint8_t message[] = { 'm', 's', 'g' };

                NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(2, ByteSpan{ message }, sig));
                NL_TEST_ASSERT(inSuite,
                               fIdx1PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig) ==
                                   CHIP_ERROR_INVALID_SIGNATURE);
            }
        }
    }
}

void TestAddNocFailSafe(nlTestSuite * inSuite, void * inContext)
{
    Credentials::TestOnlyLocalCertificateAuthority fabric11CertAuthority;
    Credentials::TestOnlyLocalCertificateAuthority fabric44CertAuthority;

    chip::TestPersistentStorageDelegate storage;

    NL_TEST_ASSERT(inSuite, fabric11CertAuthority.Init().IsSuccess());
    NL_TEST_ASSERT(inSuite, fabric44CertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);

    size_t numFabricsIterated = 0;

    size_t numStorageKeysAtStart = storage.GetNumKeys();

    // Sequence 1: Add node ID 55 on fabric 11, see that pending works, and that revert works
    {
        FabricId fabricId = 11;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabric11CertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan rcac = fabric11CertAuthority.GetRcac();
        ByteSpan noc  = fabric11CertAuthority.GetNoc();

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        FabricIndex newFabricIndex = kUndefinedFabricIndex;

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, ByteSpan{}, kVendorId, &newFabricIndex));
        NL_TEST_ASSERT(inSuite, newFabricIndex == 1);
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);

        // No storage yet
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageKeysAtStart); // Nothing yet

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

        // Revert, should see nothing yet
        fabricTable.RevertPendingFabricData();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);

        // No started except fabric index metadata
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == (numStorageKeysAtStart + 1));

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

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 0);
            NL_TEST_ASSERT(inSuite, saw1 == false);
        }
    }

    size_t numStorageAfterRevert = storage.GetNumKeys();

    // Sequence 2: Add node ID 999 on fabric 44, using operational keystore and ICAC --> Yield fabricIndex 1
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

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        FabricIndex newFabricIndex = kUndefinedFabricIndex;

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT(inSuite, newFabricIndex == 1);
        // No storage yet
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageAfterRevert);

        // Commit, now storage should have keys
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);

        NL_TEST_ASSERT_EQUALS(inSuite, storage.GetNumKeys(),
                              (numStorageAfterRevert + 5)); // 3 opcerts + fabric metadata + 1 operational key

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
        {
            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            Crypto::P256PublicKey nocPubKey;
            NL_TEST_ASSERT_SUCCESS(inSuite, VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), nocPubKey));

            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig));
            NL_TEST_ASSERT_SUCCESS(inSuite, nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
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
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 999);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 44);
                    saw1 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 1);
            NL_TEST_ASSERT(inSuite, saw1 == true);
        }
    }

    size_t numStorageAfterAdd = storage.GetNumKeys();

    // Sequence 3: Do a RevertPendingFabricData() again, see that it doesn't affect existing fabric

    {
        // Revert, should should look like a no-op
        fabricTable.RevertPendingFabricData();

        // No change of storage
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageAfterAdd);

        // Verify we can still see the fabric with the iterator
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 999);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 44);
                    saw1 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 1);
            NL_TEST_ASSERT(inSuite, saw1 == true);
        }
    }
}

void TestUpdateNocFailSafe(nlTestSuite * inSuite, void * inContext)
{
    Credentials::TestOnlyLocalCertificateAuthority fabric11CertAuthority;
    Credentials::TestOnlyLocalCertificateAuthority fabric44CertAuthority;

    chip::TestPersistentStorageDelegate storage;

    storage.SetLoggingLevel(chip::TestPersistentStorageDelegate::LoggingLevel::kLogMutation);

    NL_TEST_ASSERT(inSuite, fabric11CertAuthority.Init().IsSuccess());
    NL_TEST_ASSERT(inSuite, fabric44CertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);

    size_t numFabricsIterated = 0;

    size_t numStorageKeysAtStart = storage.GetNumKeys();

    // Sequence 1: Add node ID 999 on fabric 44, using operational keystore and ICAC --> Yield fabricIndex 1
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

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        FabricIndex newFabricIndex = kUndefinedFabricIndex;

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT(inSuite, newFabricIndex == 1);
        // No storage yet
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageKeysAtStart);

        // Commit, now storage should have keys
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);

        NL_TEST_ASSERT_EQUALS(inSuite, storage.GetNumKeys(),
                              (numStorageKeysAtStart + 6)); // 3 opcerts + fabric metadata + 1 operational key + LKGT + fabric index

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
        {
            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            Crypto::P256PublicKey nocPubKey;
            NL_TEST_ASSERT_SUCCESS(inSuite, VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), nocPubKey));

            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig));
            NL_TEST_ASSERT_SUCCESS(inSuite, nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
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
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 999);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 44);
                    saw1 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 1);
            NL_TEST_ASSERT(inSuite, saw1 == true);
        }
    }

    size_t numStorageAfterAdd = storage.GetNumKeys();

    // Sequence 2: Do an Update to NodeId 1000, with no ICAC, but revert it
    {
        FabricId fabricId       = 44;
        NodeId nodeId           = 1000;
        FabricIndex fabricIndex = 1;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };

        // Make sure to tag fabric index to pending opkey: otherwise the UpdateNOC fails
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AllocatePendingOperationalKey(chip::MakeOptional(static_cast<FabricIndex>(1)), csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabric44CertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan rcac = fabric44CertAuthority.GetRcac();
        ByteSpan noc  = fabric44CertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.UpdatePendingFabricWithOperationalKeystore(1, noc, ByteSpan{}));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);

        // No storage yet
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageAfterAdd);

        // Validate iterator sees the pending data
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 1000);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 44);
                    saw1 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 1);
            NL_TEST_ASSERT(inSuite, saw1 == true);
        }

        // Revert, should see Node ID 999 again
        fabricTable.RevertPendingFabricData();
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);

        NL_TEST_ASSERT_EQUALS(inSuite, storage.GetNumKeys(), numStorageAfterAdd);

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

            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == fabricIndex);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 999);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 44);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(fabricIndex, rootPublicKeyOfFabric));
            NL_TEST_ASSERT(inSuite, rootPublicKeyOfFabric.Matches(rcacPublicKey));
        }

        // Validate that fabric has the correct operational key by verifying a signature
        {
            uint8_t nocBuf[Credentials::kMaxCHIPCertLength];
            MutableByteSpan nocSpan{ nocBuf };
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchNOCCert(1, nocSpan));

            Credentials::ChipCertificateSet certificates;
            NL_TEST_ASSERT_SUCCESS(inSuite, certificates.Init(1));
            NL_TEST_ASSERT_SUCCESS(inSuite,
                                   certificates.LoadCert(nocSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));
            Crypto::P256PublicKey nocPubKey(certificates.GetCertSet()[0].mPublicKey);

            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(fabricIndex, ByteSpan{ message }, sig));
            NL_TEST_ASSERT_SUCCESS(inSuite, nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
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
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 999);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 44);
                    saw1 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 1);
            NL_TEST_ASSERT(inSuite, saw1 == true);
        }
    }

    // Sequence 3: Do an Update to NodeId 1001, with no ICAC, but commit it
    {
        FabricId fabricId       = 44;
        NodeId nodeId           = 1001;
        FabricIndex fabricIndex = 1;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };

        // Make sure to tag fabric index to pending opkey: otherwise the UpdateNOC fails
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AllocatePendingOperationalKey(chip::MakeOptional(static_cast<FabricIndex>(1)), csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabric44CertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan rcac = fabric44CertAuthority.GetRcac();
        ByteSpan noc  = fabric44CertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.UpdatePendingFabricWithOperationalKeystore(1, noc, ByteSpan{}));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);

        // No storage yet
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageAfterAdd);

        // Validate iterator sees the pending data
        {
            numFabricsIterated = 0;
            bool saw1          = false;
            for (const auto & iterFabricInfo : fabricTable)
            {
                ++numFabricsIterated;
                if (iterFabricInfo.GetFabricIndex() == 1)
                {
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 1001);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 44);
                    saw1 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 1);
            NL_TEST_ASSERT(inSuite, saw1 == true);
        }

        // Commit, should see Node ID 1001, and 1 less cert in the storage
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT_EQUALS(inSuite, storage.GetNumKeys(), numStorageAfterAdd - 1);

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

            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == fabricIndex);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 1001);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 44);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(fabricIndex, rootPublicKeyOfFabric));
            NL_TEST_ASSERT(inSuite, rootPublicKeyOfFabric.Matches(rcacPublicKey));
        }

        // Validate that fabric has the correct operational key by verifying a signature
        {
            Crypto::P256PublicKey nocPubKey;
            NL_TEST_ASSERT_SUCCESS(inSuite, VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), nocPubKey));

            Crypto::P256ECDSASignature sig;
            uint8_t message[] = { 'm', 's', 'g' };

            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(fabricIndex, ByteSpan{ message }, sig));
            NL_TEST_ASSERT_SUCCESS(inSuite, nocPubKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
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
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 1001);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 44);
                    saw1 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 1);
            NL_TEST_ASSERT(inSuite, saw1 == true);
        }
    }
}

void TestAddRootCertFailSafe(nlTestSuite * inSuite, void * inContext)
{
    Credentials::TestOnlyLocalCertificateAuthority fabric11CertAuthority;

    chip::TestPersistentStorageDelegate storage;

    NL_TEST_ASSERT(inSuite, fabric11CertAuthority.Init().IsSuccess());

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);

    // Add a root cert, see that pending works, and that revert works
    {
        ByteSpan rcac = fabric11CertAuthority.GetRcac();

        uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
        {
            // No pending root cert yet.
            MutableByteSpan fetchedSpan{ rcacBuf };
            NL_TEST_ASSERT(inSuite, fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan) == CHIP_ERROR_NOT_FOUND);
        }

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        {
            // Now have a pending root cert.
            MutableByteSpan fetchedSpan{ rcacBuf };
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan));
            NL_TEST_ASSERT(inSuite, fetchedSpan.data_equal(rcac));
        }

        // Revert
        fabricTable.RevertPendingFabricData();

        {
            // No pending root cert anymore.
            MutableByteSpan fetchedSpan{ rcacBuf };
            NL_TEST_ASSERT(inSuite, fabricTable.FetchPendingNonFabricAssociatedRootCert(fetchedSpan) == CHIP_ERROR_NOT_FOUND);
        }
    }
}

void TestSequenceErrors(nlTestSuite * inSuite, void * inContext)
{
    // TODO: Write test
}

void TestFabricLabelChange(nlTestSuite * inSuite, void * inContext)
{
    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority;

    chip::TestPersistentStorageDelegate storage;
    NL_TEST_ASSERT(inSuite, fabricCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);

    // First scope: add FabricID 1111, node ID 55
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan rcac = fabricCertAuthority.GetRcac();
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT(inSuite, newFabricIndex == 1);

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
        if (fabricInfo != nullptr)
        {
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 1);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 55);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 1111);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);
        }
    }
    size_t numStorageKeysAfterFirstAdd = storage.GetNumKeys();
    NL_TEST_ASSERT(inSuite, numStorageKeysAfterFirstAdd == 7); // Metadata, index, 3 certs, 1 opkey, last known good time

    // Second scope: set FabricLabel to "acme fabric", make sure it cannot be reverted
    {
        // Fabric label starts unset from prior scope
        CharSpan fabricLabel = CharSpan::fromCharString("placeholder");

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.GetFabricLabel(1, fabricLabel));
        NL_TEST_ASSERT(inSuite, fabricLabel.size() == 0);

        // Set a valid name
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SetFabricLabel(1, CharSpan::fromCharString("acme fabric")));
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.GetFabricLabel(1, fabricLabel));
        NL_TEST_ASSERT(inSuite, fabricLabel.data_equal(CharSpan::fromCharString("acme fabric")) == true);

        // Revert pending fabric data. Should not revert name since nothing pending.
        fabricTable.RevertPendingFabricData();

        fabricLabel = CharSpan::fromCharString("placeholder");
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.GetFabricLabel(1, fabricLabel));
        NL_TEST_ASSERT(inSuite, fabricLabel.data_equal(CharSpan::fromCharString("acme fabric")) == true);

        // Verify we fail to set too large a label (> kFabricLabelMaxLengthInBytes)
        CharSpan fabricLabelTooBig = CharSpan::fromCharString("012345678901234567890123456789123456");
        NL_TEST_ASSERT(inSuite, fabricLabelTooBig.size() > chip::kFabricLabelMaxLengthInBytes);

        NL_TEST_ASSERT(inSuite, fabricTable.SetFabricLabel(1, fabricLabelTooBig) == CHIP_ERROR_INVALID_ARGUMENT);

        fabricLabel = CharSpan::fromCharString("placeholder");
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.GetFabricLabel(1, fabricLabel));
        NL_TEST_ASSERT(inSuite, fabricLabel.data_equal(CharSpan::fromCharString("acme fabric")) == true);
    }

    // Third scope: set fabric label after an update, it sticks, but then goes back after revert
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 66; // Update node ID from 55 to 66

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AllocatePendingOperationalKey(chip::MakeOptional(static_cast<FabricIndex>(1)), csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.UpdatePendingFabricWithOperationalKeystore(1, noc, icac));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);

        // Validate contents prior to change/revert
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);

        if (fabricInfo != nullptr)
        {
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 1);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 66);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 1111);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);

            CharSpan fabricLabel = fabricInfo->GetFabricLabel();
            NL_TEST_ASSERT(inSuite, fabricLabel.data_equal(CharSpan::fromCharString("acme fabric")) == true);
        }

        // Update fabric label
        CharSpan fabricLabel = CharSpan::fromCharString("placeholder");
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SetFabricLabel(1, CharSpan::fromCharString("roboto fabric")));

        fabricLabel = CharSpan::fromCharString("placeholder");
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.GetFabricLabel(1, fabricLabel));
        NL_TEST_ASSERT(inSuite, fabricLabel.data_equal(CharSpan::fromCharString("roboto fabric")) == true);

        // Revert pending fabric data. Should revert name to "acme fabric"
        fabricTable.RevertPendingFabricData();

        fabricLabel = CharSpan::fromCharString("placeholder");
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.GetFabricLabel(1, fabricLabel));
        NL_TEST_ASSERT(inSuite, fabricLabel.data_equal(CharSpan::fromCharString("acme fabric")) == true);
    }
}

void TestCompressedFabricId(nlTestSuite * inSuite, void * inContext)
{
    // TODO: Write test
}

void TestFabricLookup(nlTestSuite * inSuite, void * inContext)
{
    // Initialize a fabric table.
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;
    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();
    NL_TEST_ASSERT(inSuite, LoadTestFabric_Node01_01(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, LoadTestFabric_Node02_01(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);

    // Attempt lookup of the Root01 fabric.
    {
        Crypto::P256PublicKey key;
        NL_TEST_ASSERT(inSuite, key.Length() >= TestCerts::sTestCert_Root01_PublicKey_Len);
        if (key.Length() < TestCerts::sTestCert_Root01_PublicKey_Len)
        {
            return;
        }
        memcpy(key.Bytes(), TestCerts::sTestCert_Root01_PublicKey, TestCerts::sTestCert_Root01_PublicKey_Len);
        auto fabricInfo = fabricTable.FindFabric(key, 0xFAB000000000001D);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
        if (fabricInfo == nullptr)
        {
            return;
        }
        NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 1);
    }

    // Attempt lookup of the Root02 fabric.
    {
        Crypto::P256PublicKey key;
        NL_TEST_ASSERT(inSuite, key.Length() >= TestCerts::sTestCert_Root02_PublicKey_Len);
        if (key.Length() < TestCerts::sTestCert_Root02_PublicKey_Len)
        {
            return;
        }
        memcpy(key.Bytes(), TestCerts::sTestCert_Root02_PublicKey, TestCerts::sTestCert_Root02_PublicKey_Len);
        auto fabricInfo = fabricTable.FindFabric(key, 0xFAB000000000001D);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
        if (fabricInfo == nullptr)
        {
            return;
        }
        NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 2);
    }
}

void TestFetchCATs(nlTestSuite * inSuite, void * inContext)
{
    // Initialize a fabric table.
    chip::TestPersistentStorageDelegate testStorage;
    ScopedFabricTable fabricTableHolder;
    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();
    NL_TEST_ASSERT(inSuite, LoadTestFabric_Node01_01(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, LoadTestFabric_Node02_01(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);

    // Attempt Fetching fabric index 1 CATs and verify contents.
    {
        CATValues cats;
        NL_TEST_ASSERT(inSuite, fabricTable.FetchCATs(1, cats) == CHIP_NO_ERROR);
        // Test fabric NOCs don't contain any CATs.
        NL_TEST_ASSERT(inSuite, cats == kUndefinedCATs);
    }

    // Attempt Fetching fabric index 2 CATs and verify contents.
    {
        CATValues cats;
        NL_TEST_ASSERT(inSuite, fabricTable.FetchCATs(2, cats) == CHIP_NO_ERROR);
        // Test fabric NOCs don't contain any CATs.
        NL_TEST_ASSERT(inSuite, cats == kUndefinedCATs);
    }

    // TODO(#20335): Add test cases for NOCs that actually embed CATs
}

// Validate that adding the same fabric twice fails (same root, same FabricId)
void TestAddNocRootCollision(nlTestSuite * inSuite, void * inContext)
{
    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority;

    chip::TestPersistentStorageDelegate storage;
    NL_TEST_ASSERT(inSuite, fabricCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);

    // First scope: add FabricID 1111, node ID 55
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan rcac = fabricCertAuthority.GetRcac();
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT(inSuite, newFabricIndex == 1);

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());

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

            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 1);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 55);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 1111);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric));
            NL_TEST_ASSERT(inSuite, rootPublicKeyOfFabric.Matches(rcacPublicKey));
        }
    }
    size_t numStorageKeysAfterFirstAdd = storage.GetNumKeys();
    NL_TEST_ASSERT(inSuite, numStorageKeysAfterFirstAdd == 7); // Metadata, index, 3 certs, 1 opkey, last known good time

    // Second scope: add FabricID 1111, node ID 55 *again* --> Collision of Root/FabricID with existing
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan rcac = fabricCertAuthority.GetRcac();
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        NL_TEST_ASSERT(inSuite,
                       fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex) ==
                           CHIP_ERROR_FABRIC_EXISTS);
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);

        CHIP_ERROR err = fabricTable.CommitPendingFabricData();
        printf("err = %" CHIP_ERROR_FORMAT "\n", err.Format());
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

        // Validate contents of Fabric index 1 still valid
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
        if (fabricInfo != nullptr)
        {
            Credentials::ChipCertificateSet certificates;
            NL_TEST_ASSERT_SUCCESS(inSuite, certificates.Init(1));
            NL_TEST_ASSERT_SUCCESS(inSuite,
                                   certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));
            Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 1);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 55);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 1111);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(1, rootPublicKeyOfFabric));
            NL_TEST_ASSERT(inSuite, rootPublicKeyOfFabric.Matches(rcacPublicKey));
        }
    }

    // Ensure no new persisted keys after failed colliding add
    NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageKeysAfterFirstAdd);

    // Third scope: add FabricID 2222, node ID 55 --> Not colliding, should work. The failing commit above]
    // should have been enough of a revert that this scope succeeds without any additional revert.
    {
        FabricId fabricId = 2222;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        ByteSpan rcac = fabricCertAuthority.GetRcac();
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);
        NL_TEST_ASSERT(inSuite, newFabricIndex == 2);

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());

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

            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 2);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 55);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 2222);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);

            Crypto::P256PublicKey rootPublicKeyOfFabric;
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric));
            NL_TEST_ASSERT(inSuite, rootPublicKeyOfFabric.Matches(rcacPublicKey));
        }
    }
    size_t numStorageKeysAfterSecondAdd = storage.GetNumKeys();

    NL_TEST_ASSERT(inSuite, numStorageKeysAfterSecondAdd == (numStorageKeysAfterFirstAdd + 5)); // Metadata, 3 certs, 1 opkey
}

void TestInvalidChaining(nlTestSuite * inSuite, void * inContext)
{
    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority;
    Credentials::TestOnlyLocalCertificateAuthority differentCertAuthority;

    chip::TestPersistentStorageDelegate storage;
    NL_TEST_ASSERT(inSuite, fabricCertAuthority.Init().IsSuccess());
    NL_TEST_ASSERT(inSuite, differentCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    // Initialize a fabric table.
    ScopedFabricTable fabricTableHolder;
    NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);

    // Try to add fabric with either the NOC not chaining properly, or ICAC not chaining properly, fail,
    // then succeed with proper chaining
    {
        FabricId fabricId = 1111;
        NodeId nodeId     = 55;

        uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
        MutableByteSpan csrSpan{ csrBuf };
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

        // Generate same cert chain from two different roots
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               differentCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());

        ByteSpan rcac = fabricCertAuthority.GetRcac();
        ByteSpan icac = fabricCertAuthority.GetIcac();
        ByteSpan noc  = fabricCertAuthority.GetNoc();

        ByteSpan otherIcac = differentCertAuthority.GetIcac();
        ByteSpan otherNoc  = differentCertAuthority.GetNoc();

        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));

        // Add with NOC not chaining to ICAC: fail
        {
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            CHIP_ERROR err = fabricTable.AddNewPendingFabricWithOperationalKeystore(otherNoc, icac, kVendorId, &newFabricIndex);
            NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        }

        // Add with ICAC not chaining to root: fail
        {
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            CHIP_ERROR err = fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, otherIcac, kVendorId, &newFabricIndex);
            NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        }

        // Add with NOC and ICAC chaining together, but not to root: fail
        {
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            CHIP_ERROR err =
                fabricTable.AddNewPendingFabricWithOperationalKeystore(otherNoc, otherIcac, kVendorId, &newFabricIndex);
            NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        }

        // Revert state, start tests without ICAC
        fabricTable.RevertPendingFabricData();

        // Generate same cert chain from two different roots

        csrSpan = MutableByteSpan{ csrBuf };
        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricCertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
        NL_TEST_ASSERT_SUCCESS(
            inSuite, differentCertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());

        rcac = fabricCertAuthority.GetRcac();
        noc  = fabricCertAuthority.GetNoc();

        otherNoc = differentCertAuthority.GetNoc();

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));

        // Add with NOC not chaining to RCAC: fail
        {
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            CHIP_ERROR err =
                fabricTable.AddNewPendingFabricWithOperationalKeystore(otherNoc, ByteSpan{}, kVendorId, &newFabricIndex);
            NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
        }

        // Add properly now
        FabricIndex newFabricIndex = kUndefinedFabricIndex;
        NL_TEST_ASSERT_SUCCESS(inSuite,
                               fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, ByteSpan{}, kVendorId, &newFabricIndex));
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
        NL_TEST_ASSERT(inSuite, newFabricIndex == 1);

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());

        // Validate contents
        const auto * fabricInfo = fabricTable.FindFabricWithIndex(1);
        NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
        if (fabricInfo != nullptr)
        {
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 1);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 55);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 1111);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
            NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);
        }
    }
}

void TestEphemeralKeys(nlTestSuite * inSuite, void * inContext)
{
    // Initialize a fabric table with operational keystore
    {
        chip::TestPersistentStorageDelegate storage;

        // Initialize a FabricTable
        ScopedFabricTable fabricTableHolder;
        NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
        FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

        Crypto::P256ECDSASignature sig;
        uint8_t message[] = { 'm', 's', 'g' };

        Crypto::P256Keypair * ephemeralKeypair = fabricTable.AllocateEphemeralKeypairForCASE();
        NL_TEST_ASSERT(inSuite, ephemeralKeypair != nullptr);
        NL_TEST_ASSERT_SUCCESS(inSuite, ephemeralKeypair->Initialize());

        NL_TEST_ASSERT_SUCCESS(inSuite, ephemeralKeypair->ECDSA_sign_msg(message, sizeof(message), sig));
        NL_TEST_ASSERT_SUCCESS(inSuite, ephemeralKeypair->Pubkey().ECDSA_validate_msg_signature(message, sizeof(message), sig));

        fabricTable.ReleaseEphemeralKeypair(ephemeralKeypair);
    }

    // Use a fabric table without an operational keystore: should still work
    {
        chip::TestPersistentStorageDelegate storage;

        chip::Credentials::PersistentStorageOpCertStore opCertStore;
        NL_TEST_ASSERT_SUCCESS(inSuite, opCertStore.Init(&storage));

        FabricTable fabricTable;
        FabricTable::InitParams initParams;
        initParams.storage     = &storage;
        initParams.opCertStore = &opCertStore;

        NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.Init(initParams));

        Crypto::P256ECDSASignature sig;
        uint8_t message[] = { 'm', 's', 'g' };

        Crypto::P256Keypair * ephemeralKeypair = fabricTable.AllocateEphemeralKeypairForCASE();
        NL_TEST_ASSERT(inSuite, ephemeralKeypair != nullptr);
        NL_TEST_ASSERT_SUCCESS(inSuite, ephemeralKeypair->Initialize());

        NL_TEST_ASSERT_SUCCESS(inSuite, ephemeralKeypair->ECDSA_sign_msg(message, sizeof(message), sig));
        NL_TEST_ASSERT_SUCCESS(inSuite, ephemeralKeypair->Pubkey().ECDSA_validate_msg_signature(message, sizeof(message), sig));

        fabricTable.ReleaseEphemeralKeypair(ephemeralKeypair);

        fabricTable.Shutdown();
        opCertStore.Finish();
    }
}

void TestCommitMarker(nlTestSuite * inSuite, void * inContext)
{
    Crypto::P256PublicKey fIdx1PublicKey;
    Crypto::P256PublicKey fIdx2PublicKey;

    Credentials::TestOnlyLocalCertificateAuthority fabricCertAuthority;

    chip::TestPersistentStorageDelegate storage;

    // Log verbosity on this test helps debug significantly
    storage.SetLoggingLevel(chip::TestPersistentStorageDelegate::LoggingLevel::kLogMutationAndReads);

    NL_TEST_ASSERT(inSuite, fabricCertAuthority.Init().IsSuccess());

    constexpr uint16_t kVendorId = 0xFFF1u;

    size_t numStorageKeysAfterFirstAdd = 0;

    // First scope: add 2 fabrics with same root:
    //   - FabricID 1111, Node ID 55
    //   - FabricID 2222, Node ID 66
    //      - Abort commit on second fabric
    {
        // Initialize a fabric table
        ScopedFabricTable fabricTableHolder;
        NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
        FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

        NL_TEST_ASSERT(inSuite, fabricTable.GetDeletedFabricFromCommitMarker() == kUndefinedFabricIndex);
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);

        // Add Fabric 1111 Node Id 55
        {
            FabricId fabricId = 1111;
            NodeId nodeId     = 55;

            uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
            MutableByteSpan csrSpan{ csrBuf };
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

            NL_TEST_ASSERT_SUCCESS(
                inSuite, fabricCertAuthority.SetIncludeIcac(true).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
            ByteSpan rcac = fabricCertAuthority.GetRcac();
            ByteSpan icac = fabricCertAuthority.GetIcac();
            ByteSpan noc  = fabricCertAuthority.GetNoc();

            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 0);
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            NL_TEST_ASSERT_SUCCESS(inSuite,
                                   fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, icac, kVendorId, &newFabricIndex));
            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
            NL_TEST_ASSERT(inSuite, newFabricIndex == 1);

            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.CommitPendingFabricData());

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

                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 1);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 55);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 1111);
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

                NL_TEST_ASSERT_SUCCESS(inSuite, VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), fIdx1PublicKey));

                NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.SignWithOpKeypair(newFabricIndex, ByteSpan{ message }, sig));
                NL_TEST_ASSERT_SUCCESS(inSuite, fIdx1PublicKey.ECDSA_validate_msg_signature(&message[0], sizeof(message), sig));
            }
        }
        numStorageKeysAfterFirstAdd = storage.GetNumKeys();

        NL_TEST_ASSERT(inSuite, numStorageKeysAfterFirstAdd == 7); // Metadata, index, 3 certs, 1 opkey, last known good time

        // The following test requires test methods not available on all builds.
        // TODO: Debug why some CI jobs don't set it properly.
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST

        // Add Fabric 2222 Node Id 66, no ICAC *** AND ABORT COMMIT ***
        {
            FabricId fabricId = 2222;
            NodeId nodeId     = 66;

            uint8_t csrBuf[chip::Crypto::kMAX_CSR_Length];
            MutableByteSpan csrSpan{ csrBuf };
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));

            NL_TEST_ASSERT_SUCCESS(
                inSuite, fabricCertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, csrSpan).GetStatus());
            ByteSpan rcac = fabricCertAuthority.GetRcac();
            ByteSpan noc  = fabricCertAuthority.GetNoc();

            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);
            NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.AddNewPendingTrustedRootCert(rcac));
            FabricIndex newFabricIndex = kUndefinedFabricIndex;
            NL_TEST_ASSERT_SUCCESS(
                inSuite, fabricTable.AddNewPendingFabricWithOperationalKeystore(noc, ByteSpan{}, kVendorId, &newFabricIndex));
            NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 2);
            NL_TEST_ASSERT(inSuite, newFabricIndex == 2);

            // Validate contents of pending
            const auto * fabricInfo = fabricTable.FindFabricWithIndex(2);
            NL_TEST_ASSERT(inSuite, fabricInfo != nullptr);
            if (fabricInfo != nullptr)
            {
                Credentials::ChipCertificateSet certificates;
                NL_TEST_ASSERT_SUCCESS(inSuite, certificates.Init(1));
                NL_TEST_ASSERT_SUCCESS(inSuite,
                                       certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));
                Crypto::P256PublicKey rcacPublicKey(certificates.GetCertSet()[0].mPublicKey);

                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricIndex() == 2);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetNodeId() == 66);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricId() == 2222);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetVendorId() == kVendorId);
                NL_TEST_ASSERT(inSuite, fabricInfo->GetFabricLabel().size() == 0);

                Crypto::P256PublicKey rootPublicKeyOfFabric;
                NL_TEST_ASSERT_SUCCESS(inSuite, fabricTable.FetchRootPubkey(newFabricIndex, rootPublicKeyOfFabric));
                NL_TEST_ASSERT(inSuite, rootPublicKeyOfFabric.Matches(rcacPublicKey));
            }

            // Make sure no additional storage yet
            NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageKeysAfterFirstAdd);

            // --> FORCE AN ERROR ON COMMIT that will BYPASS commit clean-up (similar to reboot during commit)
            fabricTable.SetForceAbortCommitForTest(true);
            NL_TEST_ASSERT(inSuite, fabricTable.CommitPendingFabricData() == CHIP_ERROR_INTERNAL);

            // Check that there are more keys now, partially committed: at least a Commit Marker (+1)
            // and some more keys from the aborted process.
            NL_TEST_ASSERT(inSuite, storage.GetNumKeys() > (numStorageKeysAfterFirstAdd + 1));
        }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
    }

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    {
        storage.DumpKeys();

        // Initialize a FabricTable again. Make sure it succeeds in initing.
        ScopedFabricTable fabricTableHolder;

        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() > (numStorageKeysAfterFirstAdd + 1));

        NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&storage) == CHIP_NO_ERROR);
        FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

        // Make sure that after init, the fabricTable has only 1 fabric
        NL_TEST_ASSERT_EQUALS(inSuite, fabricTable.FabricCount(), 1);

        // Make sure it caught the last partially committed fabric
        NL_TEST_ASSERT(inSuite, fabricTable.GetDeletedFabricFromCommitMarker() == 2);

        // Second read must return kUndefinedFabricIndex
        NL_TEST_ASSERT(inSuite, fabricTable.GetDeletedFabricFromCommitMarker() == kUndefinedFabricIndex);

        {
            // Here we would do other clean-ups (e.g. see Server.cpp that uses the above) and then
            // clear the commit marker after.
            fabricTable.ClearCommitMarker();
        }

        // Make sure that all other pending storage got deleted
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == numStorageKeysAfterFirstAdd);

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
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetNodeId() == 55);
                    NL_TEST_ASSERT(inSuite, iterFabricInfo.GetFabricId() == 1111);
                    saw1 = true;
                }
                if (iterFabricInfo.GetFabricIndex() == 2)
                {
                    saw2 = true;
                }
            }

            NL_TEST_ASSERT(inSuite, numFabricsIterated == 1);
            NL_TEST_ASSERT(inSuite, saw1 == true);
            NL_TEST_ASSERT(inSuite, saw2 == false);
        }
    }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
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
    NL_TEST_DEF("Test fail-safe handling for root cert", TestAddRootCertFailSafe),
    NL_TEST_DEF("Test interlock sequencing errors", TestSequenceErrors),
    NL_TEST_DEF("Test fabric label changes", TestFabricLabelChange),
    NL_TEST_DEF("Test compressed fabric ID is properly generated", TestCompressedFabricId),
    NL_TEST_DEF("Test fabric lookup by <root public key, fabric ID>", TestFabricLookup),
    NL_TEST_DEF("Test Fetching CATs", TestFetchCATs),
    NL_TEST_DEF("Test AddNOC root collision", TestAddNocRootCollision),
    NL_TEST_DEF("Test invalid chaining in AddNOC and UpdateNOC", TestInvalidChaining),
    NL_TEST_DEF("Test ephemeral keys allocation", TestEphemeralKeys),
    NL_TEST_DEF("Test proper detection of Commit Marker on init", TestCommitMarker),
    NL_TEST_DEF("Test colliding fabrics in the fabric table", TestCollidingFabrics),

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
