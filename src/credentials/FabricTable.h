/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/Optional.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Span.h>

#ifdef ENABLE_HSM_CASE_OPS_KEY
#define CASE_OPS_KEY 0xCA5EECC0
#endif

namespace chip {

static constexpr FabricIndex kMinValidFabricIndex     = 1;
static constexpr FabricIndex kMaxValidFabricIndex     = std::min<FabricIndex>(UINT8_MAX - 1, CHIP_CONFIG_MAX_FABRICS);
static constexpr uint8_t kFabricLabelMaxLengthInBytes = 32;

static_assert(kUndefinedFabricIndex < chip::kMinValidFabricIndex, "Undefined fabric index should not be valid");

// KVS store is sensitive to length of key strings, based on the underlying
// platform. Keeping them short.
constexpr char kFabricTableKeyPrefix[] = "Fabric";
constexpr char kFabricTableCountKey[]  = "NumFabrics";

class DLL_EXPORT FabricStorage
{
public:
    virtual ~FabricStorage() {}

    /**
     * Gets called when fabric data needs to be stored.
     **/
    virtual CHIP_ERROR SyncStore(FabricIndex fabricIndex, const char * key, const void * buffer, uint16_t size) = 0;

    /**
     * Gets called when fabric data needs to be loaded.
     **/
    virtual CHIP_ERROR SyncLoad(FabricIndex fabricIndex, const char * key, void * buffer, uint16_t & size) = 0;

    /**
     * Gets called when fabric data needs to be removed.
     **/
    virtual CHIP_ERROR SyncDelete(FabricIndex fabricIndex, const char * key) = 0;
};

/**
 * @brief A default implementation of Fabric storage that preserves legacy behavior of using
 *        the Persistent storage delegate directly.
 *
 *        This class automatically prefixes the Fabric Storage Keys with the FabricIndex.
 *        The keys are formatted like so: "F%02X/" + key.
 *
 */
class DLL_EXPORT SimpleFabricStorage : public FabricStorage
{
public:
    SimpleFabricStorage(){};
    SimpleFabricStorage(PersistentStorageDelegate * storage) : mStorage(storage){};
    ~SimpleFabricStorage() override { mStorage = nullptr; };

    CHIP_ERROR Initialize(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(mStorage == nullptr || storage == mStorage, CHIP_ERROR_INCORRECT_STATE);
        mStorage = storage;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncStore(FabricIndex fabricIndex, const char * key, const void * buffer, uint16_t size) override;

    CHIP_ERROR SyncLoad(FabricIndex fabricIndex, const char * key, void * buffer, uint16_t & size) override;

    CHIP_ERROR SyncDelete(FabricIndex fabricIndex, const char * key) override;

private:
    const static int MAX_KEY_SIZE = 32;

    PersistentStorageDelegate * mStorage = nullptr;
};

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
    FabricInfo()
    {
        Reset();
        mFabric = kUndefinedFabricIndex;
    }

    // Returns a span into our internal storage.
    CharSpan GetFabricLabel() const { return CharSpan(mFabricLabel, strnlen(mFabricLabel, kFabricLabelMaxLengthInBytes)); }

    CHIP_ERROR SetFabricLabel(const CharSpan & fabricLabel);

    ~FabricInfo()
    {
        if (mOperationalKey != nullptr)
        {
            chip::Platform::Delete(mOperationalKey);
        }
        ReleaseOperationalCerts();
    }

    NodeId GetNodeId() const { return mOperationalId.GetNodeId(); }
    // TODO(#15049): Refactor/rename PeerId to OperationalId or OpId throughout source
    PeerId GetPeerId() const { return mOperationalId; }
    PeerId GetPeerIdForNode(const NodeId node) const
    {
        PeerId peer = mOperationalId;
        peer.SetNodeId(node);
        return peer;
    }

    FabricId GetFabricId() const { return mFabricId; }
    FabricIndex GetFabricIndex() const { return mFabric; }
    uint16_t GetVendorId() const { return mVendorId; }

    void SetVendorId(uint16_t vendorId) { mVendorId = vendorId; }

    Crypto::P256Keypair * GetOperationalKey()
    {
        if (mOperationalKey == nullptr)
        {
#ifdef ENABLE_HSM_CASE_OPS_KEY
            mOperationalKey = chip::Platform::New<Crypto::P256KeypairHSM>();
            mOperationalKey->SetKeyId(CASE_OPS_KEY);
#else
            mOperationalKey = chip::Platform::New<Crypto::P256Keypair>();
#endif
            mOperationalKey->Initialize();
#ifdef ENABLE_HSM_CASE_OPS_KEY
            // Set provisioned_key = true , so that key is not deleted from HSM.
            mOperationalKey->provisioned_key = true;
#endif
        }
        return mOperationalKey;
    }
    CHIP_ERROR SetOperationalKeypair(const Crypto::P256Keypair * keyPair);

    // TODO - Update these APIs to take ownership of the buffer, instead of copying
    //        internally.
    // TODO - Optimize persistent storage of NOC and Root Cert in FabricInfo.
    CHIP_ERROR SetRootCert(const chip::ByteSpan & cert) { return SetCert(mRootCert, cert); }
    CHIP_ERROR SetICACert(const chip::ByteSpan & cert) { return SetCert(mICACert, cert); }
    CHIP_ERROR SetICACert(const Optional<ByteSpan> & cert) { return SetICACert(cert.ValueOr(ByteSpan())); }
    CHIP_ERROR SetNOCCert(const chip::ByteSpan & cert) { return SetCert(mNOCCert, cert); }

    bool IsInitialized() const { return IsOperationalNodeId(mOperationalId.GetNodeId()); }

    CHIP_ERROR GenerateDestinationID(const ByteSpan & ipk, const ByteSpan & random, NodeId destNodeId,
                                     MutableByteSpan & destinationId);

    CHIP_ERROR MatchDestinationID(const ByteSpan & destinationId, const ByteSpan & initiatorRandom, const ByteSpan * ipkList,
                                  size_t ipkListEntries);

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

    CHIP_ERROR VerifyCredentials(const ByteSpan & noc, const ByteSpan & icac, Credentials::ValidationContext & context,
                                 PeerId & nocPeerId, FabricId & fabricId, Crypto::P256PublicKey & nocPubkey) const;

    /**
     *  Reset the state to a completely uninitialized status.
     */
    void Reset()
    {
        mOperationalId  = PeerId();
        mVendorId       = kUndefinedVendorId;
        mFabricLabel[0] = '\0';

        if (mOperationalKey != nullptr)
        {
            chip::Platform::Delete(mOperationalKey);
            mOperationalKey = nullptr;
        }
        ReleaseOperationalCerts();
    }

    CHIP_ERROR SetFabricInfo(FabricInfo & fabric);

    /* Generate a compressed peer ID (containing compressed fabric ID) using provided fabric ID, node ID and
       root public key of the fabric. The generated compressed ID is returned via compressedPeerId
       output parameter */
    CHIP_ERROR GetCompressedId(FabricId fabricId, NodeId nodeId, PeerId * compressedPeerId) const;

    friend class FabricTable;

private:
    PeerId mOperationalId;

    FabricIndex mFabric                                 = kUndefinedFabricIndex;
    uint16_t mVendorId                                  = kUndefinedVendorId;
    char mFabricLabel[kFabricLabelMaxLengthInBytes + 1] = { '\0' };

#ifdef ENABLE_HSM_CASE_OPS_KEY
    Crypto::P256KeypairHSM * mOperationalKey = nullptr;
#else
    Crypto::P256Keypair * mOperationalKey = nullptr;
#endif

    MutableByteSpan mRootCert;
    MutableByteSpan mICACert;
    MutableByteSpan mNOCCert;

    FabricId mFabricId = 0;

    static constexpr size_t kKeySize = sizeof(kFabricTableKeyPrefix) + 2 * sizeof(FabricIndex);

    static CHIP_ERROR GenerateKey(FabricIndex id, char * key, size_t len);

    CHIP_ERROR CommitToStorage(FabricStorage * storage);
    CHIP_ERROR LoadFromStorage(FabricStorage * storage);
    static CHIP_ERROR DeleteFromStorage(FabricStorage * storage, FabricIndex fabricIndex);

    void ReleaseCert(MutableByteSpan & cert);
    void ReleaseOperationalCerts()
    {
        ReleaseCert(mRootCert);
        ReleaseCert(mICACert);
        ReleaseCert(mNOCCert);
    }

    CHIP_ERROR SetCert(MutableByteSpan & dstCert, const ByteSpan & srcCert);

    struct StorableFabricInfo
    {
        uint16_t mFabric;   /* This field is serialized in LittleEndian byte order */
        uint64_t mNodeId;   /* This field is serialized in LittleEndian byte order */
        uint64_t mFabricId; /* This field is serialized in LittleEndian byte order */
        uint16_t mVendorId; /* This field is serialized in LittleEndian byte order */

        uint16_t mRootCertLen; /* This field is serialized in LittleEndian byte order */
        uint16_t mICACertLen;  /* This field is serialized in LittleEndian byte order */
        uint16_t mNOCCertLen;  /* This field is serialized in LittleEndian byte order */

        Crypto::P256SerializedKeypair mOperationalKey;
        uint8_t mRootCert[Credentials::kMaxCHIPCertLength];
        uint8_t mICACert[Credentials::kMaxCHIPCertLength];
        uint8_t mNOCCert[Credentials::kMaxCHIPCertLength];
        char mFabricLabel[kFabricLabelMaxLengthInBytes + 1] = { '\0' };
    };
};

// Once attribute store has persistence implemented, FabricTable shoud be backed using
// attribute store so no need for this Delegate API anymore
// TODO: Reimplement FabricTable to only have one backing store.
class DLL_EXPORT FabricTableDelegate
{
    friend class FabricTable;

public:
    virtual ~FabricTableDelegate() {}
    /**
     * Gets called when a fabric is deleted from KVS store.
     **/
    virtual void OnFabricDeletedFromStorage(CompressedFabricId compressedId, FabricIndex fabricIndex) = 0;

    /**
     * Gets called when a fabric is loaded into Fabric Table from KVS store.
     **/
    virtual void OnFabricRetrievedFromStorage(FabricInfo * fabricInfo) = 0;

    /**
     * Gets called when a fabric in Fabric Table is persisted to KVS store.
     **/
    virtual void OnFabricPersistedToStorage(FabricInfo * fabricInfo) = 0;

private:
    FabricTableDelegate * mNext = nullptr;
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
    FabricTable() { Reset(); }
    CHIP_ERROR Store(FabricIndex index);
    CHIP_ERROR LoadFromStorage(FabricInfo * info);

    // Returns CHIP_ERROR_NOT_FOUND if there is no fabric for that index.
    CHIP_ERROR Delete(FabricIndex index);
    void DeleteAllFabrics();

    /**
     * Add the new fabric information to fabric table if the table has space to store
     * more fabrics. CHIP_ERROR_NO_MEMORY error will be returned if the table is full.
     *
     * The provided information will get copied to internal data structures, and the caller
     * can release the memory associated with input parameter after the call is complete.
     *
     * If the call is successful, the assigned fabric index is returned as output parameter.
     */
    CHIP_ERROR AddNewFabric(FabricInfo & fabric, FabricIndex * assignedIndex);

    void ReleaseFabricIndex(FabricIndex fabricIndex);

    FabricInfo * FindFabric(Credentials::P256PublicKeySpan rootPubKey, FabricId fabricId);
    FabricInfo * FindFabricWithIndex(FabricIndex fabricIndex);
    FabricInfo * FindFabricWithCompressedId(CompressedFabricId fabricId);

    FabricIndex FindDestinationIDCandidate(const ByteSpan & destinationId, const ByteSpan & initiatorRandom,
                                           const ByteSpan * ipkList, size_t ipkListEntries);

    void Reset();

    CHIP_ERROR Init(FabricStorage * storage);
    CHIP_ERROR AddFabricDelegate(FabricTableDelegate * delegate);

    uint8_t FabricCount() const { return mFabricCount; }

    ConstFabricIterator cbegin() const { return ConstFabricIterator(mStates, 0, CHIP_CONFIG_MAX_FABRICS); }
    ConstFabricIterator cend() const { return ConstFabricIterator(mStates, CHIP_CONFIG_MAX_FABRICS, CHIP_CONFIG_MAX_FABRICS); }
    ConstFabricIterator begin() const { return cbegin(); }
    ConstFabricIterator end() const { return cend(); }

private:
    FabricInfo mStates[CHIP_CONFIG_MAX_FABRICS];
    FabricStorage * mStorage = nullptr;

    FabricTableDelegate * mDelegate = nullptr;

    FabricIndex mNextAvailableFabricIndex = kMinValidFabricIndex;
    uint8_t mFabricCount                  = 0;
};

} // namespace chip
