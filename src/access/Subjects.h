/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <limits>

#include <access/AuthMode.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/InPlace.h>
#include <lib/core/NodeId.h>
#include <lib/support/Variant.h>

namespace chip {
namespace Access {

struct PaseSubject
{
    static constexpr uint16_t kPasscodeId = 0;
    bool operator==(const PaseSubject & that) const { return true; }
};

struct NodeSubject
{
    NodeSubject(NodeId aNodeId) : nodeId(aNodeId) {}
    NodeId nodeId;
    bool operator==(const NodeSubject & that) const { return nodeId == that.nodeId; }
};

struct GroupSubject
{
    GroupSubject(GroupId aGroupId) : groupId(aGroupId) {}
    GroupId groupId;
    bool operator==(const GroupSubject & that) const { return groupId == that.groupId; }
};

class OperationalNodeId;

/** A scoped subject is a unique identifier with-in a fabric scope. */
class ScopedSubject
{
public:
    ScopedSubject() {}

    template <typename T, class... Args>
    constexpr explicit ScopedSubject(InPlaceTemplateType<T>, Args &&... args) :
        mSubject(InPlaceTemplate<T>, std::forward<Args>(args)...)
    {}

    template <typename T, typename... Args>
    static ScopedSubject Create(Args &&... args)
    {
        return ScopedSubject(InPlaceTemplate<T>, std::forward<Args>(args)...);
    }

    Access::AuthMode GetAuthMode() const
    {
        if (mSubject.Is<PaseSubject>())
        {
            return Access::AuthMode::kPase;
        }
        else if (mSubject.Is<NodeSubject>())
        {
            return Access::AuthMode::kCase;
        }
        else if (mSubject.Is<GroupSubject>())
        {
            return Access::AuthMode::kGroup;
        }
        else
        {
            return Access::AuthMode::kNone;
        }
    }

#if CHIP_DETAIL_LOGGING
    const char * GetAuthModeString() const
    {
        switch (GetAuthMode())
        {
        case Access::AuthMode::kPase:
            return "Pase";
        case Access::AuthMode::kCase:
            return "Case";
        case Access::AuthMode::kGroup:
            return "Group";
        case Access::AuthMode::kNone:
        default:
            return "None";
        }
    }
#endif // CHIP_DETAIL_LOGGING

    /** Return subject value described in spec */
    uint64_t GetValue() const
    {
        if (mSubject.Is<PaseSubject>())
        {
            return static_cast<uint64_t>(mSubject.Get<PaseSubject>().kPasscodeId) << 48;
        }
        else if (mSubject.Is<NodeSubject>())
        {
            return mSubject.Get<NodeSubject>().nodeId;
        }
        else if (mSubject.Is<GroupSubject>())
        {
            return static_cast<uint64_t>(mSubject.Get<GroupSubject>().groupId) << 48;
        }
        else
        {
            return std::numeric_limits<uint64_t>::max();
        }
    }

    bool operator==(const ScopedSubject & that) const { return mSubject == that.mSubject; }

private:
    Variant<PaseSubject, NodeSubject, GroupSubject> mSubject;
};

/**
 * A subject is a global unique identifier. It serves 2 purposes:
 *
 *  1. Identify an entity. operator== can be used to check if 2 subjects are identical.
 *  2. Associate to ACL entries to grant privileges
 */
class Subject
{
public:
    Subject() : mFabricIndex(kUndefinedFabricIndex) {}

    template <typename T, class... Args>
    constexpr explicit Subject(FabricIndex fabricIndex, InPlaceTemplateType<T>, Args &&... args) :
        mFabricIndex(fabricIndex), mScopedSubject(InPlaceTemplate<T>, std::forward<Args>(args)...)
    {}

    template <typename T, typename... Args>
    static Subject Create(FabricIndex fabricIndex, Args &&... args)
    {
        return Subject(fabricIndex, InPlaceTemplate<T>, std::forward<Args>(args)...);
    }

    static Subject CreatePaseSubject()
    {
        // Return a unique subject for all PASE sessions.
        return Subject(kUndefinedFabricIndex, InPlaceTemplate<PaseSubject>);
    }

    FabricIndex GetFabricIndex() const { return mFabricIndex; }
    const ScopedSubject & GetScopedSubject() const { return mScopedSubject; }

    bool operator==(const Subject & that) const
    {
        return mFabricIndex == that.mFabricIndex && mScopedSubject == that.mScopedSubject;
    }

private:
    FabricIndex mFabricIndex;
    ScopedSubject mScopedSubject;

    friend bool operator==(const Subject &, const OperationalNodeId &);
};

/**
 * OperationalNodeId identifies an individual Node on a Fabric. It is a special type of Subject targeting to NodeSubject. It is
 * interchangeable with the generic Subject type but uses less memory.
 */
class OperationalNodeId
{
public:
    OperationalNodeId(FabricIndex fabricIndex, NodeId nodeId) : mFabricIndex(fabricIndex), mNodeId(nodeId) {}
    Subject ToSubject() { return Subject::Create<NodeSubject>(mFabricIndex, mNodeId); }

    FabricIndex GetFabricIndex() const { return mFabricIndex; }
    NodeId GetNodeId() const { return mNodeId; }

private:
    FabricIndex mFabricIndex;
    NodeId mNodeId;

    bool operator==(const OperationalNodeId & that) const { return mFabricIndex == that.mFabricIndex && mNodeId == that.mNodeId; }

    friend bool operator==(const Subject &, const OperationalNodeId &);
};

inline bool operator==(const Subject & subject, const OperationalNodeId & node)
{
    return subject.mFabricIndex == node.mFabricIndex && subject.mScopedSubject == ScopedSubject::Create<NodeSubject>(node.mNodeId);
}

} // namespace Access
} // namespace chip
