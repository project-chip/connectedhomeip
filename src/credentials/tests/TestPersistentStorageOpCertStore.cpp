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

#include <credentials/PersistentStorageOpCertStore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

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

void TestAddNocFlow(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    PersistentStorageOpCertStore opCertStore;

    // Failure before Init
    CHIP_ERROR err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0);

    // Init succeeds
    err = opCertStore.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Manually add existing root for the FabricIndex, should fail AddNewTrustedRootCertForFabric for
    // same fabric but succeed GetCertificate.
    const uint8_t kTestRcacBufExists[] = { 'r', 'c', 'a', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };

    err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricRCAC(kFabricIndex1).KeyName(), kTestRcacBufExists,
                                          sizeof(kTestRcacBufExists));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 1);
    NL_TEST_ASSERT(inSuite,
                   opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kRcac) == true); //< From manual add

    err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    uint8_t largeBuf[400];
    MutableByteSpan largeSpan{ largeBuf };

    err = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NOT_FOUND);

    largeSpan = MutableByteSpan{ largeBuf };

    err = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacBufExists }));

    // Adding root for another FabricIndex should work
    err = opCertStore.AddNewTrustedRootCertForFabric(kUndefinedFabricIndex, kTestRcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_FABRIC_INDEX);

    err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex2, kTestRcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 1); //< Storage count did not yet increase
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);

    // Should be able to read pending RCAC right away
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kRcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

    // Trying to commit with pending RCAC but no NOC should fail but leave everything as-is
    err = opCertStore.CommitOpCertsForFabric(kFabricIndex2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);

    // Trying to do AddNewOpCertsForFabric for fabric different that with pending RCAC should fail
    err = opCertStore.AddNewOpCertsForFabric(kOtherFabricIndex, kTestNocSpan, kTestIcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_FABRIC_INDEX);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);

    // Clear other bad cases from storage for now
    storageDelegate.ClearStorage();

    // Trying to do AddNewOpCertsForFabric for same fabric as that with pending RCAC should fail
    // if there are already existing NOC chain elements for the given fabric.
    const uint8_t kTestIcacBufExists[] = { 'i', 'c', 'a', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };
    const uint8_t kTestNocBufExists[]  = { 'n', 'o', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };

    err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricICAC(kFabricIndex2).KeyName(), kTestIcacBufExists,
                                          sizeof(kTestIcacBufExists));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 1);

    err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricNOC(kFabricIndex2).KeyName(), kTestNocBufExists,
                                          sizeof(kTestNocBufExists));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 2);

    NL_TEST_ASSERT(inSuite,
                   opCertStore.HasCertificateForFabric(kFabricIndex2, CertChainElement::kIcac) == true);         //< From manual add
    NL_TEST_ASSERT(inSuite, opCertStore.HasCertificateForFabric(kFabricIndex2, CertChainElement::kNoc) == true); //< From manual add

    err = opCertStore.AddNewOpCertsForFabric(kFabricIndex2, kTestNocSpan, kTestIcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);

    storageDelegate.SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricICAC(kFabricIndex2).KeyName());
    storageDelegate.SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricNOC(kFabricIndex2).KeyName());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0);

    // Trying to do AddNewOpCertsForFabric for same fabric as that with pending RCAC should succeed
    err = opCertStore.AddNewOpCertsForFabric(kFabricIndex2, kTestNocSpan, kTestIcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase

    // Should be able to get the pending cert even if not in persisted storage
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kIcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kNoc, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestNocSpan }));

    // Trying to do AddNewOpCertsForFabric a second time after success before commit should fail,
    // but leave state as-is
    err = opCertStore.AddNewOpCertsForFabric(kFabricIndex2, kTestNocSpan, kTestIcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);

    // Should be able to get the pending cert even if not in persisted storage, after an API error
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kIcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kNoc, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestNocSpan }));

    // Trying to commit with wrong FabricIndex should fail
    err = opCertStore.CommitOpCertsForFabric(kOtherFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_FABRIC_INDEX);
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);

    // Commiting new certs should succeed on correct fabric
    err = opCertStore.CommitOpCertsForFabric(kFabricIndex2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());

    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3); //< All certs now committed

    // Should be able to get the committed certs
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kRcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kIcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex2, CertChainElement::kNoc, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestNocSpan }));

    opCertStore.Finish();
}

void TestUpdateNocFlow(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    PersistentStorageOpCertStore opCertStore;

    // Failure before Init
    CHIP_ERROR err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0);

    // Init succeeds
    err = opCertStore.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Add a new pending trusted root to test for UpdateOpCertsForFabric failure on new root present
    err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());

    // Trying to do an UpdateOpCertsForFabric with new root pending should fail
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());

    // Revert state for next tests
    opCertStore.RevertPendingOpCerts();
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase

    // Manually add root, ICAC and NOC to validate update since existing chain required
    const uint8_t kTestRcacBufExists[] = { 'r', 'c', 'a', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };
    const uint8_t kTestIcacBufExists[] = { 'i', 'c', 'a', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };
    const uint8_t kTestNocBufExists[]  = { 'n', 'o', 'c', ' ', 'e', 'x', 'i', 's', 't', 's' };

    uint8_t largeBuf[400];
    MutableByteSpan largeSpan{ largeBuf };

    {
        err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricRCAC(kFabricIndex1).KeyName(), kTestRcacBufExists,
                                              sizeof(kTestRcacBufExists));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 1);
        NL_TEST_ASSERT(inSuite,
                       opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kRcac) == true); //< From manual add

        err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricICAC(kFabricIndex1).KeyName(), kTestIcacBufExists,
                                              sizeof(kTestIcacBufExists));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 2);
        NL_TEST_ASSERT(inSuite,
                       opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kIcac) == true); //< From manual add

        err = storageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::FabricNOC(kFabricIndex1).KeyName(), kTestNocBufExists,
                                              sizeof(kTestNocBufExists));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3);
        NL_TEST_ASSERT(inSuite,
                       opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kNoc) == true); //< From manual add

        // Test that we can manually stored certs
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacBufExists }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestIcacBufExists }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestNocBufExists }));
    }

    // Update fails on fabric with wrong FabricIndex
    err = opCertStore.UpdateOpCertsForFabric(kOtherFabricIndex, kTestNocSpan, kTestIcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());

    // Update succeeds on fabric with existing data
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain() == true);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3);

    // Can read back existing root unchanged
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacBufExists }));

    // NOC chain elements see the pending updated certs
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestIcacBuf }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestNocBuf }));

    // Trying update again fails
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain() == true);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3);

    // Trying to add a new root after update, before commit/revert fails
    err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain() == true);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3);

    // Trying to add new opcerts for any fabric after update, before commit/revert fails
    err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    err = opCertStore.AddNewOpCertsForFabric(kFabricIndex2, kTestNocSpan, kTestIcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    // Committing writes the new values (we even "background-remove" the old ICAC/NOC before commit)
    storageDelegate.SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricICAC(kFabricIndex1).KeyName());
    storageDelegate.SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricNOC(kFabricIndex1).KeyName());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 1); //< Root remains

    err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3); //< All certs now committed

    // Should be able to get the committed cert even if not in persisted storage, after an API error
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacBufExists }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestNocSpan }));

    // Calling revert doesn't undo the work we just did
    opCertStore.RevertPendingOpCerts();
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3); //< All certs now committed

    // Verify the revert after commit left all data alone
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacBufExists }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestNocSpan }));

    // Verify that RemoveOpCertsForFabric fails on fabric with no data
    err = opCertStore.RemoveOpCertsForFabric(kFabricIndex2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_FABRIC_INDEX);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3);

    // Verify that RemoveOpCertsForFabric works for fabric we just updated
    err = opCertStore.RemoveOpCertsForFabric(kFabricIndex1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); // All keys gone

    opCertStore.Finish();
}

void TestReverts(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    PersistentStorageOpCertStore opCertStore;

    // Failure before Init
    CHIP_ERROR err = opCertStore.RemoveOpCertsForFabric(kFabricIndex1);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0);

    // Init succeeds
    err = opCertStore.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Add a new pending trusted root
    uint8_t largeBuf[400];
    MutableByteSpan largeSpan{ largeBuf };

    err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());

    // Verify we can see the new trusted root
    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, largeSpan.data_equal(kTestRcacSpan));

    // Verify that after revert, we can't see the root anymore
    opCertStore.RevertPendingOpCerts();
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase

    largeSpan = MutableByteSpan{ largeBuf };
    err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NOT_FOUND);

    {
        // Add new root again, to then test review of AddNewTrustedCertificates
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());

        // Add NOC chain
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
    }

    // Make sure we can see all pending certs before revert
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(kTestRcacSpan));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(kTestIcacSpan));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(kTestNocSpan));
    }

    // Revert
    opCertStore.RevertPendingOpCerts();
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase

    // Verify that after revert, we can't see the root or chain anymore
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NOT_FOUND);

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NOT_FOUND);

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NOT_FOUND);
    }

    // Start again to add a new set, but then let's commit
    {
        // Add new root again, to then test review of AddNewTrustedCertificates
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());

        // Add NOC chain
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, kTestIcacSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
    }

    // Commiting new certs should succeed on correct fabric
    err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());

    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3); //< All certs now committed

    // Should be able to get the committed certs
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestNocSpan }));
    }

    const uint8_t kNewNoc[] = { 'n', 'o', 'c', ' ', 'n', 'e', 'w' };

    // Updating certs should work (NO ICAC)
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, ByteSpan{ kNewNoc }, ByteSpan{});
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3); //< No change to keys

    // Should see committed root, pending NOC, absent ICAC
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NOT_FOUND);
        NL_TEST_ASSERT(inSuite, !opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kIcac));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kNewNoc }));
    }

    // Revert, should be back at previous state
    opCertStore.RevertPendingOpCerts();
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3); //< Storage count did not yet change

    // Should be able to get the previously committed certs
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestIcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestNocSpan }));
    }

    // Try again to update with missing ICAC and commit
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, ByteSpan{ kNewNoc }, ByteSpan{});
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3); //< No change to keys

    err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 2); //< ICAC cert should be gone

    // Should see committed root, new NOC, absent ICAC
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));
        NL_TEST_ASSERT(inSuite, opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kRcac));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NOT_FOUND);
        NL_TEST_ASSERT(inSuite, !opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kIcac));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kNewNoc }));
        NL_TEST_ASSERT(inSuite, opCertStore.HasCertificateForFabric(kFabricIndex1, CertChainElement::kNoc));
    }

    opCertStore.Finish();
}

void TestRevertAddNoc(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    PersistentStorageOpCertStore opCertStore;

    // Init succeeds
    CHIP_ERROR err = opCertStore.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Add a new pending trusted root
    uint8_t largeBuf[400];
    MutableByteSpan largeSpan{ largeBuf };

    {
        // Add new root
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());

        // Add NOC chain, with NO ICAC
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, ByteSpan{});
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
    }

    // Make sure we get expected pending state before revert
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(kTestRcacSpan));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NOT_FOUND);

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(kTestNocSpan));
    }

    // Revert using RemoveOpCertsForFabric
    err = opCertStore.RemoveOpCertsForFabric(kFabricIndex1);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase

    // Add again, and commit
    {
        // Add new root
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());

        // Add NOC chain, with NO ICAC
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, ByteSpan{});
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase

        err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
        NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 2); //< We have RCAC, NOC, no ICAC
    }

    // Update to add an ICAC
    const uint8_t kNewIcac[] = { 'i', 'c', 'a', 'c', ' ', 'n', 'e', 'w' };
    const uint8_t kNewNoc[]  = { 'n', 'o', 'c', ' ', 'n', 'e', 'w' };

    // Updating certs should work (NO ICAC)
    err = opCertStore.UpdateOpCertsForFabric(kFabricIndex1, ByteSpan{ kNewNoc }, ByteSpan{ kNewIcac });
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 2); //< No change to keys

    // Should see committed root, pending NOC, pending ICAC
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kNewIcac }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kNewNoc }));
    }

    // Commit, should see the new ICAC appear.
    err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 3); //< We have RCAC, NOC, ICAC

    // Should see committed root, new NOC, new ICAC
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kTestRcacSpan }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kNewIcac }));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(ByteSpan{ kNewNoc }));
    }

    opCertStore.Finish();
}

void TestRevertPendingOpCertsExceptRoot(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    PersistentStorageOpCertStore opCertStore;

    // Init succeeds
    CHIP_ERROR err = opCertStore.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Add a new pending trusted root
    uint8_t largeBuf[400];
    MutableByteSpan largeSpan{ largeBuf };

    {
        // Add new root
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());

        // Add NOC chain, with NO ICAC
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, ByteSpan{});
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
    }

    // Make sure we get expected pending state before revert
    {
        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kRcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(kTestRcacSpan));

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kIcac, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NOT_FOUND);

        largeSpan = MutableByteSpan{ largeBuf };
        err       = opCertStore.GetCertificate(kFabricIndex1, CertChainElement::kNoc, largeSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, largeSpan.data_equal(kTestNocSpan));
    }

    // Revert using RevertPendingOpCertsExceptRoot
    opCertStore.RevertPendingOpCertsExceptRoot();
    NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
    NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
    NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase

    // Add again, and commit
    {
        // Add new root: should fail, since it should still be pending
        err = opCertStore.AddNewTrustedRootCertForFabric(kFabricIndex1, kTestRcacSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());

        // Add NOC chain, with NO ICAC
        err = opCertStore.AddNewOpCertsForFabric(kFabricIndex1, kTestNocSpan, ByteSpan{});
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingNocChain());
        NL_TEST_ASSERT(inSuite, opCertStore.HasPendingRootCert() == true);
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 0); //< Storage count did not yet increase

        err = opCertStore.CommitOpCertsForFabric(kFabricIndex1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingRootCert());
        NL_TEST_ASSERT(inSuite, !opCertStore.HasPendingNocChain());
        NL_TEST_ASSERT(inSuite, storageDelegate.GetNumKeys() == 2); //< We have RCAC, NOC, no ICAC
    }

    opCertStore.Finish();
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("Test AddNOC-like flows PersistentStorageOpCertStore", TestAddNocFlow),
    NL_TEST_DEF("Test UpdateNOC-like flows PersistentStorageOpCertStore", TestUpdateNocFlow),
    NL_TEST_DEF("Test revert operations of PersistentStorageOpCertStore", TestReverts),
    NL_TEST_DEF("Test revert operations with AddNOC of PersistentStorageOpCertStore", TestRevertAddNoc),
    NL_TEST_DEF("Test revert operations using RevertPendingOpCertsExceptRoot", TestRevertPendingOpCertsExceptRoot),
    NL_TEST_SENTINEL()
};

/**
 *  Set up the test suite.
 */
int Test_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    VerifyOrReturnError(error == CHIP_NO_ERROR, FAILURE);
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int Test_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

/**
 *  Main
 */
int TestPersistentStorageOpCertStore()
{
    nlTestSuite theSuite = { "PersistentStorageOpCertStore tests", &sTests[0], Test_Setup, Test_Teardown };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPersistentStorageOpCertStore)
