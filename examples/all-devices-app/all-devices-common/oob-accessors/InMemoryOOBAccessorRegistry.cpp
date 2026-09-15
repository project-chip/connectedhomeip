/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <oob-accessors/InMemoryOOBAccessorRegistry.h>

#include <lib/support/CodeUtils.h>

namespace chip::app {

CHIP_ERROR InMemoryOOBAccessorRegistry::Register(std::unique_ptr<OOBAccessor> accessor)
{
    VerifyOrReturnError(accessor != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mAccessors.push_back(std::move(accessor));
    return CHIP_NO_ERROR;
}

CHIP_ERROR InMemoryOOBAccessorRegistry::HandleAction(CharSpan action, ByteSpan tlvData)
{
    // Give registered cluster OOB accessors a chance to handle the action.
    // For "SetAttribute", read-only or constant attributes (such as OccupancySensing::Occupancy
    // or BooleanState::StateValue) cannot be written via the Matter DataModel and MUST be
    // intercepted here by their respective cluster OOB accessor calling the cluster's C++ API.
    // If no accessor claims the action or attribute, CHIP_ERROR_NOT_FOUND is returned so
    // callers (e.g. Pigweed RPC or Named Pipe) can fall back to their injected DataModel provider.
    for (const auto & accessor : mAccessors)
    {
        auto result = accessor->HandleAction(action, tlvData);
        if (result.has_value())
        {
            return *result;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

} // namespace chip::app
