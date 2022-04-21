/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Fan Control Server Cluster
 ***************************************************************************/

#include <math.h>

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FanControl;
using namespace chip::app::Clusters::FanControl::Attributes;

// =============================================================================
// Pre-change callbacks for cluster attributes
// =============================================================================

using Status = Protocols::InteractionModel::Status;

Protocols::InteractionModel::Status
MatterFanControlClusterServerPreAttributeChangedCallback(const ConcreteAttributePath & attributePath,
                                                         EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    Protocols::InteractionModel::Status res;

    switch (attributePath.mAttributeId)
    {
    case SpeedSetting::Id: {
        uint8_t speedMax;
        EmberAfStatus status = SpeedMax::Get(attributePath.mEndpointId, &speedMax);
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == status, Status::InvalidValue);

        if (*value <= speedMax)
        {
            res = Status::Success;
        }
        else
        {
            res = Status::InvalidValue;
        }
        break;
    }
    default:
        res = Status::Success;
        break;
    }

    return res;
}

void MatterFanControlClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    switch (attributePath.mAttributeId)
    {
    case FanMode::Id: {
        FanModeType mode;
        EmberAfStatus status = FanMode::Get(attributePath.mEndpointId, &mode);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status);

        // Setting the FanMode value to Off SHALL set the values of PercentSetting, PercentCurrent,
        // SpeedSetting, SpeedCurrent attributes to 0 (zero):.
        if (mode == FanModeType::kOff)
        {
            status = PercentSetting::Set(attributePath.mEndpointId, 0);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to write PercentSetting with error: 0x%02x", status));

            status = PercentCurrent::Set(attributePath.mEndpointId, 0);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to write PercentCurrent with error: 0x%02x", status));

            status = SpeedSetting::Set(attributePath.mEndpointId, 0);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to write SpeedSetting with error: 0x%02x", status));

            status = SpeedCurrent::Set(attributePath.mEndpointId, 0);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to write SpeedCurrent with error: 0x%02x", status));
        }
        break;
    }
    case PercentSetting::Id: {
        uint8_t percentSetting;
        EmberAfStatus status = PercentSetting::Get(attributePath.mEndpointId, &percentSetting);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status);

        // If PercentSetting is set to 0, the server SHALL set the FanMode attribute value to Off.
        if (percentSetting == 0)
        {
            FanModeType currentFanMode;
            status = FanMode::Get(attributePath.mEndpointId, &currentFanMode);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to get FanMode with error: 0x%02x", status));

            if (currentFanMode != FanModeType::kOff)
            {
                status = FanMode::Set(attributePath.mEndpointId, FanModeType::kOff);
                VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                               ChipLogError(Zcl, "Failed to write FanMode with error: 0x%02x", status));
            }
        }

        // Adjust SpeedSetting from a percent value change for PercentSetting
        // speed = ceil( SpeedMax * (percent * 0.01) )
        uint8_t speedMax;
        status = SpeedMax::Get(attributePath.mEndpointId, &speedMax);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Failed to get SpeedMax with error: 0x%02x", status));

        uint8_t currentSpeedSetting;
        status = SpeedSetting::Get(attributePath.mEndpointId, &currentSpeedSetting);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(Zcl, "Failed to get SpeedSetting with error: 0x%02x", status));

        float precent        = percentSetting;
        uint8_t speedSetting = static_cast<uint8_t>(ceil(speedMax * (precent * 0.01)));

        if (speedSetting != currentSpeedSetting)
        {
            status = SpeedSetting::Set(attributePath.mEndpointId, speedSetting);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to set SpeedSetting with error: 0x%02x", status));
        }
        break;
    }
    case SpeedSetting::Id: {
        uint8_t speedSetting;
        EmberAfStatus status = SpeedSetting::Get(attributePath.mEndpointId, &speedSetting);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status);

        // If SpeedSetting is set to 0, the server SHALL set the FanMode attribute value to Off.
        if (speedSetting == 0)
        {
            FanModeType currentFanMode;
            status = FanMode::Get(attributePath.mEndpointId, &currentFanMode);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to get FanMode with error: 0x%02x", status));

            if (currentFanMode != FanModeType::kOff)
            {
                status = FanMode::Set(attributePath.mEndpointId, FanModeType::kOff);
                VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                               ChipLogError(Zcl, "Failed to write FanMode with error: 0x%02x", status));
            }
        }

        // Adjust PercentSetting from a speed value change for SpeedSetting
        // percent = floor( speed/SpeedMax * 100 )
        uint8_t speedMax;
        status = SpeedMax::Get(attributePath.mEndpointId, &speedMax);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Failed to get SpeedMax with error: 0x%02x", status));

        uint8_t currentPercentSetting;
        status = PercentSetting::Get(attributePath.mEndpointId, &currentPercentSetting);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(Zcl, "Failed to get PercentSetting with error: 0x%02x", status));

        float speed            = speedSetting;
        uint8_t percentSetting = static_cast<uint8_t>(speed / speedMax * 100);

        if (percentSetting != currentPercentSetting)
        {
            status = PercentSetting::Set(attributePath.mEndpointId, percentSetting);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to set PercentSetting with error: 0x%02x", status));
        }
        break;
    }
    default:
        break;
    }
}
