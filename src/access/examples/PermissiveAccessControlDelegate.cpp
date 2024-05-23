/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "PermissiveAccessControlDelegate.h"

namespace {

using namespace chip;
using namespace chip::Access;

using Entry         = chip::Access::AccessControl::Entry;
using EntryIterator = chip::Access::AccessControl::EntryIterator;
using Target        = Entry::Target;

class AccessControlDelegate : public AccessControl::Delegate
{
public:
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    void Finish() override {}

    // Capabilities
    CHIP_ERROR GetMaxEntryCount(size_t & value) const override
    {
        value = 0;
        return CHIP_NO_ERROR;
    }

    // Actualities
    CHIP_ERROR GetEntryCount(size_t & value) const override
    {
        value = 0;
        return CHIP_NO_ERROR;
    }

    // Preparation
    CHIP_ERROR PrepareEntry(Entry & entry) override { return CHIP_NO_ERROR; }

    // CRUD
    CHIP_ERROR CreateEntry(size_t * index, const Entry & entry, FabricIndex * fabricIndex) override { return CHIP_NO_ERROR; }
    CHIP_ERROR ReadEntry(size_t index, Entry & entry, const FabricIndex * fabricIndex) const override { return CHIP_NO_ERROR; }
    CHIP_ERROR UpdateEntry(size_t index, const Entry & entry, const FabricIndex * fabricIndex) override { return CHIP_NO_ERROR; }
    CHIP_ERROR DeleteEntry(size_t index, const FabricIndex * fabricIndex) override { return CHIP_NO_ERROR; }

    // Iteration
    CHIP_ERROR Entries(EntryIterator & iterator, const FabricIndex * fabricIndex) const override { return CHIP_NO_ERROR; }

    // Check
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        return CHIP_NO_ERROR;
    }
};

} // namespace

namespace chip {
namespace Access {
namespace Examples {

AccessControl::Delegate * GetPermissiveAccessControlDelegate()
{
    static AccessControlDelegate accessControlDelegate;
    return &accessControlDelegate;
}

} // namespace Examples
} // namespace Access
} // namespace chip
