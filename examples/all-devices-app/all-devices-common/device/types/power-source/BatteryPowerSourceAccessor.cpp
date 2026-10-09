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

#include <device/types/power-source/BatteryPowerSourceAccessor.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBDataSerializer.h>

using namespace chip::app::Clusters;

namespace chip::app {

std::optional<CHIP_ERROR> BatteryPowerSourceAccessor::HandleAction(CharSpan actionName, ByteSpan tlvBuffer)
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
    if (request.path.mEndpointId != mEndpointId)
    {
        return std::nullopt;
    }

    switch (request.path.mClusterId)
    {
    case PowerSource::Id: {
        switch (request.path.mAttributeId)
        {
        case PowerSource::Attributes::BatPercentRemaining::Id: {
            DataModel::Nullable<uint8_t> batteryValue;
            ReturnErrorOnFailure(DataModel::Decode(request.value, batteryValue));
            return mCluster.SetBatPercentRemaining(batteryValue);
        }
        case PowerSource::Attributes::BatVoltage::Id: {
            DataModel::Nullable<uint32_t> voltageValue;
            ReturnErrorOnFailure(DataModel::Decode(request.value, voltageValue));
            mCluster.SetBatVoltage(voltageValue);
            return CHIP_NO_ERROR;
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
