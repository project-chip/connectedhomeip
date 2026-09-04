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

#include <memory>
#include <vector>

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <oob-accessors/OOBAccessor.h>

namespace chip::app {

class InMemoryOOBAccessorRegistry
{
public:
    static InMemoryOOBAccessorRegistry & Instance()
    {
        static InMemoryOOBAccessorRegistry instance;
        return instance;
    }

    InMemoryOOBAccessorRegistry()          = default;
    ~InMemoryOOBAccessorRegistry()         = default;
    InMemoryOOBAccessorRegistry(const InMemoryOOBAccessorRegistry &) = delete;
    InMemoryOOBAccessorRegistry & operator=(const InMemoryOOBAccessorRegistry &) = delete;

    /**
     * @brief Registers an OOB accessor instance.
     * @param accessor The accessor instance to register.
     */
    CHIP_ERROR Register(std::unique_ptr<OOBAccessor> accessor);

    /**
     * @brief Dispatches an action to registered accessors in order.
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_NOT_FOUND if unhandled, or specific error on execution failure.
     */
    CHIP_ERROR HandleAction(CharSpan action, ByteSpan tlvData);

    /**
     * @brief Clears all registered accessors during device teardown.
     */
    void Clear() { mAccessors.clear(); }

    /**
     * @brief Returns the number of registered accessors.
     */
    size_t Size() const { return mAccessors.size(); }

private:
    std::vector<std::unique_ptr<OOBAccessor>> mAccessors;
};

} // namespace chip::app
