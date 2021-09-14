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

#include <transport/UnauthenticatedSessionTable.h>

namespace chip {

class SecureSessionMgr;

class SessionHandle
{
public:
    SessionHandle(NodeId peerNodeId, FabricIndex fabric) : mPeerNodeId(peerNodeId), mFabric(fabric) {}

    SessionHandle(Transport::UnauthenticatedSessionHandle session) :
        mPeerNodeId(kPlaceholderNodeId), mFabric(Transport::kUndefinedFabricIndex), mUnauthenticatedSessionHandle(session)
    {}

    SessionHandle(NodeId peerNodeId, uint16_t localKeyId, uint16_t peerKeyId, FabricIndex fabric) :
        mPeerNodeId(peerNodeId), mFabric(fabric)
    {
        mLocalKeyId.SetValue(localKeyId);
        mPeerKeyId.SetValue(peerKeyId);
    }

    bool IsSecure() const { return !mUnauthenticatedSessionHandle.HasValue(); }

    bool HasFabricIndex() const { return (mFabric != Transport::kUndefinedFabricIndex); }
    FabricIndex GetFabricIndex() const { return mFabric; }
    void SetFabricIndex(FabricIndex fabricId) { mFabric = fabricId; }

    bool operator==(const SessionHandle & that) const
    {
        // TODO: Temporarily keep the old logic, check why only those two fields are used in comparison.
        return mPeerNodeId == that.mPeerNodeId && mPeerKeyId == that.mPeerKeyId;
    }

    bool MatchIncomingSession(const SessionHandle & that) const
    {
        if (IsSecure())
        {
            return that.IsSecure() && mLocalKeyId.Value() == that.mLocalKeyId.Value();
        }
        else
        {
            return !that.IsSecure() && mUnauthenticatedSessionHandle.Value() == that.mUnauthenticatedSessionHandle.Value();
        }
    }

    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    const Optional<uint16_t> & GetPeerKeyId() const { return mPeerKeyId; }
    const Optional<uint16_t> & GetLocalKeyId() const { return mLocalKeyId; }

    Transport::UnauthenticatedSessionHandle GetUnauthenticatedSession() { return mUnauthenticatedSessionHandle.Value(); }

private:
    friend class SecureSessionMgr;

    // Fields for secure session
    NodeId mPeerNodeId;
    Optional<uint16_t> mLocalKeyId;
    Optional<uint16_t> mPeerKeyId;
    // TODO: Re-evaluate the storing of Fabric ID in SessionHandle
    //       The Fabric ID will not be available for PASE and group sessions. So need
    //       to identify an approach that'll allow looking up the corresponding information for
    //       such sessions.
    FabricIndex mFabric;

    // Fields for unauthenticated session
    Optional<Transport::UnauthenticatedSessionHandle> mUnauthenticatedSessionHandle;
};

} // namespace chip
