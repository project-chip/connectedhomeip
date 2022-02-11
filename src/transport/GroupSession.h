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
#include <lib/support/Pool.h>
#include <transport/Session.h>

namespace chip {
namespace Transport {

class GroupSession : public Session
{
public:
    GroupSession(GroupId group, FabricIndex fabricIndex, NodeId sourceNodeId) : mGroupId(group), mSourceNodeId(sourceNodeId)
    {
        SetFabricIndex(fabricIndex);
    }
    ~GroupSession() { NotifySessionReleased(); }

    Session::SessionType GetSessionType() const override { return Session::SessionType::kGroup; }
#if CHIP_PROGRESS_LOGGING
    const char * GetSessionTypeString() const override { return "secure"; };
#endif

    Access::SubjectDescriptor GetSubjectDescriptor() const override
    {
        Access::SubjectDescriptor isd;
        isd.authMode = Access::AuthMode::kGroup;
        // TODO: fill other group subjects fields
        return isd; // return an empty ISD for unauthenticated session.
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

    NodeId GetSourceNodeId() { return mSourceNodeId; }

private:
    const GroupId mGroupId;
    const NodeId mSourceNodeId;
};

/*
 * @brief
 *   An table which manages GroupSessions
 */
template <size_t kMaxSessionCount>
class GroupSessionTable
{
public:
    ~GroupSessionTable() { mEntries.ReleaseAll(); }

    /**
     * Get a session given the peer address. If the session doesn't exist in the cache, allocate a new entry for it.
     *
     * @return the session found or allocated, nullptr if not found and allocation failed.
     */
    CHECK_RETURN_VALUE
    Optional<SessionHandle> AllocEntry(GroupId group, FabricIndex fabricIndex, NodeId sourceNodeId)
    {
        GroupSession * entry = mEntries.CreateObject(group, fabricIndex, sourceNodeId);
        if (entry != nullptr)
        {
            return MakeOptional<SessionHandle>(*entry);
        }
        else
        {
            return Optional<SessionHandle>::Missing();
        }
    }

    /**
     * Get a session using given GroupId
     */
    CHECK_RETURN_VALUE
    Optional<SessionHandle> FindEntry(GroupId group, FabricIndex fabricIndex)
    {
        GroupSession * result = nullptr;
        mEntries.ForEachActiveObject([&](GroupSession * entry) {
            if (entry->GetGroupId() == group && entry->GetFabricIndex() == fabricIndex)
            {
                result = entry;
                return Loop::Break;
            }
            return Loop::Continue;
        });
        if (result != nullptr)
        {
            return MakeOptional<SessionHandle>(*result);
        }
        else
        {
            return Optional<SessionHandle>::Missing();
        }
    }

    /**
     * @brief Deletes an entry from the object pool
     *
     * @param entry The GroupSession entry to delete
     */
    void DeleteEntry(GroupSession * entry) { mEntries.ReleaseObject(entry); }

private:
    BitMapObjectPool<GroupSession, kMaxSessionCount> mEntries;
};

} // namespace Transport
} // namespace chip
