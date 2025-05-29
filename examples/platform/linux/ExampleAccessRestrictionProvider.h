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

#include <access/AccessRestrictionProvider.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/EventLogging.h>

namespace chip {
namespace Access {

class ExampleAccessRestrictionProvider : public AccessRestrictionProvider
{
public:
    ExampleAccessRestrictionProvider() : AccessRestrictionProvider() {}

    ~ExampleAccessRestrictionProvider() {}

protected:
    CHIP_ERROR DoRequestFabricRestrictionReview(const FabricIndex fabricIndex, uint64_t token, const std::vector<Entry> & arl)
    {
        // this example simply removes all restrictions when a review command is received
        Access::GetAccessControl().GetAccessRestrictionProvider()->SetEntries(fabricIndex, std::vector<Entry>{});

        chip::app::Clusters::AccessControl::Events::FabricRestrictionReviewUpdate::Type event{ .token       = token,
                                                                                               .fabricIndex = fabricIndex };
        EventNumber eventNumber;
        ReturnErrorOnFailure(chip::app::LogEvent(event, kRootEndpointId, eventNumber));

        return CHIP_NO_ERROR;
    }
};

} // namespace Access
} // namespace chip
