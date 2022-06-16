/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 * @brief Defines a table of fabrics that have provisioned the device.
 */

#pragma once

#include <algorithm>

#include <app/util/basic-types.h>
#include <credentials/CHIPCert.h>
#include <credentials/CHIPCertificateSet.h>
#include <credentials/CertificateValidityPolicy.h>
#include <credentials/LastKnownGoodTime.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/Optional.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Span.h>

namespace chip {

static constexpr uint8_t kFabricLabelMaxLengthInBytes = 32;

static_assert(kUndefinedFabricIndex < chip::kMinValidFabricIndex, "Undefined fabric index should not be valid");

/**
 * Provides access to the core metadata for a given fabric to which a node is joined.
 */
class DLL_EXPORT FabricInfo
{
public:
    FabricInfo() { Reset(); }

    // Returns a span into our internal storage.
    CharSpan GetFabricLabel() const { return CharSpan(mFabricLabel, strnlen(mFabricLabel, kFabricLabelMaxLengthInBytes)); }

    CHIP_ERROR SetFabricLabel(const CharSpan & fabricLabel);

    ~FabricInfo()
    {
        if (!mHasExternallyOwnedOperationalKey && mOperationalKey != nullptr)
        {
            chip::Platform::Delete(mOperationalKey);
        }
        ReleaseOperationalCerts();
    }

    NodeId GetNodeId() const { return mOperationalId.GetNodeId(); }
    ScopedNodeId GetScopedNodeId() const { return ScopedNodeId(mOperationalId.GetNodeId(), mFabricIndex); }
    ScopedNodeId GetScopedNodeIdForNode(const NodeId node) const { return ScopedNodeId(node, mFabricIndex); }
    // TODO(#15049): Refactor/rename PeerId to OperationalId or OpId throughout source
    PeerId GetPeerId() const { return mOperationalId; }
    PeerId GetPeerIdForNode(const NodeId node) const
    {
        PeerId peer = mOperationalId;
        peer.SetNodeId(node);
        return peer;
    }

    FabricId GetFabricId() const { return mFabricId; }
    FabricIndex GetFabricIndex() const { return mFabricIndex; }

    CompressedFabricId GetCompressedId() const { return mOperationalId.GetCompressedFabricId(); }

    CHIP_ERROR GetCompressedId(MutableByteSpan & compressedFabricId) const
    {
        ReturnErrorCodeIf(compressedFabricId.size() != sizeof(uint64_t), CHIP_ERROR_INVALID_ARGUMENT);
        Encoding::BigEndian::Put64(compressedFabricId.data(), GetCompressedId());
        return CHIP_NO_ERROR;
    }

    uint16_t GetVendorId() const { return mVendorId; }

    void SetVendorId(uint16_t vendorId) { mVendorId = vendorId; }

    /**
     * Sets the P256Keypair used for this fabric.  This will make a copy of the keypair
     * via the P256Keypair::Serialize and P256Keypair::Deserialize methods.
     *
     * The keyPair argument is safe to deallocate once this method returns.
     *
     * If your P256Keypair does not support serialization, use the
     * `SetExternallyOwnedOperationalKeypair` method instead.
     */
    CHIP_ERROR SetOperationalKeypair(const Crypto::P256Keypair * keyPair);

    /**
     * Sets the P256Keypair used for this fabric, delegating ownership of the
     * key to the caller. The P256Keypair provided here must be freed later by
     * the caller of this method if it was allocated dynamically.
     *
     * This should be used if your P256Keypair does not support serialization
     * and deserialization (e.g. your private key is held in a secure element
     * and cannot be accessed directly), or if you back your operational
     * private keys by external implementation of the cryptographic interfaces.
     *
     * To have the ownership of the key managed for you, use
     * SetOperationalKeypair instead.
     */
    CHIP_ERROR SetExternallyOwnedOperationalKeypair(Crypto::P256Keypair * keyPair);

    CHIP_ERROR SetRootCert(const chip::ByteSpan & cert) { return SetCert(mRootCert, cert); }
    CHIP_ERROR SetICACert(const chip::ByteSpan & cert) { return SetCert(mICACert, cert); }
    CHIP_ERROR SetICACert(const Optional<ByteSpan> & cert) { return SetICACert(cert.ValueOr(ByteSpan())); }
    CHIP_ERROR SetNOCCert(const chip::ByteSpan & cert) { return SetCert(mNOCCert, cert); }

    bool IsInitialized() const { return IsOperationalNodeId(mOperationalId.GetNodeId()); }

    bool HasOperationalKey() const { return mOperationalKey != nullptr; }

    // Verifies credentials, using this fabric info's root certificate.
    CHIP_ERROR VerifyCredentials(const ByteSpan & noc, const ByteSpan & icac, Credentials::ValidationContext & context,
                                 PeerId & nocPeerId, FabricId & fabricId, Crypto::P256PublicKey & nocPubkey) const;

    // Verifies credentials, using the provided root certificate.
    static CHIP_ERROR VerifyCredentials(const ByteSpan & noc, const ByteSpan & icac, const ByteSpan & rcac,
                                        Credentials::ValidationContext & context, PeerId & nocPeerId, FabricId & fabricId,
                                        Crypto::P256PublicKey & nocPubkey);

    // Validate an NOC chain at time of adding/updating a fabric (uses VerifyCredentials with additional checks).
    // The `existingFabricId` is passed for UpdateNOC, and must match the Fabric, to make sure that we are
    // not trying to change FabricID with UpdateNOC. If set to kUndefinedFabricId, we are doing AddNOC and
    // we don't need to check match to pre-existing fabric.
    static CHIP_ERROR ValidateIncomingNOCChain(const ByteSpan & noc, const ByteSpan & icac, const ByteSpan & rcac,
                                               FabricId existingFabricId, Credentials::CertificateValidityPolicy * policy,
                                               PeerId & outOperationalId, FabricId & outFabricId,
                                               Crypto::P256PublicKey & outNocPubkey);

    /**
     *  Reset the state to a completely uninitialized status.
     */
    void Reset()
    {
        mOperationalId  = PeerId();
        mVendorId       = VendorId::NotSpecified;
        mFabricLabel[0] = '\0';

        if (!mHasExternallyOwnedOperationalKey && mOperationalKey != nullptr)
        {
            chip::Platform::Delete(mOperationalKey);
        }
        mOperationalKey = nullptr;

        ReleaseOperationalCerts();
        mFabricIndex = kUndefinedFabricIndex;
    }

    /**
     * Verify the validity of the passed fabric info, and then emplace into
     * this.  If a policy is passed, enact this for the fabric info validation.
     *
     * @param newFabric fabric to emplace into this
     * @param policy validation policy to apply, or nulllptr for none
     * @return CHIP_NO_ERROR on success, else an appopriate CHIP_ERROR
     */
    CHIP_ERROR SetFabricInfo(FabricInfo & newFabric, Credentials::CertificateValidityPolicy * policy);

    /* Generate a compressed peer ID (containing compressed fabric ID) using provided fabric ID, node ID and
       root public key of the provided root certificate. The generated compressed ID is returned via compressedPeerId
       output parameter */
    static CHIP_ERROR GeneratePeerId(const ByteSpan & rcac, FabricId fabricId, NodeId nodeId, PeerId * compressedPeerId);

    // Test-only, build a fabric using given root cert and NOC
    CHIP_ERROR TestOnlyBuildFabric(ByteSpan rootCert, ByteSpan icacCert, ByteSpan nocCert, ByteSpan nocKey);

    friend class FabricTable;

protected:
    /**
     * @brief Sign a message with the fabric's operational private key. This ONLY
     *        works if `SetOperationalKeypair` or `SetExternallyOwnedOperationalKeypair`
     *        had been called and is an API that is present ONLY to be called by FabricTable.
     *
     * @param message - message to sign
     * @param outSignature - buffer to hold the signature
     * @return CHIP_NO_ERROR on success or another CHIP_ERROR on crypto internal errors
     */
    CHIP_ERROR SignWithOpKeypair(ByteSpan message, Crypto::P256ECDSASignature & outSignature) const;

    CHIP_ERROR FetchRootCert(MutableByteSpan & outCert) const
    {
        ReturnErrorCodeIf(mRootCert.empty(), CHIP_ERROR_INCORRECT_STATE);
        return CopySpanToMutableSpan(mRootCert, outCert);
    }

    CHIP_ERROR FetchICACert(MutableByteSpan & outCert) const
    {
        if (mICACert.empty())
        {
            outCert.reduce_size(0);
            return CHIP_NO_ERROR;
        }
        return CopySpanToMutableSpan(mICACert, outCert);
    }

    CHIP_ERROR FetchNOCCert(MutableByteSpan & outCert) const
    {
        ReturnErrorCodeIf(mNOCCert.empty(), CHIP_ERROR_INCORRECT_STATE);
        return CopySpanToMutableSpan(mNOCCert, outCert);
    }

    CHIP_ERROR FetchRootPubkey(Crypto::P256PublicKey & outPublicKey) const;

    static constexpr size_t MetadataTLVMaxSize()
    {
        return TLV::EstimateStructOverhead(sizeof(VendorId), kFabricLabelMaxLengthInBytes);
    }

    static constexpr size_t OpKeyTLVMaxSize()
    {
        return TLV::EstimateStructOverhead(sizeof(uint16_t), Crypto::P256SerializedKeypair::Capacity());
    }

    PeerId mOperationalId;

    FabricIndex mFabricIndex                            = kUndefinedFabricIndex;
    uint16_t mVendorId                                  = VendorId::NotSpecified;
    char mFabricLabel[kFabricLabelMaxLengthInBytes + 1] = { '\0' };

#ifdef ENABLE_HSM_CASE_OPS_KEY
    mutable Crypto::P256KeypairHSM * mOperationalKey = nullptr;
#else
    mutable Crypto::P256Keypair * mOperationalKey = nullptr;
#endif
    bool mHasExternallyOwnedOperationalKey = false;

    MutableByteSpan mRootCert;
    MutableByteSpan mICACert;
    MutableByteSpan mNOCCert;

    FabricId mFabricId = 0;

    CHIP_ERROR CommitToStorage(PersistentStorageDelegate * storage);
    CHIP_ERROR LoadFromStorage(PersistentStorageDelegate * storage);
    static CHIP_ERROR DeleteFromStorage(PersistentStorageDelegate * storage, FabricIndex fabricIndex);

    void ReleaseCert(MutableByteSpan & cert);
    void ReleaseOperationalCerts()
    {
        ReleaseCert(mRootCert);
        ReleaseCert(mICACert);
        ReleaseCert(mNOCCert);
    }

    CHIP_ERROR SetCert(MutableByteSpan & dstCert, const ByteSpan & srcCert);
};

/**
 * Iterates over valid fabrics within a list
 */
class ConstFabricIterator
{
public:
    using value_type = FabricInfo;
    using pointer    = FabricInfo *;
    using reference  = FabricInfo &;

    ConstFabricIterator(const FabricInfo * start, size_t index, size_t maxSize) : mStart(start), mIndex(index), mMaxSize(maxSize)
    {
        if (mIndex >= maxSize)
        {
            mIndex = maxSize;
        }
        else if (!mStart[mIndex].IsInitialized())
        {
            Advance();
        }
    }
    ConstFabricIterator(const ConstFabricIterator &) = default;
    ConstFabricIterator & operator=(const ConstFabricIterator &) = default;

    ConstFabricIterator & operator++() { return Advance(); }
    ConstFabricIterator operator++(int)
    {
        ConstFabricIterator other(*this);
        Advance();
        return other;
    }

    const FabricInfo & operator*() const { return mStart[mIndex]; }
    const FabricInfo * operator->() const { return mStart + mIndex; }

    bool operator==(const ConstFabricIterator & other)
    {
        if (IsAtEnd())
        {
            return other.IsAtEnd();
        }

        return (mStart == other.mStart) && (mIndex == other.mIndex) && (mMaxSize == other.mMaxSize);
    }
    bool operator!=(const ConstFabricIterator & other) { return !(*this == other); }

    bool IsAtEnd() const { return (mIndex == mMaxSize); }

private:
    const FabricInfo * mStart;
    size_t mIndex;
    size_t mMaxSize;

    ConstFabricIterator & Advance()
    {
        do
        {
            if (mIndex < mMaxSize)
            {
                mIndex++;
            }
        } while (!IsAtEnd() && !mStart[mIndex].IsInitialized());

        return *this;
    }
};

class DLL_EXPORT FabricTable
{
public:
    class DLL_EXPORT Delegate
    {
    public:
        Delegate() {}
        virtual ~Delegate() {}

        /**
         * Gets called when a fabric is deleted, such as on FabricTable::Delete().
         **/
        virtual void OnFabricDeletedFromStorage(FabricTable & fabricTable, FabricIndex fabricIndex) = 0;

        /**
         * Gets called when a fabric is loaded into Fabric Table from storage, such as
         * during FabricTable::Init().
         **/
        virtual void OnFabricRetrievedFromStorage(FabricTable & fabricTable, FabricIndex fabricIndex) = 0;

        /**
         * Gets called when a fabric in Fabric Table is persisted to storage, such as
         * on FabricTable::AddNewFabric().
         **/
        virtual void OnFabricPersistedToStorage(FabricTable & fabricTable, FabricIndex fabricIndex) = 0;

        /**
         * Gets called when operational credentials are changed.
         **/
        virtual void OnFabricNOCUpdated(FabricTable & fabricTable, FabricIndex fabricIndex) = 0;

        // Intrusive list pointer for FabricTable to manage the entries.
        Delegate * next = nullptr;
    };

public:
    FabricTable() {}
    ~FabricTable();

    CHIP_ERROR Store(FabricIndex fabricIndex);
    CHIP_ERROR LoadFromStorage(FabricInfo * info);

    // Returns CHIP_ERROR_NOT_FOUND if there is no fabric for that index.
    CHIP_ERROR Delete(FabricIndex fabricIndex);
    void DeleteAllFabrics();

    /**
     * Add the new fabric information to fabric table if the table has space to store
     * more fabrics. CHIP_ERROR_NO_MEMORY error will be returned if the table is full.
     *
     * The provided information will get copied to internal data structures, and the caller
     * can release the memory associated with input parameter after the call is complete.
     *
     * If the call is successful, the assigned fabric index is returned as output parameter.
     * The fabric information will also be persisted to storage.
     */
    CHIP_ERROR AddNewFabric(FabricInfo & fabric, FabricIndex * assignedIndex);

    // This is same as AddNewFabric, but skip duplicate fabric check, because we have multiple nodes belongs to the same fabric in
    // test-cases
    CHIP_ERROR AddNewFabricForTest(FabricInfo & newFabric, FabricIndex * outputIndex);

    /**
     * Update fabric at the specified fabric index with the passed fabric info.
     *
     * @param fabricIndex index at which to update fabric info
     * @param fabricInfo fabric info to validate and copy into the specified index
     * @return CHIP_NO_ERROR on success, an appropriate CHIP_ERROR on failure
     */
    CHIP_ERROR UpdateFabric(FabricIndex fabricIndex, FabricInfo & fabricInfo);

    // TODO this #if CONFIG_BUILD_FOR_HOST_UNIT_TEST is temporary. There is a change incoming soon
    // that will allow triggering NOC update directly.
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    void SendUpdateFabricNotificationForTest(FabricIndex fabricIndex) { NotifyNOCUpdatedOnFabric(fabricIndex); }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

    FabricInfo * FindFabric(const Crypto::P256PublicKey & rootPubKey, FabricId fabricId);
    FabricInfo * FindFabricWithIndex(FabricIndex fabricIndex);
    const FabricInfo * FindFabricWithIndex(FabricIndex fabricIndex) const;
    FabricInfo * FindFabricWithCompressedId(CompressedFabricId fabricId);

    CHIP_ERROR Init(PersistentStorageDelegate * storage);
    CHIP_ERROR Init(PersistentStorageDelegate * storage, Crypto::OperationalKeystore * operationalKeystore);

    CHIP_ERROR AddFabricDelegate(FabricTable::Delegate * delegate);
    void RemoveFabricDelegate(FabricTable::Delegate * delegate);

    /**
     * Get the current Last Known Good Time.
     *
     * @param lastKnownGoodChipEpochTime (out) the current last known good time, if any is known
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR GetLastKnownGoodChipEpochTime(System::Clock::Seconds32 & lastKnownGoodChipEpochTime) const
    {
        return mLastKnownGoodTime.GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime);
    }

    /**
     * Validate that the passed Last Known Good Time is within bounds and then
     * store this and write back to storage.  Legal values are those which are
     * not earlier than firmware build time or any of our stored certificates'
     * NotBefore times:
     *
     *    3.5.6.1. Last Known Good UTC Time
     *
     *    A Node MAY adjust the Last Known Good UTC Time backwards if it
     *    believes the current Last Known Good UTC Time is incorrect and it has
     *    a good time value from a trusted source. The Node SHOULD NOT adjust
     *    the Last Known Good UTC to a time before the later of:
     *      • The build timestamp of its currently running software image
     *      • The not-before timestamp of any of its operational certificates
     *
     * @param lastKnownGoodChipEpochTime Last Known Good Time in seconds since CHIP epoch
     * @return CHIP_NO_ERROR on success, else an appopriate CHIP_ERROR
     */
    CHIP_ERROR SetLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime);

    /*
     * Commit the Last Known Good Time by deleting the fail-safe backup from
     * storage.
     *
     * @return CHIP_NO_ERROR on success, else an appopriate CHIP_ERROR
     */
    CHIP_ERROR CommitLastKnownGoodChipEpochTime() { return mLastKnownGoodTime.CommitLastKnownGoodChipEpochTime(); }

    /*
     * Revert the Last Known Good Time to the fail-safe backup value in
     * persistence if any exists.
     *
     * @return CHIP_NO_ERROR on success, else an appopriate CHIP_ERROR
     */
    CHIP_ERROR RevertLastKnownGoodChipEpochTime() { return mLastKnownGoodTime.RevertLastKnownGoodChipEpochTime(); }

    uint8_t FabricCount() const { return mFabricCount; }

    ConstFabricIterator cbegin() const { return ConstFabricIterator(mStates, 0, CHIP_CONFIG_MAX_FABRICS); }
    ConstFabricIterator cend() const { return ConstFabricIterator(mStates, CHIP_CONFIG_MAX_FABRICS, CHIP_CONFIG_MAX_FABRICS); }
    ConstFabricIterator begin() const { return cbegin(); }
    ConstFabricIterator end() const { return cend(); }

    CHIP_ERROR FetchRootCert(FabricIndex fabricIndex, MutableByteSpan & outCert) const;
    CHIP_ERROR FetchICACert(FabricIndex fabricIndex, MutableByteSpan & outCert) const;
    CHIP_ERROR FetchNOCCert(FabricIndex fabricIndex, MutableByteSpan & outCert) const;
    CHIP_ERROR FetchRootPubkey(FabricIndex fabricIndex, Crypto::P256PublicKey & outPublicKey) const;

    /**
     * @brief Sign a message with a given fabric's operational keypair. This is used for
     *        CASE and the only way the key should be used.
     *
     * This will use a pending key activated with `ActivatePendingOperationalKey` but
     * not yet persisted, if one is available for the fabric.
     *
     * @param fabricIndex - Fabric index whose operational key touse
     * @param message - Message to sign
     * @param outSignature - Signature object to receive the signature
     *
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if no active key is found for the given `fabricIndex` or if
     *                                         `fabricIndex` is invalid.
     * @retval other CHIP_ERROR value on internal errors
     */
    CHIP_ERROR SignWithOpKeypair(FabricIndex fabricIndex, ByteSpan message, Crypto::P256ECDSASignature & outSignature) const;

    /**
     * This initializes a new keypair for the given fabric and generates a CSR for it,
     * so that it can be passed in a CSRResponse.
     *
     * The keypair is temporary and becomes usable for `SignWithOpKeypair` only after either
     * `ActivatePendingOperationalKey` is called. It is destroyed if
     * `RevertPendingFabricData` is called before `CommitPendingFabricData`.
     *  If a pending keypair for the provided fabricIndex (if present) already existed, it is replaced by this call.
     *
     *  Only one pending operational keypair is supported at a time.
     *
     * @param fabricIndex - Existing FabricIndex for which a new keypair must be made available. If it
     *                      doesn't have a value, the key will be marked pending for the next available
     *                      fabric index that would apply for `AddNewFabric`.
     * @param outputCsr - Buffer to contain the CSR. Must be at least `kMAX_CSR_Length` large.
     *
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL if `outputCsr` buffer is too small
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if there is already a pending keypair for another `fabricIndex` value
     *                                         or if fabricIndex is an invalid value.
     * @retval other CHIP_ERROR value on internal errors
     */
    CHIP_ERROR AllocatePendingOperationalKey(Optional<FabricIndex> fabricIndex, MutableByteSpan & outputCsr);

    /**
     * @brief Temporarily activates the operational keypair last generated with `AllocatePendingOperationalKey`,
     *        so that `SignWithOpKeypair` starts using it, but only if it matches the public key passed
     *        in `nocSubjectPublicKey` gotten from a matching NOC.
     *
     * This is to be used by AddNOC and UpdateNOC so that a prior key generated by AllocatePendingOperationalKey
     * can be used for CASE while not committing it yet to permanent storage to remain after fail-safe.
     *
     * @param nocSubjectPublicKey - Subject public key associated with an incoming NOC
     *
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if there is no pending operational keypair
     * @retval CHIP_ERROR_INVALID_PUBLIC_KEY if `nocSubjectPublicKey` does not match the public key associated
     *                                       with the key pair from last `AllocatePendingOperationalKey`.
     * @retval other CHIP_ERROR value on internal errors
     */
    CHIP_ERROR ActivatePendingOperationalKey(const Crypto::P256PublicKey & nocSubjectPublicKey);

    /**
     * @brief Returns whether an operational key is pending (true if `AllocatePendingOperationalKey` was
     *        previously successfully called, false otherwise
     */
    bool HasPendingOperationalKey() const;

    /**
     * @brief Commit any pending temporary FabricTable state. This is used mostly for affecting
     *        CommissioningComplete.
     *
     * @return CHIP_NO_ERROR on success or any toher CHIO_ERROR value on internal errors
     */
    CHIP_ERROR CommitPendingFabricData();

    /**
     * @brief Revert any pending state. This is used to handle fail-safe expiry of partially
     *        configured fabrics.
     */
    void RevertPendingFabricData();

private:
    static constexpr size_t IndexInfoTLVMaxSize()
    {
        // We have a single next-available index and an array of anonymous-tagged
        // fabric indices.
        //
        // The max size of the list is (1 byte control + bytes for actual value)
        // times max number of list items, plus one byte for the list terminator.
        return TLV::EstimateStructOverhead(sizeof(FabricIndex), CHIP_CONFIG_MAX_FABRICS * (1 + sizeof(FabricIndex)) + 1);
    }

    /**
     * UpdateNextAvailableFabricIndex should only be called when
     * mNextAvailableFabricIndex has a value and that value stops being
     * available.  It will set mNextAvailableFabricIndex to the next available
     * value, or no value if there is none available.
     */
    void UpdateNextAvailableFabricIndex();

    /**
     * Ensure that we have a valid next available fabric index, if that's at all possible.  This covers
     * some FabricIndex allocation corner cases.  After this is called, the only way we can fail to have
     * a next available fabric index is if our fabric table is max-sized (254 entries) and full.
     */
    void EnsureNextAvailableFabricIndexUpdated();

    /**
     * Store our current fabric index state: what our next available index is
     * and what indices we're using right now.
     */
    CHIP_ERROR StoreFabricIndexInfo() const;

    /**
     * Read our fabric index info from the given TLV reader and set up the
     * fabric table accordingly.
     */
    CHIP_ERROR ReadFabricInfo(TLV::ContiguousBufferTLVReader & reader);

    CHIP_ERROR AddNewFabricInner(FabricInfo & fabric, FabricIndex * assignedIndex);

    CHIP_ERROR NotifyNOCUpdatedOnFabric(FabricIndex fabricIndex);

    FabricInfo mStates[CHIP_CONFIG_MAX_FABRICS];
    PersistentStorageDelegate * mStorage               = nullptr;
    Crypto::OperationalKeystore * mOperationalKeystore = nullptr;

    // FabricTable::Delegate link to first node, since FabricTable::Delegate is a form
    // of intrusive linked-list item.
    FabricTable::Delegate * mDelegateListRoot = nullptr;

    // We may not have an mNextAvailableFabricIndex if our table is as large as
    // it can go and is full.
    Optional<FabricIndex> mNextAvailableFabricIndex;
    uint8_t mFabricCount = 0;

    // If true, we are in the process of a fail-safe and there was at least one
    // operation that caused partial data in the fabric table.
    bool mIsPendingFabricDataPresent = false;

    // When mIsPendingFabricDataPresent is true, this holds the index of the fabric
    // for which there is currently pending data.
    FabricIndex mFabricIndexWithPendingState = kUndefinedFabricIndex;

    LastKnownGoodTime mLastKnownGoodTime;
};

} // namespace chip
