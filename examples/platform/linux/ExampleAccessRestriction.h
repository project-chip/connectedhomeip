/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

/*
 * AccessRestriction implementation for Linux examples.
 */

#pragma once

#include <access/AccessRestriction.h>

namespace chip {
namespace Access {

class ExampleAccessRestriction : public AccessRestriction
{
public:
    class ExampleAccessRestrictionEntryListener : public AccessRestriction::EntryListener
    {
        void OnEntryChanged(FabricIndex fabricIndex, size_t index, SharedPtr<AccessRestriction::Entry> entry,
                            ChangeType changeType) override
        {
            ChipLogProgress(NotSpecified, "AccessRestriction Entry changed: fabricIndex %d, index %ld, changeType %d", fabricIndex,
                            index, static_cast<int>(changeType));
        }

        void OnFabricRestrictionReviewUpdate(FabricIndex fabricIndex, uint64_t token, const char * instruction,
                                             const char * redirectUrl) override
        {}
    };

    ExampleAccessRestriction() : AccessRestriction() { AccessRestriction::AddListener(listener); }

    ~ExampleAccessRestriction() { AccessRestriction::RemoveListener(listener); }

protected:
    CHIP_ERROR DoRequestFabricRestrictionReview(FabricIndex fabricIndex, uint64_t token, const std::vector<Entry> * arl)
    {
        // this example simply removes all restrictions
        while (Access::GetAccessControl().GetAccessRestriction()->DeleteEntry(0, fabricIndex) == CHIP_NO_ERROR)
            ;

        return CHIP_NO_ERROR;
    }

private:
    ExampleAccessRestrictionEntryListener listener;
};

} // namespace Access
} // namespace chip