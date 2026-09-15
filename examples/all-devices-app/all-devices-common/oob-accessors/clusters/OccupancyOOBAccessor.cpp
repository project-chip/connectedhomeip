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

#include <access/SubjectDescriptor.h>
#include <app/AttributeValueDecoder.h>
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
    if (action.data_equal("SetAttribute"_span))
    {
        return HandleSetAttribute(tlvData);
    }
    if (action.data_equal("SetOccupancy"_span))
    {
        return HandleSetOccupancy(tlvData);
    }
    if (action.data_equal("SetHoldTime"_span))
    {
        return HandleSetHoldTime(tlvData);
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
        Access::SubjectDescriptor subjectDescriptor{ .authMode = Access::AuthMode::kInternalDeviceAccess };
        AttributeValueDecoder decoder(request.value, subjectDescriptor);
        BitMask<Clusters::OccupancySensing::OccupancyBitmap> occupancy;
        ReturnErrorOnFailure(decoder.Decode(occupancy));
        mCluster.SetOccupancy(occupancy.Has(Clusters::OccupancySensing::OccupancyBitmap::kOccupied));
        return CHIP_NO_ERROR;
    }
    default:
        // HoldTime and the PIR/ultrasonic delays are writable per spec: decline so
        // the caller falls through to the regular data-model write.
        return std::nullopt;
    }
}

std::optional<CHIP_ERROR> OccupancyOOBAccessor::HandleSetOccupancy(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    bool occupancy        = false;
    bool hasEndpointId    = false;
    bool hasOccupancy     = false;

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2: {
            TLV::TLVType type = reader.GetType();
            if (type == TLV::kTLVType_Boolean)
            {
                ReturnErrorOnFailure(reader.Get(occupancy));
            }
            else
            {
                uint8_t occVal = 0;
                ReturnErrorOnFailure(reader.Get(occVal));
                occupancy = (occVal != 0);
            }
            hasOccupancy = true;
            break;
        }
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId && hasOccupancy, CHIP_ERROR_INVALID_ARGUMENT);

    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    mCluster.SetOccupancy(occupancy);
    return CHIP_NO_ERROR;
}

std::optional<CHIP_ERROR> OccupancyOOBAccessor::HandleSetHoldTime(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    uint16_t holdTime     = 0;
    bool hasEndpointId    = false;
    bool hasHoldTime      = false;

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2:
            ReturnErrorOnFailure(reader.Get(holdTime));
            hasHoldTime = true;
            break;
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId && hasHoldTime, CHIP_ERROR_INVALID_ARGUMENT);

    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    auto status = mCluster.SetHoldTime(holdTime);
    if (!status.IsSuccess())
    {
        return status.GetUnderlyingError();
    }
    return CHIP_NO_ERROR;
}

} // namespace chip::app
