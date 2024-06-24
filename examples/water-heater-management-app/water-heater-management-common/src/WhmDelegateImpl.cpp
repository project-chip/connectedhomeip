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

static Instance * gWhmInstance = nullptr;
static Delegate * gWhmDelegate = nullptr;

WaterHeaterManagement::Instance * GetWaterHeaterManagementInstance()
{
    return gWhmInstance;
}

void WaterHeaterManagement::Shutdown()
{
    if (gWhmInstance != nullptr)
    {
        delete gWhmInstance;
        gWhmInstance = nullptr;
    }

    if (gWhmDelegate != nullptr)
    {
        delete gWhmDelegate;
        gWhmDelegate = nullptr;
    }
}

#if 0
void emberAfWaterHeaterManagementClusterInitCallback(chip::EndpointId endpointId)
{
}
#endif

WaterHeaterManagementDelegate::WaterHeaterManagementDelegate(EndpointId clustersEndpoint):
    mWaterHeaterModeInstance(this, clustersEndpoint, WaterHeaterMode::Id, 0),
    mTankPercentage(0),
    mBoostState(BoostStateEnum::kActive),
    mBoostTargetTemperatureReached(false)
{
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
    mHeaterTypes = heaterTypes;
}

void WaterHeaterManagementDelegate::SetHeatDemand(BitMask<WaterHeaterDemandBitmap> heatDemand)
{
    ChipLogProgress(Zcl, "WaterHeaterManagementDelegate::SetHeatDemand %d", heatDemand.Raw());

    mHeatDemand = heatDemand;
}

void WaterHeaterManagementDelegate::SetTankVolume(uint16_t tankVolume)
{
    mTankVolume = tankVolume;
}

void WaterHeaterManagementDelegate::SetEstimatedHeatRequired(int64_t estimatedHeatRequired)
{
    mEstimatedHeatRequired = estimatedHeatRequired;
}

void WaterHeaterManagementDelegate::SetTankPercentage(Percent tankPercentage)
{
    ChipLogProgress(Zcl, "WaterHeaterManagementDelegate::SetBoostState tankPercentage %d", tankPercentage);

    mTankPercentage = tankPercentage;

    CheckHeatDemand();
}

void WaterHeaterManagementDelegate::SetBoostState(BoostStateEnum boostState)
{
    ChipLogProgress(Zcl, "WaterHeaterManagementDelegate::SetBoostState boostState %d", static_cast<int>(boostState));

    mBoostState = boostState;
}

Protocols::InteractionModel::Status WaterHeaterManagementDelegate::HandleBoost(uint32_t durationS, Optional<bool> oneShot, Optional<bool> emergencyBoost, Optional<int16_t> temporarySetpoint, Optional<chip::Percent> targetPercentage, Optional<chip::Percent> targetReheat)
{
    ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleBoost oneShot %d", (int)(oneShot.HasValue()?oneShot.Value():0));

    mBoostOneShot = oneShot;
    mBoostEmergencyBoost = emergencyBoost;
    mBoostTemporarySetpoint = temporarySetpoint;
    mBoostTargetPercentage = targetPercentage;
    mBoostTargetReheat = targetReheat;
    mBoostTargetTemperatureReached = false;

    if (oneShot.HasValue() && oneShot.Value() && HasWaterTemperatureReachedTarget())
    {
        ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleBoost oneShot==true and waterTemperature >= targetWaterTemperature");

        DeviceLayer::SystemLayer().CancelTimer(BoostTimerExpiry, this);

        SetBoostState(BoostStateEnum::kInactive);

        return Status::Success;
    }

    // If a timer is running, cancel it so we can start it with the new duration
    if (mBoostState == BoostStateEnum::kActive)
    {
        DeviceLayer::SystemLayer().CancelTimer(BoostTimerExpiry, this);
    }
    else
    {
        CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(durationS), BoostTimerExpiry, this);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Unable to start a Boost timer: %" CHIP_ERROR_FORMAT, err.Format());
            return Status::Failure;
        }
    }

    SetBoostState(BoostStateEnum::kActive);

    ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleBoost2 oneShot %d", (int)(oneShot.HasValue()?oneShot.Value():0));
    CheckHeatDemand();

    ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleBoost3 oneShot %d", (int)(oneShot.HasValue()?oneShot.Value():0));
    return Status::Success;
}

/**
 * @brief Timer for handling the PowerAdjustRequest
 *
 * This static function calls the non-static HandlePowerAdjustTimerExpiry method.
 */
void WaterHeaterManagementDelegate::BoostTimerExpiry(System::Layer * systemLayer, void * delegate)
{
    WaterHeaterManagementDelegate * dg = reinterpret_cast<WaterHeaterManagementDelegate *>(delegate);

    dg->HandleBoostTimerExpiry();
}

/**
 * @brief Timer for handling the completion of a PowerAdjustRequest
 *
 *  When the timer expires:
 *   1) notify the appliance's that it can resume its intended power setting (or go idle)
 *   2) generate a PowerAdjustEnd event with cause NormalCompletion
 *   3) if necessary, update the forecast with new expected end time
 */
void WaterHeaterManagementDelegate::HandleBoostTimerExpiry()
{
    ChipLogError(AppServer, "WaterHeaterManagementDelegate::HandleBoostTimerExpiry");

    // The PowerAdjustment is no longer in progress
    SetBoostState(BoostStateEnum::kInactive);

    CheckHeatDemand();
}

Status WaterHeaterManagementDelegate::HandleCancelBoost()
{
    ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleCancelBoost");

    Status status = Status::Success;

    if (mBoostState == BoostStateEnum::kActive)
    {
        SetBoostState(BoostStateEnum::kInactive);

        DeviceLayer::SystemLayer().CancelTimer(BoostTimerExpiry, this);

        CheckHeatDemand();
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
    ChipLogError(Zcl, "WaterHeaterManagementDelegate::HandleChangeToMode");
    response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
}

CHIP_ERROR WaterHeaterManagementDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    ChipLogError(Zcl, "WaterHeaterManagementDelegate::GetModeLabelByIndex");
    if (modeIndex >= ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR WaterHeaterManagementDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    ChipLogError(Zcl, "WaterHeaterManagementDelegate::GetModeValueByIndex");
    if (modeIndex >= ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR WaterHeaterManagementDelegate::GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags)
{
    ChipLogError(Zcl, "WaterHeaterManagementDelegate::GetModeTagsByIndex");
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
    mHotWaterTemperature = waterTemperature;

    mTankPercentage = 100;

    CheckHeatDemand();
}

void WaterHeaterManagementDelegate::SetTargetWaterTemperature(uint16_t targetWaterTemperature)
{
    mTargetWaterTemperature = targetWaterTemperature;

    CheckHeatDemand();
}

void WaterHeaterManagementDelegate::DrawOffHotWater(uint8_t percentageReplaced, uint16_t replacedWaterTemperature)
{
    if (mTankPercentage >= percentageReplaced)
    {
        mTankPercentage -= percentageReplaced;
    }
    else
    {
        mTankPercentage = 0;
    }

    mReplacedWaterTemperature = replacedWaterTemperature;

    CheckHeatDemand();
}

bool WaterHeaterManagementDelegate::HasWaterTemperatureReachedTarget() const
{
    bool reached = false;

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
        targetPercentage = (mBoostState == BoostStateEnum::kActive && mBoostTargetPercentage.HasValue()) ? mBoostTargetPercentage.Value() : 100;
    }

    ChipLogError(Zcl, "WaterHeaterManagementDelegate::HasWaterTemperatureReachedTarget: mHotWaterTemperature %u mBoostState %d mBoostTargetPercentage %u targetPercentage %u targetTemperature %u", mHotWaterTemperature, (int) mBoostState, mBoostTargetPercentage.HasValue() ? mBoostTargetPercentage.Value() : 255, targetPercentage, targetTemperature);

    reached = (mTankPercentage >= targetPercentage) && (mHotWaterTemperature >= targetTemperature);

    return reached;
}

void WaterHeaterManagementDelegate::CheckHeatDemand()
{
    ChipLogError(Zcl, "WaterHeaterManagementDelegate::CheckHeatDemand mHotWaterTemperature %u mTargetWaterTemperature %u mHeatDemand 0x%02x boostState %d mBoostTargetTemperatureReached %d", mHotWaterTemperature, mTargetWaterTemperature, mHeatDemand.Raw(), (int)BoostStateEnum::kActive, mBoostTargetTemperatureReached);

    bool turningHeatOff = false;

    if (!HasWaterTemperatureReachedTarget())
    {
        uint8_t mode = mInstance->GetCurrentMode();
        if (mHeatDemand.Raw() == 0)
        {
            if (mBoostState == BoostStateEnum::kActive)
            {
                mBoostTargetTemperatureReached = false;
            }

            if (mBoostState == BoostStateEnum::kActive || mode == ModeManual)
            {
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
                    ChipLogError(Zcl, "WaterHeaterManagementDelegate::CheckHeatDemand Turning heat on1");
                    SetHeatDemand(BitMask<WaterHeaterDemandBitmap>(1 << bit));
                }
                else
                {
                    ChipLogError(Zcl, "WaterHeaterManagementDelegate::CheckHeatDemand Failed to find heaterType");
                }
            }
        }
        else if (mBoostState == BoostStateEnum::kInactive && mode == ModeOff)
        {
            ChipLogError(Zcl, "WaterHeaterManagementDelegate::CheckHeatDemand turning heating off due to mode");
            SetHeatDemand(BitMask<WaterHeaterDemandBitmap>(0));
            turningHeatOff = true;
        }
    }
    else if (mHeatDemand.Raw() != 0)
    {
        ChipLogError(Zcl, "WaterHeaterManagementDelegate::CheckHeatDemand Turning heat on2");
        SetHeatDemand(BitMask<WaterHeaterDemandBitmap>(0));
        mTankPercentage = 100;
        turningHeatOff = true;

        mBoostTargetTemperatureReached = (mBoostState == BoostStateEnum::kActive);
    }

    if (turningHeatOff)
    {
        if (mBoostState == BoostStateEnum::kActive && mBoostOneShot.HasValue() && mBoostOneShot.Value())
        {
            ChipLogError(Zcl, "WaterHeaterManagementDelegate::CheckHeatDemand mBoostState %d mBoostOneShot %d", (int) mBoostState, (int)(mBoostOneShot.HasValue() ? mBoostOneShot.Value() : 0))
            SetBoostState(BoostStateEnum::kInactive);

            DeviceLayer::SystemLayer().CancelTimer(BoostTimerExpiry, this);
        }
    }
}

void WaterHeaterManagementDelegate::SetWaterHeaterMode(uint8_t modeValue)
{
    bool isSupported = mInstance->IsSupportedMode(modeValue);
    if (!isSupported)
    {
        ChipLogError(Zcl, "WaterHeaterManagementDelegate::SetWaterHeaterMode bad mode");
        return;
    }

    Protocols::InteractionModel::Status status = mInstance->UpdateCurrentMode(modeValue);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "WaterHeaterManagementDelegate::SetWaterHeaterMode updateMode failed");
        return;
    }

    ChipLogError(Zcl, "SetWaterHeaterMode modeValue %u currentMode %u", modeValue, mInstance->GetCurrentMode());
    CheckHeatDemand();
}

