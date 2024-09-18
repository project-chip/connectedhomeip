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
#include <credentials/OperationalCertificateStore.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/Optional.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/core/TLV.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Span.h>

namespace chip {

static constexpr uint8_t kFabricLabelMaxLengthInBytes = 32;

static_assert(kUndefinedFabricIndex < chip::kMinValidFabricIndex, "Undefined fabric index should not be valid");

/**
 * Provides access to the core metadata for a given fabric to which a node is joined.
 *
 * This metadata includes:
 *
 * - FabricIndex within the local set of fabrics
 * - Operational Identity
 *   - NodeId
 *   - Fabric Id
 *   - Public key of operational root CA (to avoid keeping/reloading RCAC (Root CA Certificate) too often)
 *   - Pre-computed "Compressed Fabric ID" used for discovery
 * - Operational public key (if externally injected as opposed to present in an OperationalKeystore)
 * - Fabric Label
 * - VendorID allocated at fabric joining by commissioner
 *
 * NOTE: All the setters of this class are private and only accessible by FabricTable, the
 *       friend class that owns these. The reason is that there are data dependencies between
 *       fabrics that require FabricTable to be the single entrypoint for all mutations, rather
 *       than directly on a FabricInfo instance.
 */
class DLL_EXPORT FabricInfo
{
public:
    FabricInfo() { Reset(); }
    ~FabricInfo() { Reset(); }

    // Non-copyable
    FabricInfo(FabricInfo const &)     = delete;
    void operator=(FabricInfo const &) = delete;

    // Returns a span into our internal storage.
    CharSpan GetFabricLabel() const { return CharSpan(mFabricLabel, strnlen(mFabricLabel, kFabricLabelMaxLengthInBytes)); }
    CHIP_ERROR SetFabricLabel(const CharSpan & fabricLabel);

    NodeId GetNodeId() const { return mNodeId; }
    ScopedNodeId GetScopedNodeId() const { return ScopedNodeId(mNodeId, mFabricIndex); }
    ScopedNodeId GetScopedNodeIdForNode(const NodeId node) const { return ScopedNodeId(node, mFabricIndex); }

    // TODO(#15049): Refactor/rename PeerId to OperationalId or OpId throughout source
    PeerId GetPeerId() const { return PeerId(mCompressedFabricId, mNodeId); }
    PeerId GetPeerIdForNode(const NodeId node) const { return PeerId(mCompressedFabricId, node); }

    FabricId GetFabricId() const { return mFabricId; }
    FabricIndex GetFabricIndex() const { return mFabricIndex; }

    CompressedFabricId GetCompressedFabricId() const { return mCompressedFabricId; }
    CHIP_ERROR GetCompressedFabricIdBytes(MutableByteSpan & compressedFabricId) const
    {
        ReturnErrorCodeIf(compressedFabricId.size() != sizeof(uint64_t), CHIP_ERROR_INVALID_ARGUMENT);
        Encoding::BigEndian::Put64(compressedFabricId.data(), GetCompressedFabricId());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchRootPubkey(Crypto::P256PublicKey & outPublicKey) const;

    VendorId GetVendorId() const { return mVendorId; }

    bool IsInitialized() const { return (mFabricIndex != kUndefinedFabricIndex) && IsOperationalNodeId(mNodeId); }

    bool HasOperationalKey() const { return mOperationalKey != nullptr; }

    bool ShouldAdvertiseIdentity() const { return mShouldAdvertiseIdentity; }

    friend class FabricTable;

private:
    struct InitParams
    {
        CompressedFabricId compressedFabricId    = kUndefinedCompressedFabricId;
        NodeId nodeId                            = kUndefinedNodeId;
        FabricIndex fabricIndex                  = kUndefinedFabricIndex;
        Crypto::P256Keypair * operationalKeypair = nullptr;
        FabricId fabricId                        = kUndefinedFabricId;
        Crypto::P256PublicKey rootPublicKey;
        VendorId vendorId              = VendorId::NotSpecified; /**< Vendor ID for commissioner of fabric */
        bool hasExternallyOwnedKeypair = false;
        bool advertiseIdentity         = false;

        CHIP_ERROR AreValid() const
        {
            VerifyOrReturnError((fabricId != kUndefinedFabricId) && (fabricIndex != kUndefinedFabricIndex),
                                CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(IsOperationalNodeId(nodeId), CHIP_ERROR_INVALID_ARGUMENT);
            // We don't check the root public key validity or the compressed fabric ID, since in the
            // very small usage that exists in private use, the rest should be OK.
            return CHIP_NO_ERROR;
        }
    };

    // Move assignment operator to support setting from pending on fabric table commit
    void operator=(FabricInfo && other);

    /**
     * @brief Initialize a FabricInfo object's metadata given init parameters.
     *
     * Note that certificates are never owned by this object and are assumed pre-validated
     *
     * @param initParams Init parameters to use to initialize the given fabric.
     * @return CHIP_NO_ERROR on success or another internal CHIP_ERROR_* value on failure
     */
    CHIP_ERROR Init(const InitParams & initParams);

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

    /**
     *  Reset the state to a completely uninitialized status.
     */
    void Reset()
    {
        mNodeId             = kUndefinedNodeId;
        mFabricId           = kUndefinedFabricId;
        mFabricIndex        = kUndefinedFabricIndex;
        mCompressedFabricId = kUndefinedCompressedFabricId;

        mVendorId       = VendorId::NotSpecified;
        mFabricLabel[0] = '\0';

        if (!mHasExternallyOwnedOperationalKey && mOperationalKey != nullptr)
        {
            chip::Platform::Delete(mOperationalKey);
        }
        mOperationalKey                   = nullptr;
        mHasExternallyOwnedOperationalKey = false;
        mShouldAdvertiseIdentity          = true;

        mFabricIndex = kUndefinedFabricIndex;
        mNodeId      = kUndefinedNodeId;
    }

    void SetShouldAdvertiseIdentity(bool advertiseIdentity) { mShouldAdvertiseIdentity = advertiseIdentity; }

    static constexpr size_t MetadataTLVMaxSize()
    {
        return TLV::EstimateStructOverhead(sizeof(uint16_t), kFabricLabelMaxLengthInBytes);
    }

    static constexpr size_t OpKeyTLVMaxSize()
    {
        return TLV::EstimateStructOverhead(sizeof(uint16_t), Crypto::P256SerializedKeypair::Capacity());
    }

    NodeId mNodeId     = kUndefinedNodeId;
    FabricId mFabricId = kUndefinedFabricId;
    // We cache the compressed fabric id since it's used so often and costly to get.
    CompressedFabricId mCompressedFabricId = kUndefinedCompressedFabricId;
    // We cache the root public key since it's used so often and costly to get.
    Crypto::P256PublicKey mRootPublicKey;

    // mFabricLabel is 33 bytes, so ends on a 1 mod 4 byte boundary.
    char mFabricLabel[kFabricLabelMaxLengthInBytes + 1] = { '\0' };

    // mFabricIndex, mVendorId, mHasExternallyOwnedOperationalKey,
    // mShouldAdvertiseIdentity are 5 bytes and do not include any padding if
    // they come after the 33-byte mFabricLabel, so end on a 2 mod 4 byte
    // boundary.
    FabricIndex mFabricIndex               = kUndefinedFabricIndex;
    VendorId mVendorId                     = VendorId::NotSpecified;
    bool mHasExternallyOwnedOperationalKey = false;
    bool mShouldAdvertiseIdentity          = true;

    // 2 bytes of padding here, since mOperationalKey needs to be void*-aligned,
    // so has to be at a 0 mod 4 byte location.

    mutable Crypto::P256Keypair * mOperationalKey = nullptr;

    CHIP_ERROR CommitToStorage(PersistentStorageDelegate * storage) const;
    CHIP_ERROR LoadFromStorage(PersistentStorageDelegate * storage, FabricIndex newFabricIndex, const ByteSpan & rcac,
                               const ByteSpan & noc);
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

    ConstFabricIterator(const FabricInfo * start, const FabricInfo * pending, size_t index, size_t maxSize) :
        mStart(start), mPending(pending), mIndex(index), mMaxSize(maxSize)
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
    ConstFabricIterator(const ConstFabricIterator &)             = default;
    ConstFabricIterator & operator=(const ConstFabricIterator &) = default;

    ConstFabricIterator & operator++() { return Advance(); }
    ConstFabricIterator operator++(int)
    {
        ConstFabricIterator other(*this);
        Advance();
        return other;
    }

    const FabricInfo & operator*() const
    {
        VerifyOrDie(!IsAtEnd());

        return *GetCurrent();
    }
    const FabricInfo * operator->() const
    {
        VerifyOrDie(!IsAtEnd());

        return GetCurrent();
    }

    bool operator==(const ConstFabricIterator & other) const
    {
        if (IsAtEnd())
        {
            return other.IsAtEnd();
        }

        // Pending entry does not participate in finding this.
        return (mStart == other.mStart) && (mIndex == other.mIndex) && (mMaxSize == other.mMaxSize);
    }
    bool operator!=(const ConstFabricIterator & other) const { return !(*this == other); }

    bool IsAtEnd() const { return (mIndex == mMaxSize); }

private:
    const FabricInfo * mStart;
    const FabricInfo * mPending; ///< Pointer to the shadow pending entry, nullptr if none
    size_t mIndex;
    size_t mMaxSize;

    // Helper to get either a given entry of the fabric table, or its pending shadow if
    // a fabric update is currently pending.
    const FabricInfo * GetCurrent() const
    {
        const auto * current = mStart + mIndex;

        // If we reached the pending entry, return that instead of the underlying entry from the mStates.
        if ((mPending != nullptr) && mPending->IsInitialized() && (current->GetFabricIndex() == mPending->GetFabricIndex()))
        {
            current = mPending;
        }

        return current;
    }

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
    struct DLL_EXPORT InitParams
    {
        // PersistentStorageDelegate for Fabric Info metadata storage and Fabric Table index (MANDATORY).
        PersistentStorageDelegate * storage = nullptr;
        // Operational Keystore to abstract access to key. Mandatory for commissionable devices  (e.g.
        // chip::Server-based things) and recommended for controllers. With this set to false, FabricInfo
        // added as new fabrics need to have directly injected operational keys with FabricInfo::Set*OperationalKey.
        Crypto::OperationalKeystore * operationalKeystore = nullptr;
        // Operational Certificate store to hold the NOC/ICAC/RCAC chains (MANDATORY).
        Credentials::OperationalCertificateStore * opCertStore = nullptr;
    };

    class DLL_EXPORT Delegate
    {
    public:
        Delegate() {}
        virtual ~Delegate() {}

        /**
         * Gets called when a fabric is about to be deleted, such as on
         * FabricTable::Delete().  This allows actions to be taken that need the
         * fabric to still be around before we delete it.
         **/
        virtual void FabricWillBeRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) {}

        /**
         * Gets called when a fabric is deleted, such as on FabricTable::Delete().
         **/
        virtual void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) {}

        /**
         * Gets called when a fabric in Fabric Table is persisted to storage, by CommitPendingFabricData.
         **/
        virtual void OnFabricCommitted(const FabricTable & fabricTable, FabricIndex fabricIndex){};

        /**
         * Gets called when operational credentials are changed, which may not be persistent.
         *
         * Can be used to affect what is needed for UpdateNOC prior to commit.
         **/
        virtual void OnFabricUpdated(const FabricTable & fabricTable, FabricIndex fabricIndex){};

        // Intrusive list pointer for FabricTable to manage the entries.
        Delegate * next = nullptr;
    };

public:
    FabricTable()  = default;
    ~FabricTable() = default;

    // Non-copyable
    FabricTable(FabricTable const &)    = delete;
    void operator=(FabricTable const &) = delete;

    enum class AdvertiseIdentity : uint8_t
    {
        Yes,
        No
    };

    // Returns CHIP_ERROR_NOT_FOUND if there is no fabric for that index.
    CHIP_ERROR Delete(FabricIndex fabricIndex);
    void DeleteAllFabrics();

    // TODO this #if CONFIG_BUILD_FOR_HOST_UNIT_TEST is temporary. There is a change incoming soon
    // that will allow triggering NOC update directly.
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    void SendUpdateFabricNotificationForTest(FabricIndex fabricIndex) { NotifyFabricUpdated(fabricIndex); }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

    /**
     * Collection of methods to help find a matching FabricInfo instance given a set of query criteria
     *
     */

    /**
     * Finds a matching FabricInfo instance given a root public key and fabric ID that uniquely identifies the fabric in any scope.
     *
     * Returns nullptr if no matching instance is found.
     *
     */
    const FabricInfo * FindFabric(const Crypto::P256PublicKey & rootPubKey, FabricId fabricId) const;

    /**
     * Finds a matching FabricInfo instance given a locally-scoped fabric index.
     *
     * Returns nullptr if no matching instance is found.
     *
     */
    const FabricInfo * FindFabricWithIndex(FabricIndex fabricIndex) const;

    /**
     * Finds a matching FabricInfo instance given a root public key, fabric ID AND a matching NodeId. This variant of find
     * is only to be used when it is possible to have colliding fabrics in the table that are on the same logical fabric
     * but may be associated with different node identities.
     *
     * Returns nullptr if no matching instance is found.
     *
     */
    const FabricInfo * FindIdentity(const Crypto::P256PublicKey & rootPubKey, FabricId fabricId, NodeId nodeId) const;

    /**
     * Finds a matching FabricInfo instance given a compressed fabric ID. If there are multiple
     * matching FabricInfo instances given the low but non-zero probability of collision, there is no guarantee
     * on which instance will be returned.
     *
     * Returns nullptr if no matching instance is found.
     */
    const FabricInfo * FindFabricWithCompressedId(CompressedFabricId compressedFabricId) const;

    CHIP_ERROR Init(const FabricTable::InitParams & initParams);
    void Shutdown();

    /**
     * @brief If `Init()` caused a Delete due to partial commit, the fabric index at play is returned.
     *
     * Allows caller to schedule more clean-up. This is because at Init() time, none of the delegates
     * are registered yet, so no other modules would learn of the removal.
     *
     * The value is auto-reset to `kUndefinedFabricIndex` on being returned, so that subsequent
     * `GetDeletedFabricFromCommitMarker()` after one that has a fabric index to give will provide
     * `kUndefinedFabricIndex`.
     *
     * @return the fabric index of a just-deleted fabric, or kUndefinedFabricIndex if none were deleted.
     */
    FabricIndex GetDeletedFabricFromCommitMarker();

    /**
     * @brief Clear the commit marker when we are sure we have proceeded with any remaining clean-up
     */
    void ClearCommitMarker();

    // Forget a fabric in memory: doesn't delete any persistent state, just
    // reverts any pending state (blindly) and then resets the fabric table
    // entry.
    //
    // TODO: We have to determine if we should remove this call.
    void Forget(FabricIndex fabricIndex);

    CHIP_ERROR AddFabricDelegate(FabricTable::Delegate * delegate);
    void RemoveFabricDelegate(FabricTable::Delegate * delegate);

    /**
     * @brief Set the Fabric Label for the fabric referred by `fabricIndex`.
     *
     * If a fabric add/update is pending, only the pending version will be updated,
     * so that on fail-safe expiry, you would actually see the only fabric label if
     * Update fails. If the fabric label is set before UpdateNOC, then the change is immediate.
     *
     * @param fabricIndex - Fabric Index for which to set the label
     * @param fabricLabel - Label to set on the fabric
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if fabricIndex does not refer to an fabric in the table
     * @retval CHIP_ERROR_INVALID_ARGUMENT on fabric label error (e.g. too large)
     * @retval other CHIP_ERROR on internal errors
     */
    CHIP_ERROR SetFabricLabel(FabricIndex fabricIndex, const CharSpan & fabricLabel);

    /**
     * @brief Get the Fabric Label for a given fabric
     *
     * NOTE: The outFabricLabel argument points to internal memory of the fabric info.
     *       It may become invalid on the next FabricTable API call due to shadow
     *       storage of data.
     *
     * @param fabricIndex - Fabric index for which to get the label
     * @param outFabricLabel - char span that will be set to the label value
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX on error
     * @retval other CHIP_ERROR on internal errors
     */
    CHIP_ERROR GetFabricLabel(FabricIndex fabricIndex, CharSpan & outFabricLabel);

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

    /**
     * @return the number of fabrics currently accessible/usable/iterable.
     */
    uint8_t FabricCount() const { return mFabricCount; }

    ConstFabricIterator cbegin() const
    {
        const FabricInfo * pending = GetShadowPendingFabricEntry();
        return ConstFabricIterator(mStates, pending, 0, CHIP_CONFIG_MAX_FABRICS);
    }
    ConstFabricIterator cend() const
    {
        return ConstFabricIterator(mStates, nullptr, CHIP_CONFIG_MAX_FABRICS, CHIP_CONFIG_MAX_FABRICS);
    }
    ConstFabricIterator begin() const { return cbegin(); }
    ConstFabricIterator end() const { return cend(); }

    /**
     * @brief Get the RCAC (operational root certificate) associated with a fabric.
     *
     * If a root is pending for `fabricIndex` from `AddNewPendingTrustedRootCert`, it is returned.
     *
     * @param fabricIndex - Fabric for which to get the RCAC
     * @param outCert - MutableByteSpan to receive the certificate. Resized to actual size.
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL if `outCert` is too small
     * @retval CHIP_ERROR_NOT_FOUND if not found/available
     * @retval other CHIP_ERROR values on invalid arguments or internal errors.
     */
    CHIP_ERROR FetchRootCert(FabricIndex fabricIndex, MutableByteSpan & outCert) const;

    /**
     * @brief Get the pending root certificate which is not associated with a fabric, if there is one.
     *
     * If a root is pending from `AddNewPendingTrustedRootCert`, and there is no
     * fabric associated with the corresponding fabric index yet
     * (i.e. `AddNewPendingFabric*` has not been called yet) it is returned.
     *
     * @param outCert - MutableByteSpan to receive the certificate. Resized to actual size.
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL if `outCert` is too small.
     * @retval CHIP_ERROR_NOT_FOUND if there is no pending root certificate
     *                              that's not yet associated with a fabric.
     * @retval other CHIP_ERROR values on invalid arguments or internal errors.
     */
    CHIP_ERROR FetchPendingNonFabricAssociatedRootCert(MutableByteSpan & outCert) const;

    /**
     * @brief Get the ICAC (operational intermediate certificate) associated with a fabric.
     *
     * If a fabric is pending from add/update operation for the given `fabricIndex`, its
     * ICAC is returned.
     *
     * If an NOC exists, but the ICAC is not present in the chain, CHIP_NO_ERROR is
     * returned and `outCert` is resized to 0 length so that its `empty()` method returns true.
     *
     * @param fabricIndex - Fabric for which to get the ICAC
     * @param outCert - MutableByteSpan to receive the certificate. Resized to actual size.
     * @retval CHIP_NO_ERROR on success, including if absent within an existing chain
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL if `outCert` is too small
     * @retval CHIP_ERROR_NOT_FOUND if not found/available
     * @retval other CHIP_ERROR values on invalid arguments or internal errors.
     */
    CHIP_ERROR FetchICACert(FabricIndex fabricIndex, MutableByteSpan & outCert) const;

    /**
     * @brief Get the NOC (Node Operational Certificate) associated with a fabric.
     *
     * If a fabric is pending from add/update operation for the given `fabricIndex`, its
     * NOC is returned.
     *
     * @param fabricIndex - Fabric for which to get the NOC
     * @param outCert - MutableByteSpan to receive the certificate. Resized to actual size.
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL if `outCert` is too small
     * @retval CHIP_ERROR_NOT_FOUND if not found/available
     * @retval other CHIP_ERROR values on invalid arguments or internal errors.
     */
    CHIP_ERROR FetchNOCCert(FabricIndex fabricIndex, MutableByteSpan & outCert) const;

    /**
     * @brief Get the root public key by value for the given `fabricIndex`.
     *
     * @param fabricIndex - Fabric for which to get the root public key (subject public key of RCAC)
     * @param outPublicKey - PublicKey instance to receive the public key contents
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL if `outCert` is too small
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if not found/available, or `fabricIndex` has a bad value
     * @retval other CHIP_ERROR values on other invalid arguments or internal errors.
     */
    CHIP_ERROR FetchRootPubkey(FabricIndex fabricIndex, Crypto::P256PublicKey & outPublicKey) const;

    /**
     * @brief Get the CASE Authenticated Tags from the NOC for the given `fabricIndex`.
     *
     * @param fabricIndex - Fabric for which to get the root public key (subject public key of RCAC)
     * @param cats - CATValues struct to write the NOC CATs for the given fabric index
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if not found/available, or `fabricIndex` has a bad value
     * @retval other CHIP_ERROR values on other invalid arguments or internal errors.
     */
    CHIP_ERROR FetchCATs(const FabricIndex fabricIndex, CATValues & cats) const;

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
     * @brief Create an ephemeral keypair for use in session establishment.
     *
     * WARNING: The return value MUST be released by `ReleaseEphemeralKeypair`. This is because
     *          Matter CHIPMem.h does not properly support UniquePtr in a way that would
     *          safely allow classes derived from Crypto::P256Keypair to be released properly.
     *
     * This delegates to the OperationalKeystore if one exists, otherwise it directly allocates a base
     * Crypto::P256Keypair instance
     *
     * @return a pointer to a dynamically P256Keypair (or derived class thereof), which may evaluate to nullptr
     *         if running out of memory.
     */
    Crypto::P256Keypair * AllocateEphemeralKeypairForCASE();

    /**
     * @brief Release an ephemeral keypair previously created by `AllocateEphemeralKeypairForCASE()`
     */
    void ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair);

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
     * @param outputCsr - Buffer to contain the CSR. Must be at least `kMIN_CSR_Buffer_Size` large.
     *
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL if `outputCsr` buffer is too small
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if there is already a pending keypair for another `fabricIndex` value
     *                                         or if fabricIndex is an invalid value.
     * @retval other CHIP_ERROR value on internal errors
     */
    CHIP_ERROR AllocatePendingOperationalKey(Optional<FabricIndex> fabricIndex, MutableByteSpan & outputCsr);

    /**
     * @brief Returns whether an operational key is pending (true if `AllocatePendingOperationalKey` was
     *        previously successfully called, false otherwise).
     *
     * @param outIsPendingKeyForUpdateNoc this is set to true if the `AllocatePendingOperationalKey` had an
     *                                    associated fabric index attached, indicating it's for UpdateNoc
     */
    bool HasPendingOperationalKey(bool & outIsPendingKeyForUpdateNoc) const;

    /**
     * @brief Returns whether an operational key can be used to sign for given FabricIndex
     *
     * @param fabricIndex - Fabric index for which an operational key must be found
     * @return true if a pending fabric or committed fabric for fabricIndex has an operational key, false otherwise.
     */
    bool HasOperationalKeyForFabric(FabricIndex fabricIndex) const;

    /**
     * @brief If a newly-added fabric is pending, this returns its index, or kUndefinedFabricIndex if none are pending.
     *
     * A newly-added fabric is pending if AddNOC has been previously called successfully but the
     * fabric is not yet fully committed by CommissioningComplete.
     *
     * NOTE: that this never returns a value other than kUndefinedFabricIndex when UpdateNOC is pending.
     *
     * @return the fabric index of the pending fabric, or kUndefinedFabricIndex if no fabrics are pending.
     */
    FabricIndex GetPendingNewFabricIndex() const;

    /**
     * @brief Returns the operational keystore. This is used for
     *        CASE and the only way the keystore should be used.
     *
     * @return The operational keystore, nullptr otherwise.
     */
    const Crypto::OperationalKeystore * GetOperationalKeystore() { return mOperationalKeystore; }

    /**
     * @brief Add a pending trusted root certificate for the next fabric created with `AddNewPendingFabric*` methods.
     *
     * The root only becomes actually pending when the `AddNewPendingFabric*` is called afterwards. It is reverted
     * if `RevertPendingFabricData` is called.
     *
     * This method with fail with CHIP_ERROR_INCORRECT_STATE in a variety of illogical/inconsistent conditions,
     * which always can be cleared with `RevertPendingFabricData`. Such a situation is calling this method after
     * `UpdatePendingFabric` which would mean logical collision of an addition and an update.
     *
     * @param rcac - Root certificate in Matter Operational Certificate Encoding (TLV) format
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if this is called in an inconsistent order
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient memory to make the root pending
     * @retval CHIP_ERROR_INVALID_ARGUMENT if the RCAC is too large (further checks are done on `AddNewPendingFabric*`)
     * @retval other CHIP_ERROR on internal errors.
     */
    CHIP_ERROR AddNewPendingTrustedRootCert(const ByteSpan & rcac);

    /**
     * @brief Use an NOC and optional ICAC chaining back to the pending RCAC to activate a new fabric
     *
     * Operational key is assumed to be pending or committed in the associated mOperationalKeystore.
     *
     * The fabric becomes temporarily active for purposes of `Fetch*` and `SignWithOpKeyPair`, etc.
     * The new fabric becomes permanent/persisted on successful `CommitPendingFabricData`. It disappears
     * on `RevertPendingFabricData` or `RevertPendingOpCertsExceptRoot`.
     *
     * This method with fail with CHIP_ERROR_INCORRECT_STATE in a variety of illogical/inconsistent conditions,
     * which always can be cleared with `RevertPendingFabricData`. Such a situation is calling this method after
     * `UpdatePendingFabric*` which would mean logical collision of an addition and an update.
     *
     * If a pending key was present in the OperationalKeystore associated with this FabricTable,
     * it is activated on success.
     *
     *
     * @param noc - NOC for the fabric. Must match an existing or pending operational keypair in the mOperationalKeystore.
     * @param icac - ICAC for the fabric. Can be empty if absent from the chain.
     * @param vendorId - VendorID to use for the new fabric
     * @param outNewFabricIndex - Pointer where the new fabric index for the fabric just added will be set. Cannot be nullptr.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_INCORRECT_STATE if this is called in an inconsistent order.
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient memory to make the fabric pending.
     * @retval CHIP_ERROR_INVALID_ARGUMENT if any of the arguments are invalid such as too large or out of bounds.
     * @retval CHIP_ERROR_FABRIC_EXISTS if operational identity collides with one already present.
     * @retval other CHIP_ERROR_* on internal errors or certificate validation errors.
     */
    CHIP_ERROR AddNewPendingFabricWithOperationalKeystore(const ByteSpan & noc, const ByteSpan & icac, uint16_t vendorId,
                                                          FabricIndex * outNewFabricIndex,
                                                          AdvertiseIdentity advertiseIdentity = AdvertiseIdentity::Yes)
    {
        return AddNewPendingFabricCommon(noc, icac, vendorId, nullptr, false, advertiseIdentity, outNewFabricIndex);
    };

    /**
     * @brief Use an NOC and optional ICAC chaining back to the pending RCAC to activate a new fabric
     *
     * Operational key is injected, and then owned by the fabric (!isExistingOpKeyExternallyOwned) or
     * owned externally if `isExistingOpKeyExternallyOwned` is true).
     *
     * WARNING: Copying keypairs is unsafe and not recommended. Consider using
     *          AddNewPendingFabricWithOperationalKeystore and an associated OperationalKeystore
     *          or always using `isExistingOpKeyExternallyOwned`, with `existingOpKey` being a safe
     *          class derived from P256Keypair that avoids the true private key persisting in memory.
     *
     * For rest of semantics outside of operational key, @see AddNewPendingFabricWithOperationalKeystore
     *
     * @param noc - NOC for the fabric. Public key must match the `existingOpKey`'s public key
     * @param icac - ICAC for the fabric. Can be empty if absent from the chain.
     * @param vendorId - VendorID to use for the new fabric
     * @param existingOpKey - Existing operational key to ingest for use in the fabric. Cannot be nullptr.
     * @param isExistingOpKeyExternallyOwned - if true, operational key must outlive the fabric. If false, the key is
     *                                         copied using P256Keypair::Serialize/Deserialize and owned in heap of a FabricInfo.
     * @param outNewFabricIndex - Pointer where the new fabric index for the fabric just added will be set. Cannot be nullptr.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_INCORRECT_STATE if this is called in an inconsistent order.
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient memory to make the fabric pending.
     * @retval CHIP_ERROR_INVALID_ARGUMENT if any of the arguments are invalid such as too large or out of bounds.
     * @retval CHIP_ERROR_FABRIC_EXISTS if operational identity collides with one already present.
     * @retval other CHIP_ERROR_* on internal errors or certificate validation errors.
     */
    CHIP_ERROR AddNewPendingFabricWithProvidedOpKey(const ByteSpan & noc, const ByteSpan & icac, uint16_t vendorId,
                                                    Crypto::P256Keypair * existingOpKey, bool isExistingOpKeyExternallyOwned,
                                                    FabricIndex * outNewFabricIndex,
                                                    AdvertiseIdentity advertiseIdentity = AdvertiseIdentity::Yes)
    {
        return AddNewPendingFabricCommon(noc, icac, vendorId, existingOpKey, isExistingOpKeyExternallyOwned, advertiseIdentity,
                                         outNewFabricIndex);
    };

    /**
     * @brief Use an NOC and optional ICAC to update an existing fabric
     *
     * Operational key is assumed to be pending or committed in the associated mOperationalKeystore.
     *
     * The new NOC chain becomes temporarily active for purposes of `Fetch*` and `SignWithOpKeyPair`, etc.
     * The RCAC remains as before. For this method call to succeed, NOC chain must chain back to the existing RCAC.
     * The update fabric becomes permanent/persisted on successful `CommitPendingFabricData`. Changes revert
     * on `RevertPendingFabricData` or `RevertPendingOpCertsExceptRoot`. FabricId CANNOT be updated, but
     * CAT tags and Node ID in NOC can change between previous and new NOC for a given FabricId.
     *
     * This method with fail with CHIP_ERROR_INCORRECT_STATE in a variety of illogical/inconsistent conditions,
     * which always can be cleared with `RevertPendingFabricData`. Such a situation is calling this method after
     * `AddNewPending*` which would mean logical collision of an addition and an update.
     *
     * If a pending key was present in the OperationalKeystore associated with this FabricTable,
     * it is activated on success.
     *
     * @param fabricIndex - fabricIndex of the existing fabric to update
     * @param noc - Updated NOC for the fabric. Must match an existing or pending operational keypair in the mOperationalKeystore.
     * @param icac - Update ICAC for the fabric. Can be empty if absent from the chain.
     *
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if the `fabricIndex` is not an existing fabric
     * @retval CHIP_ERROR_INCORRECT_STATE if this is called in an inconsistent order
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient memory to store the pending updates
     * @retval CHIP_ERROR_INVALID_ARGUMENT if any of the arguments are invalid such as too large or out of bounds.
     * @retval other CHIP_ERROR_* on internal errors or certificate validation errors.
     */
    CHIP_ERROR UpdatePendingFabricWithOperationalKeystore(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac,
                                                          AdvertiseIdentity advertiseIdentity = AdvertiseIdentity::Yes)
    {
        return UpdatePendingFabricCommon(fabricIndex, noc, icac, nullptr, false, advertiseIdentity);
    }

    /**
     * @brief Use an NOC and optional ICAC to update an existing fabric
     *
     * Operational key is injected, and then owned by the fabric (!isExistingOpKeyExternallyOwned) or
     * owned externally if `isExistingOpKeyExternallyOwned` is true).
     *
     * WARNING: Copying keypairs is unsafe and not recommended. Consider using
     *          AddNewPendingFabricWithOperationalKeystore and an associated OperationalKeystore
     *          or always using `isExistingOpKeyExternallyOwned`, with `existingOpKey` being a safe
     *          class derived from P256Keypair that avoids the true private key persisting in memory.
     *
     * For rest of semantics outside of operational key, @see UpdatePendingFabricWithOperationalKeystore
     *
     * @param fabricIndex - fabricIndex of the existing fabric to update
     * @param noc - Updated NOC for the fabric. Must match an existing or pending operational keypair in the mOperationalKeystore.
     * @param icac - Update ICAC for the fabric. Can be empty if absent from the chain.
     * @param existingOpKey - Existing operational key to ingest for use in the fabric with new NOC. Cannot be nullptr.
     * @param isExistingOpKeyExternallyOwned - if true, operational key must outlive the fabric. If false, the key is
     *                                         copied using P256Keypair::Serialize/Deserialize and owned in heap of a FabricInfo.
     *
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if the `fabricIndex` is not an existing fabric
     * @retval CHIP_ERROR_INCORRECT_STATE if this is called in an inconsistent order
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient memory to store the pending updates
     * @retval CHIP_ERROR_INVALID_ARGUMENT if any of the arguments are invalid such as too large or out of bounds.
     * @retval other CHIP_ERROR_* on internal errors or certificate validation errors.
     */

    CHIP_ERROR UpdatePendingFabricWithProvidedOpKey(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac,
                                                    Crypto::P256Keypair * existingOpKey, bool isExistingOpKeyExternallyOwned,
                                                    AdvertiseIdentity advertiseIdentity = AdvertiseIdentity::Yes)
    {
        return UpdatePendingFabricCommon(fabricIndex, noc, icac, existingOpKey, isExistingOpKeyExternallyOwned, advertiseIdentity);
    }

    /**
     * @brief Commit any pending temporary FabricTable state. This is used mostly for affecting
     *        CommissioningComplete.
     *
     * On success, any pending information is committed such that after a restart, it would
     * be found to be the same in persistent storage.
     *
     * If no changes were pending and state is internally consistent, this appears as a no-op and returns
     * CHIP_NO_ERROR.
     *
     * If there is any internally inconsistent state, this methods acts the same as RevertPendingFabricData(),
     * and all state is lost.
     *
     * In rare circumstances, and depending on the storage backend for opcerts and operational keys,
     * an inconsistent state could be left, such as if restarting during storage writes of
     * CommitPendingFabricData(). If this happens, the next FabricTable::Init() will attempt
     * to clean-up the pieces.
     *
     * @return CHIP_NO_ERROR on success or any other CHIP_ERROR value on internal errors
     */
    CHIP_ERROR CommitPendingFabricData();

    /**
     * @brief Revert any pending state.
     *
     * This is used to handle fail-safe expiry of partially configured fabrics, or to recover
     * from situations where partial state was written and configuration cannot continue properly.
     *
     * All pending certificates and operational keys and pending fabric metadata are cleared.
     */
    void RevertPendingFabricData();

    /**
     * @brief Revert only the pending NOC/ICAC and pending added fabric, not RCAC. Used for error handling
     *        during commissioning.
     */
    void RevertPendingOpCertsExceptRoot();

    // Verifies credentials, using the root certificate of the provided fabric index.
    CHIP_ERROR VerifyCredentials(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac,
                                 Credentials::ValidationContext & context, CompressedFabricId & outCompressedFabricId,
                                 FabricId & outFabricId, NodeId & outNodeId, Crypto::P256PublicKey & outNocPubkey,
                                 Crypto::P256PublicKey * outRootPublicKey = nullptr) const;

    // Verifies credentials, using the provided root certificate.
    static CHIP_ERROR VerifyCredentials(const ByteSpan & noc, const ByteSpan & icac, const ByteSpan & rcac,
                                        Credentials::ValidationContext & context, CompressedFabricId & outCompressedFabricId,
                                        FabricId & outFabricId, NodeId & outNodeId, Crypto::P256PublicKey & outNocPubkey,
                                        Crypto::P256PublicKey * outRootPublicKey = nullptr);
    /**
     * @brief Enables FabricInfo instances to collide and reference the same logical fabric (i.e Root Public Key + FabricId).
     *
     * *WARNING* This is ONLY to be used when creating multiple controllers on the same fabric OR for test.
     *
     */
    void PermitCollidingFabrics() { mStateFlags.Set(StateFlags::kAreCollidingFabricsIgnored); }

    // Add a new fabric for testing. The Operational Key is a raw P256Keypair (public key and private key raw bits) that will
    // get copied (directly) into the fabric table.
    CHIP_ERROR AddNewFabricForTest(const ByteSpan & rootCert, const ByteSpan & icacCert, const ByteSpan & nocCert,
                                   const ByteSpan & opKeySpan, FabricIndex * outFabricIndex);

    // Add a new fabric for testing. The Operational Key is a raw P256Keypair (public key and private key raw bits) that will
    // get copied (directly) into the fabric table. The fabric will NOT be committed, and will remain pending.
    CHIP_ERROR AddNewUncommittedFabricForTest(const ByteSpan & rootCert, const ByteSpan & icacCert, const ByteSpan & nocCert,
                                              const ByteSpan & opKeySpan, FabricIndex * outFabricIndex);

    // Same as AddNewFabricForTest, but ignore if we are colliding with same <Root Public Key, Fabric Id>, so
    // that a single fabric table can have N nodes for same fabric. This usually works, but is bad form.
    CHIP_ERROR AddNewFabricForTestIgnoringCollisions(const ByteSpan & rootCert, const ByteSpan & icacCert, const ByteSpan & nocCert,
                                                     const ByteSpan & opKeySpan, FabricIndex * outFabricIndex)
    {
        PermitCollidingFabrics();
        CHIP_ERROR err = AddNewFabricForTest(rootCert, icacCert, nocCert, opKeySpan, outFabricIndex);
        mStateFlags.Clear(StateFlags::kAreCollidingFabricsIgnored);
        return err;
    }

    // For test only. See definition of `StateFlags::kAbortCommitForTest`.
    void SetForceAbortCommitForTest(bool abortCommitForTest)
    {
        (void) abortCommitForTest;
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        if (abortCommitForTest)
        {
            mStateFlags.Set(StateFlags::kAbortCommitForTest);
        }
        else
        {
            mStateFlags.Clear(StateFlags::kAbortCommitForTest);
        }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
    }

    /**
     * Get the fabric index that will be used for the next fabric that will be
     * added.  Returns error if no more fabrics can be added, otherwise writes
     * the fabric index that will be used for the next addition into the
     * outparam.
     */
    CHIP_ERROR PeekFabricIndexForNextAddition(FabricIndex & outIndex);

    /**
     * Set the fabric index that will be used fo the next fabric added.
     *
     * Returns an error if the |fabricIndex| is already in use.
     */
    CHIP_ERROR SetFabricIndexForNextAddition(FabricIndex fabricIndex);

    /**
     * @brief Set the advertising behavior for the fabric identified by `fabricIndex`.
     *
     * It is the caller's responsibility to actually restart DNS-SD advertising
     * as needed after updating this state.
     *
     * @param fabricIndex - Fabric Index for which to set the label
     * @param advertiseIdentity - whether the identity for this fabric should be advertised.
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if fabricIndex does not refer to a fabric in the table
     */
    CHIP_ERROR SetShouldAdvertiseIdentity(FabricIndex fabricIndex, AdvertiseIdentity advertiseIdentity);

private:
    enum class StateFlags : uint16_t
    {
        // If true, we are in the process of a fail-safe and there was at least one
        // operation that caused partial data in the fabric table.
        kIsPendingFabricDataPresent = (1u << 0),
        kIsTrustedRootPending       = (1u << 1),
        kIsUpdatePending            = (1u << 2),
        kIsAddPending               = (1u << 3),

        // Only true when `AllocatePendingOperationalKey` has been called
        kIsOperationalKeyPending = (1u << 4),
        // True if `AllocatePendingOperationalKey` was for an existing fabric
        kIsPendingKeyForUpdateNoc = (1u << 5),

        // True if we allow more than one fabric with same root and fabricId in the fabric table
        // for test purposes. This disables a collision check.
        kAreCollidingFabricsIgnored = (1u << 6),

        // If set to true (only possible on test builds), will cause `CommitPendingFabricData()` to early
        // return during commit, skipping clean-ups, so that we can validate commit marker fabric removal.
        kAbortCommitForTest = (1u << 7),
    };

    // Stored to indicate a commit is in progress, so that it can be cleaned-up on next boot
    // if stopped in the middle.
    struct CommitMarker
    {
        CommitMarker() = default;
        CommitMarker(FabricIndex fabricIndex_, bool isAddition_)
        {
            this->fabricIndex = fabricIndex_;
            this->isAddition  = isAddition_;
        }
        FabricIndex fabricIndex = kUndefinedFabricIndex;
        bool isAddition         = false;
    };

    /**
     * @brief Get a mutable FabricInfo entry from the table by FabricIndex.
     *
     * NOTE: This is private for use within the FabricTable itself. All mutations have to go through the
     *       FabricTable public methods that take a FabricIndex so that there are no mutations about which
     *       the FabricTable is unaware, since this would break expectations regarding shadow/pending
     *       entries used during fail-safe.
     *
     * @param fabricIndex - fabric index for which to get a mutable FabricInfo entry
     * @return the FabricInfo entry for the fabricIndex if found, or nullptr if not found
     */
    FabricInfo * GetMutableFabricByIndex(FabricIndex fabricIndex);

    // Load a FabricInfo metatada item from storage for a given new fabric index. Returns internal error on failure.
    CHIP_ERROR LoadFromStorage(FabricInfo * fabric, FabricIndex newFabricIndex);

    // Store a given fabric metadata directly/immediately. Used by internal operations.
    CHIP_ERROR StoreFabricMetadata(const FabricInfo * fabricInfo) const;

    // Tries to set `mFabricIndexWithPendingState` and returns false if there's a clash.
    bool SetPendingDataFabricIndex(FabricIndex fabricIndex);

    // Core validation logic for fabric additions/updates
    CHIP_ERROR AddOrUpdateInner(FabricIndex fabricIndex, bool isAddition, Crypto::P256Keypair * existingOpKey,
                                bool isExistingOpKeyExternallyOwned, uint16_t vendorId, AdvertiseIdentity advertiseIdentity);

    // Common code for fabric addition, for either OperationalKeystore or injected key scenarios.
    CHIP_ERROR AddNewPendingFabricCommon(const ByteSpan & noc, const ByteSpan & icac, uint16_t vendorId,
                                         Crypto::P256Keypair * existingOpKey, bool isExistingOpKeyExternallyOwned,
                                         AdvertiseIdentity advertiseIdentity, FabricIndex * outNewFabricIndex);

    // Common code for fabric updates, for either OperationalKeystore or injected key scenarios.
    CHIP_ERROR UpdatePendingFabricCommon(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac,
                                         Crypto::P256Keypair * existingOpKey, bool isExistingOpKeyExternallyOwned,
                                         AdvertiseIdentity advertiseIdentity);

    // Common code for looking up a fabric given a root public key, a fabric ID and an optional node id scoped to that fabric.
    const FabricInfo * FindFabricCommon(const Crypto::P256PublicKey & rootPubKey, FabricId fabricId,
                                        NodeId nodeId = kUndefinedNodeId) const;

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
     * @brief Delete all metadata from storage for the given fabric
     *
     * @param fabricIndex FabricIndex for which to delete the metadadata
     * @return CHIP_NO_ERROR on success or another CHIP_ERROR on failure
     */
    CHIP_ERROR DeleteMetadataFromStorage(FabricIndex fabricIndex);

    /**
     * @brief Determine if a collision (undesired on AddNOC, necessary on UpdateNOC) exists
     *        between the FabricID in the given noc, and the RCAC found for `currentFabricIndex`
     *        in the op cert store, against an existing fabric in the FabricTable (which could be pending)
     *
     * @param currentFabricIndex - pending fabricIndex for which we are trying to Add/Update a NOC
     * @param noc - NOC cert received that contains FabricID whose collision we care to validate
     * @param outMatchingFabricIndex - set to the FabricIndex matching the collision or kUndefinedFabricIndex on no collision found
     * @return CHIP_NO_ERROR on successful update of outMatchingFabricIndex or other CHIP_ERROR on internal errors
     */
    CHIP_ERROR FindExistingFabricByNocChaining(FabricIndex currentFabricIndex, const ByteSpan & noc,
                                               FabricIndex & outMatchingFabricIndex) const;

    /**
     * @brief Get the shadow FabricInfo entry that is pending for updates, if an
     *        update is in progress.
     *
     * @return a pointer to the shadow pending fabric or nullptr if none is active.
     */
    const FabricInfo * GetShadowPendingFabricEntry() const { return HasPendingFabricUpdate() ? &mPendingFabric : nullptr; }

    // Returns true if we have a shadow entry pending for a fabric update.
    bool HasPendingFabricUpdate() const
    {
        return mPendingFabric.IsInitialized() &&
            mStateFlags.HasAll(StateFlags::kIsPendingFabricDataPresent, StateFlags::kIsUpdatePending);
    }

    // Validate an NOC chain at time of adding/updating a fabric (uses VerifyCredentials with additional checks).
    // The `existingFabricId` is passed for UpdateNOC, and must match the Fabric, to make sure that we are
    // not trying to change FabricID with UpdateNOC. If set to kUndefinedFabricId, we are doing AddNOC and
    // we don't need to check match to pre-existing fabric.
    static CHIP_ERROR ValidateIncomingNOCChain(const ByteSpan & noc, const ByteSpan & icac, const ByteSpan & rcac,
                                               FabricId existingFabricId, Credentials::CertificateValidityPolicy * policy,
                                               CompressedFabricId & outCompressedFabricId, FabricId & outFabricId,
                                               NodeId & outNodeId, Crypto::P256PublicKey & outNocPubkey,
                                               Crypto::P256PublicKey & outRootPubkey);

    /**
     * Read our fabric index info from the given TLV reader and set up the
     * fabric table accordingly.
     */
    CHIP_ERROR ReadFabricInfo(TLV::ContiguousBufferTLVReader & reader);

    CHIP_ERROR NotifyFabricUpdated(FabricIndex fabricIndex);
    CHIP_ERROR NotifyFabricCommitted(FabricIndex fabricIndex);

    // Commit management clean-up APIs
    CHIP_ERROR StoreCommitMarker(const CommitMarker & commitMarker);
    CHIP_ERROR GetCommitMarker(CommitMarker & outCommitMarker);

    FabricInfo mStates[CHIP_CONFIG_MAX_FABRICS];
    // Used for UpdateNOC pending fabric updates
    FabricInfo mPendingFabric;
    PersistentStorageDelegate * mStorage                    = nullptr;
    Crypto::OperationalKeystore * mOperationalKeystore      = nullptr;
    Credentials::OperationalCertificateStore * mOpCertStore = nullptr;

    // FabricTable::Delegate link to first node, since FabricTable::Delegate is a form
    // of intrusive linked-list item.
    FabricTable::Delegate * mDelegateListRoot = nullptr;

    // When mStateFlags.Has(kIsPendingFabricDataPresent) is true, this holds the index of the fabric
    // for which there is currently pending data.
    FabricIndex mFabricIndexWithPendingState = kUndefinedFabricIndex;

    // For when a revert occurs during init, so that more clean-up can be scheduled by caller.
    FabricIndex mDeletedFabricIndexFromInit = kUndefinedFabricIndex;

    LastKnownGoodTime mLastKnownGoodTime;

    // We may not have an mNextAvailableFabricIndex if our table is as large as
    // it can go and is full.
    Optional<FabricIndex> mNextAvailableFabricIndex;
    uint8_t mFabricCount = 0;

    BitFlags<StateFlags> mStateFlags;
};

} // namespace chip
