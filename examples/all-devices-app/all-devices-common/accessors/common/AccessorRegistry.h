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

#include <accessors/common/SingleEndpointDeviceAccessor.h>
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
     * Once registered, the registry can route OOB requests to this accessor
     * based on its endpoint ID. Only one accessor can be registered per endpoint.
     *
     * @param accessor The accessor to register.
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_DUPLICATE_KEY_ID if an accessor is already registered for this endpoint.
     */
    CHIP_ERROR Register(SingleEndpointDeviceAccessor * accessor)
    {
        if (accessor == nullptr)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        EndpointId endpointId = accessor->GetEndpointId();
        for (auto & existing : mAccessors)
        {
            if (existing.GetEndpointId() == endpointId)
            {
                ChipLogError(Support, "Accessor already registered for endpoint %d", endpointId);
                return CHIP_ERROR_DUPLICATE_KEY_ID;
            }
        }
        mAccessors.PushBack(accessor);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Unregisters a device accessor from the registry.
     *
     * @param accessor The accessor to unregister.
     */
    void Unregister(SingleEndpointDeviceAccessor * accessor)
    {
        if (accessor != nullptr)
        {
            mAccessors.Remove(accessor);
        }
    }

    /**
     * @brief Finds the registered accessor for a given endpoint.
     *
     * @param endpointId The endpoint ID to search for.
     * @return A pointer to the accessor if found, or nullptr otherwise.
     */
    SingleEndpointDeviceAccessor * GetAccessor(EndpointId endpointId)
    {
        for (auto & accessor : mAccessors)
        {
            if (accessor.GetEndpointId() == endpointId)
            {
                ChipLogProgress(Support, "Found single endpoint device accessor for endpoint %d", endpointId);
                return &accessor;
            }
        }
        ChipLogProgress(Support, "No single endpoint device accessor found for endpoint %d", endpointId);
        return nullptr;
    }

    /**
     * @brief Routes an attribute write request to the appropriate accessor.
     *
     * The registry looks up the accessor matching the endpoint ID in the path
     * and forwards the request to it.
     *
     * @param path The concrete path of the attribute being written.
     * @param decoder The decoder containing the value to be written.
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_KEY_NOT_FOUND if no accessor is registered for the endpoint.
     *         Other CHIP_ERROR codes from the accessor's SetAttribute implementation.
     */
    CHIP_ERROR SetAttribute(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder)
    {
        auto * accessor = GetAccessor(path.mEndpointId);
        if (accessor == nullptr)
        {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }
        return accessor->SetAttribute(path, decoder);
    }

    /**
     * @brief Routes an action/simulation invocation to the appropriate accessor.
     *
     * The registry looks up the accessor matching the endpoint ID and forwards
     * the action invocation to it.
     *
     * @param endpointId The target endpoint ID for the action.
     * @param actionName The name of the action to invoke.
     * @param arguments A TLVReader positioned at the arguments for the action.
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_KEY_NOT_FOUND if no accessor is registered for the endpoint.
     *         Other CHIP_ERROR codes from the accessor's InvokeAction implementation.
     */
    CHIP_ERROR InvokeAction(EndpointId endpointId, CharSpan actionName, chip::TLV::TLVReader & arguments)
    {
        auto * accessor = GetAccessor(endpointId);
        if (accessor == nullptr)
        {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }
        return accessor->InvokeAction(actionName, arguments);
    }

private:
    AccessorRegistry()  = default;
    ~AccessorRegistry() = default;

    IntrusiveList<SingleEndpointDeviceAccessor, IntrusiveMode::AutoUnlink> mAccessors;
};

} // namespace chip::app
