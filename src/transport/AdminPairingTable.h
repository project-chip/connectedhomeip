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
 * @brief Defines a table of admins that have provisioned the device.
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

typedef uint16_t AdminId;
static constexpr AdminId kUndefinedAdminId            = UINT16_MAX;
static constexpr uint8_t kFabricLabelMaxLengthInBytes = 32;

// KVS store is sensitive to length of key strings, based on the underlying
// platform. Keeping them short.
constexpr char kAdminTableKeyPrefix[] = "CHIPAdmin";
constexpr char kAdminTableCountKey[]  = "CHIPAdminNextId";

struct AccessControlList
{
    uint32_t placeholder;
};

/**
 * Defines state of a pairing established by an admin.
 * ACL data can be mutated throughout the lifetime of the admin pairing.
 * Node ID is only settable using the device operational credentials.
 *
 * Information contained within the state:
 *   - Admin identification
 *   - Node Id assigned by the admin to the device
 *   - Vendor Id
 *   - Fabric Id
 *   - Device operational credentials
 *   - Access control list
 */
class DLL_EXPORT AdminPairingInfo
{
public:
    AdminPairingInfo() { Reset(); }

    // Returns a pointer to a null terminated char array
    const uint8_t * GetFabricLabel() const { return Uint8::from_const_char(mFabricLabel); };

    // Expects a pointer to a null terminated char array
    CHIP_ERROR SetFabricLabel(const uint8_t * fabricLabel);

    ~AdminPairingInfo()
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

    AdminId GetAdminId() const { return mAdmin; }
    void SetAdminId(AdminId adminId) { mAdmin = adminId; }

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

    CHIP_ERROR GetCredentials(Credentials::OperationalCredentialSet & credentials, Credentials::ChipCertificateSet & certSet,
                              Credentials::CertificateKeyId & rootKeyId);

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

    bool IsInitialized() const { return (mAdmin != kUndefinedAdminId); }

    /**
     *  Reset the state to a completely uninitialized status.
     */
    void Reset()
    {
        mNodeId         = kUndefinedNodeId;
        mAdmin          = kUndefinedAdminId;
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

    friend class AdminPairingTable;

private:
    NodeId mNodeId                                      = kUndefinedNodeId;
    FabricId mFabricId                                  = kUndefinedFabricId;
    AdminId mAdmin                                      = kUndefinedAdminId;
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

    static CHIP_ERROR GenerateKey(AdminId id, char * key, size_t len);

    CHIP_ERROR StoreIntoKVS(PersistentStorageDelegate * kvs);
    CHIP_ERROR FetchFromKVS(PersistentStorageDelegate * kvs);
    static CHIP_ERROR DeleteFromKVS(PersistentStorageDelegate * kvs, AdminId id);

    void ReleaseNOCCert();
    void ReleaseICACert();
    void ReleaseRootCert();

    struct StorableAdminPairingInfo
    {
        uint16_t mAdmin;    /* This field is serialized in LittleEndian byte order */
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

// Once attribute store has persistence implemented, AdminPairingTable shoud be backed using
// attribute store so no need for this Delegate API anymore
// TODO: Reimplement AdminPairingTable to only have one backing store.
class DLL_EXPORT AdminPairingTableDelegate
{
public:
    virtual ~AdminPairingTableDelegate() {}
    /**
     * Gets called when an admin is deleted from KVS store.
     **/
    virtual void OnAdminDeletedFromStorage(AdminId adminId) = 0;

    /**
     * Gets called when an admin is loaded into Admin Pairing Table from KVS store.
     **/
    virtual void OnAdminRetrievedFromStorage(AdminPairingInfo * adminInfo) = 0;

    /**
     * Gets called when an admin in Admin Pairing Table is persisted to KVS store.
     **/
    virtual void OnAdminPersistedToStorage(AdminPairingInfo * adminInfo) = 0;
};

/**
 * Iterates over valid admins within a list
 */
class ConstAdminIterator
{
public:
    using value_type = AdminPairingInfo;
    using pointer    = AdminPairingInfo *;
    using reference  = AdminPairingInfo &;

    ConstAdminIterator(const AdminPairingInfo * start, size_t index, size_t maxSize) :
        mStart(start), mIndex(index), mMaxSize(maxSize)
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
    ConstAdminIterator(const ConstAdminIterator &) = default;
    ConstAdminIterator & operator=(const ConstAdminIterator &) = default;

    ConstAdminIterator & operator++() { return Advance(); }
    ConstAdminIterator operator++(int)
    {
        ConstAdminIterator other(*this);
        Advance();
        return other;
    }

    const AdminPairingInfo & operator*() const { return mStart[mIndex]; }
    const AdminPairingInfo * operator->() const { return mStart + mIndex; }

    bool operator==(const ConstAdminIterator & other)
    {
        if (IsAtEnd())
        {
            return other.IsAtEnd();
        }

        return (mStart == other.mStart) && (mIndex == other.mIndex) && (mMaxSize == other.mMaxSize);
    }
    bool operator!=(const ConstAdminIterator & other) { return !(*this == other); }

    bool IsAtEnd() const { return (mIndex == mMaxSize); }

private:
    const AdminPairingInfo * mStart;
    size_t mIndex;
    size_t mMaxSize;

    ConstAdminIterator & Advance()
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

class DLL_EXPORT AdminPairingTable
{
public:
    CHIP_ERROR Store(AdminId id);
    CHIP_ERROR LoadFromStorage(AdminId id);
    CHIP_ERROR Delete(AdminId id);

    AdminPairingInfo * AssignAdminId(AdminId adminId);

    AdminPairingInfo * AssignAdminId(AdminId adminId, NodeId nodeId);

    void ReleaseAdminId(AdminId adminId);

    AdminPairingInfo * FindAdminWithId(AdminId adminId);

    AdminPairingInfo * FindAdminForNode(FabricId fabricId, NodeId nodeId = kUndefinedNodeId,
                                        uint16_t vendorId = kUndefinedVendorId);

    void Reset();

    CHIP_ERROR Init(PersistentStorageDelegate * storage);
    CHIP_ERROR SetAdminPairingDelegate(AdminPairingTableDelegate * delegate);

    ConstAdminIterator cbegin() const { return ConstAdminIterator(mStates, 0, CHIP_CONFIG_MAX_DEVICE_ADMINS); }
    ConstAdminIterator cend() const
    {
        return ConstAdminIterator(mStates, CHIP_CONFIG_MAX_DEVICE_ADMINS, CHIP_CONFIG_MAX_DEVICE_ADMINS);
    }
    ConstAdminIterator begin() const { return cbegin(); }
    ConstAdminIterator end() const { return cend(); }

private:
    AdminPairingInfo mStates[CHIP_CONFIG_MAX_DEVICE_ADMINS];
    PersistentStorageDelegate * mStorage = nullptr;

    // TODO: Admin Pairing table should be backed by a single backing store (attribute store), remove delegate callbacks #6419
    AdminPairingTableDelegate * mDelegate = nullptr;
};

} // namespace Transport
} // namespace chip
