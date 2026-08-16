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

#include <access/AccessControl.h>
#include <lib/core/CHIPError.h>

namespace chip {

class FabricTable;

namespace Credentials {
class GroupDataProvider;
} // namespace Credentials

namespace Access {

/**
 * Abstract base for an AccessControl::Delegate that supplies auxiliary ACL
 * entries derived from groupcast / group-data state.
 *
 * This interface decouples Server / CommonCaseDeviceServerInitParams from any
 * specific implementation: the SDK ships Examples::GroupAuxiliaryAccessControlDelegateImpl
 * as the default, and applications may substitute their own subclass.
 *
 * Lifecycle: Initialize() must be called exactly once before the delegate is
 * registered with AccessControl or otherwise used. Server::Init() will call
 * Initialize() on a not-yet-initialized delegate received via ServerInitParams,
 * passing the Server-owned FabricTable so subclasses can iterate provisioned
 * fabric indices efficiently. Applications that pre-initialize their own
 * delegate must make IsInitialized() return true to skip this step.
 *
 * Named Initialize / Shutdown rather than Init / Deinit so they do not shadow
 * the no-op AccessControl::Delegate::Init() virtual.
 */
class GroupAuxiliaryAccessControlDelegate : public AccessControl::Delegate
{
public:
    ~GroupAuxiliaryAccessControlDelegate() override = default;

    /**
     * Wires the delegate to its collaborators. Implementations must succeed on
     * the first call and return CHIP_ERROR_INCORRECT_STATE on subsequent calls
     * until Shutdown() has been invoked.
     *
     * @param groupDataProvider Required. Source of group / endpoint membership data.
     * @param fabricTable       Optional. Pass a valid FabricTable so auxiliary-entry
     *                          iteration walks only provisioned fabric indices.
     *                          Passing nullptr is allowed but discouraged: iteration
     *                          then walks [kMinValidFabricIndex, kMaxValidFabricIndex]
     *                          linearly.
     */
    virtual CHIP_ERROR Initialize(Credentials::GroupDataProvider * groupDataProvider, FabricTable * fabricTable) = 0;

    /**
     * Releases references taken in Initialize and returns the delegate to its
     * uninitialized state. Idempotent. Callers are responsible for unregistering
     * the delegate from AccessControl before Shutdown if it was registered.
     */
    virtual void Shutdown() = 0;

    /** @return true if Initialize has completed successfully. */
    virtual bool IsInitialized() const = 0;
};

} // namespace Access
} // namespace chip
