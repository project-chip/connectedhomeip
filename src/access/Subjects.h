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

// An wildcard subject to grant access based on Authentication Mode
class WildcardSubject
{
public:
    WildcardSubject(Access::AuthMode authMode) : mAuthMode(authMode) {}
    Access::AuthMode GetAuthMode() const { return mAuthMode; }
    bool Match(const WildcardSubject & that) const { return mAuthMode == that.mAuthMode; }

private:
    const Access::AuthMode mAuthMode;
};

class PaseSubject
{
public:
    static constexpr uint16_t kPasscodeId = 0;
    bool Match(const PaseSubject & that) const { return true; }
};

class NodeSubject
{
public:
    NodeSubject(NodeId nodeId) : mNodeId(nodeId) {}
    NodeId GetNodeId() const { return mNodeId; }
    bool Match(const NodeSubject & that) const { return mNodeId == that.mNodeId; }

private:
    const NodeId mNodeId;
};

class GroupSubject
{
public:
    GroupSubject(GroupId groupId) : mGroupId(groupId) {}
    GroupId GetGroupId() const { return mGroupId; }
    bool Match(const GroupSubject & that) const { return mGroupId == that.mGroupId; }

private:
    const GroupId mGroupId;
};

class CaseAuthenticatedTagSubject
{
public:
    CaseAuthenticatedTagSubject(uint16_t caseAuthenticatedTag, uint16_t version) :
        mCaseAuthenticatedTag(caseAuthenticatedTag), mVersion(version)
    {}
    uint16_t GetCaseAuthenticatedTag() const { return mCaseAuthenticatedTag; }
    bool Match(const CaseAuthenticatedTagSubject & that) const
    {
        return mVersion >= that.mVersion && mCaseAuthenticatedTag == that.mCaseAuthenticatedTag;
    }

private:
    const uint16_t mCaseAuthenticatedTag;
    const uint16_t mVersion;
};

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
        if (!mSubject.Is<WildcardSubject>())
        {
            if (mSubject.Is<PaseSubject>())
            {
                return Access::AuthMode::kPase;
            }
            else if (mSubject.Is<NodeSubject>() || mSubject.Is<CaseAuthenticatedTagSubject>())
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
        else
        {
            return mSubject.Get<WildcardSubject>().GetAuthMode();
        }
    }

#if CHIP_DETAIL_LOGGING
    const char * GetAuthModeString() const
    {
        if (!mSubject.Is<WildcardSubject>())
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
        else
        {
            return "Wildcard";
        }
    }

    uint64_t GetSubjectIdLogValue() const
    {
        if (!mSubject.Is<WildcardSubject>())
        {
            return GetSubjectId();
        }
        else
        {
            return static_cast<uint64_t>(mSubject.Get<WildcardSubject>().GetAuthMode());
        }
    }
#endif // CHIP_DETAIL_LOGGING

    /** Return SubjectId value  */
    uint64_t GetSubjectId() const
    {
        if (mSubject.Is<PaseSubject>())
        {
            return static_cast<uint64_t>(mSubject.Get<PaseSubject>().kPasscodeId) << 48;
        }
        else if (mSubject.Is<NodeSubject>())
        {
            return mSubject.Get<NodeSubject>().GetNodeId();
        }
        else if (mSubject.Is<CaseAuthenticatedTagSubject>())
        {
            return kCaseAuthenticatedTagBase ||
                (static_cast<uint64_t>(mSubject.Get<CaseAuthenticatedTagSubject>().GetCaseAuthenticatedTag()) << 16);
        }
        else if (mSubject.Is<GroupSubject>())
        {
            return static_cast<uint64_t>(mSubject.Get<GroupSubject>().GetGroupId()) << 48;
        }
        else
        {
            VerifyOrDie(false);
            return kUndefinedNodeId;
        }
    }

    // Match a subject retrieved from a session (this) against a subject in an ACL entry (that).
    bool Match(const ScopedSubject & that) const
    {
        VerifyOrDie(!mSubject.Is<WildcardSubject>());
        if (that.mSubject.Is<WildcardSubject>())
        {
            return that.mSubject.Get<WildcardSubject>().GetAuthMode() == GetAuthMode();
        }

        if (mSubject.GetType() != that.mSubject.GetType())
        {
            return false;
        }

        if (mSubject.Is<PaseSubject>())
        {
            return mSubject.Get<PaseSubject>().Match(that.mSubject.Get<PaseSubject>());
        }
        else if (mSubject.Is<NodeSubject>())
        {
            return mSubject.Get<NodeSubject>().Match(that.mSubject.Get<NodeSubject>());
        }
        else if (mSubject.Is<CaseAuthenticatedTagSubject>())
        {
            return mSubject.Get<CaseAuthenticatedTagSubject>().Match(that.mSubject.Get<CaseAuthenticatedTagSubject>());
        }
        else if (mSubject.Is<GroupSubject>())
        {
            return mSubject.Get<GroupSubject>().Match(that.mSubject.Get<GroupSubject>());
        }
        else
        {
            VerifyOrDie(false);
            return false;
        }
    }

private:
    Variant<PaseSubject, NodeSubject, GroupSubject, CaseAuthenticatedTagSubject, WildcardSubject> mSubject;

    static constexpr const uint64_t kCaseAuthenticatedTagBase = 0xFFFFFFFD00000000ull;
};

class OperationalNodeId;

/**
 * A subject is a unique subject identifier within the context of a given node. It serves 2 purposes:
 *
 *  1. Identify an entity. Match function can be used to check if a subject matches another subject.
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

    // Match a subject retrieved from a session (this) against a subject in an ACL entry (that).
    bool Match(const Subject & that) const
    {
        return mFabricIndex == that.mFabricIndex && mScopedSubject.Match(that.mScopedSubject);
    }

private:
    FabricIndex mFabricIndex;
    ScopedSubject mScopedSubject;
};

/**
 * OperationalNodeId identifies an individual node on a fabric from the point of view of some specific node. It can be converted to
 * a subject to be matched against another subject (probably retrieved from a session).
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
};

} // namespace Access
} // namespace chip
