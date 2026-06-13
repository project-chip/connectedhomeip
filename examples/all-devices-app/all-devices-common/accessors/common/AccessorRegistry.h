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

#pragma once

#include <accessors/common/OOBAccessor.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

class AccessorRegistry
{
public:
    /**
     * @brief Returns the singleton instance of the AccessorRegistry.
     */
    static AccessorRegistry & Instance()
    {
        static AccessorRegistry instance;
        return instance;
    }

    /**
     * @brief Registers a device accessor with the registry.
     *
     * @param accessor The accessor to register.
     * @return CHIP_NO_ERROR on success.
     */
    CHIP_ERROR Register(OOBAccessor * accessor)
    {
        if (accessor == nullptr)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        mAccessors.PushBack(accessor);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Unregisters a device accessor from the registry.
     *
     * @param accessor The accessor to unregister.
     */
    void Unregister(OOBAccessor * accessor)
    {
        if (accessor != nullptr)
        {
            mAccessors.Remove(accessor);
        }
    }

    /**
     * @brief Routes an action/simulation invocation to the registered accessors.
     *
     * @param actionName The name of the action to invoke.
     * @param arguments A TLVReader positioned at the arguments for the action.
     * @param path Optional pointer to ConcreteDataAttributePath.
     * @return std::nullopt if no accessor handled the request.
     *         A non-null optional containing the CHIP_ERROR result from the handling accessor.
     */
    std::optional<CHIP_ERROR> HandleAction(CharSpan actionName, chip::TLV::TLVReader & arguments,
                                           const ConcreteDataAttributePath * path = nullptr)
    {
        for (auto & accessor : mAccessors)
        {
            auto result = accessor.HandleAction(actionName, arguments, path);
            if (result.has_value())
            {
                return result;
            }
        }
        return std::nullopt;
    }

private:
    AccessorRegistry()  = default;
    ~AccessorRegistry() = default;

    IntrusiveList<OOBAccessor, IntrusiveMode::AutoUnlink> mAccessors;
};

} // namespace chip::app
