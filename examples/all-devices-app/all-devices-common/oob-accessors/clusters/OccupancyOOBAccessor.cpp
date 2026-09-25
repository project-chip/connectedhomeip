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

#include <oob-accessors/clusters/OccupancyOOBAccessor.h>

#include <app/data-model/Decode.h>
#include <clusters/OccupancySensing/AttributeIds.h>
#include <clusters/OccupancySensing/ClusterId.h>
#include <clusters/OccupancySensing/Enums.h>
#include <lib/core/TLV.h>
#include <lib/support/BitMask.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBDataSerializer.h>

namespace chip::app {

std::optional<CHIP_ERROR> OccupancyOOBAccessor::HandleAction(CharSpan action, ByteSpan tlvData)
{
    if (action.data_equal(OOBDataSerializer::kSetAttributeAction))
    {
        return HandleSetAttribute(tlvData);
    }
    return std::nullopt;
}

std::optional<CHIP_ERROR> OccupancyOOBAccessor::HandleSetAttribute(ByteSpan tlvData) const
{
    auto parseResult = OOBDataSerializer::ParseAttributeRequest(tlvData);
    if (std::holds_alternative<CHIP_ERROR>(parseResult))
    {
        CHIP_ERROR err = std::get<CHIP_ERROR>(parseResult);
        ChipLogError(Support, "Failed to parse OOB attribute request: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    auto & request = std::get<OOBDataSerializer::AttributeRequest>(parseResult);
    VerifyOrReturnValue(request.path.mEndpointId == mEndpointId, std::nullopt);
    VerifyOrReturnValue(request.path.mClusterId == Clusters::OccupancySensing::Id, std::nullopt);

    switch (request.path.mAttributeId)
    {
    case Clusters::OccupancySensing::Attributes::Occupancy::Id: {
        // Occupancy is read-only per spec; only the cluster API can set it.
        BitMask<Clusters::OccupancySensing::OccupancyBitmap> occupancy;
        ReturnErrorOnFailure(DataModel::Decode(request.value, occupancy));
        mCluster.SetOccupancy(occupancy.Has(Clusters::OccupancySensing::OccupancyBitmap::kOccupied));
        return CHIP_NO_ERROR;
    }
    case Clusters::OccupancySensing::Attributes::HoldTime::Id: {
        uint16_t holdTime = 0;
        ReturnErrorOnFailure(DataModel::Decode(request.value, holdTime));
        return mCluster.SetHoldTime(holdTime).GetUnderlyingError();
    }
    default:
        return std::nullopt;
    }
}

} // namespace chip::app
