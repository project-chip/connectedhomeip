/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <stddef.h>

#include <app/ConcreteCommandPath.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>

#include <optional>

namespace chip {
namespace app {

struct CommandPathRegistryEntry
{
    ConcreteCommandPath requestPath = ConcreteCommandPath(0, 0, 0);
    std::optional<uint16_t> ref;
};

class CommandPathRegistry
{
public:
    virtual ~CommandPathRegistry() = default;

    virtual std::optional<CommandPathRegistryEntry> Find(const ConcreteCommandPath & requestPath) const  = 0;
    virtual std::optional<CommandPathRegistryEntry> GetFirstEntry() const                                = 0;
    virtual CHIP_ERROR Add(const ConcreteCommandPath & requestPath, const std::optional<uint16_t> & ref) = 0;
    virtual size_t Count() const                                                                         = 0;
    virtual size_t MaxSize() const                                                                       = 0;
};

/**
 * @class BasicCommandPathRegistry
 *
 * @brief Allows looking up CommandRef using the requested ConcreteCommandPath.
 *
 * While there are faster implementations, right now batch commands are capped at a low number due to
 * message size constraints. All commands need to be contained within a single InvokeRequest. In
 * practice this is usually less than 60 commands (but could be much more with TCP transports or
 * newer transports).
 */
template <size_t N>
class BasicCommandPathRegistry : public CommandPathRegistry
{
public:
    std::optional<CommandPathRegistryEntry> Find(const ConcreteCommandPath & requestPath) const override
    {
        for (size_t i = 0; i < mCount; i++)
        {
            if (mTable[i].requestPath == requestPath)
            {
                return std::make_optional(mTable[i]);
            }
        }
        return std::nullopt;
    }

    std::optional<CommandPathRegistryEntry> GetFirstEntry() const override
    {
        if (mCount > 0)
        {
            return std::make_optional(mTable[0]);
        }
        return std::nullopt;
    }

    CHIP_ERROR Add(const ConcreteCommandPath & requestPath, const std::optional<uint16_t> & ref) override
    {
        if (mCount >= N)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        for (size_t i = 0; i < mCount; i++)
        {
            if (mTable[i].requestPath == requestPath)
            {
                return CHIP_ERROR_DUPLICATE_KEY_ID;
            }
            // No need to check if either has value. This is because if there is more than
            // 1 entry in the table expectation is to have all entirely unique ref values
            // so duplicate optional would mean we would want to error out.
            if (mTable[i].ref == ref)
            {
                return CHIP_ERROR_DUPLICATE_KEY_ID;
            }
        }

        mTable[mCount] = CommandPathRegistryEntry{ requestPath, ref };
        mCount++;
        return CHIP_NO_ERROR;
    }

    virtual size_t Count() const override { return mCount; }
    virtual size_t MaxSize() const override { return N; }

private:
    size_t mCount = 0;
    CommandPathRegistryEntry mTable[N];
};

} // namespace app
} // namespace chip
