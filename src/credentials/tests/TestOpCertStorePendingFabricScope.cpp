/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *      The operational certificate store keeps a single set of pending certificates, owned by
 *      one fabric index at a time. Removing the certificates of a different fabric index must
 *      leave that pending set intact, whether or not the index being removed holds a fabric.
 */

#include <pw_unit_test/framework.h>

#include <credentials/FabricTable.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/TestOnlyLocalCertificateAuthority.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

using namespace chip;
using namespace chip::Credentials;
using CertChainElement = OperationalCertificateStore::CertChainElement;

namespace {

constexpr FabricIndex kOtherFabricIndex      = 2;
constexpr FabricIndex kUnoccupiedFabricIndex = 7;

// The store does not inspect certificate contents, so constants are enough at the store level.
const uint8_t kRcacBuf[]  = { 'r', 'c', 'a', 'c' };
const uint8_t kNocBuf[]   = { 'n', 'o', 'c' };
const uint8_t kIcacBuf[]  = { 'i', 'c', 'a', 'c' };
const uint8_t kRcac2Buf[] = { 'r', 'c', 'a', 'c', '2' };

const ByteSpan kRcacSpan{ kRcacBuf };
const ByteSpan kNocSpan{ kNocBuf };
const ByteSpan kIcacSpan{ kIcacBuf };
const ByteSpan kRcac2Span{ kRcac2Buf };

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
    Credentials::PersistentStorageOpCertStore & GetOpCertStore() { return mOpCertStore; }

private:
    FabricTable mFabricTable;
    PersistentStorageOperationalKeystore mOpKeyStore;
    Credentials::PersistentStorageOpCertStore mOpCertStore;
};

struct TestOpCertStorePendingFabricScope : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// An index that holds nothing, pending or persisted, must be rejected, and the pending root of
// the fabric that does own the pending set must survive the call.
TEST_F(TestOpCertStorePendingFabricScope, RemoveForUnoccupiedIndexKeepsPendingRootOfOtherFabric)
{
    chip::TestPersistentStorageDelegate storage;
    PersistentStorageOpCertStore opCertStore;
    ASSERT_SUCCESS(opCertStore.Init(&storage));

    ASSERT_SUCCESS(opCertStore.AddNewTrustedRootCertForFabric(kOtherFabricIndex, kRcacSpan));
    ASSERT_TRUE(opCertStore.HasPendingRootCert());

    EXPECT_EQ(opCertStore.RemoveOpCertsForFabric(kUnoccupiedFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    EXPECT_TRUE(opCertStore.HasPendingRootCert());

    uint8_t certBuf[kMaxCHIPCertLength];
    MutableByteSpan certSpan{ certBuf };
    EXPECT_SUCCESS(opCertStore.GetCertificate(kOtherFabricIndex, CertChainElement::kRcac, certSpan));
    EXPECT_TRUE(certSpan.data_equal(kRcacSpan));

    opCertStore.RevertPendingOpCerts();
    opCertStore.Finish();
}

// Removing an index that does hold persisted certificates is a legitimate removal and must
// succeed, but it still must not discard the pending set owned by a different fabric index.
TEST_F(TestOpCertStorePendingFabricScope, RemoveForOccupiedIndexKeepsPendingRootOfOtherFabric)
{
    constexpr FabricIndex kRemovedFabricIndex = 1;

    chip::TestPersistentStorageDelegate storage;
    PersistentStorageOpCertStore opCertStore;
    ASSERT_SUCCESS(opCertStore.Init(&storage));

    // Give kRemovedFabricIndex a fully persisted chain of its own.
    ASSERT_SUCCESS(opCertStore.AddNewTrustedRootCertForFabric(kRemovedFabricIndex, kRcacSpan));
    ASSERT_SUCCESS(opCertStore.AddNewOpCertsForFabric(kRemovedFabricIndex, kNocSpan, kIcacSpan));
    ASSERT_SUCCESS(opCertStore.CommitOpCertsForFabric(kRemovedFabricIndex));
    ASSERT_FALSE(opCertStore.HasPendingRootCert());
    ASSERT_TRUE(opCertStore.HasCertificateForFabric(kRemovedFabricIndex, CertChainElement::kNoc));

    // A different fabric now owns the pending set.
    ASSERT_SUCCESS(opCertStore.AddNewTrustedRootCertForFabric(kOtherFabricIndex, kRcac2Span));
    ASSERT_TRUE(opCertStore.HasPendingRootCert());

    EXPECT_SUCCESS(opCertStore.RemoveOpCertsForFabric(kRemovedFabricIndex));

    EXPECT_FALSE(opCertStore.HasCertificateForFabric(kRemovedFabricIndex, CertChainElement::kNoc));
    EXPECT_TRUE(opCertStore.HasPendingRootCert());

    uint8_t certBuf[kMaxCHIPCertLength];
    MutableByteSpan certSpan{ certBuf };
    EXPECT_SUCCESS(opCertStore.GetCertificate(kOtherFabricIndex, CertChainElement::kRcac, certSpan));
    EXPECT_TRUE(certSpan.data_equal(kRcac2Span));

    opCertStore.RevertPendingOpCerts();
    opCertStore.Finish();
}

// A fabric with an update in flight must survive a removal that names an index holding no fabric,
// and the update must still commit.
TEST_F(TestOpCertStorePendingFabricScope, DeleteOfUnoccupiedIndexDoesNotDisturbFabricUnderUpdate)
{
    constexpr uint16_t kVendorId   = 0xFFF1u;
    constexpr FabricId kFabricId   = 44;
    constexpr NodeId kNodeIdBefore = 999;
    constexpr NodeId kNodeIdAfter  = 1000;

    Credentials::TestOnlyLocalCertificateAuthority certAuthority;
    ASSERT_TRUE(certAuthority.Init().IsSuccess());

    chip::TestPersistentStorageDelegate storage;
    ScopedFabricTable fabricTableHolder;
    ASSERT_SUCCESS(fabricTableHolder.Init(&storage));
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    FabricIndex updatedIndex = kUndefinedFabricIndex;

    {
        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        ASSERT_SUCCESS(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));
        ASSERT_SUCCESS(certAuthority.SetIncludeIcac(true).GenerateNocChain(kFabricId, kNodeIdBefore, csrSpan).GetStatus());
        ASSERT_SUCCESS(fabricTable.AddNewPendingTrustedRootCert(certAuthority.GetRcac()));
        ASSERT_SUCCESS(fabricTable.AddNewPendingFabricWithOperationalKeystore(certAuthority.GetNoc(), certAuthority.GetIcac(),
                                                                              kVendorId, &updatedIndex));
        ASSERT_SUCCESS(fabricTable.CommitPendingFabricData());
    }

    ASSERT_EQ(fabricTable.FabricCount(), 1);
    ASSERT_NE(updatedIndex, kUnoccupiedFabricIndex);
    ASSERT_EQ(fabricTable.FindFabricWithIndex(kUnoccupiedFabricIndex), nullptr);

    {
        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        ASSERT_SUCCESS(fabricTable.AllocatePendingOperationalKey(chip::MakeOptional(updatedIndex), csrSpan));
        ASSERT_SUCCESS(certAuthority.SetIncludeIcac(false).GenerateNocChain(kFabricId, kNodeIdAfter, csrSpan).GetStatus());
        ASSERT_SUCCESS(fabricTable.UpdatePendingFabricWithOperationalKeystore(updatedIndex, certAuthority.GetNoc(), ByteSpan{}));
    }

    EXPECT_EQ(fabricTable.Delete(kUnoccupiedFabricIndex), CHIP_ERROR_NOT_FOUND);

    EXPECT_SUCCESS(fabricTable.CommitPendingFabricData());
    EXPECT_EQ(fabricTable.FabricCount(), 1);

    const FabricInfo * fabricInfo = fabricTable.FindFabricWithIndex(updatedIndex);
    ASSERT_NE(fabricInfo, nullptr);
    EXPECT_EQ(fabricInfo->GetNodeId(), kNodeIdAfter);

    uint8_t certBuf[kMaxCHIPCertLength];
    MutableByteSpan certSpan{ certBuf };
    EXPECT_SUCCESS(fabricTableHolder.GetOpCertStore().GetCertificate(updatedIndex, CertChainElement::kNoc, certSpan));
}

// The destructive case: a second fabric is genuinely removed while a commissioned fabric has an
// UpdateNOC in flight. Before the pending set was scoped to its owning index, the removal cleared
// the updating fabric's pending certificates while FabricTable kept its pending flags, so
// CommitPendingFabricData passed every pre-flight, committed the metadata, then failed in
// CommitOpCertsForFabric and deleted the already-commissioned fabric.
TEST_F(TestOpCertStorePendingFabricScope, DeleteOfOccupiedIndexDoesNotDisturbFabricUnderUpdate)
{
    constexpr uint16_t kVendorId    = 0xFFF1u;
    constexpr FabricId kFabricIdOne = 44;
    constexpr FabricId kFabricIdTwo = 45;
    constexpr NodeId kNodeIdBefore  = 999;
    constexpr NodeId kNodeIdAfter   = 1000;
    constexpr NodeId kNodeIdRemoved = 1001;

    Credentials::TestOnlyLocalCertificateAuthority certAuthorityOne;
    Credentials::TestOnlyLocalCertificateAuthority certAuthorityTwo;
    ASSERT_TRUE(certAuthorityOne.Init().IsSuccess());
    ASSERT_TRUE(certAuthorityTwo.Init().IsSuccess());

    chip::TestPersistentStorageDelegate storage;
    ScopedFabricTable fabricTableHolder;
    ASSERT_SUCCESS(fabricTableHolder.Init(&storage));
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    FabricIndex updatedIndex = kUndefinedFabricIndex;
    FabricIndex removedIndex = kUndefinedFabricIndex;

    // Commission the fabric that will later be updated.
    {
        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        ASSERT_SUCCESS(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));
        ASSERT_SUCCESS(certAuthorityOne.SetIncludeIcac(true).GenerateNocChain(kFabricIdOne, kNodeIdBefore, csrSpan).GetStatus());
        ASSERT_SUCCESS(fabricTable.AddNewPendingTrustedRootCert(certAuthorityOne.GetRcac()));
        ASSERT_SUCCESS(fabricTable.AddNewPendingFabricWithOperationalKeystore(certAuthorityOne.GetNoc(), certAuthorityOne.GetIcac(),
                                                                              kVendorId, &updatedIndex));
        ASSERT_SUCCESS(fabricTable.CommitPendingFabricData());
    }

    // Commission a second, unrelated fabric under its own root.
    {
        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        ASSERT_SUCCESS(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));
        ASSERT_SUCCESS(certAuthorityTwo.SetIncludeIcac(true).GenerateNocChain(kFabricIdTwo, kNodeIdRemoved, csrSpan).GetStatus());
        ASSERT_SUCCESS(fabricTable.AddNewPendingTrustedRootCert(certAuthorityTwo.GetRcac()));
        ASSERT_SUCCESS(fabricTable.AddNewPendingFabricWithOperationalKeystore(certAuthorityTwo.GetNoc(), certAuthorityTwo.GetIcac(),
                                                                              kVendorId, &removedIndex));
        ASSERT_SUCCESS(fabricTable.CommitPendingFabricData());
    }

    ASSERT_EQ(fabricTable.FabricCount(), 2);
    ASSERT_NE(updatedIndex, removedIndex);

    // Start an UpdateNOC on the first fabric and leave it pending.
    {
        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        ASSERT_SUCCESS(fabricTable.AllocatePendingOperationalKey(chip::MakeOptional(updatedIndex), csrSpan));
        ASSERT_SUCCESS(certAuthorityOne.SetIncludeIcac(false).GenerateNocChain(kFabricIdOne, kNodeIdAfter, csrSpan).GetStatus());
        ASSERT_SUCCESS(fabricTable.UpdatePendingFabricWithOperationalKeystore(updatedIndex, certAuthorityOne.GetNoc(), ByteSpan{}));
    }

    // The unrelated fabric is legitimately removed mid-update. This must succeed and must not touch
    // the pending set owned by the fabric being updated.
    EXPECT_SUCCESS(fabricTable.Delete(removedIndex));
    EXPECT_EQ(fabricTable.FabricCount(), 1);
    EXPECT_EQ(fabricTable.FindFabricWithIndex(removedIndex), nullptr);

    // The update must still commit, and the updated fabric must survive with its new identity.
    EXPECT_SUCCESS(fabricTable.CommitPendingFabricData());
    EXPECT_EQ(fabricTable.FabricCount(), 1);

    const FabricInfo * fabricInfo = fabricTable.FindFabricWithIndex(updatedIndex);
    ASSERT_NE(fabricInfo, nullptr);
    EXPECT_EQ(fabricInfo->GetNodeId(), kNodeIdAfter);
    EXPECT_EQ(fabricInfo->GetFabricId(), kFabricIdOne);

    uint8_t certBuf[kMaxCHIPCertLength];
    MutableByteSpan certSpan{ certBuf };
    EXPECT_SUCCESS(fabricTableHolder.GetOpCertStore().GetCertificate(updatedIndex, CertChainElement::kNoc, certSpan));
}

// The pending VID verification elements are staged in the same slot as the pending certificates and
// are cleared by the same revert, so they must be protected by the same index scoping. They can only
// exist while mPendingFabricIndex is set, which is why an unrelated removal must leave them alone.
TEST_F(TestOpCertStorePendingFabricScope, DeleteOfOccupiedIndexKeepsPendingVidVerificationOfFabricUnderUpdate)
{
    constexpr uint16_t kVendorId    = 0xFFF1u;
    constexpr FabricId kFabricIdOne = 44;
    constexpr FabricId kFabricIdTwo = 45;
    constexpr NodeId kNodeIdBefore  = 999;
    constexpr NodeId kNodeIdAfter   = 1000;
    constexpr NodeId kNodeIdRemoved = 1001;

    // Contents are opaque to the store, so a fixed statement of the mandated length is enough.
    const uint8_t kVidVerificationStatementBuf[Crypto::kVendorIdVerificationStatementV1Size] = { 1, 2, 3 };
    ByteSpan kVidVerificationStatementSpan{ kVidVerificationStatementBuf };

    Credentials::TestOnlyLocalCertificateAuthority certAuthorityOne;
    Credentials::TestOnlyLocalCertificateAuthority certAuthorityTwo;
    ASSERT_TRUE(certAuthorityOne.Init().IsSuccess());
    ASSERT_TRUE(certAuthorityTwo.Init().IsSuccess());

    chip::TestPersistentStorageDelegate storage;
    ScopedFabricTable fabricTableHolder;
    ASSERT_SUCCESS(fabricTableHolder.Init(&storage));
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    FabricIndex updatedIndex = kUndefinedFabricIndex;
    FabricIndex removedIndex = kUndefinedFabricIndex;

    {
        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        ASSERT_SUCCESS(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));
        ASSERT_SUCCESS(certAuthorityOne.SetIncludeIcac(true).GenerateNocChain(kFabricIdOne, kNodeIdBefore, csrSpan).GetStatus());
        ASSERT_SUCCESS(fabricTable.AddNewPendingTrustedRootCert(certAuthorityOne.GetRcac()));
        ASSERT_SUCCESS(fabricTable.AddNewPendingFabricWithOperationalKeystore(certAuthorityOne.GetNoc(), certAuthorityOne.GetIcac(),
                                                                              kVendorId, &updatedIndex));
        ASSERT_SUCCESS(fabricTable.CommitPendingFabricData());
    }

    {
        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        ASSERT_SUCCESS(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));
        ASSERT_SUCCESS(certAuthorityTwo.SetIncludeIcac(true).GenerateNocChain(kFabricIdTwo, kNodeIdRemoved, csrSpan).GetStatus());
        ASSERT_SUCCESS(fabricTable.AddNewPendingTrustedRootCert(certAuthorityTwo.GetRcac()));
        ASSERT_SUCCESS(fabricTable.AddNewPendingFabricWithOperationalKeystore(certAuthorityTwo.GetNoc(), certAuthorityTwo.GetIcac(),
                                                                              kVendorId, &removedIndex));
        ASSERT_SUCCESS(fabricTable.CommitPendingFabricData());
    }

    ASSERT_EQ(fabricTable.FabricCount(), 2);

    // Start the update and stage a VID verification statement inside the same pending window.
    {
        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        ASSERT_SUCCESS(fabricTable.AllocatePendingOperationalKey(chip::MakeOptional(updatedIndex), csrSpan));
        ASSERT_SUCCESS(certAuthorityOne.SetIncludeIcac(false).GenerateNocChain(kFabricIdOne, kNodeIdAfter, csrSpan).GetStatus());
        ASSERT_SUCCESS(fabricTable.UpdatePendingFabricWithOperationalKeystore(updatedIndex, certAuthorityOne.GetNoc(), ByteSpan{}));
    }

    OperationalCertificateStore & opCertStore = fabricTableHolder.GetOpCertStore();
    ASSERT_SUCCESS(opCertStore.UpdateVidVerificationStatementForFabric(updatedIndex, kVidVerificationStatementSpan));

    EXPECT_SUCCESS(fabricTable.Delete(removedIndex));

    EXPECT_SUCCESS(fabricTable.CommitPendingFabricData());

    uint8_t vvsBuf[Crypto::kVendorIdVerificationStatementV1Size];
    MutableByteSpan vvsSpan{ vvsBuf };
    EXPECT_SUCCESS(opCertStore.GetVidVerificationElement(
        updatedIndex, OperationalCertificateStore::VidVerificationElement::kVidVerificationStatement, vvsSpan));
    EXPECT_TRUE(vvsSpan.data_equal(kVidVerificationStatementSpan));
}

// Removing an existing fabric while a different fabric is being added must not interfere with the
// addition in progress.
TEST_F(TestOpCertStorePendingFabricScope, DeleteOfExistingFabricDoesNotDisturbFabricBeingAdded)
{
    constexpr uint16_t kVendorId    = 0xFFF1u;
    constexpr FabricId kFabricIdOne = 44;
    constexpr FabricId kFabricIdTwo = 45;
    constexpr NodeId kNodeIdOne     = 999;
    constexpr NodeId kNodeIdTwo     = 1000;

    Credentials::TestOnlyLocalCertificateAuthority certAuthorityOne;
    Credentials::TestOnlyLocalCertificateAuthority certAuthorityTwo;
    ASSERT_TRUE(certAuthorityOne.Init().IsSuccess());
    ASSERT_TRUE(certAuthorityTwo.Init().IsSuccess());

    chip::TestPersistentStorageDelegate storage;
    ScopedFabricTable fabricTableHolder;
    ASSERT_SUCCESS(fabricTableHolder.Init(&storage));
    FabricTable & fabricTable = fabricTableHolder.GetFabricTable();

    FabricIndex firstIndex = kUndefinedFabricIndex;

    {
        uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
        MutableByteSpan csrSpan{ csrBuf };
        ASSERT_SUCCESS(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));
        ASSERT_SUCCESS(certAuthorityOne.SetIncludeIcac(true).GenerateNocChain(kFabricIdOne, kNodeIdOne, csrSpan).GetStatus());
        ASSERT_SUCCESS(fabricTable.AddNewPendingTrustedRootCert(certAuthorityOne.GetRcac()));
        ASSERT_SUCCESS(fabricTable.AddNewPendingFabricWithOperationalKeystore(certAuthorityOne.GetNoc(), certAuthorityOne.GetIcac(),
                                                                              kVendorId, &firstIndex));
        ASSERT_SUCCESS(fabricTable.CommitPendingFabricData());
    }

    ASSERT_EQ(fabricTable.FabricCount(), 1);

    // A second fabric starts being added: the trusted root is staged first.
    uint8_t csrBuf[chip::Crypto::kMIN_CSR_Buffer_Size];
    MutableByteSpan csrSpan{ csrBuf };
    ASSERT_SUCCESS(fabricTable.AllocatePendingOperationalKey(chip::NullOptional, csrSpan));
    ASSERT_SUCCESS(certAuthorityTwo.SetIncludeIcac(true).GenerateNocChain(kFabricIdTwo, kNodeIdTwo, csrSpan).GetStatus());
    ASSERT_SUCCESS(fabricTable.AddNewPendingTrustedRootCert(certAuthorityTwo.GetRcac()));

    // The first fabric is removed while that addition is still in progress.
    EXPECT_SUCCESS(fabricTable.Delete(firstIndex));

    FabricIndex secondIndex = kUndefinedFabricIndex;
    EXPECT_SUCCESS(fabricTable.AddNewPendingFabricWithOperationalKeystore(certAuthorityTwo.GetNoc(), certAuthorityTwo.GetIcac(),
                                                                          kVendorId, &secondIndex));
    EXPECT_SUCCESS(fabricTable.CommitPendingFabricData());

    const FabricInfo * fabricInfo = fabricTable.FindFabricWithIndex(secondIndex);
    ASSERT_NE(fabricInfo, nullptr);
    EXPECT_EQ(fabricInfo->GetNodeId(), kNodeIdTwo);
    EXPECT_EQ(fabricInfo->GetFabricId(), kFabricIdTwo);
}

} // namespace
