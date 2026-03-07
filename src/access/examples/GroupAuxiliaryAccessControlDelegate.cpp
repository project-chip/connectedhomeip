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

#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/NodeId.h>
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
    void Init(EntryIterator & iterator, Credentials::GroupDataProvider * groupDataProvider, FabricIndex fabric)
    {
        mGroupDataProvider = groupDataProvider;
        mFabric            = fabric;
        if (mGroupDataProvider)
        {
            mGroupInfoIterator = mGroupDataProvider->IterateGroupInfo(mFabric);
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

        while (mGroupInfoIterator != nullptr || mEndpointIterator != nullptr)
        {
            if (mEndpointIterator != nullptr)
            {
                Credentials::GroupDataProvider::GroupEndpoint endpoint;
                if (mEndpointIterator->Next(endpoint))
                {
                    auto * delegate = Platform::New<EntryDelegate>();
                    if (delegate == nullptr)
                    {
                        return CHIP_ERROR_NO_MEMORY;
                    }
                    delegate->Init(entry, mFabric, mGroupId, endpoint.endpoint_id);
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
                        mEndpointIterator = mGroupDataProvider->IterateEndpoints(mFabric, mGroupId);
                    }
                }
                else
                {
                    mGroupInfoIterator->Release();
                    mGroupInfoIterator = nullptr;
                }
            }
        }

        return CHIP_ERROR_SENTINEL;
    }

private:
    Credentials::GroupDataProvider * mGroupDataProvider;
    FabricIndex mFabric;
    Credentials::GroupDataProvider::GroupInfoIterator * mGroupInfoIterator = nullptr;
    Credentials::GroupDataProvider::EndpointIterator * mEndpointIterator   = nullptr;
    GroupId mGroupId                                                       = kUndefinedGroupId;
};

} // namespace

namespace chip {
namespace Access {
namespace Examples {

CHIP_ERROR GroupAuxiliaryAccessControlDelegate::AuxiliaryEntries(AccessControl::EntryIterator & iterator,
                                                                 const FabricIndex * fabricIndex) const
{
    VerifyOrReturnError(fabricIndex != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    auto * delegate = Platform::New<AuxiliaryEntryIteratorDelegate>();
    if (delegate)
    {
        delegate->Init(iterator, mGroupDataProvider, *fabricIndex);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR GroupAuxiliaryAccessControlDelegate::Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                                                      Privilege requestPrivilege)
{
    AccessControl::EntryIterator iterator;
    ReturnErrorOnFailure(AuxiliaryEntries(iterator, &subjectDescriptor.fabricIndex));

    AccessControl::Entry entry;
    while (iterator.Next(entry) == CHIP_NO_ERROR)
    {
        // Simplest form of Auxliliary ACL Entry format assumes only 1 subject and 1 target per entry.
        // With more compelx ACL entry format, checks should be done accross all subjects and targets
        size_t numSubjects;
        size_t numTargets;
        ReturnErrorOnFailure(entry.GetSubjectCount(numSubjects));
        ReturnErrorOnFailure(entry.GetTargetCount(numTargets));
        VerifyOrReturnError(numSubjects == 1, CHIP_ERROR_SENTINEL);
        VerifyOrReturnError(numTargets == 1, CHIP_ERROR_SENTINEL);

        NodeId subjectFromEntry;
        ReturnErrorOnFailure(entry.GetSubject(0, subjectFromEntry));
        if (subjectFromEntry != subjectDescriptor.subject)
        {
            continue;
        }

        AccessControl::Entry::Target targetFromEntry;
        ReturnErrorOnFailure(entry.GetTarget(0, targetFromEntry));
        if (targetFromEntry.endpoint != requestPath.endpoint)
        {
            continue;
        }

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_ACCESS_DENIED;
}

} // namespace Examples
} // namespace Access
} // namespace chip
