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

#include <access/AccessControl.h>
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
    using SubjectDescriptor = Access::SubjectDescriptor;

    SessionHandle(NodeId peerNodeId, FabricIndex fabric) : mPeerNodeId(peerNodeId), mFabric(fabric) {}

    SessionHandle(Transport::UnauthenticatedSessionHandle session) :
        mPeerNodeId(kPlaceholderNodeId), mFabric(kUndefinedFabricIndex), mUnauthenticatedSessionHandle(session)
    {}

    SessionHandle(Transport::SecureSession & session) : mPeerNodeId(session.GetPeerNodeId()), mFabric(session.GetFabricIndex())
    {
        mLocalSessionId.SetValue(session.GetLocalSessionId());
        mPeerSessionId.SetValue(session.GetPeerSessionId());
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

    SubjectDescriptor GetSubjectDescriptor() const;

    bool operator==(const SessionHandle & that) const
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

    const ReliableMessageProtocolConfig & GetMRPConfig(SessionManager * sessionManager) const;
    void SetMRPConfig(SessionManager * sessionManager, const ReliableMessageProtocolConfig & config);

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
