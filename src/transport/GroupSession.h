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

class GroupSessionDeleter
{
public:
    static void Release(IncomingGroupSession * entry) {}
    static void Release(OutgoingGroupSession * entry) {}
};

class IncomingGroupSession : public Session,
                             // The group session is ephemeral, its lifespan is controlled by whoever is using it. To prevent the
                             // object being destroyed while there are still SessionHandle or SessionHolder pointing to it, we
                             // enforce a reference counter check at its destruction in debug build.
                             public ReferenceCounted<IncomingGroupSession, GroupSessionDeleter, 0>
{
public:
    IncomingGroupSession(GroupId group, FabricIndex fabricIndex, NodeId sourceNodeId) : mGroupId(group), mSourceNodeId(sourceNodeId)
    {
        SetFabricIndex(fabricIndex);
    }
    ~IncomingGroupSession() override;

    void Retain() override;
    void Release() override;

    Session::SessionType GetSessionType() const override { return Session::SessionType::kGroupIncoming; }
#if CHIP_PROGRESS_LOGGING
    const char * GetSessionTypeString() const override { return "incoming group"; };
#endif

    Access::SubjectDescriptor GetSubjectDescriptor() const override
    {
        Access::SubjectDescriptor subjectDescriptor;
        subjectDescriptor.authMode    = Access::AuthMode::kGroup;
        subjectDescriptor.subject     = NodeIdFromGroupId(mGroupId);
        subjectDescriptor.fabricIndex = GetFabricIndex();
        return subjectDescriptor;
    }

    bool RequireMRP() const override { return false; }

    const ReliableMessageProtocolConfig & GetMRPConfig() const override
    {
        static const ReliableMessageProtocolConfig cfg(GetLocalMRPConfig());
        VerifyOrDie(false);
        return cfg;
    }

    System::Clock::Milliseconds32 GetAckTimeout() const override
    {
        VerifyOrDie(false);
        return System::Clock::Timeout();
    }

    GroupId GetGroupId() const { return mGroupId; }

    NodeId GetSourceNodeId() const { return mSourceNodeId; }

private:
    const GroupId mGroupId;
    const NodeId mSourceNodeId;
};

class OutgoingGroupSession : public Session,
                             // The group session is ephemeral, its lifespan is controlled by whoever is using it. To prevent the
                             // object being destroyed while there are still SessionHandle or SessionHolder pointing to it, we
                             // enforce a reference counter check at its destruction in debug build.
                             public ReferenceCounted<OutgoingGroupSession, GroupSessionDeleter, 0>
{
public:
    OutgoingGroupSession(GroupId group, FabricIndex fabricIndex, NodeId sourceNodeId) : mGroupId(group), mSourceNodeId(sourceNodeId)
    {
        SetFabricIndex(fabricIndex);
    }
    ~OutgoingGroupSession() override;

    void Retain() override;
    void Release() override;

    Session::SessionType GetSessionType() const override { return Session::SessionType::kGroupOutgoing; }
#if CHIP_PROGRESS_LOGGING
    const char * GetSessionTypeString() const override { return "outgoing group"; };
#endif

    Access::SubjectDescriptor GetSubjectDescriptor() const override
    {
        return Access::SubjectDescriptor(); // no subject exists for outgoing group session.
    }

    bool RequireMRP() const override { return false; }

    const ReliableMessageProtocolConfig & GetMRPConfig() const override
    {
        static const ReliableMessageProtocolConfig cfg(GetLocalMRPConfig());
        VerifyOrDie(false);
        return cfg;
    }

    System::Clock::Milliseconds32 GetAckTimeout() const override
    {
        VerifyOrDie(false);
        return System::Clock::Timeout();
    }

    GroupId GetGroupId() const { return mGroupId; }

    NodeId GetSourceNodeId() const { return mSourceNodeId; }

private:
    const GroupId mGroupId;
    const NodeId mSourceNodeId;
};

} // namespace Transport
} // namespace chip
