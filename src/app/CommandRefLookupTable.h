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

#include <app/ConcreteCommandPath.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {

/**
 * @class CommandRefLookupTable
 *
 * @brief Allows looking up CommandRef using the requested ConcreteCommandPath.
 *
 * While there are faster implementations, right now batch commands are capped at a low number due to
 * message size constrains. All commands need to be contained within a single InvokeRequest. In
 * practice this is less than 60 commands.
 *
 */
class CommandRefLookupTable
{
public:
    struct CommandRefTableEntry
    {
        ConcreteCommandPath requestPath = ConcreteCommandPath(0, 0, 0);
        Optional<uint16_t> ref;
    };

    /**
     * IsRequestedPathAndRefUnique() is used to determine if requestPath and ref (if it has value)
     * are unique and do NOT already exists in the table. This is to help validate incoming requests.
     */
    bool IsRequestedPathAndRefUnique(const ConcreteCommandPath & requestPath, const Optional<uint16_t> & ref)
    {
        for (int i = 0; i < mCount; i++)
        {
            if (mTable[i].requestPath == requestPath)
            {
                return false;
            }
            if (mTable[i].ref.HasValue() && mTable[i].ref == ref)
            {
                return false;
            }
        }
        return true;
    }

    const CommandRefTableEntry * Find(const ConcreteCommandPath & requestPath)
    {
        for (int i = 0; i < mCount; i++)
        {
            if (mTable[i].requestPath == requestPath)
            {
                return &mTable[i];
            }
        }
        return nullptr;
    }

    CommandRefTableEntry * GetCommandRefTableEntryIfSingleRequest()
    {
        if (mCount == 1)
        {
            return &mTable[0];
        }
        return nullptr;
    }

    CHIP_ERROR Add(const ConcreteCommandPath & requestPath, const Optional<uint16_t> & ref)
    {
        if (mCount > CHIP_CONFIG_MAX_PATHS_PER_INVOKE)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        mTable[mCount] = CommandRefTableEntry{ requestPath, ref };
        mCount++;
        return CHIP_NO_ERROR;
    }

private:
    uint16_t mCount = 0;
    CommandRefTableEntry mTable[CHIP_CONFIG_MAX_PATHS_PER_INVOKE];
};

} // namespace app
} // namespace chip
