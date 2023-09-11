/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
