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

class GroupAuxiliaryAccessControlDelegate : public AccessControl::Delegate
{
public:
    GroupAuxiliaryAccessControlDelegate(Credentials::GroupDataProvider * groupDataProvider) : mGroupDataProvider(groupDataProvider) {}
    ~GroupAuxiliaryAccessControlDelegate() override = default;

    // Delegate implementation
    CHIP_ERROR AuxiliaryEntries(AccessControl::EntryIterator & iterator, const FabricIndex * fabricIndex) const override;

    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

private:
    Credentials::GroupDataProvider * mGroupDataProvider;
};

class AuxiliaryEntryIteratorDelegate : public AccessControl::EntryIterator::Delegate
{
public:
    AuxiliaryEntryIteratorDelegate(Credentials::GroupDataProvider * groupDataProvider, FabricIndex fabric) :
        mGroupDataProvider(groupDataProvider), mFabric(fabric)
    {
        mGroupInfoIterator = mGroupDataProvider->IterateGroupInfo(mFabric);
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

    CHIP_ERROR Next(AccessControl::Entry & entry) override
    {
        while (mGroupInfoIterator != nullptr || mEndpointIterator != nullptr)
        {
            if (mEndpointIterator != nullptr)
            {
                Credentials::GroupDataProvider::GroupEndpoint endpoint;
                if (mEndpointIterator->Next(endpoint))
                {
                    // TODO: This PrepareEntry() call goes through the access control delegate set in 
                    // the access controol module, NOT the group auxiliary ACL delegate. Needs to be addressed.
                    ReturnErrorOnFailure(Access::GetAccessControl().PrepareEntry(entry));
                    ReturnErrorOnFailure(entry.SetFabricIndex(mFabric));
                    ReturnErrorOnFailure(entry.SetPrivilege(Privilege::kOperate));
                    ReturnErrorOnFailure(entry.SetAuthMode(AuthMode::kGroup));
                    ReturnErrorOnFailure(entry.SetAuxiliaryType(AuxiliaryType::kGroupcast));
                    ReturnErrorOnFailure(entry.AddSubject(nullptr, NodeIdFromGroupId(mGroupId)));
                    AccessControl::Entry::Target target;
                    target.flags    = AccessControl::Entry::Target::kEndpoint;
                    target.endpoint = endpoint.endpoint_id;
                    ReturnErrorOnFailure(entry.AddTarget(nullptr, target));
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

CHIP_ERROR GroupAuxiliaryAccessControlDelegate::AuxiliaryEntries(AccessControl::EntryIterator & iterator,
                                                                  const FabricIndex * fabricIndex) const
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

} // namespace namespace

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
