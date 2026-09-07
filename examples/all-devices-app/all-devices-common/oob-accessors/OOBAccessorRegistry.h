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

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBAccessor.h>

namespace chip::app {

class OOBAccessorRegistry
{
public:
    static OOBAccessorRegistry & Instance()
    {
        static OOBAccessorRegistry instance;
        return instance;
    }

    void Register(OOBAccessor & accessor) { mAccessors.PushBack(&accessor); }

    /**
     * @brief Routes a simulation or out-of-band control action to the registered accessors.
     *
     * The schema of `tlvBuffer` is action-specific and defined by the accessor handling the action.
     * For example, the standard "SetAttribute" action expects a flat TLV Structure containing:
     *   - Context Tag 1: Endpoint ID (uint16_t)
     *   - Context Tag 2: Cluster ID (uint32_t)
     *   - Context Tag 3: Attribute ID (uint32_t)
     *   - Context Tag 4: Attribute Value (Any valid TLV element matching the attribute's type)
     *
     * See OOBDataSerializer::ParseAttributeRequest and OOBDataSerializer::BuildSetAttributeRequest for a concrete
     * parsing-building implementation.
     *
     * @return CHIP_ERROR_NOT_FOUND if no accessor handles this action; otherwise, the result of the action.
     */
    CHIP_ERROR HandleAction(CharSpan actionName, ByteSpan tlvBuffer)
    {
        for (auto & accessor : mAccessors)
        {
            auto result = accessor.HandleAction(actionName, tlvBuffer);
            if (result.has_value())
            {
                return *result;
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

private:
    OOBAccessorRegistry()  = default;
    ~OOBAccessorRegistry() = default;

    IntrusiveList<OOBAccessor, IntrusiveMode::AutoUnlink> mAccessors;
};

} // namespace chip::app
