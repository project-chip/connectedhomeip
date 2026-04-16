/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include "GroupAuxiliaryAccessControlDelegate.h"

#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/NodeId.h>
#include <lib/core/Optional.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>

namespace {

using namespace chip;

using chip::Access::AccessControl;
using chip::Access::AuthMode;
using chip::Access::AuxiliaryType;
using chip::Access::Privilege;
using chip::Access::RequestPath;
using chip::Access::SubjectDescriptor;

using Entry         = chip::Access::AccessControl::Entry;
using EntryIterator = chip::Access::AccessControl::EntryIterator;
using Target        = Entry::Target;

class EntryDelegate : public AccessControl::Entry::Delegate
{
public:
    void Init(Entry & entry, FabricIndex fabricIndex, GroupId groupId, EndpointId endpointId)
    {
        mFabricIndex = fabricIndex;
        mGroupId     = groupId;
        mEndpointId  = endpointId;
        entry.SetDelegate(*this);
    }

    void Release() override { Platform::Delete(this); }

    CHIP_ERROR GetAuthMode(AuthMode & authMode) const override
    {
        authMode = AuthMode::kGroup;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetFabricIndex(FabricIndex & fabricIndex) const override
    {
        fabricIndex = mFabricIndex;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetPrivilege(Privilege & privilege) const override
    {
        privilege = Privilege::kOperate;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetAuxiliaryType(AuxiliaryType & auxiliaryType) const override
    {
        auxiliaryType = AuxiliaryType::kGroupcast;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSubjectCount(size_t & count) const override
    {
        count = 1;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSubject(size_t index, NodeId & subject) const override
    {
        if (index == 0)
        {
            subject = NodeIdFromGroupId(mGroupId);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR GetTargetCount(size_t & count) const override
    {
        count = 1;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetTarget(size_t index, Target & target) const override
    {
        if (index == 0)
        {
            target.flags    = Target::kEndpoint;
            target.endpoint = mEndpointId;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_SENTINEL;
    }

private:
    FabricIndex mFabricIndex;
    GroupId mGroupId;
    EndpointId mEndpointId;
};

class AuxiliaryEntryIteratorDelegate : public EntryIterator::Delegate
{
public:
    void Init(EntryIterator & iterator, Credentials::GroupDataProvider * groupDataProvider, FabricTable * fabricTable,
              FabricIndex fabricIndex)
    {
        mGroupDataProvider = groupDataProvider;
        mFabricTable       = fabricTable;
        mFabricIndex       = fabricIndex;

        if (mFabricIndex == kUndefinedFabricIndex)
        {
            mIterateOverFabricIndices = true;
            // If the fabric table is defined, it can be used to find and iterate over all
            // valid existing fabric indices. Otherwise, iteration can be done starting from
            // the minimum fabric index and going up
            if (mFabricTable)
            {
                mFabricTableIter.SetValue(mFabricTable->begin());
                if (mFabricTableIter.Value() != mFabricTable->end())
                {
                    mFabricIndex = mFabricTableIter.Value()->GetFabricIndex();
                }
            }
            else
            {
                mFabricIndex = kMinValidFabricIndex;
            }
        }

        if (mGroupDataProvider)
        {
            mGroupInfoIterator = mGroupDataProvider->IterateGroupInfo(mFabricIndex);
        }
        iterator.SetDelegate(*this);
    }

    ~AuxiliaryEntryIteratorDelegate() override
    {
        if (mGroupInfoIterator)
        {
            mGroupInfoIterator->Release();
        }
        if (mEndpointIterator)
        {
            mEndpointIterator->Release();
        }
    }

    void Release() override { Platform::Delete(this); }

    CHIP_ERROR Next(Entry & entry) override
    {
        if (mGroupDataProvider == nullptr)
        {
            return CHIP_ERROR_SENTINEL;
        }

        while (mGroupInfoIterator != nullptr || mEndpointIterator != nullptr || mIterateOverFabricIndices)
        {
            if (mEndpointIterator != nullptr)
            {
                Credentials::GroupDataProvider::GroupEndpoint endpoint;
                if (mEndpointIterator->Next(endpoint))
                {
                    // Groups cannot be created with endpoint 0, so this state should never be reached
                    // because of restrictions with creating/joining groups. We skip here in the case
                    // this does occur, no entry would be needed to validate against for endpoint 0.
                    if (endpoint.endpoint_id == kRootEndpointId)
                    {
                        continue;
                    }

                    auto * delegate = Platform::New<EntryDelegate>();
                    if (delegate == nullptr)
                    {
                        return CHIP_ERROR_NO_MEMORY;
                    }
                    delegate->Init(entry, mFabricIndex, mGroupId, endpoint.endpoint_id);
                    return CHIP_NO_ERROR;
                }
                mEndpointIterator->Release();
                mEndpointIterator = nullptr;
            }

            if (mGroupInfoIterator != nullptr)
            {
                Credentials::GroupDataProvider::GroupInfo info;
                if (mGroupInfoIterator->Next(info))
                {
                    if (info.flags & to_underlying(Credentials::GroupDataProvider::GroupInfo::Flags::kHasAuxiliaryACL))
                    {
                        mGroupId          = info.group_id;
                        mEndpointIterator = mGroupDataProvider->IterateEndpoints(mFabricIndex, mGroupId);
                    }
                    continue;
                }
                mGroupInfoIterator->Release();
                mGroupInfoIterator = nullptr;
            }

            if (mIterateOverFabricIndices)
            {
                // This indicates that there are no more fabric indices to iterate over (unless it is
                // determined in the conditions below that at least 1 more is remaining, in which
                // case this will be set to true again).
                mIterateOverFabricIndices = false;

                if (mFabricTable && mFabricTableIter.HasValue())
                {
                    if ((mFabricTableIter.Value() != mFabricTable->end()) && (++mFabricTableIter.Value() != mFabricTable->end()))
                    {
                        mFabricIndex              = mFabricTableIter.Value()->GetFabricIndex();
                        mGroupInfoIterator        = mGroupDataProvider->IterateGroupInfo(mFabricIndex);
                        mIterateOverFabricIndices = true;
                    }
                }
                else if (mFabricIndex < kMaxValidFabricIndex)
                {
                    mFabricIndex++;
                    mGroupInfoIterator        = mGroupDataProvider->IterateGroupInfo(mFabricIndex);
                    mIterateOverFabricIndices = true;
                }
            }
        }

        return CHIP_ERROR_SENTINEL;
    }

private:
    Credentials::GroupDataProvider * mGroupDataProvider;
    FabricTable * mFabricTable;
    FabricIndex mFabricIndex;
    chip::Optional<chip::ConstFabricIterator> mFabricTableIter;
    Credentials::GroupDataProvider::GroupInfoIterator * mGroupInfoIterator = nullptr;
    Credentials::GroupDataProvider::EndpointIterator * mEndpointIterator   = nullptr;
    GroupId mGroupId                                                       = kUndefinedGroupId;

    // When true, this indicates the entries are not fabric scoped. AuxiliaryEntries() through
    // the Next() function will iterate over all fabrics to fetch auxiliary ACL entries. This
    // will be set to true when a fabric index is not specified (kUndefinedFabricIndex).
    bool mIterateOverFabricIndices = false;
};

} // namespace

namespace chip {
namespace Access {
namespace Examples {

/*
 * This function (in conjunction with Next() from the AuxiliaryEntryIteratorDelegate) will create an auxiliary
 * ACL entry for every <fabric index, group ID, endpoint ID> that belongs based on the information from
 * the group data provider. This is the simplest base case of what a set of auxiliary ACL entries will look
 * like. The structure of auxiliary ACL entries can be formatted differently, as long as the equivalence class
 * maps to this simplest base case.
 */
CHIP_ERROR GroupAuxiliaryAccessControlDelegate::AuxiliaryEntries(AccessControl::EntryIterator & iterator,
                                                                 const FabricIndex * fabricIndex) const
{
    auto * delegate = Platform::New<AuxiliaryEntryIteratorDelegate>();
    if (delegate)
    {
        delegate->Init(iterator, mGroupDataProvider, mFabricTable, fabricIndex ? *fabricIndex : kUndefinedFabricIndex);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR GroupAuxiliaryAccessControlDelegate::Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                                                      Privilege requestPrivilege)
{
    if (IsGroupId(subjectDescriptor.subject) && (mGroupDataProvider != nullptr))
    {
        GroupId groupId = GroupIdFromNodeId(subjectDescriptor.subject);
        if ((requestPath.endpoint != kRootEndpointId) &&
            (mGroupDataProvider->HasEndpoint(subjectDescriptor.fabricIndex, groupId, requestPath.endpoint)) &&
            (requestPath.requestType == Access::RequestType::kCommandInvokeRequest) && (requestPrivilege == Privilege::kOperate) &&
            (subjectDescriptor.authMode == Access::AuthMode::kGroup))
        {
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_ACCESS_DENIED;
}

} // namespace Examples
} // namespace Access
} // namespace chip
