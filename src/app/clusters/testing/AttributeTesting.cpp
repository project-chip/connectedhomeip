/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "AttributeTesting.h"

#include <app/data-model-provider/MetadataTypes.h>
#include <map>

namespace chip {
namespace Testing {

bool EqualAttributeSets(Span<const app::DataModel::AttributeEntry> a, Span<const app::DataModel::AttributeEntry> b)
{

    std::map<AttributeId, const app::DataModel::AttributeEntry *> entriesA;
    std::map<AttributeId, const app::DataModel::AttributeEntry *> entriesB;

    for (const app::DataModel::AttributeEntry & entry : a)
    {
        if (!entriesA.emplace(entry.attributeId, &entry).second)
        {
            ChipLogError(Test, "Duplicate attribute ID in span A: 0x%08X", static_cast<int>(entry.attributeId));
            return false;
        }
    }

    for (const app::DataModel::AttributeEntry & entry : b)
    {
        if (!entriesB.emplace(entry.attributeId, &entry).second)
        {
            ChipLogError(Test, "Duplicate attribute ID in span B: 0x%08X", static_cast<int>(entry.attributeId));
            return false;
        }
    }

    if (entriesA.size() != entriesB.size())
    {
        ChipLogError(Test, "Sets of different sizes.");

        for (const auto it : entriesA)
        {
            if (entriesB.find(it.first) == entriesB.end())
            {
                ChipLogError(Test, "Attribute 0x%08X missing in B", static_cast<int>(it.first));
            }
        }

        for (const auto it : entriesB)
        {
            if (entriesA.find(it.first) == entriesA.end())
            {
                ChipLogError(Test, "Attribute 0x%08X missing in A", static_cast<int>(it.first));
            }
        }

        return false;
    }

    for (const auto it : entriesA)
    {
        const auto other = entriesB.find(it.first);
        if (other == entriesB.end())
        {

            ChipLogError(Test, "Missing entry: 0x%08X", static_cast<int>(it.first));
            return false;
        }

        if (*it.second != *other->second)
        {

            ChipLogError(Test, "Different content (different flags?): 0x%08X", static_cast<int>(it.first));
            return false;
        }
    }
    // set sizes are the same and all entriesA have a corresponding entriesB, so sets should match
    return true;
}

bool EqualAcceptedCommandSets(Span<const app::DataModel::AcceptedCommandEntry> a,
                              Span<const app::DataModel::AcceptedCommandEntry> b)
{

    std::map<CommandId, const app::DataModel::AcceptedCommandEntry *> entriesA;
    std::map<CommandId, const app::DataModel::AcceptedCommandEntry *> entriesB;

    for (const app::DataModel::AcceptedCommandEntry & entry : a)
    {
        if (!entriesA.emplace(entry.commandId, &entry).second)
        {
            ChipLogError(Test, "Duplicate command ID in span A: 0x%08X", static_cast<int>(entry.commandId));
            return false;
        }
    }

    for (const app::DataModel::AcceptedCommandEntry & entry : b)
    {
        if (!entriesB.emplace(entry.commandId, &entry).second)
        {
            ChipLogError(Test, "Duplicate command ID in span B: 0x%08X", static_cast<int>(entry.commandId));
            return false;
        }
    }

    if (entriesA.size() != entriesB.size())
    {
        ChipLogError(Test, "Sets of different sizes.");

        for (const auto it : entriesA)
        {
            if (entriesB.find(it.first) == entriesB.end())
            {
                ChipLogError(Test, "Command 0x%08X missing in B", static_cast<int>(it.first));
            }
        }

        for (const auto it : entriesB)
        {
            if (entriesA.find(it.first) == entriesA.end())
            {
                ChipLogError(Test, "Command 0x%08X missing in A", static_cast<int>(it.first));
            }
        }

        return false;
    }

    for (const auto it : entriesA)
    {
        const auto other = entriesB.find(it.first);
        if (other == entriesB.end())
        {

            ChipLogError(Test, "Missing entry: 0x%08X", static_cast<int>(it.first));
            return false;
        }

        if (*it.second != *other->second)
        {

            ChipLogError(Test, "Different content (different flags?): 0x%08X", static_cast<int>(it.first));
            return false;
        }
    }
    // set sizes are the same and all entriesA have a corresponding entriesB, so sets should match
    return true;
}

} // namespace Testing
} // namespace chip
