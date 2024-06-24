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

void emberAfWaterHeaterManagementClusterInitCallback(chip::EndpointId endpointId)
{
#if 0
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gWhmInstance == nullptr && gWhmDelegate == nullptr);

    EndpointId waterHeaterManagementEndpoint = 0x01;

    gWhmDelegate = new WaterHeaterManagementDelegate(waterHeaterManagementEndpoint);
    gWhmInstance =
        new WaterHeaterManagement::Instance(waterHeaterManagementEndpoint,
                                            *gWhmDelegate,
                                            BitMask<Feature, uint32_t>(Feature::kEnergyManagement, Feature::kTankPercent));

    gWhmInstance->Init();
#endif
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
    ChipLogError(Zcl, "WaterHeaterManagementDelegate::SetHeatDemand %d", heatDemand.Raw());
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
    mTankPercentage = tankPercentage;
}

void WaterHeaterManagementDelegate::SetBoostState(BoostStateEnum boostState)
{
    mBoostState = boostState;
}

Protocols::InteractionModel::Status WaterHeaterManagementDelegate::HandleBoost(uint32_t durationS, Optional<bool> oneShot, Optional<bool> emergencyBoost, Optional<int16_t> temporarySetpoint, Optional<chip::Percent> targetPercentage, Optional<chip::Percent> targetReheat)
{
    ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleBoost");

    if (oneShot.HasValue() && oneShot.Value() && mWaterTemperature >= mTargetWaterTemperature)
    {
        ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleBoost oneShot==true and waterTemperature >= targetWaterTemperature");
        DeviceLayer::SystemLayer().CancelTimer(BoostTimerExpiry, this);
        mBoostInProgress = false;
        SetBoostState(BoostStateEnum::kActive);
        return Status::Success;
    }

    // If a timer is running, cancel it so we can start it with the new duration
    if (mBoostInProgress)
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

    mBoostInProgress = true;

    SetBoostState(BoostStateEnum::kActive);

    CheckHeatDemand();

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
    mBoostInProgress = false;

    SetBoostState(BoostStateEnum::kInactive);

    CheckHeatDemand();
}

Protocols::InteractionModel::Status WaterHeaterManagementDelegate::HandleCancelBoost()
{
    ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleCancelBoost");

    return Status::Success;
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
    mWaterTemperature = waterTemperature;

    CheckHeatDemand();
}

void WaterHeaterManagementDelegate::SetTargetWaterTemperature(uint16_t targetWaterTemperature)
{
    mTargetWaterTemperature = targetWaterTemperature;

    CheckHeatDemand();
}

void WaterHeaterManagementDelegate::CheckHeatDemand()
{
    ChipLogError(Zcl, "WaterHeaterManagementDelegate::CheckHeatDemand mWaterTemperature %u mTargetWaterTemperature %u mHeatDemand 0x%02x", mWaterTemperature, mTargetWaterTemperature, mHeatDemand.Raw());

    bool turningHeatOff = false;

    if (mWaterTemperature < mTargetWaterTemperature)
    {
        uint8_t mode = mInstance->GetCurrentMode();
        if ((mBoostInProgress || mode == ModeManual) && mHeatDemand.Raw() == 0)
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
        else if (mHeatDemand.Raw() != 0)
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
            turningHeatOff = true;
    }

    if (turningHeatOff)
    {
        if (mBoostState == BoostStateEnum::kActive)
        {
            mBoostState = BoostStateEnum::kInactive;

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

