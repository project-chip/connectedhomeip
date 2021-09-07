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

#include <app/util/basic-types.h>
#include <credentials/CHIPOperationalCredentials.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Span.h>
#include <transport/raw/MessageHeader.h>

#ifdef ENABLE_HSM_CASE_OPS_KEY
#define CASE_OPS_KEY 0xCA5EECC0
#endif

namespace chip {
namespace Transport {

static constexpr FabricIndex kUndefinedFabricIndex    = 0;
static constexpr FabricIndex kMinValidFabricIndex     = 1;
static constexpr FabricIndex kMaxValidFabricIndex     = std::min(UINT8_MAX, CHIP_CONFIG_MAX_DEVICE_ADMINS);
static constexpr uint8_t kFabricLabelMaxLengthInBytes = 32;

// KVS store is sensitive to length of key strings, based on the underlying
// platform. Keeping them short.
constexpr char kFabricTableKeyPrefix[] = "Fabric";
constexpr char kFabricTableCountKey[]  = "NumFabrics";

struct AccessControlList
{
    uint32_t placeholder;
};

/**
 * Defines state of a pairing established by a fabric.
 * ACL data can be mutated throughout the lifetime of the fabric pairing.
 * Node ID is only settable using the device operational credentials.
 *
 * Information contained within the state:
 *   - Fabric identification
 *   - Node Id assigned by the fabric to the device
 *   - Vendor Id
 *   - Fabric Id
 *   - Device operational credentials
 *   - Access control list
 */
class DLL_EXPORT FabricInfo
{
public:
    FabricInfo()
    {
        Reset();
        mFabric = kUndefinedFabricIndex;
    }

    // Returns a pointer to a null terminated char array
    const uint8_t * GetFabricLabel() const { return Uint8::from_const_char(mFabricLabel); };

    // Expects a pointer to a null terminated char array
    CHIP_ERROR SetFabricLabel(const uint8_t * fabricLabel);

    ~FabricInfo()
    {
        if (mOperationalKey != nullptr)
        {
            chip::Platform::Delete(mOperationalKey);
        }
        ReleaseRootCert();
        ReleaseOperationalCerts();
    }

    PeerId GetPeerId() const { return mOperationalId; }
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
        }
        return mOperationalKey;
    }
    CHIP_ERROR SetEphemeralKey(const Crypto::P256Keypair * key);

    bool AreCredentialsAvailable() const
    {
        return (mRootCert != nullptr && mOperationalCerts != nullptr && mRootCertLen != 0 && mOperationalCertsLen != 0);
    }

    // TODO - Update these APIs to take ownership of the buffer, instead of copying
    //        internally.
    CHIP_ERROR SetOperationalCertsFromCertArray(const chip::ByteSpan & certArray);
    CHIP_ERROR SetRootCert(const chip::ByteSpan & cert);

    const AccessControlList & GetACL() const { return mACL; }
    AccessControlList & GetACL() { return mACL; }
    void SetACL(const AccessControlList & acl) { mACL = acl; }

    bool IsInitialized() const { return IsOperationalNodeId(mOperationalId.GetNodeId()); }

    CHIP_ERROR GenerateDestinationID(const ByteSpan & ipk, const ByteSpan & random, NodeId destNodeId,
                                     MutableByteSpan & destinationId);

    CHIP_ERROR MatchDestinationID(const ByteSpan & destinationId, const ByteSpan & initiatorRandom, const ByteSpan * ipkList,
                                  size_t ipkListEntries);

    CHIP_ERROR GetOperationalCredentials(MutableByteSpan & credentials)
    {
        // TODO - Refactor storing and loading of fabric info from persistent storage.
        //        The op cert array doesn't need to be in RAM except when it's being
        //        transmitted to peer node during CASE session setup.
        ReturnErrorCodeIf(!AreCredentialsAvailable(), CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(credentials.size() < mOperationalCertsLen, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(credentials.data(), mOperationalCerts, mOperationalCertsLen);
        credentials.reduce_size(mOperationalCertsLen);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetRootCert(ByteSpan & cert)
    {
        ReturnErrorCodeIf(!AreCredentialsAvailable(), CHIP_ERROR_INCORRECT_STATE);
        cert = ByteSpan(mRootCert, mRootCertLen);
        return CHIP_NO_ERROR;
    }

    uint16_t GetOperationalCredentialsLength() { return mOperationalCertsLen; }

    Credentials::CertificateKeyId GetTrustedRootId()
    {
        return mRootKeyIdLen == Credentials::kKeyIdentifierLength ? Credentials::CertificateKeyId(mRootKeyId)
                                                                  : Credentials::CertificateKeyId();
    }

    CHIP_ERROR VerifyCredentials(const ByteSpan & noc, Credentials::ValidationContext & context, PeerId & nocPeerId,
                                 FabricId & fabricId, Crypto::P256PublicKey & nocPubkey) const;

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
        ReleaseRootCert();
        ReleaseOperationalCerts();
    }

    CHIP_ERROR SetFabricInfo(FabricInfo & fabric);

    const Crypto::P256PublicKey & GetRootPubkey() const { return mRootPubkey; }

    friend class FabricTable;

private:
    PeerId mOperationalId;

    FabricIndex mFabric                                 = kUndefinedFabricIndex;
    uint16_t mVendorId                                  = kUndefinedVendorId;
    char mFabricLabel[kFabricLabelMaxLengthInBytes + 1] = { '\0' };

    AccessControlList mACL;

    uint8_t mRootKeyId[Credentials::kKeyIdentifierLength];
    uint16_t mRootKeyIdLen = 0;

#ifdef ENABLE_HSM_CASE_OPS_KEY
    Crypto::P256KeypairHSM * mOperationalKey = nullptr;
#else
    Crypto::P256Keypair * mOperationalKey = nullptr;
#endif

    Crypto::P256PublicKey mRootPubkey;

    uint8_t * mRootCert            = nullptr;
    uint16_t mRootCertLen          = 0;
    uint16_t mRootCertAllocatedLen = 0;
    uint8_t * mOperationalCerts    = nullptr;
    uint16_t mOperationalCertsLen  = 0;

    FabricId mFabricId = 0;

    static constexpr size_t kKeySize = sizeof(kFabricTableKeyPrefix) + 2 * sizeof(FabricIndex);

    static CHIP_ERROR GenerateKey(FabricIndex id, char * key, size_t len);

    CHIP_ERROR StoreIntoKVS(PersistentStorageDelegate * kvs);
    CHIP_ERROR FetchFromKVS(PersistentStorageDelegate * kvs);
    static CHIP_ERROR DeleteFromKVS(PersistentStorageDelegate * kvs, FabricIndex id);

    void ReleaseOperationalCerts();
    void ReleaseRootCert();

    /* Generate a compressed peer ID (containing compressed fabric ID) using provided fabric ID, node ID and
       root public key of the fabric. The generated compressed ID is returned via compressedPeerId
       output parameter */
    CHIP_ERROR GetCompressedId(FabricId fabricId, NodeId nodeId, PeerId * compressedPeerId) const;

    struct StorableFabricInfo
    {
        uint16_t mFabric;   /* This field is serialized in LittleEndian byte order */
        uint64_t mNodeId;   /* This field is serialized in LittleEndian byte order */
        uint64_t mFabricId; /* This field is serialized in LittleEndian byte order */
        uint16_t mVendorId; /* This field is serialized in LittleEndian byte order */

        uint16_t mRootCertLen;         /* This field is serialized in LittleEndian byte order */
        uint16_t mOperationalCertsLen; /* This field is serialized in LittleEndian byte order */

        Crypto::P256SerializedKeypair mOperationalKey;
        uint8_t mRootCert[Credentials::kMaxCHIPCertLength];
        uint8_t mOperationalCerts[Credentials::kMaxCHIPOpCertArrayLength];
        char mFabricLabel[kFabricLabelMaxLengthInBytes + 1] = { '\0' };
    };
};

// Once attribute store has persistence implemented, FabricTable shoud be backed using
// attribute store so no need for this Delegate API anymore
// TODO: Reimplement FabricTable to only have one backing store.
class DLL_EXPORT FabricTableDelegate
{
public:
    virtual ~FabricTableDelegate() {}
    /**
     * Gets called when a fabric is deleted from KVS store.
     **/
    virtual void OnFabricDeletedFromStorage(FabricIndex fabricIndex) = 0;

    /**
     * Gets called when a fabric is loaded into Fabric Table from KVS store.
     **/
    virtual void OnFabricRetrievedFromStorage(FabricInfo * fabricInfo) = 0;

    /**
     * Gets called when a fabric in Fabric Table is persisted to KVS store.
     **/
    virtual void OnFabricPersistedToStorage(FabricInfo * fabricInfo) = 0;
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
    CHIP_ERROR Store(FabricIndex id);
    CHIP_ERROR LoadFromStorage(FabricInfo * info);

    CHIP_ERROR Delete(FabricIndex id);
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

    FabricInfo * FindFabricWithIndex(FabricIndex fabricIndex);

    FabricIndex FindDestinationIDCandidate(const ByteSpan & destinationId, const ByteSpan & initiatorRandom,
                                           const ByteSpan * ipkList, size_t ipkListEntries);

    void Reset();

    CHIP_ERROR Init(PersistentStorageDelegate * storage);
    CHIP_ERROR SetFabricDelegate(FabricTableDelegate * delegate);

    ConstFabricIterator cbegin() const { return ConstFabricIterator(mStates, 0, CHIP_CONFIG_MAX_DEVICE_ADMINS); }
    ConstFabricIterator cend() const
    {
        return ConstFabricIterator(mStates, CHIP_CONFIG_MAX_DEVICE_ADMINS, CHIP_CONFIG_MAX_DEVICE_ADMINS);
    }
    ConstFabricIterator begin() const { return cbegin(); }
    ConstFabricIterator end() const { return cend(); }

private:
    FabricInfo mStates[CHIP_CONFIG_MAX_DEVICE_ADMINS];
    PersistentStorageDelegate * mStorage = nullptr;

    // TODO: Fabric table should be backed by a single backing store (attribute store), remove delegate callbacks #6419
    FabricTableDelegate * mDelegate = nullptr;

    FabricIndex mNextAvailableFabricIndex = kMinValidFabricIndex;
};

} // namespace Transport
} // namespace chip
