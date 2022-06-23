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
#include <lib/core/CHIPPersistentStorageDelegate.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/Optional.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/support/BitFlags.h>
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
    ~FabricInfo() { Reset(); }

    // Non-copyable
    FabricInfo(FabricInfo const &) = delete;
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

    bool IsInitialized() const { return (mFabricIndex != kUndefinedFabricIndex) && IsOperationalNodeId(mNodeId); }

    bool HasOperationalKey() const { return mOperationalKey != nullptr; }

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
        mOperationalKey = nullptr;

        mFabricIndex = kUndefinedFabricIndex;
        mNodeId      = kUndefinedNodeId;
    }

    friend class FabricTable;

protected:
    struct InitParams
    {
        NodeId nodeId                         = kUndefinedNodeId;
        FabricId fabricId                     = kUndefinedFabricId;
        FabricIndex fabricIndex               = kUndefinedFabricIndex;
        CompressedFabricId compressedFabricId = kUndefinedCompressedFabricId;
        Crypto::P256PublicKey rootPublicKey;
        uint16_t vendorId                        = VendorId::NotSpecified; /**< Vendor ID for commissioner of fabric */
        Crypto::P256Keypair * operationalKeypair = nullptr;
        bool hasExternallyOwnedKeypair           = false;

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
     * @brief Sign a message with the fabric's operational private key. This ONLY
     *        works if `SetOperationalKeypair` or `SetExternallyOwnedOperationalKeypair`
     *        had been called and is an API that is present ONLY to be called by FabricTable.
     *
     * @param message - message to sign
     * @param outSignature - buffer to hold the signature
     * @return CHIP_NO_ERROR on success or another CHIP_ERROR on crypto internal errors
     */
    CHIP_ERROR SignWithOpKeypair(ByteSpan message, Crypto::P256ECDSASignature & outSignature) const;

    CHIP_ERROR FetchRootPubkey(Crypto::P256PublicKey & outPublicKey) const;

    static constexpr size_t MetadataTLVMaxSize()
    {
        return TLV::EstimateStructOverhead(sizeof(uint16_t), kFabricLabelMaxLengthInBytes);
    }

    static constexpr size_t OpKeyTLVMaxSize()
    {
        return TLV::EstimateStructOverhead(sizeof(uint16_t), Crypto::P256SerializedKeypair::Capacity());
    }

    NodeId mNodeId           = kUndefinedNodeId;
    FabricId mFabricId       = kUndefinedFabricId;
    FabricIndex mFabricIndex = kUndefinedFabricIndex;
    // We cache the compressed fabric id since it's used so often and costly to get.
    CompressedFabricId mCompressedFabricId = kUndefinedCompressedFabricId;
    // We cache the root public key since it's used so often and costly to get.
    Crypto::P256PublicKey mRootPublicKey;

    uint16_t mVendorId                                  = static_cast<uint16_t>(VendorId::NotSpecified);
    char mFabricLabel[kFabricLabelMaxLengthInBytes + 1] = { '\0' };

#ifdef ENABLE_HSM_CASE_OPS_KEY
    mutable Crypto::P256KeypairHSM * mOperationalKey = nullptr;
#else
    mutable Crypto::P256Keypair * mOperationalKey = nullptr;
#endif
    bool mHasExternallyOwnedOperationalKey = false;

    CHIP_ERROR CommitToStorage(PersistentStorageDelegate * storage) const;
    CHIP_ERROR LoadFromStorage(PersistentStorageDelegate * storage, FabricIndex newFabricIndex, const ByteSpan & rcac,
                               const ByteSpan & noc);
};

/**
 * Iterates over valid fabrics within a list
 */

// TODO: Convert iterator to deal with pending fabric!
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
    FabricTable(FabricTable const &) = delete;
    void operator=(FabricTable const &) = delete;

    // Returns CHIP_ERROR_NOT_FOUND if there is no fabric for that index.
    CHIP_ERROR Delete(FabricIndex fabricIndex);
    void DeleteAllFabrics();

    // TODO this #if CONFIG_BUILD_FOR_HOST_UNIT_TEST is temporary. There is a change incoming soon
    // that will allow triggering NOC update directly.
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    void SendUpdateFabricNotificationForTest(FabricIndex fabricIndex) { NotifyFabricUpdated(fabricIndex); }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

    FabricInfo * FindFabric(const Crypto::P256PublicKey & rootPubKey, FabricId fabricId);
    FabricInfo * FindFabricWithIndex(FabricIndex fabricIndex);
    const FabricInfo * FindFabricWithIndex(FabricIndex fabricIndex) const;
    FabricInfo * FindFabricWithCompressedId(CompressedFabricId compressedFabricId);

    CHIP_ERROR Init(const FabricTable::InitParams & initParams);
    void Shutdown();

    // Forget a fabric in memory: doesn't delete any persistent state, just
    // reverts any pending state (blindly) and then make the fabric table
    // entry get reset.
    void Forget(FabricIndex fabricIndex);

    CHIP_ERROR AddFabricDelegate(FabricTable::Delegate * delegate);
    void RemoveFabricDelegate(FabricTable::Delegate * delegate);

    // Set the Fabric Label for the given fabricIndex. If a fabric add/update is pending,
    // only the pending version will be updated, so that on fail-safe expiry, you would
    // actually see the only fabric label if Update fails. If the fabric label is
    // set before UpdateNOC, then the change is immediate.
    CHIP_ERROR SetFabricLabel(FabricIndex fabricIndex, const CharSpan & fabricLabel);

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
     *        previously successfully called, false otherwise.
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

    // TODO: REVIEW DOCS
    CHIP_ERROR AddNewPendingTrustedRootCert(const ByteSpan & rcac);

    // TODO: REVIEW DOCS
    CHIP_ERROR AddNewPendingFabricWithOperationalKeystore(const ByteSpan & noc, const ByteSpan & icac, uint16_t vendorId,
                                                          FabricIndex * outNewFabricIndex)
    {
        return AddNewPendingFabricCommon(noc, icac, vendorId, nullptr, false, outNewFabricIndex);
    };

    // TODO: REVIEW DOCS
    CHIP_ERROR AddNewPendingFabricWithProvidedOpKey(const ByteSpan & noc, const ByteSpan & icac, uint16_t vendorId,
                                                    Crypto::P256Keypair * existingOpKey, bool isExistingOpKeyExternallyOwned,
                                                    FabricIndex * outNewFabricIndex)
    {
        return AddNewPendingFabricCommon(noc, icac, vendorId, existingOpKey, isExistingOpKeyExternallyOwned, outNewFabricIndex);
    };

    // TODO: REVIEW DOCS
    CHIP_ERROR UpdatePendingFabricWithOperationalKeystore(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac)
    {
        return UpdatePendingFabricCommon(fabricIndex, noc, icac, nullptr, false);
    }

    CHIP_ERROR UpdatePendingFabricWithProvidedOpKey(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac,
                                                    Crypto::P256Keypair * existingOpKey, bool isExistingOpKeyExternallyOwned)
    {
        return UpdatePendingFabricCommon(fabricIndex, noc, icac, existingOpKey, isExistingOpKeyExternallyOwned);
    }

    /**
     * @brief Commit any pending temporary FabricTable state. This is used mostly for affecting
     *        CommissioningComplete.
     *
     * @return CHIP_NO_ERROR on success or any other CHIP_ERROR value on internal errors
     */
    CHIP_ERROR CommitPendingFabricData();

    /**
     * @brief Revert any pending state. This is used to handle fail-safe expiry of partially
     *        configured fabrics.
     */
    void RevertPendingFabricData();

    /**
     * @brief Revert only the pending NOC/ICAC, not RCAC. Used for error handling during commissioning.
     */
    void RevertPendingOpCertsExceptRoot();

    // Verifies credentials, with the fabric's root under fabricIndex, and extract critical bits.
    // This call is used for CASE.
    CHIP_ERROR VerifyCredentials(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac,
                                 Credentials::ValidationContext & context, CompressedFabricId & outCompressedFabricId,
                                 FabricId & outFabricId, NodeId & outNodeId, Crypto::P256PublicKey & outNocPubkey,
                                 Crypto::P256PublicKey * outRootPublicKey = nullptr) const;

    // Verifies credentials, using the provided root certificate.
    // This call is done whenever a fabric is "directly" added
    static CHIP_ERROR VerifyCredentials(const ByteSpan & noc, const ByteSpan & icac, const ByteSpan & rcac,
                                        Credentials::ValidationContext & context, CompressedFabricId & outCompressedFabricId,
                                        FabricId & outFabricId, NodeId & outNodeId, Crypto::P256PublicKey & outNocPubkey,
                                        Crypto::P256PublicKey * outRootPublicKey);

    // Validate an NOC chain at time of adding/updating a fabric (uses VerifyCredentials with additional checks).
    // The `existingFabricId` is passed for UpdateNOC, and must match the Fabric, to make sure that we are
    // not trying to change FabricID with UpdateNOC. If set to kUndefinedFabricId, we are doing AddNOC and
    // we don't need to check match to pre-existing fabric.
    CHIP_ERROR ValidateIncomingNOCChain(const ByteSpan & noc, const ByteSpan & icac, const ByteSpan & rcac,
                                        FabricId existingFabricId, Credentials::CertificateValidityPolicy * policy,
                                        CompressedFabricId & outCompressedFabricId, FabricId & outFabricId, NodeId & outNodeId,
                                        Crypto::P256PublicKey & outNocPubkey) const;

    // Add a new fabric for testing. The Operational Key is a raw P256Keypair (public key and private key raw bits) that will
    // get copied (directly) into the fabric table.
    CHIP_ERROR AddNewFabricForTest(const ByteSpan & rootCert, const ByteSpan & icacCert, const ByteSpan & nocCert,
                                   const ByteSpan & opKeySpan, FabricIndex * outFabricIndex);

    // Same as AddNewFabricForTest, but ignore if we are colliding with same <Root Public Key, Fabric Id>, so
    // that a single fabric table can have N nodes for same fabric. This usually works, but is bad form.
    CHIP_ERROR AddNewFabricForTestIgnoringCollisions(const ByteSpan & rootCert, const ByteSpan & icacCert, const ByteSpan & nocCert,
                                                     const ByteSpan & opKeySpan, FabricIndex * outFabricIndex)
    {
        mStateFlags.Set(StateFlags::kAreCollidingFabricsIgnored);
        CHIP_ERROR err = AddNewFabricForTest(rootCert, icacCert, nocCert, opKeySpan, outFabricIndex);
        mStateFlags.Clear(StateFlags::kAreCollidingFabricsIgnored);
        return err;
    }

private:
    enum class StateFlags : uint8_t
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
    };

    static constexpr size_t IndexInfoTLVMaxSize()
    {
        // We have a single next-available index and an array of anonymous-tagged
        // fabric indices.
        //
        // The max size of the list is (1 byte control + bytes for actual value)
        // times max number of list items, plus one byte for the list terminator.
        return TLV::EstimateStructOverhead(sizeof(FabricIndex), CHIP_CONFIG_MAX_FABRICS * (1 + sizeof(FabricIndex)) + 1);
    }

    // Load a FabricInfo metatada item from storage for a given new fabric index Returns internal error on failure.
    CHIP_ERROR LoadFromStorage(FabricInfo * fabric, FabricIndex newFabricIndex);

    // Store a given fabric metadata directly/immediately. Used by internal operations.
    CHIP_ERROR StoreFabricMetadata(const FabricInfo * fabricInfo) const;

    // Tries to set `mFabricIndexWithPendingState` and returns false if there's a clash
    bool SetPendingDataFabricIndex(FabricIndex fabricIndex);

    CHIP_ERROR AddOrUpdateInner(FabricIndex fabricIndex, Crypto::P256Keypair * existingOpKey, bool isExistingOpKeyExternallyOwned,
                                uint16_t vendorId, FabricIndex * outputIndex);

    CHIP_ERROR AddNewPendingFabricCommon(const ByteSpan & noc, const ByteSpan & icac, uint16_t vendorId,
                                         Crypto::P256Keypair * existingOpKey, bool isExistingOpKeyExternallyOwned,
                                         FabricIndex * outNewFabricIndex);
    CHIP_ERROR UpdatePendingFabricCommon(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac,
                                         Crypto::P256Keypair * existingOpKey, bool isExistingOpKeyExternallyOwned);

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
     * Read our fabric index info from the given TLV reader and set up the
     * fabric table accordingly.
     */
    CHIP_ERROR ReadFabricInfo(TLV::ContiguousBufferTLVReader & reader);

    CHIP_ERROR NotifyFabricUpdated(FabricIndex fabricIndex);
    CHIP_ERROR NotifyFabricCommitted(FabricIndex fabricIndex);

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

    LastKnownGoodTime mLastKnownGoodTime;

    // We may not have an mNextAvailableFabricIndex if our table is as large as
    // it can go and is full.
    Optional<FabricIndex> mNextAvailableFabricIndex;
    uint8_t mFabricCount = 0;

    BitFlags<StateFlags> mStateFlags;
};

} // namespace chip
