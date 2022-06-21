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

#include <credentials/tests/CHIPCert_test_vectors.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/asn1/ASN1.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <platform/ConfigurationManager.h>

#include <stdarg.h>

using namespace chip;

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
        initParams.storage = storage;
        initParams.operationalKeystore = &mOpKeyStore;
        initParams.opCertStore = &mOpCertStore;

        ReturnErrorOnFailure(mOpKeyStore.Init(storage));
        ReturnErrorOnFailure(mOpCertStore.Init(storage));
        return mFabricTable.Init(initParams);
    }

    FabricTable & GetFabricTable()
    {
        return mFabricTable;
    }

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

    CHIP_ERROR err = fabricTable.AddNewPendingFabricWithProvidedOpKey(nocSpan, icacSpan, VendorId::TestVendor1, &gFabric1OpKey, /*hasExternallyOwnedKeypair =*/ true, &fabricIndex);
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

            // Load a test fabric
            NL_TEST_ASSERT(inSuite, LoadTestFabric(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it hasn't moved forward.
            // This test case was written after the test certs' NotBefore time and we
            // are using a configuration manager that should reflect a real build time.
            // Therefore, we expect that build time is after NotBefore and so Last
            // Known Good Time will be set to the later of these, build time, even
            // after installing the new fabric.
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

            // Verify that we can call the fail-safe roll back interface.
            // Because we didn't advance Last Known Good Time, this should be a
            // no-op.
            NL_TEST_ASSERT(inSuite, fabricTable.RevertLastKnownGoodChipEpochTime() == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);
        }
        {
            // Reload again from persistence to verify the fail-safe rollback
            // left the time intact.
            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

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
    // build time do result in Last Known Good Times set to these.  Then test
    // that we can do a fail-safe roll back.
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

            // Load a test fabric
            NL_TEST_ASSERT(inSuite, LoadTestFabric(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it is now set to the certificate
            // NotBefore time, as this should be at or after firmware build time.
            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == testCertNotBeforeTime);
        }
// TODO: Fix the fail-safe handling of LKGT
#if 0
        {
            // Test reloading last known good time from persistence.
            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Verify that last known good time was retained.
            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == testCertNotBeforeTime);

            // Verify that we can do a fail-safe roll back.
            NL_TEST_ASSERT(inSuite, fabricTable.RevertLastKnownGoodChipEpochTime() == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);
        }
        {
            // Reload again from persistence to verify the fail-safe rollback
            // persisted the reverted time.
            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == buildTime);
        }
#endif
    }
    // Test that certificate NotBefore times that are at or after the Firmware
    // build time do result in Last Known Good Times set to these.  Then test
    // that we can commit these to storage.  Attempted fail-safe roll back after
    // commit will be a no-op.
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

            // Load a test fabric
            NL_TEST_ASSERT(inSuite, LoadTestFabric(inSuite, fabricTable, /* doCommit = */ true) == CHIP_NO_ERROR);

            // Read Last Known Good Time and verify that it is now set to the certificate
            // NotBefore time, as this should be at or after firmware build time.
            System::Clock::Seconds32 lastKnownGoodTime;
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == testCertNotBeforeTime);

            // Commit to storage.
            NL_TEST_ASSERT(inSuite, fabricTable.CommitLastKnownGoodChipEpochTime() == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == testCertNotBeforeTime);

            // Verify that we can do a no-op fail-safe roll back.
            NL_TEST_ASSERT(inSuite, fabricTable.RevertLastKnownGoodChipEpochTime() == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, fabricTable.GetLastKnownGoodChipEpochTime(lastKnownGoodTime) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, lastKnownGoodTime == testCertNotBeforeTime);
        }
        {
            // Test reloading last known good time from persistence.
            ScopedFabricTable fabricTableHolder;
            NL_TEST_ASSERT(inSuite, fabricTableHolder.Init(&testStorage) == CHIP_NO_ERROR);
            FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

            // Verify that last known good time was retained.
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

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Last Known Good Time Init",    TestLastKnownGoodTimeInit),
    NL_TEST_DEF("Update Last Known Good Time",    TestUpdateLastKnownGoodTime),
    NL_TEST_DEF("Set Last Known Good Time",    TestSetLastKnownGoodTime),
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
