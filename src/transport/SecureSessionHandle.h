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

#pragma once

namespace chip {

class SecureSessionMgr;

class SecureSessionHandle
{
public:
    SecureSessionHandle() : mPeerNodeId(kAnyNodeId), mPeerKeyId(0), mAdmin(Transport::kUndefinedAdminId) {}
    SecureSessionHandle(NodeId peerNodeId, uint16_t peerKeyId, Transport::AdminId admin) :
        mPeerNodeId(peerNodeId), mPeerKeyId(peerKeyId), mAdmin(admin)
    {}

    bool HasAdminId() const { return (mAdmin != Transport::kUndefinedAdminId); }
    Transport::AdminId GetAdminId() const { return mAdmin; }
    void SetAdminId(Transport::AdminId adminId) { mAdmin = adminId; }

    bool operator==(const SecureSessionHandle & that) const
    {
        return mPeerNodeId == that.mPeerNodeId && mPeerKeyId == that.mPeerKeyId && mAdmin == that.mAdmin;
    }

    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    uint16_t GetPeerKeyId() const { return mPeerKeyId; }

private:
    friend class SecureSessionMgr;
    NodeId mPeerNodeId;
    uint16_t mPeerKeyId;
    // TODO: Re-evaluate the storing of Admin ID in SecureSessionHandle
    //       The Admin ID will not be available for PASE and group sessions. So need
    //       to identify an approach that'll allow looking up the corresponding information for
    //       such sessions.
    Transport::AdminId mAdmin;
};

} // namespace chip
