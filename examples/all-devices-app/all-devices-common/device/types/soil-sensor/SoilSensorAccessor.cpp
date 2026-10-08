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

#include <device/types/soil-sensor/SoilSensorAccessor.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBDataSerializer.h>

using namespace chip::app::Clusters;

namespace chip::app {

std::optional<CHIP_ERROR> SoilSensorAccessor::HandleAction(CharSpan actionName, ByteSpan tlvBuffer)
{
    if (!actionName.data_equal(OOBDataSerializer::kSetAttributeAction))
    {
        return std::nullopt;
    }

    auto parseResult = OOBDataSerializer::ParseAttributeRequest(tlvBuffer);
    if (std::holds_alternative<CHIP_ERROR>(parseResult))
    {
        CHIP_ERROR err = std::get<CHIP_ERROR>(parseResult);
        ChipLogError(Support, "Failed to parse attribute request: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    auto & request = std::get<OOBDataSerializer::AttributeRequest>(parseResult);
    if (request.path.mEndpointId != mDevice.GetEndpointId())
    {
        return std::nullopt;
    }

    return SetAttribute(request.path, request.value);
}

std::optional<CHIP_ERROR> SoilSensorAccessor::SetAttribute(const ConcreteDataAttributePath & path, TLV::TLVReader & reader)
{
    switch (path.mClusterId)
    {
    case SoilMeasurement::Id: {
        switch (path.mAttributeId)
        {
        case SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id: {
            DataModel::Nullable<chip::Percent> measuredValue;
            ReturnErrorOnFailure(DataModel::Decode(reader, measuredValue));
            return mDevice.SoilMeasurementCluster().SetSoilMoistureMeasuredValue(measuredValue);
        }
        default:
            return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
        }
    }
    case TemperatureMeasurement::Id: {
        VerifyOrReturnError(mDevice.HasTemperature(), CHIP_IM_GLOBAL_STATUS(UnsupportedCluster));
        switch (path.mAttributeId)
        {
        case TemperatureMeasurement::Attributes::MeasuredValue::Id: {
            DataModel::Nullable<int16_t> tempValue;
            ReturnErrorOnFailure(DataModel::Decode(reader, tempValue));
            return mDevice.TemperatureMeasurementCluster().SetMeasuredValue(tempValue);
        }
        default:
            return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
        }
    }
    default:
        return std::nullopt;
    }
}

} // namespace chip::app
