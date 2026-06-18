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

#include <vector>

#include <accessors/common/OOBAccessor.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

class AccessorRegistry
{
public:
    static AccessorRegistry & Instance()
    {
        static AccessorRegistry instance;
        return instance;
    }

    void Register(OOBAccessor & accessor) { mAccessors.PushBack(&accessor); }

    /**
     * @brief Routes an action/simulation invocation to the registered accessors.
     *
     * @param actionName Action name. An OOBAccessor that handles this action must be registered.
     * @param tlvBuffer Buffer containing TLV data for the action request.
     * @return CHIP_ERROR_NOT_FOUND if no accessor handles this request. OR the status from the accssor.
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
    AccessorRegistry()  = default;
    ~AccessorRegistry() = default;

    IntrusiveList<OOBAccessor, IntrusiveMode::AutoUnlink> mAccessors;
};

} // namespace chip::app
