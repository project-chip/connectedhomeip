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
            // TODO: These are inaccurate and need to be updated
            target.cluster  = 0;
            target.deviceType = 0;
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
    AuxiliaryEntryIteratorDelegate(Credentials::GroupDataProvider * groupDataProvider, FabricIndex fabric) :
        mGroupDataProvider(groupDataProvider), mFabric(fabric)
    {
        if (mGroupDataProvider)
        {
            mGroupInfoIterator = mGroupDataProvider->IterateGroupInfo(mFabric);
        }
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

class GroupAuxiliaryAccessControlDelegate : public AccessControl::Delegate
{
public:
    GroupAuxiliaryAccessControlDelegate(Credentials::GroupDataProvider * groupDataProvider) : mGroupDataProvider(groupDataProvider) {}
    ~GroupAuxiliaryAccessControlDelegate() override = default;

    // Delegate implementation
    CHIP_ERROR AuxiliaryEntries(EntryIterator & iterator, const FabricIndex * fabricIndex) const override
    {
        VerifyOrReturnError(fabricIndex != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        auto * delegate = Platform::New<AuxiliaryEntryIteratorDelegate>(mGroupDataProvider, *fabricIndex);
        if (delegate)
        {
            iterator.SetDelegate(*delegate);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

private:
    Credentials::GroupDataProvider * mGroupDataProvider;
};

} // namespace

namespace chip {
namespace Access {
namespace Examples {

AccessControl::Delegate * GetGroupAuxiliaryAccessControlDelegate()
{
    static GroupAuxiliaryAccessControlDelegate accessControlDelegate(Credentials::GetGroupDataProvider());
    return &accessControlDelegate;
}

} // namespace Examples
} // namespace Access
} // namespace chip
