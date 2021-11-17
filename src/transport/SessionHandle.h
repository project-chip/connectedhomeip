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

#include <app/util/basic-types.h>
#include <lib/core/NodeId.h>
#include <lib/core/Optional.h>
#include <transport/UnauthenticatedSessionTable.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

class SessionManager;

class SessionHandle
{
public:
    SessionHandle(NodeId peerNodeId, FabricIndex fabric) : mPeerNodeId(peerNodeId), mFabric(fabric) {}

    SessionHandle(Transport::UnauthenticatedSessionHandle session) :
        mPeerNodeId(kPlaceholderNodeId), mFabric(kUndefinedFabricIndex), mUnauthenticatedSessionHandle(session)
    {}

    SessionHandle(NodeId peerNodeId, uint16_t localSessionId, uint16_t peerSessionId, FabricIndex fabric) :
        mPeerNodeId(peerNodeId), mFabric(fabric)
    {
        mLocalSessionId.SetValue(localSessionId);
        mPeerSessionId.SetValue(peerSessionId);
    }

    SessionHandle(NodeId peerNodeId, GroupId groupId, FabricIndex fabric) : mPeerNodeId(peerNodeId), mFabric(fabric)
    {
        mGroupId.SetValue(groupId);
    }

    bool IsSecure() const { return !mUnauthenticatedSessionHandle.HasValue(); }

    bool HasFabricIndex() const { return (mFabric != kUndefinedFabricIndex); }
    FabricIndex GetFabricIndex() const { return mFabric; }
    void SetFabricIndex(FabricIndex fabricId) { mFabric = fabricId; }
    void SetGroupId(GroupId groupId) { mGroupId.SetValue(groupId); }

    bool operator==(const SessionHandle & that) const
    {
        // TODO: Temporarily keep the old logic, check why only those two fields are used in comparison.
        return mPeerNodeId == that.mPeerNodeId && mPeerSessionId == that.mPeerSessionId;
    }

    bool MatchIncomingSession(const SessionHandle & that) const
    {
        if (IsSecure())
        {
            return that.IsSecure() && mLocalSessionId.Value() == that.mLocalSessionId.Value();
        }
        else
        {
            return !that.IsSecure() && mUnauthenticatedSessionHandle.Value() == that.mUnauthenticatedSessionHandle.Value();
        }
    }

    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    bool IsGroupSession() const { return mGroupId.HasValue(); }
    const Optional<GroupId> & GetGroupId() const { return mGroupId; }
    const Optional<uint16_t> & GetPeerSessionId() const { return mPeerSessionId; }
    const Optional<uint16_t> & GetLocalSessionId() const { return mLocalSessionId; }

    // Return the peer address for this session.  May return null if the peer
    // address is not known.  This can happen for secure sessions that have been
    // torn down, at the very least.
    const Transport::PeerAddress * GetPeerAddress(SessionManager * sessionManager) const;

    CHIP_ERROR GetMRPIntervals(SessionManager * sessionManager, uint32_t & mrpIdleInterval, uint32_t & mrpActiveInterval);

    Transport::UnauthenticatedSessionHandle GetUnauthenticatedSession() const { return mUnauthenticatedSessionHandle.Value(); }

private:
    friend class SessionManager;

    // Fields for secure session
    NodeId mPeerNodeId;
    Optional<uint16_t> mLocalSessionId;
    Optional<uint16_t> mPeerSessionId;
    Optional<GroupId> mGroupId;
    // TODO: Re-evaluate the storing of Fabric ID in SessionHandle
    //       The Fabric ID will not be available for PASE and group sessions. So need
    //       to identify an approach that'll allow looking up the corresponding information for
    //       such sessions.
    FabricIndex mFabric;

    // Fields for unauthenticated session
    Optional<Transport::UnauthenticatedSessionHandle> mUnauthenticatedSessionHandle;
};

} // namespace chip
