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

#include <algorithm>
#include <lib/support/CodeUtils.h>

namespace chip::app {

CHIP_ERROR InMemoryOOBAccessorRegistry::Register(std::unique_ptr<OOBAccessor> accessor)
{
    VerifyOrReturnError(accessor != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mAccessors.push_back(std::move(accessor));
    for (auto * callback : mAccessorRegisteredCallbacks)
    {
        callback->OnRegistered(mAccessors.back().get());
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR InMemoryOOBAccessorRegistry::Unregister(OOBAccessor & accessor)
{
    auto it = std::find_if(mAccessors.begin(), mAccessors.end(),
                           [&accessor](const std::unique_ptr<OOBAccessor> & ptr) { return ptr.get() == &accessor; });
    VerifyOrReturnError(it != mAccessors.end(), CHIP_ERROR_NOT_FOUND);
    mAccessors.erase(it);
    return CHIP_NO_ERROR;
}

CHIP_ERROR InMemoryOOBAccessorRegistry::HandleAction(CharSpan action, ByteSpan tlvData)
{
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

void InMemoryOOBAccessorRegistry::RemoveOOBAccessorRegisteredCallback(OOBAccessorRegisteredCallback & callback)
{
    auto it = std::find(mAccessorRegisteredCallbacks.begin(), mAccessorRegisteredCallbacks.end(), callback);
    if (it != mAccessorRegisteredCallbacks.end())
    {
        mAccessorRegisteredCallbacks.erase(it);
    }
}

} // namespace chip::app
