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

#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Transport {

typedef uint16_t AdminId;
static constexpr AdminId kUndefinedAdminId = UINT16_MAX;

struct OperationalCredentials
{
    uint32_t placeholder;
};

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
 *   - Device operational credentials
 *   - Access control list
 */
class DLL_EXPORT AdminPairingInfo
{
public:
    AdminPairingInfo() { Reset(); }

    NodeId GetNodeId() const { return mNodeId; }
    void SetNodeId(NodeId nodeId) { mNodeId = nodeId; }

    AdminId GetAdminId() const { return mAdmin; }
    void SetAdminId(AdminId adminId) { mAdmin = adminId; }

    const OperationalCredentials & GetOperationalCreds() const { return mOpCred; }
    OperationalCredentials & GetOperationalCreds() { return mOpCred; }
    void SetOperationalCreds(const OperationalCredentials & creds) { mOpCred = creds; }

    const AccessControlList & GetACL() const { return mACL; }
    AccessControlList & GetACL() { return mACL; }
    void SetACL(const AccessControlList & acl) { mACL = acl; }

    bool IsInitialized() { return (mNodeId != kUndefinedNodeId && mAdmin != kUndefinedAdminId); }

    /**
     *  Reset the state to a completely uninitialized status.
     */
    void Reset()
    {
        mNodeId = kUndefinedNodeId;
        mAdmin  = kUndefinedAdminId;
    }

private:
    AdminId mAdmin = kUndefinedAdminId;
    NodeId mNodeId = kUndefinedNodeId;

    OperationalCredentials mOpCred;
    AccessControlList mACL;
};

class DLL_EXPORT AdminPairingTable
{
public:
    AdminPairingInfo * AssignAdminId(AdminId adminId)
    {
        for (size_t i = 0; i < CHIP_CONFIG_MAX_DEVICE_ADMINS; i++)
        {
            if (!mStates[i].IsInitialized())
            {
                mStates[i].SetAdminId(adminId);

                return &mStates[i];
            }
        }

        return nullptr;
    }

    AdminPairingInfo * AssignAdminId(AdminId adminId, NodeId nodeId)
    {
        AdminPairingInfo * admin = AssignAdminId(adminId);

        if (admin != nullptr)
        {
            admin->SetNodeId(nodeId);
        }

        return admin;
    }

    void ReleaseAdminId(AdminId adminId)
    {
        AdminPairingInfo * admin = FindAdmin(adminId);
        if (admin != nullptr)
        {
            admin->Reset();
        }
    }

    AdminPairingInfo * FindAdmin(AdminId adminId)
    {
        for (size_t i = 0; i < CHIP_CONFIG_MAX_DEVICE_ADMINS; i++)
        {
            if (mStates[i].IsInitialized() && mStates[i].GetAdminId() == adminId)
            {
                return &mStates[i];
            }
        }

        return nullptr;
    }

    void Reset()
    {
        for (size_t i = 0; i < CHIP_CONFIG_MAX_DEVICE_ADMINS; i++)
        {
            return mStates[i].Reset();
        }
    }

private:
    AdminPairingInfo mStates[CHIP_CONFIG_MAX_DEVICE_ADMINS];
};

} // namespace Transport
} // namespace chip
