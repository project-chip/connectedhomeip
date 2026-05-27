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

#pragma once

#include <access/GroupAuxiliaryAccessControlDelegate.h>
#include <credentials/GroupDataProvider.h>
#include <lib/core/DataModelTypes.h>

namespace chip {

class FabricTable;

namespace Access {
namespace Examples {

/**
 * Default SDK implementation of GroupAuxiliaryAccessControlDelegate.
 *
 * Reports auxiliary ACL entries for every <fabricIndex, groupId, endpointId>
 * triple derived from the supplied GroupDataProvider. This is the simplest
 * "base case" shape of an auxiliary ACL entry set; products that need a
 * different layout should subclass GroupAuxiliaryAccessControlDelegate
 * directly rather than reusing this class.
 */
class GroupAuxiliaryAccessControlDelegateImpl : public GroupAuxiliaryAccessControlDelegate
{
public:
    GroupAuxiliaryAccessControlDelegateImpl()           = default;
    ~GroupAuxiliaryAccessControlDelegateImpl() override = default;

    CHIP_ERROR Initialize(Credentials::GroupDataProvider * groupDataProvider, FabricTable * fabricTable) override;
    void Shutdown() override;
    bool IsInitialized() const override { return mGroupDataProvider != nullptr; }

    // AccessControl::Delegate
    CHIP_ERROR AuxiliaryEntries(AccessControl::EntryIterator & iterator, const FabricIndex * fabricIndex) const override;
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                     Privilege requestPrivilege) override;

private:
    Credentials::GroupDataProvider * mGroupDataProvider = nullptr;
    FabricTable * mFabricTable                          = nullptr;
};

} // namespace Examples
} // namespace Access
} // namespace chip
