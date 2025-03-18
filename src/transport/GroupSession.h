/*
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
#include <lib/core/GroupId.h>
#include <lib/core/ReferenceCounted.h>
#include <lib/support/Pool.h>
#include <transport/Session.h>

namespace chip {
namespace Transport {

class IncomingGroupSession : public Session, public ReferenceCounted<IncomingGroupSession, NoopDeletor<IncomingGroupSession>, 0>
{
public:
    IncomingGroupSession(GroupId group, FabricIndex fabricIndex, NodeId peerNodeId) : mGroupId(group), mPeerNodeId(peerNodeId)
    {
        SetFabricIndex(fabricIndex);
    }
    ~IncomingGroupSession() override
    {
        NotifySessionReleased();
        VerifyOrDie(GetReferenceCount() == 0);
    }

    void Retain() override { ReferenceCounted<IncomingGroupSession, NoopDeletor<IncomingGroupSession>, 0>::Retain(); }
    void Release() override { ReferenceCounted<IncomingGroupSession, NoopDeletor<IncomingGroupSession>, 0>::Release(); }

    bool IsActiveSession() const override { return true; }

    Session::SessionType GetSessionType() const override { return Session::SessionType::kGroupIncoming; }

    ScopedNodeId GetPeer() const override { return ScopedNodeId(mPeerNodeId, GetFabricIndex()); }
    ScopedNodeId GetLocalScopedNodeId() const override { return ScopedNodeId(kUndefinedNodeId, GetFabricIndex()); }

    Access::SubjectDescriptor GetSubjectDescriptor() const override
    {
        Access::SubjectDescriptor subjectDescriptor;
        subjectDescriptor.authMode    = Access::AuthMode::kGroup;
        subjectDescriptor.subject     = NodeIdFromGroupId(mGroupId);
        subjectDescriptor.fabricIndex = GetFabricIndex();
        return subjectDescriptor;
    }

    bool AllowsMRP() const override { return false; }
    bool AllowsLargePayload() const override { return false; }

    const SessionParameters & GetRemoteSessionParameters() const override
    {
        static const SessionParameters cfg(GetDefaultMRPConfig());
        VerifyOrDie(false);
        return cfg;
    }

    System::Clock::Timestamp GetMRPBaseTimeout() const override { return System::Clock::kZero; }

    System::Clock::Milliseconds32 GetAckTimeout(bool isFirstMessageOnExchange) const override
    {
        VerifyOrDie(false);
        return System::Clock::Timeout();
    }

    System::Clock::Milliseconds32 GetMessageReceiptTimeout(System::Clock::Timestamp ourLastActivity,
                                                           bool isFirstMessageOnExchange) const override
    {
        // There are no timeouts for group sessions.
        VerifyOrDie(false);
        return System::Clock::Timeout();
    }

    GroupId GetGroupId() const { return mGroupId; }

private:
    const GroupId mGroupId;
    const NodeId mPeerNodeId;
};

class OutgoingGroupSession : public Session, public ReferenceCounted<OutgoingGroupSession, NoopDeletor<OutgoingGroupSession>, 0>
{
public:
    OutgoingGroupSession(GroupId group, FabricIndex fabricIndex) : mGroupId(group) { SetFabricIndex(fabricIndex); }
    ~OutgoingGroupSession() override
    {
        NotifySessionReleased();
        VerifyOrDie(GetReferenceCount() == 0);
    }

    void Retain() override { ReferenceCounted<OutgoingGroupSession, NoopDeletor<OutgoingGroupSession>, 0>::Retain(); }
    void Release() override { ReferenceCounted<OutgoingGroupSession, NoopDeletor<OutgoingGroupSession>, 0>::Release(); }

    bool IsActiveSession() const override { return true; }

    Session::SessionType GetSessionType() const override { return Session::SessionType::kGroupOutgoing; }

    // Peer node ID is unused: users care about the group, not the node
    ScopedNodeId GetPeer() const override { return ScopedNodeId(); }
    // Local node ID is unused: users care about the group, not the node
    ScopedNodeId GetLocalScopedNodeId() const override { return ScopedNodeId(); }

    Access::SubjectDescriptor GetSubjectDescriptor() const override
    {
        return Access::SubjectDescriptor(); // no subject exists for outgoing group session.
    }

    bool AllowsMRP() const override { return false; }
    bool AllowsLargePayload() const override { return false; }

    const SessionParameters & GetRemoteSessionParameters() const override
    {
        static const SessionParameters cfg(GetDefaultMRPConfig());
        VerifyOrDie(false);
        return cfg;
    }

    System::Clock::Timestamp GetMRPBaseTimeout() const override { return System::Clock::kZero; }

    System::Clock::Milliseconds32 GetAckTimeout(bool isFirstMessageOnExchange) const override
    {
        VerifyOrDie(false);
        return System::Clock::Timeout();
    }

    System::Clock::Milliseconds32 GetMessageReceiptTimeout(System::Clock::Timestamp ourLastActivity,
                                                           bool isFirstMessageOnExchange) const override
    {
        // There are no timeouts for group sessions.
        VerifyOrDie(false);
        return System::Clock::Timeout();
    }

    GroupId GetGroupId() const { return mGroupId; }

private:
    const GroupId mGroupId;
};

} // namespace Transport
} // namespace chip
