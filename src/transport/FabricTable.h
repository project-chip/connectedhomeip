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
#include <core/CHIPPersistentStorageDelegate.h>
#include <credentials/CHIPOperationalCredentials.h>
#include <crypto/CHIPCryptoPAL.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif
#include <lib/core/CHIPSafeCasts.h>
#include <support/CHIPMem.h>
#include <support/DLLUtil.h>
#include <support/Span.h>
#include <transport/raw/MessageHeader.h>

#ifdef ENABLE_HSM_CASE_OPS_KEY
#define CASE_OPS_KEY 0xCA5EECC0
#endif

namespace chip {
namespace Transport {

static constexpr FabricIndex kUndefinedFabricIndex    = UINT8_MAX;
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
    FabricInfo() { Reset(); }

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
        ReleaseICACert();
        ReleaseNOCCert();
    }

    NodeId GetNodeId() const { return mNodeId; }
    void SetNodeId(NodeId nodeId) { mNodeId = nodeId; }

    FabricId GetFabricId() const { return mFabricId; }
    void SetFabricId(FabricId fabricId) { mFabricId = fabricId; }

    FabricIndex GetFabricIndex() const { return mFabric; }
    void SetFabricIndex(FabricIndex fabricId) { mFabric = fabricId; }

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
    CHIP_ERROR SetOperationalKey(const Crypto::P256Keypair & key);

    bool AreCredentialsAvailable() const
    {
        return (mRootCert != nullptr && mNOCCert != nullptr && mRootCertLen != 0 && mNOCCertLen != 0);
    }

    /**
     * @brief
     *   Retrieve the credentials corresponding to the device being commissioned in form of OperationalCredentialSet.
     *
     * @param credentials Credential Set object containing  the device's certificate set and keypair.
     * @param certSet Set of Root [+ ICA] certificates corresponding to the device's credential set.
     * @param rootKeyId Trusted Root Id corresponding to the device's credential set.
     * @param credentialsIndex Index for the retrieved credentials corresponding to this device's credential set.
     *
     * @return CHIP_ERROR
     */
    CHIP_ERROR GetCredentials(Credentials::OperationalCredentialSet & credentials, Credentials::ChipCertificateSet & certSet,
                              Credentials::CertificateKeyId & rootKeyId, uint8_t & credentialsIndex);

    const uint8_t * GetTrustedRoot(uint16_t & size)
    {
        size = mRootCertLen;
        return mRootCert;
    }

    // TODO - Update these APIs to take ownership of the buffer, instead of copying
    //        internally.
    CHIP_ERROR SetOperationalCertsFromCertArray(const chip::ByteSpan & certArray);
    CHIP_ERROR SetNOCCert(const chip::ByteSpan & cert);
    CHIP_ERROR SetICACert(const chip::ByteSpan & cert);
    CHIP_ERROR SetRootCert(const chip::ByteSpan & cert);

    const AccessControlList & GetACL() const { return mACL; }
    AccessControlList & GetACL() { return mACL; }
    void SetACL(const AccessControlList & acl) { mACL = acl; }

    bool IsInitialized() const { return (mFabric != kUndefinedFabricIndex); }

    /**
     *  Reset the state to a completely uninitialized status.
     */
    void Reset()
    {
        mNodeId         = kUndefinedNodeId;
        mFabric         = kUndefinedFabricIndex;
        mFabricId       = kUndefinedFabricId;
        mVendorId       = kUndefinedVendorId;
        mFabricLabel[0] = '\0';

        if (mOperationalKey != nullptr)
        {
            mOperationalKey->Initialize();
        }
        ReleaseRootCert();
        ReleaseICACert();
        ReleaseNOCCert();
    }

    friend class FabricTable;

private:
    NodeId mNodeId                                      = kUndefinedNodeId;
    FabricId mFabricId                                  = kUndefinedFabricId;
    FabricIndex mFabric                                 = kUndefinedFabricIndex;
    uint16_t mVendorId                                  = kUndefinedVendorId;
    char mFabricLabel[kFabricLabelMaxLengthInBytes + 1] = { '\0' };

    AccessControlList mACL;

#ifdef ENABLE_HSM_CASE_OPS_KEY
    Crypto::P256KeypairHSM * mOperationalKey = nullptr;
#else
    Crypto::P256Keypair * mOperationalKey = nullptr;
#endif

    uint8_t * mRootCert            = nullptr;
    uint16_t mRootCertLen          = 0;
    uint16_t mRootCertAllocatedLen = 0;
    uint8_t * mICACert             = nullptr;
    uint16_t mICACertLen           = 0;
    uint8_t * mNOCCert             = nullptr;
    uint16_t mNOCCertLen           = 0;

    static constexpr size_t KeySize();

    static CHIP_ERROR GenerateKey(FabricIndex id, char * key, size_t len);

    CHIP_ERROR StoreIntoKVS(PersistentStorageDelegate * kvs);
    CHIP_ERROR FetchFromKVS(PersistentStorageDelegate * kvs);
    static CHIP_ERROR DeleteFromKVS(PersistentStorageDelegate * kvs, FabricIndex id);

    void ReleaseNOCCert();
    void ReleaseICACert();
    void ReleaseRootCert();

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
public:
    virtual ~FabricTableDelegate() {}
    /**
     * Gets called when a fabric is deleted from KVS store.
     **/
    virtual void OnFabricDeletedFromStorage(FabricIndex fabricId) = 0;

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
    CHIP_ERROR Store(FabricIndex id);
    CHIP_ERROR LoadFromStorage(FabricIndex id);
    CHIP_ERROR Delete(FabricIndex id);

    FabricInfo * AssignFabricIndex(FabricIndex fabricId);

    FabricInfo * AssignFabricIndex(FabricIndex fabricId, NodeId nodeId);

    void ReleaseFabricIndex(FabricIndex fabricId);

    FabricInfo * FindFabricWithIndex(FabricIndex fabricId);

    FabricInfo * FindFabricForNode(FabricId fabricId, NodeId nodeId = kUndefinedNodeId, uint16_t vendorId = kUndefinedVendorId);

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
};

} // namespace Transport
} // namespace chip
