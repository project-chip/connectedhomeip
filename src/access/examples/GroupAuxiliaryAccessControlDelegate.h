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
#include <lib/core/DataModelTypes.h>

namespace chip {

class FabricTable;

namespace Access {
namespace Examples {

class GroupAuxiliaryAccessControlDelegate : public AccessControl::Delegate
{
public:
    GroupAuxiliaryAccessControlDelegate()           = default;
    ~GroupAuxiliaryAccessControlDelegate() override = default;

    /**
     * Wires the delegate to its collaborators. Must be called exactly once before the
     * delegate is registered with AccessControl or otherwise used.
     *
     * Named Initialize (not Init) to avoid hiding the no-op AccessControl::Delegate::Init()
     * virtual that the access-control framework may invoke through the base pointer.
     *
     * @param groupDataProvider Required. Source of group / endpoint membership data.
     * @param fabricTable       Required. Pass a valid FabricTable so that auxiliary-entry
     *                          iteration walks only provisioned fabric indices. Passing
     *                          nullptr is allowed but discouraged: iteration then walks
     *                          [kMinValidFabricIndex, kMaxValidFabricIndex] linearly.
     *
     * @retval CHIP_ERROR_INVALID_ARGUMENT if @a groupDataProvider is null.
     * @retval CHIP_ERROR_INCORRECT_STATE  if Initialize has already been called.
     */
    CHIP_ERROR Initialize(Credentials::GroupDataProvider * groupDataProvider, FabricTable * fabricTable);

    /**
     * Resets the delegate to its uninitialized state, releasing references to the
     * GroupDataProvider and FabricTable. Idempotent (no-op when already uninitialized).
     * After Shutdown, Initialize may be called again. Callers are responsible for
     * unregistering the delegate from AccessControl before Shutdown if it was registered.
     */
    void Shutdown();

    /** @return true if Initialize has completed successfully. */
    bool IsInitialized() const { return mGroupDataProvider != nullptr; }

    // Delegate implementation
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
