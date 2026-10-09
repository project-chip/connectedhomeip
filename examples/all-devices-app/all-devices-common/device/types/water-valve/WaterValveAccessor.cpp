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

#include <device/types/water-valve/WaterValveAccessor.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBDataSerializer.h>

using namespace chip::app::Clusters;

namespace chip::app {

std::optional<CHIP_ERROR> WaterValveAccessor::HandleAction(CharSpan actionName, ByteSpan tlvBuffer)
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

std::optional<CHIP_ERROR> WaterValveAccessor::SetAttribute(const ConcreteDataAttributePath & path, TLV::TLVReader & reader)
{
    switch (path.mClusterId)
    {
    case ValveConfigurationAndControl::Id: {
        switch (path.mAttributeId)
        {
        case ValveConfigurationAndControl::Attributes::CurrentState::Id: {
            ValveConfigurationAndControl::ValveStateEnum state;
            ReturnErrorOnFailure(DataModel::Decode(reader, state));
            VerifyOrReturnError(state == ValveConfigurationAndControl::ValveStateEnum::kOpen ||
                                    state == ValveConfigurationAndControl::ValveStateEnum::kClosed,
                                CHIP_IM_GLOBAL_STATUS(ConstraintError));

            if (state == ValveConfigurationAndControl::ValveStateEnum::kClosed)
            {
                return mDevice.ValveConfigurationAndControlCluster().CloseValve();
            }

            return mDevice.ValveConfigurationAndControlCluster().OpenValve(
                DataModel::MakeNullable<Percent>(mDevice.LastOpenLevel()), DataModel::NullNullable);
        }
        case ValveConfigurationAndControl::Attributes::CurrentLevel::Id: {
            DataModel::Nullable<Percent> level;
            ReturnErrorOnFailure(DataModel::Decode(reader, level));
            VerifyOrReturnError(!level.IsNull(), CHIP_IM_GLOBAL_STATUS(ConstraintError));
            VerifyOrReturnError(level.Value() <= 100, CHIP_IM_GLOBAL_STATUS(ConstraintError));

            if (level.Value() == 0)
            {
                return mDevice.ValveConfigurationAndControlCluster().CloseValve();
            }

            return mDevice.ValveConfigurationAndControlCluster().OpenValve(level, DataModel::NullNullable);
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
