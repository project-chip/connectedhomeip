/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/util/basic-types.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {
struct DataVersionFilter
{
    DataVersionFilter(EndpointId aEndpointId, ClusterId aClusterId, DataVersion aDataVersion) :
        mClusterId(aClusterId), mDataVersion(aDataVersion), mEndpointId(aEndpointId)
    {}

    DataVersionFilter() {}

    bool IsValidDataVersionFilter() const
    {
        return (mEndpointId != kInvalidEndpointId) && (mClusterId != kInvalidClusterId) && (mDataVersion.HasValue());
    }

    bool operator==(const DataVersionFilter & aOther) const
    {
        return mEndpointId == aOther.mEndpointId && mClusterId == aOther.mClusterId && mDataVersion == aOther.mDataVersion;
    }

    ClusterId mClusterId = kInvalidClusterId;    // uint32
    Optional<DataVersion> mDataVersion;          // uint32
    EndpointId mEndpointId = kInvalidEndpointId; // uint16
};
} // namespace app
} // namespace chip
