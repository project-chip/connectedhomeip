/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/clusters/water-heater-management-server/water-heater-management-server.h>

#include <WhmDelegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WaterHeaterManagement;

using Protocols::InteractionModel::Status;

WaterHeaterManagementDelegate::WaterHeaterManagementDelegate(EndpointId clustersEndpoint):
    mpWhmInstance(nullptr),
    mBoostTargetTemperatureReached(false),
    mWaterHeaterModeInstance(this, clustersEndpoint, WaterHeaterMode::Id, 0),
    mTankVolume(0),
    mEstimatedHeatRequired(0),
    mTankPercentage(0),
    mBoostState(BoostStateEnum::kInactive)
{
}

void WaterHeaterManagementDelegate::SetWaterHeaterManagementInstance(WaterHeaterManagement::Instance & instance)
{
    mpWhmInstance = &instance;

    if (!mpWhmInstance->HasFeature(Feature::kTankPercent))
    {
        // If the feature kTankPercent is not supported then set mTankPercentage to 100% so calculations below need
        // less code for the feature kTankPercent case.
        mTankPercentage = 100;
    }
}

/*********************************************************************************
 *
 * Methods implementing the WaterHeaterManagement::Delegate interace
 *
 *********************************************************************************/

BitMask<WaterHeaterTypeBitmap> WaterHeaterManagementDelegate::GetHeaterTypes()
{
    return mHeaterTypes;
}

BitMask<WaterHeaterDemandBitmap> WaterHeaterManagementDelegate::GetHeatDemand()
{
    return mHeatDemand;
}

uint16_t WaterHeaterManagementDelegate::GetTankVolume()
{
    return mTankVolume;
}

int64_t WaterHeaterManagementDelegate::GetEstimatedHeatRequired()
{
    return mEstimatedHeatRequired;
}

Percent WaterHeaterManagementDelegate::GetTankPercentage()
{
    return mTankPercentage;
}

BoostStateEnum WaterHeaterManagementDelegate::GetBoostState()
{
    return mBoostState;
}

void WaterHeaterManagementDelegate::SetHeaterTypes(BitMask<WaterHeaterTypeBitmap> heaterTypes)
{
    if (mHeaterTypes != heaterTypes)
    {
        mHeaterTypes = heaterTypes;

        MatterReportingAttributeChangeCallback(mEndpointId, WaterHeaterManagement::Id, Attributes::HeaterTypes::Id);
    }
}

void WaterHeaterManagementDelegate::SetHeatDemand(BitMask<WaterHeaterDemandBitmap> heatDemand)
{
    if (mHeatDemand != heatDemand)
    {
        mHeatDemand = heatDemand;

        MatterReportingAttributeChangeCallback(mEndpointId, WaterHeaterManagement::Id, Attributes::HeatDemand::Id);
    }
}

void WaterHeaterManagementDelegate::SetTankVolume(uint16_t tankVolume)
{
    if (mTankVolume != tankVolume)
    {
        mTankVolume = tankVolume;

        MatterReportingAttributeChangeCallback(mEndpointId, WaterHeaterManagement::Id, Attributes::TankVolume::Id);
    }
}

void WaterHeaterManagementDelegate::SetEstimatedHeatRequired(int64_t estimatedHeatRequired)
{
    if (mEstimatedHeatRequired != estimatedHeatRequired)
    {
        mEstimatedHeatRequired = estimatedHeatRequired;

        MatterReportingAttributeChangeCallback(mEndpointId, WaterHeaterManagement::Id, Attributes::EstimatedHeatRequired::Id);
    }
}

void WaterHeaterManagementDelegate::SetTankPercentage(Percent tankPercentage)
{
    if (mpWhmInstance != nullptr && mpWhmInstance->HasFeature(Feature::kTankPercent))
    {
        if (mTankPercentage != tankPercentage)
        {
            mTankPercentage = tankPercentage;

            CheckIfHeatNeedsToBeTurnedOnOrOff();

            MatterReportingAttributeChangeCallback(mEndpointId, WaterHeaterManagement::Id, Attributes::TankPercentage::Id);
        }
    }
}

void WaterHeaterManagementDelegate::SetBoostState(BoostStateEnum boostState)
{
    if (mBoostState != boostState)
    {
        mBoostState = boostState;

        MatterReportingAttributeChangeCallback(mEndpointId, WaterHeaterManagement::Id, Attributes::BoostState::Id);
    }
}

/**
 * @brief Handles the boost command
 *
 * Upon receipt, the Water Heater SHALL transition into the BOOST state, which SHALL cause the water in the tank (or
 * the TargetPercentage of the water, if included) to be heated towards the set point (or the TemporarySetpoint, if
 * included), which in turn may cause a call for heat, even if the mode is OFF, or is TIMED and it is during one of
 * the Off periods.
 */
Status WaterHeaterManagementDelegate::HandleBoost(uint32_t durationS, Optional<bool> oneShot, Optional<bool> emergencyBoost, Optional<int16_t> temporarySetpoint, Optional<chip::Percent> targetPercentage, Optional<chip::Percent> targetReheat)
{
    ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleBoost");

    // Keep track of the boost command parameters
    mBoostOneShot = oneShot;
    mBoostEmergencyBoost = emergencyBoost;
    mBoostTemporarySetpoint = temporarySetpoint;
    mBoostTargetPercentage = targetPercentage;
    mBoostTargetReheat = targetReheat;

    mBoostTargetTemperatureReached = false;

    // If a timer is running, cancel it so we can start a new boost command with the new duration
    if (mBoostState == BoostStateEnum::kActive)
    {
        DeviceLayer::SystemLayer().CancelTimer(BoostTimerExpiry, this);
    }

    // See if the water temperature is already at the target temperature in which case nothing to do
    if (oneShot.HasValue() && oneShot.Value() && HasWaterTemperatureReachedTarget())
    {
        ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleBoost oneShot==true and waterTemperature >= targetWaterTemperature");

        // Cancel any previous timers
        DeviceLayer::SystemLayer().CancelTimer(BoostTimerExpiry, this);

        SetBoostState(BoostStateEnum::kInactive);

        return Status::Success;
    }

    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(durationS), BoostTimerExpiry, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Unable to start a Boost timer: %" CHIP_ERROR_FORMAT, err.Format());

        // Not a lot we can do -> just set the boost state to inactive
        SetBoostState(BoostStateEnum::kInactive);

        return Status::Failure;
    }

    // Now running a boost command
    SetBoostState(BoostStateEnum::kActive);

    // See if the heat needs to be turned on or off as a result of this boost command
    CheckIfHeatNeedsToBeTurnedOnOrOff();

    return Status::Success;
}

void WaterHeaterManagementDelegate::BoostTimerExpiry(System::Layer * systemLayer, void * delegate)
{
    WaterHeaterManagementDelegate * dg = reinterpret_cast<WaterHeaterManagementDelegate *>(delegate);

    dg->HandleBoostTimerExpiry();
}

/**
 * @brief Timer for handling the completion of a boost command
 */
void WaterHeaterManagementDelegate::HandleBoostTimerExpiry()
{
    ChipLogError(AppServer, "WaterHeaterManagementDelegate::HandleBoostTimerExpiry");

    // The PowerAdjustment is no longer in progress
    SetBoostState(BoostStateEnum::kInactive);

    CheckIfHeatNeedsToBeTurnedOnOrOff();
}

/**
 * @brief Cancels a boost command
 *
 * Upon receipt, the Water Heater SHALL transition back from the BOOST state to the previous mode (e.g. OFF, MANUAL or TIMED).
 */
Status WaterHeaterManagementDelegate::HandleCancelBoost()
{
    ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleCancelBoost");

    Status status = Status::Success;

    if (mBoostState == BoostStateEnum::kActive)
    {
        SetBoostState(BoostStateEnum::kInactive);

        DeviceLayer::SystemLayer().CancelTimer(BoostTimerExpiry, this);

        CheckIfHeatNeedsToBeTurnedOnOrOff();
    }
    else
    {
        status = Status::InvalidInState;
    }

    return status;
}

/*********************************************************************************
 *
 * Methods implementing the ModeBase::Delegate interface
 *
 *********************************************************************************/

CHIP_ERROR WaterHeaterManagementDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void WaterHeaterManagementDelegate::HandleChangeToMode(uint8_t NewMode,
                                                       ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
}

CHIP_ERROR WaterHeaterManagementDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR WaterHeaterManagementDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    value = kModeOptions[modeIndex].mode;

    return CHIP_NO_ERROR;
}

CHIP_ERROR WaterHeaterManagementDelegate::GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags)
{
    if (modeIndex >= ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    if (tags.size() < kModeOptions[modeIndex].modeTags.size())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::copy(kModeOptions[modeIndex].modeTags.begin(), kModeOptions[modeIndex].modeTags.end(), tags.begin());
    tags.reduce_size(kModeOptions[modeIndex].modeTags.size());

    return CHIP_NO_ERROR;
}

/*********************************************************************************
 *
 * WaterHeaterManagementDelegate specific methods
 *
 *********************************************************************************/

void WaterHeaterManagementDelegate::SetWaterTemperature(uint16_t waterTemperature)
{
    // This method assumed 100% of the water in the tank has reached the waterTemperature specified
    mHotWaterTemperature = waterTemperature;

    // Do not change mTankPercentage if the kTankPercent feature is not supported
    if (mpWhmInstance != nullptr && mpWhmInstance->HasFeature(Feature::kTankPercent))
    {
        mTankPercentage = 100;
    }

    // See if the heat needs to be turned on or off
    CheckIfHeatNeedsToBeTurnedOnOrOff();
}

void WaterHeaterManagementDelegate::SetTargetWaterTemperature(uint16_t targetWaterTemperature)
{
    mTargetWaterTemperature = targetWaterTemperature;

    // See if the heat needs to be turned on or off
    CheckIfHeatNeedsToBeTurnedOnOrOff();
}

void WaterHeaterManagementDelegate::DrawOffHotWater(uint8_t percentageReplaced, uint16_t replacedWaterTemperature)
{
    // Only supported in the kTankPercent is supported.
    // Replaces percentageReplaced% of the water in the tank with water of a temperature replacedWaterTemperature
    if (mpWhmInstance != nullptr && mpWhmInstance->HasFeature(Feature::kTankPercent))
    {
        // See if all of the water has now been replaced with replacedWaterTemperature
        if (mTankPercentage >= percentageReplaced)
        {
            mTankPercentage -= percentageReplaced;
        }
        else
        {
            mTankPercentage = 0;
        }

        mReplacedWaterTemperature = replacedWaterTemperature;

        CheckIfHeatNeedsToBeTurnedOnOrOff();
    }
}

bool WaterHeaterManagementDelegate::HasWaterTemperatureReachedTarget() const
{
    // Determine the target temperature. If a boost command is in progress and has a mBoostTemporarySetpoint value use that as the target temperature
    uint16_t targetTemperature = (mBoostState == BoostStateEnum::kActive && mBoostTemporarySetpoint.HasValue()) ? mBoostTemporarySetpoint.Value() : mTargetWaterTemperature;
    uint8_t targetPercentage;

    if (mBoostState == BoostStateEnum::kActive && mBoostTargetTemperatureReached && mBoostTargetReheat.HasValue())
    {
        // If the tank supports the TankPercent feature, and the heating by this Boost command has ceased because the TargetPercentage
        // of the water in the tank has been heated to the set point (or TemporarySetpoint if included), this field indicates the
        // percentage to which the hot water in the tank SHALL be allowed to fall before again beginning to reheat it.
        //
        // For example if the TargetPercentage was 80%, and the TargetReheat was 40%, then after initial heating to 80% hot water,
        // the tank may have hot water drawn off until only 40% hot water remains. At this point the heater will begin to heat back
        // up to 80% of hot water. If this field and the OneShot field were both omitted, heating would begin again after any water
        // draw which reduced the TankPercentage below 80%.

        // If this field is included then the TargetPercentage field SHALL also be included, and the OneShot excluded.

        targetPercentage = mBoostTargetReheat.Value();
    }
    else
    {
        // Determine the target %. If a boost command is in progress and has a mBoostTargetPercentage value use that as the target %,
        // otherwise 100% of the water in the tank must be at the target temperature
        targetPercentage = (mBoostState == BoostStateEnum::kActive && mBoostTargetPercentage.HasValue()) ? mBoostTargetPercentage.Value() : 100;
    }

    // Return whether the water is at the target temperature
    return (mTankPercentage >= targetPercentage) && (mHotWaterTemperature >= targetTemperature);
}

void WaterHeaterManagementDelegate::CheckIfHeatNeedsToBeTurnedOnOrOff()
{
    bool turningHeatOff = false;

    if (!HasWaterTemperatureReachedTarget())
    {
        uint8_t mode = mInstance->GetCurrentMode();

        // The water in the tank is not at the target temperature. See if we heating is currently off
        if (mHeatDemand.Raw() == 0)
        {
            // Need to track whether the water temperature has reached the target temperature for the boost
            // command when a oneShot option has been applied.
            if (mBoostState == BoostStateEnum::kActive)
            {
                mBoostTargetTemperatureReached = false;
            }

            // If a boost command is in progress or in manual mode, find a heating source and "turn it on".
            if (mBoostState == BoostStateEnum::kActive || mode == ModeManual)
            {
                // Look through the heaterTypes attribute for a valid source
                bool found = false;
                uint8_t rawBitmask = mHeaterTypes.Raw();
                uint8_t bit = 0;
                while (rawBitmask != 0 && !found)
                {
                    if (rawBitmask & 1)
                    {
                        found = true;
                    }
                    else
                    {
                        bit++;
                        rawBitmask >>= 1;
                    }
                }

                if (found)
                {
                    // Found a source - use it to turn the heating on
                    SetHeatDemand(BitMask<WaterHeaterDemandBitmap>(1 << bit));
                }
                else
                {
                    ChipLogError(Zcl, "WaterHeaterManagementDelegate::CheckIfHeatNeedsToBeTurnedOnOrOff Failed to find heaterType");
                }
            }
        }
        else if (mBoostState == BoostStateEnum::kInactive && mode == ModeOff)
        {
            // The water temperature is not at the target temperature but there is no boost command in progress and the mode is Off
            // so need to ensure the heating is turned off.
            ChipLogError(Zcl, "WaterHeaterManagementDelegate::CheckIfHeatNeedsToBeTurnedOnOrOff turning heating off due to mode");

            SetHeatDemand(BitMask<WaterHeaterDemandBitmap>(0));

            turningHeatOff = true;
        }
    }
    else if (mHeatDemand.Raw() != 0)
    {
        // The water in the tank has reached the target temperature - need to turn the heating off
        SetHeatDemand(BitMask<WaterHeaterDemandBitmap>(0));

        turningHeatOff = true;

        // If a boost command is in progress, record that the target temperature has been reached.
        mBoostTargetTemperatureReached = (mBoostState == BoostStateEnum::kActive);
    }

    if (turningHeatOff)
    {
        // If running a boost command with the oneShot parameter and turning heat off, then must have
        // reached the boost command target temperature -> that's the boost command complete.
        if (mBoostState == BoostStateEnum::kActive && mBoostOneShot.HasValue() && mBoostOneShot.Value())
        {
            SetBoostState(BoostStateEnum::kInactive);

            DeviceLayer::SystemLayer().CancelTimer(BoostTimerExpiry, this);
        }
    }
}

void WaterHeaterManagementDelegate::SetWaterHeaterMode(uint8_t modeValue)
{
    if (!mInstance->IsSupportedMode(modeValue))
    {
        ChipLogError(Zcl, "WaterHeaterManagementDelegate::SetWaterHeaterMode bad mode");
        return;
    }

    Status status = mInstance->UpdateCurrentMode(modeValue);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "WaterHeaterManagementDelegate::SetWaterHeaterMode updateMode failed");
        return;
    }

    CheckIfHeatNeedsToBeTurnedOnOrOff();
}

