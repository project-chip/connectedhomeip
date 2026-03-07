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

#include "access/AccessControl.h"
#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>

namespace chip {
namespace Access {
namespace Examples {

class GroupAuxiliaryAccessControlDelegate : public AccessControl::Delegate
{
public:
    GroupAuxiliaryAccessControlDelegate(Credentials::GroupDataProvider * groupDataProvider) : mGroupDataProvider(groupDataProvider)
    {}
    ~GroupAuxiliaryAccessControlDelegate() override = default;

    // Delegate implementation
    CHIP_ERROR AuxiliaryEntries(AccessControl::EntryIterator & iterator, const FabricIndex * fabricIndex) const override;

    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                     Privilege requestPrivilege) override;

private:
    Credentials::GroupDataProvider * mGroupDataProvider;
};

} // namespace Examples
} // namespace Access
} // namespace chip
