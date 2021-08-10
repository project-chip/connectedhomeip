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

class SessionHandle
{
public:
    SessionHandle() :
        mPeerNodeId(kPlaceholderNodeId), mLocalKeyId(0), mPeerKeyId(0), mFabric(Transport::kUndefinedFabricIndex)
    {}
    SessionHandle(NodeId peerNodeId, uint16_t localKeyId, uint16_t peerKeyId, FabricIndex fabric) :
        mPeerNodeId(peerNodeId), mLocalKeyId(localKeyId), mPeerKeyId(peerKeyId), mFabric(fabric)
    {}

    bool HasFabricIndex() const { return (mFabric != Transport::kUndefinedFabricIndex); }
    FabricIndex GetFabricIndex() const { return mFabric; }
    void SetFabricIndex(FabricIndex fabricId) { mFabric = fabricId; }

    bool operator==(const SessionHandle & that) const
    {
        return mPeerNodeId == that.mPeerNodeId && mPeerKeyId == that.mPeerKeyId;
    }

    bool match(const SessionHandle & that) const
    {
        return mLocalKeyId == that.mLocalKeyId;
    }    

    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    uint16_t GetPeerKeyId() const { return mPeerKeyId; }
    uint16_t GetLocalKeyId() const { return mLocalKeyId; }

private:
    friend class SecureSessionMgr;
    NodeId mPeerNodeId;
    uint16_t mLocalKeyId;
    uint16_t mPeerKeyId;
    // TODO: Re-evaluate the storing of Fabric ID in SessionHandle
    //       The Fabric ID will not be available for PASE and group sessions. So need
    //       to identify an approach that'll allow looking up the corresponding information for
    //       such sessions.
    FabricIndex mFabric;
};

} // namespace chip
