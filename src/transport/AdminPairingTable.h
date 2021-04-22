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

#include <core/CHIPPersistentStorageDelegate.h>
#include <support/DLLUtil.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Transport {

typedef uint16_t AdminId;
typedef uint64_t FabricId;

static constexpr AdminId kUndefinedAdminId   = UINT16_MAX;
// static constexpr FabricId kUndefinedFabricId = UINT64_MAX;
static constexpr uint16_t kUndefinedVendorId = UINT16_MAX;
// KVS store is sensitive to length of key strings, based on the underlying
// platform. Keeping them short.
constexpr char kAdminTableKeyPrefix[] = "CHIPAdmin";
constexpr char kAdminTableCountKey[]  = "CHIPAdminNextId";

struct OperationalCredentials
{
    uint32_t placeholder;
};

struct AccessControlList
{
    uint32_t placeholder;
};

class DLL_EXPORT AdminPairingTableDelegate
{
public:
    virtual ~AdminPairingTableDelegate() {}

    virtual void OnAdminDeletedFromStorage(AdminId adminId);
    virtual void OnAdminRetrievedFromStorage(AdminId adminId, FabricId fabricId, NodeId nodeId);
    virtual void OnAdminPersistedToStorage(AdminId adminId, FabricId fabricId, NodeId nodeId);
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

    NodeId GetNodeId() const { return mNodeId; }
    void SetNodeId(NodeId nodeId) { mNodeId = nodeId; }

    FabricId GetFabricId() const { return mFabricId; }
    void SetFabricId(FabricId fabricId) { mFabricId = fabricId; }

    AdminId GetAdminId() const { return mAdmin; }
    void SetAdminId(AdminId adminId) { mAdmin = adminId; }

    uint16_t GetVendorId() const { return mVendorId; }
    void SetVendorId(uint16_t vendorId) { mVendorId = vendorId; }

    const OperationalCredentials & GetOperationalCreds() const { return mOpCred; }
    OperationalCredentials & GetOperationalCreds() { return mOpCred; }
    void SetOperationalCreds(const OperationalCredentials & creds) { mOpCred = creds; }

    const AccessControlList & GetACL() const { return mACL; }
    AccessControlList & GetACL() { return mACL; }
    void SetACL(const AccessControlList & acl) { mACL = acl; }

    bool IsInitialized() const { return (mAdmin != kUndefinedAdminId); }

    /**
     *  Reset the state to a completely uninitialized status.
     */
    void Reset()
    {
        mNodeId   = kUndefinedNodeId;
        mAdmin    = kUndefinedAdminId;
        mFabricId = kUndefinedFabricId;
        mVendorId = kUndefinedVendorId;
    }

    CHIP_ERROR StoreIntoKVS();

    CHIP_ERROR FetchFromKVS();

    static CHIP_ERROR DeleteFromKVS(AdminId id);

private:
    AdminId mAdmin     = kUndefinedAdminId;
    NodeId mNodeId     = kUndefinedNodeId;
    FabricId mFabricId = kUndefinedFabricId;
    uint16_t mVendorId = kUndefinedVendorId;

    OperationalCredentials mOpCred;
    AccessControlList mACL;

    static constexpr size_t KeySize();

    static CHIP_ERROR GenerateKey(AdminId id, char * key, size_t len);

    struct StorableAdminPairingInfo
    {
        uint16_t mAdmin;    /* This field is serialized in LittleEndian byte order */
        uint64_t mNodeId;   /* This field is serialized in LittleEndian byte order */
        uint64_t mFabricId; /* This field is serialized in LittleEndian byte order */
        uint16_t mVendorId; /* This field is serialized in LittleEndian byte order */
    };
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
    AdminPairingInfo * AssignAdminId(AdminId adminId);

    AdminPairingInfo * AssignAdminId(AdminId adminId, NodeId nodeId);

    void ReleaseAdminId(AdminId adminId);

    AdminPairingInfo * FindAdmin(AdminId adminId);

    AdminPairingInfo * FindAdmin(FabricId fabricId);

    AdminPairingInfo * FindAdmin(FabricId fabricId, NodeId nodeId);

    AdminPairingInfo * FindAdmin(FabricId fabricId, NodeId nodeId, uint16_t vendorId);

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
};

} // namespace Transport
} // namespace chip
