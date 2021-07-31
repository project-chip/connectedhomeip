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
    SecureSessionHandle() : mPeerNodeId(kPlaceholderNodeId), mPeerKeyId(0), mFabric(Transport::kUndefinedFabricIndex) {}
    SecureSessionHandle(NodeId peerNodeId, uint16_t peerKeyId, FabricIndex fabric) :
        mPeerNodeId(peerNodeId), mPeerKeyId(peerKeyId), mFabric(fabric)
    {}

    bool HasFabricIndex() const { return (mFabric != Transport::kUndefinedFabricIndex); }
    FabricIndex GetFabricIndex() const { return mFabric; }
    void SetFabricIndex(FabricIndex fabricId) { mFabric = fabricId; }

    bool operator==(const SecureSessionHandle & that) const
    {
        return mPeerNodeId == that.mPeerNodeId && mPeerKeyId == that.mPeerKeyId && mFabric == that.mFabric;
    }

    explicit operator bool() const { return !(*this == SecureSessionHandle()); }

    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    uint16_t GetPeerKeyId() const { return mPeerKeyId; }

private:
    friend class SecureSessionMgr;
    NodeId mPeerNodeId;
    uint16_t mPeerKeyId;
    // TODO: Re-evaluate the storing of Fabric ID in SecureSessionHandle
    //       The Fabric ID will not be available for PASE and group sessions. So need
    //       to identify an approach that'll allow looking up the corresponding information for
    //       such sessions.
    FabricIndex mFabric;
};

} // namespace chip
