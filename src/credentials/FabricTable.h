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
#include <crypto/CHIPCryptoPAL.h>
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
 * Defines state of a pairing established by a fabric.
 * Node ID is only settable using the device operational credentials.
 *
 * Information contained within the state:
 *   - Fabric identification
 *   - Node Id assigned by the fabric to the device
 *   - Vendor Id
 *   - Fabric Id
 *   - Device operational credentials
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

    Crypto::P256Keypair * GetOperationalKey() const { return mOperationalKey; }

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

    // TODO - Update these APIs to take ownership of the buffer, instead of copying
    //        internally.
    // TODO - Optimize persistent storage of NOC and Root Cert in FabricInfo.
    CHIP_ERROR SetRootCert(const chip::ByteSpan & cert) { return SetCert(mRootCert, cert); }
    CHIP_ERROR SetICACert(const chip::ByteSpan & cert) { return SetCert(mICACert, cert); }
    CHIP_ERROR SetICACert(const Optional<ByteSpan> & cert) { return SetICACert(cert.ValueOr(ByteSpan())); }
    CHIP_ERROR SetNOCCert(const chip::ByteSpan & cert) { return SetCert(mNOCCert, cert); }

    bool IsInitialized() const { return IsOperationalNodeId(mOperationalId.GetNodeId()); }

    // TODO - Refactor storing and loading of fabric info from persistent storage.
    //        The op cert array doesn't need to be in RAM except when it's being
    //        transmitted to peer node during CASE session setup.
    CHIP_ERROR GetRootCert(ByteSpan & cert) const
    {
        ReturnErrorCodeIf(mRootCert.empty(), CHIP_ERROR_INCORRECT_STATE);
        cert = mRootCert;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetICACert(ByteSpan & cert) const
    {
        cert = mICACert;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetNOCCert(ByteSpan & cert) const
    {
        ReturnErrorCodeIf(mNOCCert.empty(), CHIP_ERROR_INCORRECT_STATE);
        cert = mNOCCert;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetTrustedRootId(Credentials::CertificateKeyId & skid) const
    {
        return Credentials::ExtractSKIDFromChipCert(mRootCert, skid);
    }

    CHIP_ERROR GetRootPubkey(Credentials::P256PublicKeySpan & publicKey) const
    {
        return Credentials::ExtractPublicKeyFromChipCert(mRootCert, publicKey);
    }

    /**
     * Get the effective NotBefore time for a certificate chain, which is
     * the latest NotBefore time in the chain.  Note that chip TLV certificate
     * format stores the  X.509/RFC5280 special time 99991231235959Z 'no
     * well-defined expiration date' as 0.  In this context, we can interpret
     * that as either NotBefore == CHIP epoch or NotBefore == Don't care, and
     * this will evaluate the same in both cases.
     *
     * @param certChainNotBefore (out) effective NotBefore time for the certiticate chain in seconds from CHIP epoch
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR GetNotBeforeChipEpochTime(chip::System::Clock::Seconds32 & certChainNotBefore) const;

    // Verifies credentials, using this fabric info's root certificate.
    CHIP_ERROR VerifyCredentials(const ByteSpan & noc, const ByteSpan & icac, Credentials::ValidationContext & context,
                                 PeerId & nocPeerId, FabricId & fabricId, Crypto::P256PublicKey & nocPubkey) const;

    // Verifies credentials, using the provided root certificate.
    static CHIP_ERROR VerifyCredentials(const ByteSpan & noc, const ByteSpan & icac, const ByteSpan & rcac,
                                        Credentials::ValidationContext & context, PeerId & nocPeerId, FabricId & fabricId,
                                        Crypto::P256PublicKey & nocPubkey);

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

    CHIP_ERROR SetFabricInfo(FabricInfo & fabric);

    /* Generate a compressed peer ID (containing compressed fabric ID) using provided fabric ID, node ID and
       root public key of the provided root certificate. The generated compressed ID is returned via compressedPeerId
       output parameter */
    static CHIP_ERROR GeneratePeerId(const ByteSpan & rcac, FabricId fabricId, NodeId nodeId, PeerId * compressedPeerId);

    friend class FabricTable;

    // Test-only, build a fabric using given root cert and NOC
    CHIP_ERROR TestOnlyBuildFabric(ByteSpan rootCert, ByteSpan icacCert, ByteSpan nocCert, ByteSpan nocKey);

private:
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

    FabricInfo * FindFabric(Credentials::P256PublicKeySpan rootPubKey, FabricId fabricId);
    FabricInfo * FindFabricWithIndex(FabricIndex fabricIndex);
    FabricInfo * FindFabricWithCompressedId(CompressedFabricId fabricId);

    CHIP_ERROR Init(PersistentStorageDelegate * storage);
    CHIP_ERROR AddFabricDelegate(FabricTable::Delegate * delegate);
    void RemoveFabricDelegate(FabricTable::Delegate * delegate);

    uint8_t FabricCount() const { return mFabricCount; }

    ConstFabricIterator cbegin() const { return ConstFabricIterator(mStates, 0, CHIP_CONFIG_MAX_FABRICS); }
    ConstFabricIterator cend() const { return ConstFabricIterator(mStates, CHIP_CONFIG_MAX_FABRICS, CHIP_CONFIG_MAX_FABRICS); }
    ConstFabricIterator begin() const { return cbegin(); }
    ConstFabricIterator end() const { return cend(); }

    /**
     * Get the current Last Known Good Time.
     *
     * @param lastKnownGoodChipEpochTime (out) the current last known good time, if any is known
     * &return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR GetLastKnownGoodChipEpochTime(System::Clock::Seconds32 & lastKnownGoodChipEpochTime) const
    {
        VerifyOrReturnError(mLastKnownGoodChipEpochTime.HasValue(), CHIP_ERROR_INCORRECT_STATE);
        lastKnownGoodChipEpochTime = mLastKnownGoodChipEpochTime.Value();
        return CHIP_NO_ERROR;
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
     * Revert the Last Known Good Time to the fail-safe backup value in
     * persistence if any exists.
     *
     * @return CHIP_NO_ERROR on success, else an appopriate CHIP_ERROR
     */
    CHIP_ERROR RevertLastKnownGoodChipEpochTime();

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
     * Store our current fabric index state: what our next available index is
     * and what indices we're using right now.
     */
    CHIP_ERROR StoreFabricIndexInfo() const;

    static constexpr size_t LastKnownGoodTimeTLVMaxSize()
    {
        // We have Last Known Good Time and, optionally, a previous Last known
        // good time for fail safe cleanup.
        return TLV::EstimateStructOverhead(sizeof(uint32_t), sizeof(uint32_t));
    }

    /**
     * Initialize Last Known Good Time to the later of firmware build time or
     * last known good time persisted in storage.  Persist the selected value
     * if this differs from the value in storage or no value is yet persisted.
     *
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR InitLastKnownGoodChipEpochTime();

    /**
     * Update the Last Known Good Time to the later of the current value and
     * the passed value and persist to storage.  If the value is changed, also
     * store the current value for fail-safe recovery.
     *
     * We can only support storage of a single fail-safe recovery value.  But by
     * by being private and only calling from AddNewFabric, we ensure that on
     * nodes and fabric additions where fail-safe recovery is required, we will
     * be called from within a fail-safe context.
     *
     * @param lastKnownGoodChipEpochTime Last Known Good Time in seconds since CHIP epoch
     * @return CHIP_NO_ERROR on success, else an appopriate CHIP_ERROR
     */
    CHIP_ERROR UpdateLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime);

    /**
     * Load the Last Known Good Time from storage and, optionally, a fail-safe
     * value to fall back to if any exists.
     *
     * @param lastKnownGoodChipEpochTime (out) Last Known Good Time as seconds from CHIP epoch
     * @param failSafeBackup (out) an optional Fail Safe context last known good time value to fall back to, also in seconds from
     * CHIP epoch from CHIP epoch
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR LoadLastKnownGoodChipEpochTime(System::Clock::Seconds32 & lastKnownGoodChipEpochTime,
                                              Optional<System::Clock::Seconds32> & failSafeBackup) const;

    /**
     * Load the Last Known Good Time from storage.
     *
     * @param lastKnownGoodChipEpochTime (out) Last Known Good Time as seconds from CHIP epoch
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR LoadLastKnownGoodChipEpochTime(System::Clock::Seconds32 & lastKnownGoodChipEpochTime) const;

    /**
     * Store the Last Known Good Time to storage, and optionally, a fail-safe
     * value to fall back to if the fail safe timer expires.
     *
     * @param lastKnownGoodChipEpochTime Last Known Good Time as seconds from CHIP epoch
     * @param failSafeBackup fail safe backup of the previous Last Known Good Time
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR StoreLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime,
                                               Optional<System::Clock::Seconds32> failSafeBackup) const;

    /**
     * Store the Last Known Good Time to storage.  This overload also clears
     * the fail safe Last Known Good Time from storage.
     *
     * @param lastKnownGoodChipEpochTime Last Known Good Time as seconds from CHIP epoch
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR StoreLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime) const;

    /**
     * Format print a CHIP epoch time as a null-terminated ISO 8601 string to a
     * static internal buffer and return a pointer to this.
     *
     * This is provided here as a logging convenience for local code, which
     * must frequently print Last Known Good Time.  Callers do not own the
     * return buffer and should not try to use it later.
     *
     * @param chipEpochTime time in seconds from the CHIP epoch
     * @return null-terminted ISO8601 string
     */
    static const char * FormatChipEpochTime(System::Clock::Seconds32 chipEpochTime);

    /**
     * Read our fabric index info from the given TLV reader and set up the
     * fabric table accordingly.
     */
    CHIP_ERROR ReadFabricInfo(TLV::ContiguousBufferTLVReader & reader);

    CHIP_ERROR AddNewFabricInner(FabricInfo & fabric, FabricIndex * assignedIndex);

    FabricInfo mStates[CHIP_CONFIG_MAX_FABRICS];
    PersistentStorageDelegate * mStorage = nullptr;

    // FabricTable::Delegate link to first node, since FabricTable::Delegate is a form
    // of intrusive linked-list item.
    FabricTable::Delegate * mDelegateListRoot = nullptr;

    // We may not have an mNextAvailableFabricIndex if our table is as large as
    // it can go and is full.
    Optional<FabricIndex> mNextAvailableFabricIndex;
    uint8_t mFabricCount = 0;
    Optional<System::Clock::Seconds32> mLastKnownGoodChipEpochTime;
};

} // namespace chip
