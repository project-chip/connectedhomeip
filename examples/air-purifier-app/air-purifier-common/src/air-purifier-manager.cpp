/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <air-purifier-manager.h>
#include <app/util/error-mapping.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using Protocols::InteractionModel::Status;

void AirPurifierManager::Init()
{
    FanControl::SetDefaultDelegate(mEndpointId, this);

    activatedCarbonFilterInstance.Init();
    hepaFilterInstance.Init();
    mAirQualitySensorManager.Init();
    mTemperatureSensorManager.Init();
    mHumiditySensorManager.Init();

    DataModel::Nullable<Percent> percentSetting;
    EmberAfStatus status = FanControl::Attributes::PercentSetting::Get(mEndpointId, percentSetting);
    if (EMBER_ZCL_STATUS_SUCCESS == status)
    {
        if (percentSetting.IsNull())
        {
            PercentSettingWriteCallback(0);
        }
        else
        {
            PercentSettingWriteCallback(percentSetting.Value());
        }
    }

    DataModel::Nullable<uint8_t> speedSetting;
    status = FanControl::Attributes::SpeedSetting::Get(mEndpointId, speedSetting);
    if (EMBER_ZCL_STATUS_SUCCESS == status)
    {
        if (speedSetting.IsNull())
        {
            SpeedSettingWriteCallback(0);
        }
        else
        {
            SpeedSettingWriteCallback(speedSetting.Value());
        }
    }

    // Set up some sane initial values for temperature and humidity - note these are fixed values for testing purposes only
    mTemperatureSensorManager.OnTemperatureChangeHandler(2000);
    mHumiditySensorManager.OnHumidityChangeHandler(5000);
}

void AirPurifierManager::PostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId,
                                                     uint8_t type, uint16_t size, uint8_t * value)
{
    switch (clusterId)
    {
    case FanControl::Id: {
        HandleFanControlAttributeChange(attributeId, type, size, value);
        break;
    }

    default:
        break;
    }
}

Status AirPurifierManager::HandleStep(FanControl::StepDirectionEnum aDirection, bool aWrap, bool aLowestOff)
{
    ChipLogProgress(NotSpecified, "AirPurifierManager::HandleStep aDirection %d, aWrap %d, aLowestOff %d",
                    to_underlying(aDirection), aWrap, aLowestOff);

    VerifyOrReturnError(aDirection != FanControl::StepDirectionEnum::kUnknownEnumValue, Status::InvalidCommand);

    uint8_t speedMax;
    FanControl::Attributes::SpeedMax::Get(mEndpointId, &speedMax);

    DataModel::Nullable<uint8_t> speedSetting;
    FanControl::Attributes::SpeedSetting::Get(mEndpointId, speedSetting);

    uint8_t newSpeedSetting = speedSetting.IsNull() ? 0 : speedSetting.Value();

    if (aDirection == FanControl::StepDirectionEnum::kIncrease)
    {
        if (speedSetting.IsNull())
        {
            newSpeedSetting = 1;
        }
        else if (speedSetting.Value() < speedMax)
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() + 1);
        }
        else if (speedSetting.Value() == speedMax)
        {
            if (aWrap)
            {
                newSpeedSetting = aLowestOff ? 0 : 1;
            }
        }
    }
    else if (aDirection == FanControl::StepDirectionEnum::kDecrease)
    {
        if (speedSetting.IsNull())
        {
            newSpeedSetting = aLowestOff ? 0 : 1;
        }
        else if ((speedSetting.Value() > 1) && (speedSetting.Value() <= speedMax))
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() - 1);
        }
        else if (speedSetting.Value() == 1)
        {
            if (aLowestOff)
            {
                newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() - 1);
            }
            else if (aWrap)
            {
                newSpeedSetting = speedMax;
            }
        }
        else if (speedSetting.Value() == 0)
        {
            if (aWrap)
            {
                newSpeedSetting = speedMax;
            }
            else if (!aLowestOff)
            {
                newSpeedSetting = 1;
            }
        }
    }

    return ToInteractionModelStatus(FanControl::Attributes::SpeedSetting::Set(mEndpointId, newSpeedSetting));
}

void AirPurifierManager::HandleFanControlAttributeChange(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value)
{
    switch (attributeId)
    {
    case FanControl::Attributes::PercentSetting::Id: {
        DataModel::Nullable<Percent> percentSetting = static_cast<DataModel::Nullable<uint8_t>>(*value);
        if (percentSetting.IsNull())
        {
            PercentSettingWriteCallback(0);
        }
        else
        {
            PercentSettingWriteCallback(percentSetting.Value());
        }
        break;
    }

    case FanControl::Attributes::SpeedSetting::Id: {
        DataModel::Nullable<uint8_t> speedSetting = static_cast<DataModel::Nullable<uint8_t>>(*value);
        if (speedSetting.IsNull())
        {
            SpeedSettingWriteCallback(0);
        }
        else
        {
            SpeedSettingWriteCallback(speedSetting.Value());
        }
        break;
    }

    case FanControl::Attributes::FanMode::Id: {
        FanControl::FanModeEnum fanMode = static_cast<FanControl::FanModeEnum>(*value);
        FanModeWriteCallback(fanMode);
        break;
    }

    default: {
        break;
    }
    }
}

void AirPurifierManager::PercentSettingWriteCallback(uint8_t aNewPercentSetting)
{
    if (aNewPercentSetting != percentCurrent)
    {
        ChipLogDetail(NotSpecified, "AirPurifierManager::PercentSettingWriteCallback: %d", aNewPercentSetting);
        percentCurrent       = aNewPercentSetting;
        EmberAfStatus status = FanControl::Attributes::PercentCurrent::Set(mEndpointId, percentCurrent);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(NotSpecified,
                         "AirPurifierManager::PercentSettingWriteCallback: failed to set PercentCurrent attribute: %d", status);
        }
    }
}

void AirPurifierManager::SpeedSettingWriteCallback(uint8_t aNewSpeedSetting)
{
    if (aNewSpeedSetting != speedCurrent)
    {
        ChipLogDetail(NotSpecified, "AirPurifierManager::SpeedSettingWriteCallback: %d", aNewSpeedSetting);
        speedCurrent         = aNewSpeedSetting;
        EmberAfStatus status = FanControl::Attributes::SpeedCurrent::Set(mEndpointId, speedCurrent);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(NotSpecified, "AirPurifierManager::SpeedSettingWriteCallback: failed to set SpeedCurrent attribute: %d",
                         status);
        }

        // Determine if the speed change should also change the fan mode
        if (speedCurrent == 0)
        {
            FanControl::Attributes::FanMode::Set(mEndpointId, FanControl::FanModeEnum::kOff);
        }
        else if (speedCurrent <= FAN_MODE_LOW_UPPER_BOUND)
        {
            FanControl::Attributes::FanMode::Set(mEndpointId, FanControl::FanModeEnum::kLow);
        }
        else if (speedCurrent <= FAN_MODE_MEDIUM_UPPER_BOUND)
        {
            FanControl::Attributes::FanMode::Set(mEndpointId, FanControl::FanModeEnum::kMedium);
        }
        else if (speedCurrent <= FAN_MODE_HIGH_UPPER_BOUND)
        {
            FanControl::Attributes::FanMode::Set(mEndpointId, FanControl::FanModeEnum::kHigh);
        }
    }
}

void AirPurifierManager::FanModeWriteCallback(FanControl::FanModeEnum aNewFanMode)
{
    ChipLogDetail(NotSpecified, "AirPurifierManager::FanModeWriteCallback: %d", (uint8_t) aNewFanMode);
    switch (aNewFanMode)
    {
    case FanControl::FanModeEnum::kOff: {
        if (speedCurrent != 0)
        {
            DataModel::Nullable<uint8_t> speedSetting(0);
            SetSpeedSetting(speedSetting);
        }
        break;
    }
    case FanControl::FanModeEnum::kLow: {
        if (speedCurrent < FAN_MODE_LOW_LOWER_BOUND || speedCurrent > FAN_MODE_LOW_UPPER_BOUND)
        {
            DataModel::Nullable<uint8_t> speedSetting(FAN_MODE_LOW_LOWER_BOUND);
            SetSpeedSetting(speedSetting);
        }
        break;
    }
    case FanControl::FanModeEnum::kMedium: {
        if (speedCurrent < FAN_MODE_MEDIUM_LOWER_BOUND || speedCurrent > FAN_MODE_MEDIUM_UPPER_BOUND)
        {
            DataModel::Nullable<uint8_t> speedSetting(FAN_MODE_MEDIUM_LOWER_BOUND);
            SetSpeedSetting(speedSetting);
        }
        break;
    }
    case FanControl::FanModeEnum::kOn:
    case FanControl::FanModeEnum::kHigh: {
        if (speedCurrent < FAN_MODE_HIGH_LOWER_BOUND || speedCurrent > FAN_MODE_HIGH_UPPER_BOUND)
        {
            DataModel::Nullable<uint8_t> speedSetting(FAN_MODE_HIGH_LOWER_BOUND);
            SetSpeedSetting(speedSetting);
        }
        break;
    }
    case FanControl::FanModeEnum::kSmart:
    case FanControl::FanModeEnum::kAuto: {
        ChipLogProgress(NotSpecified, "AirPurifierManager::FanModeWriteCallback: Auto");
        break;
    }
    case FanControl::FanModeEnum::kUnknownEnumValue: {
        ChipLogProgress(NotSpecified, "AirPurifierManager::FanModeWriteCallback: Unknown");
        break;
    }
    }
}

void AirPurifierManager::SetSpeedSetting(DataModel::Nullable<uint8_t> aNewSpeedSetting)
{
    if (aNewSpeedSetting.IsNull())
    {
        ChipLogError(NotSpecified, "AirPurifierManager::SetSpeedSetting: invalid value");
        return;
    }

    if (aNewSpeedSetting.Value() != speedCurrent)
    {
        EmberAfStatus status = FanControl::Attributes::SpeedSetting::Set(mEndpointId, aNewSpeedSetting);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(NotSpecified, "AirPurifierManager::SetSpeedSetting: failed to set SpeedSetting attribute: %d", status);
        }
    }
}
