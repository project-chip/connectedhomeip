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
    mThermostatManager.Init();

    DataModel::Nullable<Percent> percentSetting;
    EmberAfStatus status = FanControl::Attributes::PercentSetting::Get(mEndpointId, percentSetting);
    if (EMBER_ZCL_STATUS_SUCCESS == status)
    {
        if (percentSetting.IsNull())
        {
            PercentSettingChangedCallback(0);
        }
        else
        {
            PercentSettingChangedCallback(percentSetting.Value());
        }
    }

    DataModel::Nullable<uint8_t> speedSetting;
    status = FanControl::Attributes::SpeedSetting::Get(mEndpointId, speedSetting);
    if (EMBER_ZCL_STATUS_SUCCESS == status)
    {
        if (speedSetting.IsNull())
        {
            SpeedSettingChangedCallback(0);
        }
        else
        {
            SpeedSettingChangedCallback(speedSetting.Value());
        }
    }

    // Set up some sane initial values
    mTemperatureSensorManager.OnTemperatureChangeHandler(2000);
    mThermostatManager.OnLocalTemperatureChangeCallback(2000);
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

    case Thermostat::Id: {
        HandleThermostatAttributeChange(attributeId, type, size, value);
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
            PercentSettingChangedCallback(0);
        }
        else
        {
            PercentSettingChangedCallback(percentSetting.Value());
        }
        break;
    }

    case FanControl::Attributes::SpeedSetting::Id: {
        DataModel::Nullable<uint8_t> speedSetting = static_cast<DataModel::Nullable<uint8_t>>(*value);
        if (speedSetting.IsNull())
        {
            SpeedSettingChangedCallback(0);
        }
        else
        {
            SpeedSettingChangedCallback(speedSetting.Value());
        }
        break;
    }

    case FanControl::Attributes::FanMode::Id: {
        FanControl::FanModeEnum fanMode = static_cast<FanControl::FanModeEnum>(*value);
        FanModeChangedCallback(fanMode);
        break;
    }

    default: {
        break;
    }
    }
}

void AirPurifierManager::PercentSettingChangedCallback(uint8_t aNewPercentSetting)
{
    if (aNewPercentSetting != percentCurrent)
    {
        ChipLogDetail(NotSpecified, "AirPurifierManager::PercentSettingChangedCallback: %d", aNewPercentSetting);
        percentCurrent       = aNewPercentSetting;
        EmberAfStatus status = FanControl::Attributes::PercentCurrent::Set(mEndpointId, percentCurrent);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(NotSpecified,
                         "AirPurifierManager::PercentSettingChangedCallback: failed to set PercentCurrent attribute: %d", status);
        }
    }
}

void AirPurifierManager::SpeedSettingChangedCallback(uint8_t aNewSpeedSetting)
{
    if (aNewSpeedSetting != speedCurrent)
    {
        ChipLogDetail(NotSpecified, "AirPurifierManager::SpeedSettingChangedCallback: %d", aNewSpeedSetting);
        speedCurrent         = aNewSpeedSetting;
        EmberAfStatus status = FanControl::Attributes::SpeedCurrent::Set(mEndpointId, speedCurrent);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(NotSpecified, "AirPurifierManager::SpeedSettingChangedCallback: failed to set SpeedCurrent attribute: %d",
                         status);
        }
    }
}

void AirPurifierManager::FanModeChangedCallback(FanControl::FanModeEnum aNewFanMode)
{
    ChipLogDetail(NotSpecified, "AirPurifierManager::FanModeChangedCallback: %d", (uint8_t) aNewFanMode);
    switch (aNewFanMode)
    {
    case FanControl::FanModeEnum::kOff: {
        DataModel::Nullable<chip::Percent> percentSetting(0);
        EmberAfStatus status = FanControl::Attributes::PercentSetting::Set(mEndpointId, percentSetting);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(NotSpecified, "AirPurifierManager::FanModeChangedCallback: failed to set PercentSetting attribute: %d",
                         status);
        }
        break;
    }
    case FanControl::FanModeEnum::kLow: {
        DataModel::Nullable<chip::Percent> percentSetting(30);
        EmberAfStatus status = FanControl::Attributes::PercentSetting::Set(mEndpointId, percentSetting);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(NotSpecified, "AirPurifierManager::FanModeChangedCallback: failed to set PercentSetting attribute: %d",
                         status);
        }
        break;
    }
    case FanControl::FanModeEnum::kMedium: {
        DataModel::Nullable<chip::Percent> percentSetting(60);
        EmberAfStatus status = FanControl::Attributes::PercentSetting::Set(mEndpointId, percentSetting);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(NotSpecified, "AirPurifierManager::FanModeChangedCallback: failed to set PercentSetting attribute: %d",
                         status);
        }
        break;
    }
    case FanControl::FanModeEnum::kOn:
    case FanControl::FanModeEnum::kHigh: {
        DataModel::Nullable<chip::Percent> percentSetting(100);
        EmberAfStatus status = FanControl::Attributes::PercentSetting::Set(mEndpointId, percentSetting);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(NotSpecified, "AirPurifierManager::FanModeChangedCallback: failed to set PercentSetting attribute: %d",
                         status);
        }
        break;
    }
    case FanControl::FanModeEnum::kSmart:
    case FanControl::FanModeEnum::kAuto: {
        ChipLogProgress(NotSpecified, "AirPurifierManager::FanModeChangedCallback: Auto");
        break;
    }
    case FanControl::FanModeEnum::kUnknownEnumValue: {
        ChipLogProgress(NotSpecified, "AirPurifierManager::FanModeChangedCallback: Unknown");
        break;
    }
    }
}

void AirPurifierManager::HandleThermostatAttributeChange(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value)
{
    switch (attributeId)
    {
    case Thermostat::Attributes::OccupiedHeatingSetpoint::Id: {
        int16_t heatingSetpoint = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        ThermostatHeatingSetpointWriteCallback(heatingSetpoint);
        break;
    }
    case Thermostat::Attributes::SystemMode::Id: {
        uint8_t systemMode = static_cast<uint8_t>(*value);
        ThermostatSystemModeWriteCallback(systemMode);
        break;
    }
    }
}

void AirPurifierManager::ThermostatHeatingSetpointWriteCallback(int16_t aNewHeatingSetpoint)
{
    mThermostatManager.HeatingSetpointWriteCallback(aNewHeatingSetpoint);
}

void AirPurifierManager::ThermostatSystemModeWriteCallback(uint8_t aNewSystemMode)
{
    mThermostatManager.SystemModeWriteCallback(aNewSystemMode);
}
