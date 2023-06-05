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

#include <assert.h>
#include <math.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/attribute-storage.h>
#include <app/util/error-mapping.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FanControl;
using namespace chip::app::Clusters::FanControl::Attributes;

namespace {

// Indicates if the write operation is from the cluster server itself
bool gWriteFromClusterLogic = false;

EmberAfStatus SetFanModeToOff(EndpointId endpointId)
{
    FanModeType currentFanMode;
    EmberAfStatus status = FanMode::Get(endpointId, &currentFanMode);
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == status, status);

    if (currentFanMode != FanModeType::kOff)
    {
        status = FanMode::Set(endpointId, FanModeType::kOff);
    }

    return status;
}

bool HasFeature(EndpointId endpoint, Feature feature)
{
    bool success;
    uint32_t featureMap;
    success = (Attributes::FeatureMap::Get(endpoint, &featureMap) == EMBER_ZCL_STATUS_SUCCESS);

    return success ? ((featureMap & to_underlying(feature)) != 0) : false;
}

inline bool SupportsMultiSpeed(chip::EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kMultiSpeed);
}

inline bool SupportsAuto(chip::EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kAuto);
}

inline bool SupportsRocking(chip::EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kRocking);
}

inline bool SupportsWind(chip::EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kWind);
}

inline bool SupportsStep(chip::EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kStep);
}

inline bool SupportsAirflowDirection(chip::EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kAirflowDirection);
}

} // anonymous namespace

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
    case FanMode::Id: {
        if (*value == to_underlying(FanModeType::kOn))
        {
            FanMode::Set(attributePath.mEndpointId, FanModeType::kHigh);
            res = Status::WriteIgnored;
        }
        else if (*value == to_underlying(FanModeType::kSmart))
        {
            FanModeSequenceType fanModeSequence;
            EmberAfStatus status = FanModeSequence::Get(attributePath.mEndpointId, &fanModeSequence);
            VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == status, Status::Failure);

            if (SupportsAuto(attributePath.mEndpointId) &&
                ((fanModeSequence == FanModeSequenceType::kOffLowHighAuto) ||
                 (fanModeSequence == FanModeSequenceType::kOffLowMedHighAuto)))
            {
                FanMode::Set(attributePath.mEndpointId, FanModeType::kAuto);
            }
            else
            {
                FanMode::Set(attributePath.mEndpointId, FanModeType::kHigh);
            }
            res = Status::WriteIgnored;
        }
        else
        {
            res = Status::Success;
        }
        break;
    }
    case SpeedSetting::Id: {
        if (SupportsMultiSpeed(attributePath.mEndpointId))
        {
            // Check if the SpeedSetting is null.
            if (NumericAttributeTraits<uint8_t>::IsNullValue(*value))
            {

                if (gWriteFromClusterLogic)
                {
                    res                    = Status::Success;
                    gWriteFromClusterLogic = false;
                }
                else
                {
                    res = Status::WriteIgnored;
                }
            }
            else
            {
                uint8_t speedMax;
                EmberAfStatus status = SpeedMax::Get(attributePath.mEndpointId, &speedMax);
                VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == status, Status::ConstraintError);

                if (*value <= speedMax)
                {
                    res = Status::Success;
                }
                else
                {
                    res = Status::ConstraintError;
                }
            }
        }
        else
        {
            res = Status::UnsupportedAttribute;
        }
        break;
    }
    case PercentSetting::Id: {
        // Check if the PercentSetting is null.
        if (NumericAttributeTraits<uint8_t>::IsNullValue(*value))
        {
            if (gWriteFromClusterLogic)
            {
                res                    = Status::Success;
                gWriteFromClusterLogic = false;
            }
            else
            {
                res = Status::WriteIgnored;
            }
        }
        else
        {
            res = Status::Success;
        }
        break;
    }
    case RockSetting::Id: {
        if (SupportsRocking(attributePath.mEndpointId))
        {
            uint8_t rockSupport;
            EmberAfStatus status = RockSupport::Get(attributePath.mEndpointId, &rockSupport);
            VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == status, Status::ConstraintError);
            if ((*value & rockSupport) == *value)
            {
                res = Status::Success;
            }
            else
            {
                res = Status::ConstraintError;
            }
        }
        else
        {
            res = Status::UnsupportedAttribute;
        }
        break;
    }
    case WindSupport::Id: {
        if (SupportsWind(attributePath.mEndpointId))
        {
            uint8_t windSupport;
            EmberAfStatus status = WindSupport::Get(attributePath.mEndpointId, &windSupport);
            VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == status, Status::ConstraintError);
            if ((*value & windSupport) == *value)
            {
                res = Status::Success;
            }
            else
            {
                res = Status::ConstraintError;
            }
        }
        else
        {
            res = Status::UnsupportedAttribute;
        }
        break;
    }
    case AirflowDirection::Id: {
        if (SupportsAirflowDirection(attributePath.mEndpointId))
        {
            res = Status::Success;
        }
        else
        {
            res = Status::UnsupportedAttribute;
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
        // SpeedSetting, SpeedCurrent attributes to 0 (zero).
        if (mode == FanModeType::kOff)
        {
            status = PercentSetting::Set(attributePath.mEndpointId, 0);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to write PercentSetting with error: 0x%02x", status));

            status = PercentCurrent::Set(attributePath.mEndpointId, 0);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to write PercentCurrent with error: 0x%02x", status));

            if (SupportsMultiSpeed(attributePath.mEndpointId))
            {
                status = SpeedSetting::Set(attributePath.mEndpointId, 0);
                VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                               ChipLogError(Zcl, "Failed to write SpeedSetting with error: 0x%02x", status));

                status = SpeedCurrent::Set(attributePath.mEndpointId, 0);
                VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                               ChipLogError(Zcl, "Failed to write SpeedCurrent with error: 0x%02x", status));
            }
        }

        // Setting the attribute value to Auto SHALL set the values of PercentSetting, SpeedSetting (if present)
        // to null.
        if (mode == FanModeType::kAuto)
        {
            gWriteFromClusterLogic = true;
            status                 = PercentSetting::SetNull(attributePath.mEndpointId);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to write PercentSetting with error: 0x%02x", status));

            if (SupportsMultiSpeed(attributePath.mEndpointId))
            {
                gWriteFromClusterLogic = true;
                status                 = SpeedSetting::SetNull(attributePath.mEndpointId);
                VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                               ChipLogError(Zcl, "Failed to write SpeedSetting with error: 0x%02x", status));
            }
        }
        break;
    }
    case PercentSetting::Id: {
        DataModel::Nullable<uint8_t> percentSetting;
        EmberAfStatus status = PercentSetting::Get(attributePath.mEndpointId, percentSetting);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status && !percentSetting.IsNull());

        // If PercentSetting is set to 0, the server SHALL set the FanMode attribute value to Off.
        if (percentSetting.Value() == 0)
        {
            status = SetFanModeToOff(attributePath.mEndpointId);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to set FanMode to off with error: 0x%02x", status));
        }

        if (SupportsMultiSpeed(attributePath.mEndpointId))
        {
            // Adjust SpeedSetting from a percent value change for PercentSetting
            // speed = ceil( SpeedMax * (percent * 0.01) )
            uint8_t speedMax;
            status = SpeedMax::Get(attributePath.mEndpointId, &speedMax);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to get SpeedMax with error: 0x%02x", status));

            DataModel::Nullable<uint8_t> currentSpeedSetting;
            status = SpeedSetting::Get(attributePath.mEndpointId, currentSpeedSetting);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to get SpeedSetting with error: 0x%02x", status));

            float percent        = percentSetting.Value();
            uint8_t speedSetting = static_cast<uint8_t>(ceil(speedMax * (percent * 0.01)));

            if (currentSpeedSetting.IsNull() || speedSetting != currentSpeedSetting.Value())
            {
                status = SpeedSetting::Set(attributePath.mEndpointId, speedSetting);
                VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                               ChipLogError(Zcl, "Failed to set SpeedSetting with error: 0x%02x", status));
            }
        }
        break;
    }
    case SpeedSetting::Id: {
        if (SupportsMultiSpeed(attributePath.mEndpointId))
        {
            DataModel::Nullable<uint8_t> speedSetting;
            EmberAfStatus status = SpeedSetting::Get(attributePath.mEndpointId, speedSetting);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status && !speedSetting.IsNull());

            // If SpeedSetting is set to 0, the server SHALL set the FanMode attribute value to Off.
            if (speedSetting.Value() == 0)
            {
                status = SetFanModeToOff(attributePath.mEndpointId);
                VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                               ChipLogError(Zcl, "Failed to set FanMode to off with error: 0x%02x", status));
            }

            // Adjust PercentSetting from a speed value change for SpeedSetting
            // percent = floor( speed/SpeedMax * 100 )
            uint8_t speedMax;
            status = SpeedMax::Get(attributePath.mEndpointId, &speedMax);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to get SpeedMax with error: 0x%02x", status));

            DataModel::Nullable<uint8_t> currentPercentSetting;
            status = PercentSetting::Get(attributePath.mEndpointId, currentPercentSetting);
            VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                           ChipLogError(Zcl, "Failed to get PercentSetting with error: 0x%02x", status));

            float speed            = speedSetting.Value();
            uint8_t percentSetting = static_cast<uint8_t>(speed / speedMax * 100);

            if (currentPercentSetting.IsNull() || percentSetting != currentPercentSetting.Value())
            {
                status = PercentSetting::Set(attributePath.mEndpointId, percentSetting);
                VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                               ChipLogError(Zcl, "Failed to set PercentSetting with error: 0x%02x", status));
            }
        }
        break;
    }
    default:
        break;
    }
}

bool emberAfFanControlClusterStepCallback(chip::app::CommandHandler * commandObj,
                                          const chip::app::ConcreteCommandPath & commandPath,
                                          const chip::app::Clusters::FanControl::Commands::Step::DecodableType & commandData)
{
    /*
     * TODO: Clarification needed in spec issue #6496 - if this is tied to the SpeedSetting attribute, then
     * the attribute can be updated here, if it is supposed to be implementation specific, then the command
     * will have to be handed off to an application specific callback which will require some sort of delegate.
     */

    Protocols::InteractionModel::Status status = Status::Success;

    if (!SupportsStep(commandPath.mEndpointId))
    {
        status = Status::UnsupportedCommand;
    }
    commandObj->AddStatus(commandPath, status);
    return true;
}
