/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <inttypes.h>

#include <pw_unit_test/framework.h>

#include <credentials/PersistentStorageOpCertStore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>

using namespace chip;
using namespace chip::Credentials;
using CertChainElement = OperationalCertificateStore::CertChainElement;

namespace {

constexpr FabricIndex kFabricIndex1     = 1;
constexpr FabricIndex kFabricIndex2     = 2;
constexpr FabricIndex kOtherFabricIndex = static_cast<FabricIndex>(kFabricIndex1 + 10u);

// The PersistentStorageOpCertStore does not validate cert contents, so we can use simple constants
const uint8_t kTestRcacBuf[] = { 'r', 'c', 'a', 'c' };
const ByteSpan kTestRcacSpan{ kTestRcacBuf };

const uint8_t kTestIcacBuf[] = { 'i', 'c', 'a', 'c' };
const ByteSpan kTestIcacSpan{ kTestIcacBuf };

const uint8_t kTestNocBuf[] = { 'n', 'o', 'c' };
const ByteSpan kTestNocSpan{ kTestNocBuf };

struct TestPersistentStorageOpCertStore : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestPersistentStorageOpCertStore, TestAddNocFlow)
{
    TestPersistentStorageDelegate storageDelegate;
    PersistentStorageOpCertStore opCertStore;

    // Failure before Init
    CHIP_ERROR err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u);

    // Init succeeds
    err = opCertStore.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Manually add existing root for the FabricIndex, should fail AddNewTrustedRootCertForFabric for
    // same fabric but succeed GetCertificate.
    const uint8_t kTestRcacBufExists[] = { 'r', 'c', 'a', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };

    err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricRCAC(kFabricIndex1).KeyName(), kTestRcacBufExists,
                                          sizeof(kTestRcacBufExists));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 1u);
    EXPECT_TRUE(opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kRcac)); //< From manual add

    err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    uint8_t largeBuf[400];
    MutableByteSpan largeSpan{ largeBuf };

    err = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
    EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);

    largeSpan = MutableByteSpan{ largeBuf };

    err = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacBufExists }));

    // Adding root for another FabricIndex should work
    err = opCertStore.AddNewTrustedRootCertForFabric(kUndefinedFabricIndex, kTestRcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);

    err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex2, kTestRcacSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 1u); //< Storage count did not yet increase
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_TRUE(opCertStore.HasPendingRootCert());

    // Should be able to read pending RCAC right away
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kRcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

    // Trying to commit with pending RCAC but no NOC should fail but leave everything as-is
    err = opCertStore.CommitOpCertsForFabric(kFabricIndex2);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_TRUE(opCertStore.HasPendingRootCert());

    // Trying to do AddNewOpCertsForFabric for fabric different that with pending RCAC should fail
    err = opCertStore.AddNewOpCertsForFabric(kOtherFabricIndex, kTestNocSpan, kTestIcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_TRUE(opCertStore.HasPendingRootCert());

    // Clear other bad cases from storage for now
    storageDelegate.ClearStorage();

    // Trying to do AddNewOpCertsForFabric for same fabric as that with pending RCAC should fail
    // if there are already existing NOC chain elements for the given fabric.
    const uint8_t kTestIcacBufExists[] = { 'i', 'c', 'a', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };
    const uint8_t kTestNocBufExists[]  = { 'n', 'o', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };

    err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricICAC(kFabricIndex2).KeyName(), kTestIcacBufExists,
                                          sizeof(kTestIcacBufExists));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 1u);

    err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricNOC(kFabricIndex2).KeyName(), kTestNocBufExists,
                                          sizeof(kTestNocBufExists));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 2u);

    EXPECT_TRUE(opCertStore.HasCertificateForFabric(kFabricIndex2, CertChainElement::kIcac)); //< From manual add
    EXPECT_TRUE(opCertStore.HasCertificateForFabric(kFabricIndex2, CertChainElement::kNoc));  //< From manual add

    err = opCertStore.AddNewOpCertsForFabric(kFabricIndex2, kTestNocSpan, kTestIcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_TRUE(opCertStore.HasPendingRootCert());

    storageDelegate.SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricICAC(kFabricIndex2).KeyName());
    storageDelegate.SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricNOC(kFabricIndex2).KeyName());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u);

    // Trying to do AddNewOpCertsForFabric for same fabric as that with pending RCAC should succeed
    err = opCertStore.AddNewOpCertsForFabric(kFabricIndex2, kTestNocSpan, kTestIcacSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(opCertStore.HasPendingNocChain());
    EXPECT_TRUE(opCertStore.HasPendingRootCert());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase

    // Should be able to get the pending cert even if not in persisted storage
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kIcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kNoc, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestNocSpan }));

    // Trying to do AddNewOpCertsForFabric a second time after success before commit should fail,
    // but leave state as-is
    err = opCertStore.AddNewOpCertsForFabric(kFabricIndex2, kTestNocSpan, kTestIcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(opCertStore.HasPendingNocChain());
    EXPECT_TRUE(opCertStore.HasPendingRootCert());

    // Should be able to get the pending cert even if not in persisted storage, after an API error
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kIcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kNoc, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestNocSpan }));

    // Trying to commit with wrong FabricIndex should fail
    err = opCertStore.CommitOpCertsForFabric(kOtherFabricIndex);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_TRUE(opCertStore.HasPendingNocChain());
    EXPECT_TRUE(opCertStore.HasPendingRootCert());

    // Commiting new certs should succeed on correct fabric
    err = opCertStore.CommitOpCertsForFabric(kFabricIndex2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_FALSE(opCertStore.HasPendingRootCert());

    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u); //< All certs now committed

    // Should be able to get the committed certs
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kRcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kIcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kNoc, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestNocSpan }));

    opCertStore.Finish();
}

TEST_F(TestPersistentStorageOpCertStore, TestUpdateNocFlow)
{
    TestPersistentStorageDelegate storageDelegate;
    PersistentStorageOpCertStore opCertStore;

    // Failure before Init
    CHIP_ERROR err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u);

    // Init succeeds
    err = opCertStore.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Add a new pending trusted root to test for UpdateOpCertsForFabric failure on new root present
    err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
    EXPECT_TRUE(opCertStore.HasPendingRootCert());
    EXPECT_FALSE(opCertStore.HasPendingNocChain());

    // Trying to do an UpdateOpCertsForFabric with new root pending should fail
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(opCertStore.HasPendingRootCert());
    EXPECT_FALSE(opCertStore.HasPendingNocChain());

    // Revert state for next tests
    opCertStore.RevertPendingOpCerts();
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase

    // Manually add root, ICAC and NOC to validate update since existing chain required
    const uint8_t kTestRcacBufExists[] = { 'r', 'c', 'a', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };
    const uint8_t kTestIcacBufExists[] = { 'i', 'c', 'a', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };
    const uint8_t kTestNocBufExists[]  = { 'n', 'o', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };

    uint8_t largeBuf[400];
    MutableByteSpan largeSpan{ largeBuf };

    {
        err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricRCAC(kFabricIndex1).KeyName(), kTestRcacBufExists,
                                              sizeof(kTestRcacBufExists));
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(storageDelegate.GetNumKeys(), 1u);
        EXPECT_TRUE(opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kRcac)); //< From manual add

        err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricICAC(kFabricIndex1).KeyName(), kTestIcacBufExists,
                                              sizeof(kTestIcacBufExists));
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(storageDelegate.GetNumKeys(), 2u);
        EXPECT_TRUE(opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kIcac)); //< From manual add

        err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricNOC(kFabricIndex1).KeyName(), kTestNocBufExists,
                                              sizeof(kTestNocBufExists));
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(storageDelegate.GetNumKeys(), 3u);
        EXPECT_TRUE(opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kNoc)); //< From manual add

        // Test that we can manually stored certs
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacBufExists }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestIcacBufExists }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestNocBufExists }));
    }

    // Update fails on fabric with wrong FabricIndex
    err = opCertStore.UpdateOpCertsForFabric(kOtherFabricIndex, kTestNocSpan, kTestIcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_FALSE(opCertStore.HasPendingNocChain());

    // Update succeeds on fabric with existing data
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_TRUE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u);

    // Can read back existing root unchanged
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacBufExists }));

    // NOC chain elements see the pending updated certs
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestIcacBuf }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestNocBuf }));

    // Trying update again fails
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_TRUE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u);

    // Trying to add a new root after update, before commit/revert fails
    err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_TRUE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u);

    // Trying to add new opcerts for any fabric after update, before commit/revert fails
    err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    err = opCertStore.AddNewOpCertsForFabric(kFabricIndex2, kTestNocSpan, kTestIcacSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    // Committing writes the new values (we even "background-remove" the old ICAC/NOC before commit)
    storageDelegate.SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricICAC(kFabricIndex1).KeyName());
    storageDelegate.SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricNOC(kFabricIndex1).KeyName());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 1u); //< Root remains

    err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u); //< All certs now committed

    // Should be able to get the committed cert even if not in persisted storage, after an API error
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacBufExists }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestNocSpan }));

    // Calling revert doesn't undo the work we just did
    opCertStore.RevertPendingOpCerts();
    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u); //< All certs now committed

    // Verify the revert after commit left all data alone
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacBufExists }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestNocSpan }));

    // Verify that RemoveOpCertsForFabric fails on fabric with no data
    err = opCertStore.RemoveOpCertsForFabric(kFabricIndex2);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u);

    // Verify that RemoveOpCertsForFabric works for fabric we just updated
    err = opCertStore.RemoveOpCertsForFabric(kFabricIndex1);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); // All keys gone

    opCertStore.Finish();
}

TEST_F(TestPersistentStorageOpCertStore, TestReverts)
{
    TestPersistentStorageDelegate storageDelegate;
    PersistentStorageOpCertStore opCertStore;

    // Failure before Init
    CHIP_ERROR err = opCertStore.RemoveOpCertsForFabric(kFabricIndex1);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u);

    // Init succeeds
    err = opCertStore.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Add a new pending trusted root
    uint8_t largeBuf[400];
    MutableByteSpan largeSpan{ largeBuf };

    err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
    EXPECT_TRUE(opCertStore.HasPendingRootCert());
    EXPECT_FALSE(opCertStore.HasPendingNocChain());

    // Verify we can see the new trusted root
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(largeSpan.data_equal(kTestRcacSpan));

    // Verify that after revert, we can't see the root anymore
    opCertStore.RevertPendingOpCerts();
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);

    {
        // Add new root again, to then test review of AddNewTrustedCertificates
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_FALSE(opCertStore.HasPendingNocChain());

        // Add NOC chain
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(opCertStore.HasPendingNocChain());
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
    }

    // Make sure we can see all pending certs before revert
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(kTestRcacSpan));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(kTestIcacSpan));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(kTestNocSpan));
    }

    // Revert
    opCertStore.RevertPendingOpCerts();
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase

    // Verify that after revert, we can't see the root or chain anymore
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);
    }

    // Start again to add a new set, but then let's commit
    {
        // Add new root again, to then test review of AddNewTrustedCertificates
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_FALSE(opCertStore.HasPendingNocChain());

        // Add NOC chain
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(opCertStore.HasPendingNocChain());
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
    }

    // Commiting new certs should succeed on correct fabric
    err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_FALSE(opCertStore.HasPendingRootCert());

    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u); //< All certs now committed

    // Should be able to get the committed certs
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestNocSpan }));
    }

    const uint8_t kNewNoc[] = { 'n', 'o', 'c', ' ', 'n', 'e', 'w' };

    // Updating certs should work (NO ICAC)
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, ByteSpan{ kNewNoc }, ByteSpan{});
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_TRUE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u); //< No change to keys

    // Should see committed root, pending NOC, absent ICAC
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);
        EXPECT_FALSE(opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kIcac));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kNewNoc }));
    }

    // Revert, should be back at previous state
    opCertStore.RevertPendingOpCerts();
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u); //< Storage count did not yet change

    // Should be able to get the previously committed certs
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestNocSpan }));
    }

    // Try again to update with missing ICAC and commit
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, ByteSpan{ kNewNoc }, ByteSpan{});
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_TRUE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u); //< No change to keys

    err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 2u); //< ICAC cert should be gone

    // Should see committed root, new NOC, absent ICAC
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));
        EXPECT_TRUE(opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kRcac));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);
        EXPECT_FALSE(opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kIcac));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kNewNoc }));
        EXPECT_TRUE(opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kNoc));
    }

    opCertStore.Finish();
}

TEST_F(TestPersistentStorageOpCertStore, TestRevertAddNoc)
{
    TestPersistentStorageDelegate storageDelegate;
    PersistentStorageOpCertStore opCertStore;

    // Init succeeds
    CHIP_ERROR err = opCertStore.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Add a new pending trusted root
    uint8_t largeBuf[400];
    MutableByteSpan largeSpan{ largeBuf };

    {
        // Add new root
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_FALSE(opCertStore.HasPendingNocChain());

        // Add NOC chain, with NO ICAC
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, ByteSpan{});
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(opCertStore.HasPendingNocChain());
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
    }

    // Make sure we get expected pending state before revert
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(kTestRcacSpan));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(kTestNocSpan));
    }

    // Revert using RemoveOpCertsForFabric
    err = opCertStore.RemoveOpCertsForFabric(kFabricIndex1);
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase

    // Add again, and commit
    {
        // Add new root
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_FALSE(opCertStore.HasPendingNocChain());

        // Add NOC chain, with NO ICAC
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, ByteSpan{});
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(opCertStore.HasPendingNocChain());
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase

        err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_FALSE(opCertStore.HasPendingRootCert());
        EXPECT_FALSE(opCertStore.HasPendingNocChain());
        EXPECT_EQ(storageDelegate.GetNumKeys(), 2u); //< We have RCAC, NOC, no ICAC
    }

    // Update to add an ICAC
    const uint8_t kNewIcac[] = { 'i', 'c', 'a', 'c', ' ', 'n', 'e', 'w' };
    const uint8_t kNewNoc[]  = { 'n', 'o', 'c', ' ', 'n', 'e', 'w' };

    // Updating certs should work (NO ICAC)
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, ByteSpan{ kNewNoc }, ByteSpan{ kNewIcac });
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_TRUE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 2u); //< No change to keys

    // Should see committed root, pending NOC, pending ICAC
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kNewIcac }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kNewNoc }));
    }

    // Commit, should see the new ICAC appear.
    err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opCertStore.HasPendingRootCert());
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 3u); //< We have RCAC, NOC, ICAC

    // Should see committed root, new NOC, new ICAC
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kNewIcac }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(ByteSpan{ kNewNoc }));
    }

    opCertStore.Finish();
}

TEST_F(TestPersistentStorageOpCertStore, TestRevertPendingOpCertsExceptRoot)
{
    TestPersistentStorageDelegate storageDelegate;
    PersistentStorageOpCertStore opCertStore;

    // Init succeeds
    CHIP_ERROR err = opCertStore.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Add a new pending trusted root
    uint8_t largeBuf[400];
    MutableByteSpan largeSpan{ largeBuf };

    {
        // Add new root
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_FALSE(opCertStore.HasPendingNocChain());

        // Add NOC chain, with NO ICAC
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, ByteSpan{});
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(opCertStore.HasPendingNocChain());
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
    }

    // Make sure we get expected pending state before revert
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(kTestRcacSpan));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(largeSpan.data_equal(kTestNocSpan));
    }

    // Revert using RevertPendingOpCertsExceptRoot
    opCertStore.RevertPendingOpCertsExceptRoot();
    EXPECT_TRUE(opCertStore.HasPendingRootCert());
    EXPECT_FALSE(opCertStore.HasPendingNocChain());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase

    // Add again, and commit
    {
        // Add new root: should fail, since it should still be pending
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_FALSE(opCertStore.HasPendingNocChain());

        // Add NOC chain, with NO ICAC
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, ByteSpan{});
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(opCertStore.HasPendingNocChain());
        EXPECT_TRUE(opCertStore.HasPendingRootCert());
        EXPECT_EQ(storageDelegate.GetNumKeys(), 0u); //< Storage count did not yet increase

        err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_FALSE(opCertStore.HasPendingRootCert());
        EXPECT_FALSE(opCertStore.HasPendingNocChain());
        EXPECT_EQ(storageDelegate.GetNumKeys(), 2u); //< We have RCAC, NOC, no ICAC
    }

    opCertStore.Finish();
}

} // namespace
