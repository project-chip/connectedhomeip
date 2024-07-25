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
#include <WhmManufacturer.h>
#include <water-heater-mode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WaterHeaterManagement;

using Protocols::InteractionModel::Status;

WaterHeaterManagementDelegate::WaterHeaterManagementDelegate(EndpointId clustersEndpoint) :
    mpWhmInstance(nullptr), mpWhmManufacturer(nullptr), mBoostTargetTemperatureReached(false),
    mTankVolume(0), mEstimatedHeatRequired(0),
    mTankPercentage(0), mBoostState(BoostStateEnum::kInactive)
{
}

void WaterHeaterManagementDelegate::SetWaterHeaterManagementInstance(WaterHeaterManagement::Instance & instance)
{
    mpWhmInstance = &instance;
}

void WaterHeaterManagementDelegate::SetWhmManufacturer(WhmManufacturer & whmManufacturer)
{
    mpWhmManufacturer = &whmManufacturer;
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
Status WaterHeaterManagementDelegate::HandleBoost(uint32_t durationS, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                                  Optional<int16_t> temporarySetpoint, Optional<chip::Percent> targetPercentage,
                                                  Optional<chip::Percent> targetReheat)
{
    Status status = Status::Success;

    ChipLogProgress(AppServer, "HandleBoost");

    // Keep track of the boost command parameters
    mBoostOneShot           = oneShot;
    mBoostEmergencyBoost    = emergencyBoost;
    mBoostTemporarySetpoint = temporarySetpoint;
    mBoostTargetPercentage  = targetPercentage;
    mBoostTargetReheat      = targetReheat;

    mBoostTargetTemperatureReached = false;

    // If a timer is running, cancel it so we can start a new boost command with the new duration
    if (mBoostState == BoostStateEnum::kActive)
    {
        DeviceLayer::SystemLayer().CancelTimer(BoostTimerExpiry, this);
    }

    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(durationS), BoostTimerExpiry, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "HandleBoost: Unable to start a Boost timer: %" CHIP_ERROR_FORMAT, err.Format());

        // Not a lot we can do -> just set the boost state to inactive
        SetBoostState(BoostStateEnum::kInactive);

        return Status::Failure;
    }

    // Now running a boost command
    SetBoostState(BoostStateEnum::kActive);

    if (mpWhmManufacturer != nullptr)
    {
        status = mpWhmManufacturer->BoostCommandStarted(durationS, oneShot, emergencyBoost, temporarySetpoint, targetPercentage,
                                                        targetReheat);
    }
    else
    {
        status = Status::InvalidInState;
        ChipLogError(AppServer, "HandleBoost: mpWhmManufacturer == nullptr");
    }

    if (status == Status::Success)
    {
        // See if the heat needs to be turned on or off as a result of this boost command
        status = CheckIfHeatNeedsToBeTurnedOnOrOff();
    }

    return status;
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
    ChipLogError(AppServer, "HandleBoostTimerExpiry");

    // The PowerAdjustment is no longer in progress
    SetBoostState(BoostStateEnum::kInactive);

    if (mpWhmManufacturer != nullptr)
    {
        mpWhmManufacturer->BoostCommandFinished();
    }
    else
    {
        ChipLogError(AppServer, "HandleBoostTimerExpiry: mpWhmManufacturer == nullptr");
    }

    CheckIfHeatNeedsToBeTurnedOnOrOff();
}

/**
 * @brief Cancels a boost command
 *
 * Upon receipt, the Water Heater SHALL transition back from the BOOST state to the previous mode (e.g. OFF, MANUAL or TIMED).
 */
Status WaterHeaterManagementDelegate::HandleCancelBoost()
{
    Status status = Status::Success;

    ChipLogProgress(AppServer, "HandleCancelBoost");

    if (mBoostState == BoostStateEnum::kActive)
    {
        SetBoostState(BoostStateEnum::kInactive);

        DeviceLayer::SystemLayer().CancelTimer(BoostTimerExpiry, this);

        if (mpWhmManufacturer != nullptr)
        {
            status = mpWhmManufacturer->BoostCommandCancelled();
        }
        else
        {
            status = Status::InvalidInState;
            ChipLogError(AppServer, "HandleCancelBoost: mpWhmManufacturer == nullptr");
        }

        status = CheckIfHeatNeedsToBeTurnedOnOrOff();
    }

    return status;
}

/*********************************************************************************
 *
 * Methods implementing the ModeBase::Delegate interface
 *
 *********************************************************************************/

/*********************************************************************************
 *
 * WaterHeaterManagementDelegate specific methods
 *
 *********************************************************************************/

void WaterHeaterManagementDelegate::SetWaterTemperature(uint16_t waterTemperature)
{
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
    // Determine the target temperature. If a boost command is in progress and has a mBoostTemporarySetpoint value use that as the
    // target temperature.
    // Note, in practise the actual heating is likely to be controlled by the thermostat's occupiedHeatingSetpoint most of the
    // time, and the TemporarySetpoint (if not null) would be overiding the thermostat's occupiedHeatingSetpoint.
    // However, this code doesn't rely upon the thermostat cluster.
    uint16_t targetTemperature = (mBoostState == BoostStateEnum::kActive && mBoostTemporarySetpoint.HasValue())
        ? static_cast<uint16_t>(mBoostTemporarySetpoint.Value())
        : mTargetWaterTemperature;
    uint8_t targetPercentage;

    if (mBoostState == BoostStateEnum::kActive && mBoostTargetTemperatureReached && mBoostTargetReheat.HasValue())
    {
        // If the tank supports the TankPercent feature, and the heating by this Boost command has ceased because the
        // TargetPercentage of the water in the tank has been heated to the set point (or TemporarySetpoint if included), this field
        // indicates the percentage to which the hot water in the tank SHALL be allowed to fall before again beginning to reheat it.
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
        // Determine the target %. If a boost command is in progress and has a mBoostTargetPercentage value use that as the target
        // %, otherwise 100% of the water in the tank must be at the target temperature
        targetPercentage =
            (mBoostState == BoostStateEnum::kActive && mBoostTargetPercentage.HasValue()) ? mBoostTargetPercentage.Value() : 100;
    }

    // Return whether the water is at the target temperature
    return (mTankPercentage >= targetPercentage) && (mHotWaterTemperature >= targetTemperature);
}

Status WaterHeaterManagementDelegate::CheckIfHeatNeedsToBeTurnedOnOrOff()
{
    Status status       = Status::Success;
    bool turningHeatOff = false;

    if (!HasWaterTemperatureReachedTarget())
    {
        uint8_t mode = WaterHeaterMode::Instance()->GetCurrentMode();

        // The water in the tank is not at the target temperature. See if heating is currently off
        if (mHeatDemand.Raw() == 0)
        {
            // Need to track whether the water temperature has reached the target temperature for the boost
            // command when a oneShot option has been applied.
            if (mBoostState == BoostStateEnum::kActive)
            {
                mBoostTargetTemperatureReached = false;
            }

            // If a boost command is in progress or in manual mode, find a heating source and "turn it on".
            if (mBoostState == BoostStateEnum::kActive || mode == WaterHeaterMode::ModeManual)
            {
                if (mpWhmManufacturer != nullptr)
                {
                    // Find out from the manufacturer object the heating sources to use.
                    BitMask<WaterHeaterDemandBitmap> heaterDemand = mpWhmManufacturer->DetermineHeatingSources();

                    SetHeatDemand(heaterDemand);

                    // And turn the heating of the water tank on.
                    status = mpWhmManufacturer->TurnHeatingOn();
                }
                else
                {
                    status = Status::InvalidInState;
                    ChipLogError(AppServer, "CheckIfHeatNeedsToBeTurnedOnOrOff: Failed as mpWhmManufacturer == nullptr");
                }
            }
        }
        else if (mBoostState == BoostStateEnum::kInactive && mode == WaterHeaterMode::ModeOff)
        {
            // The water temperature is not at the target temperature but there is no boost command in progress and the mode is Off
            // so need to ensure the heating is turned off.
            ChipLogError(AppServer, "CheckIfHeatNeedsToBeTurnedOnOrOff turning heating off due to mode");

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

            if (mpWhmManufacturer != nullptr)
            {
                status = mpWhmManufacturer->BoostCommandCancelled();
            }
            else
            {
                status = Status::InvalidInState;
                ChipLogError(AppServer, "CheckIfHeatNeedsToBeTurnedOnOrOff: mpWhmManufacturer == nullptr");
            }
        }

        // Turn the heating off
        if (mpWhmManufacturer != nullptr)
        {
            status = mpWhmManufacturer->TurnHeatingOff();
        }
        else
        {
            status = Status::InvalidInState;
            ChipLogError(AppServer,
                         "CheckIfHeatNeedsToBeTurnedOnOrOff: Failed to turn the heating off as mpWhmManufacturer == nullptr");
        }
    }

    return status;
}

void WaterHeaterManagementDelegate::SetWaterHeaterMode(uint8_t modeValue)
{
    if (!WaterHeaterMode::Instance()->IsSupportedMode(modeValue))
    {
        ChipLogError(AppServer, "SetWaterHeaterMode bad mode");
        return;
    }

    Status status = WaterHeaterMode::Instance()->UpdateCurrentMode(modeValue);
    if (status != Status::Success)
    {
        ChipLogError(AppServer, "SetWaterHeaterMode updateMode failed");
        return;
    }

    CheckIfHeatNeedsToBeTurnedOnOrOff();
}
