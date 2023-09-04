#include <air-purifier-manager.h>
#include <app/util/error-mapping.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using Protocols::InteractionModel::Status;

void AirPurifierManager::Init()
{
    FanControl::SetDefaultDelegate(mEndpointId, this);

    mAirQualitySensorManager.Init();
    activatedCarbonFilterInstance.Init();
    hepaFilterInstance.Init();

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
}

void AirPurifierManager::PostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId,
                                                     uint8_t type, uint16_t size, uint8_t * value)
{
    switch (clusterId)
    {
    case FanControl::Id:
        HandleFanControlAttributeChange(attributeId, type, size, value);
        break;

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

    default: {
        break;
    }
    }
}
