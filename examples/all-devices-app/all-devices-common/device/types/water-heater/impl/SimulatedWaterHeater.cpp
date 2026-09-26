/*
 *
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
#include <clusters/WaterHeaterMode/Enums.h>
#include <device/types/water-heater/impl/SimulatedWaterHeater.h>
#include <lib/support/CodeUtils.h>

#include <algorithm>

using chip::Protocols::InteractionModel::Status;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WaterHeaterManagement;
using namespace chip::app::Clusters::WaterHeaterManagement::Attributes;
using namespace chip::app::Clusters::Thermostat;

namespace chip::app {

namespace {

constexpr uint32_t kStepDurationSeconds = 3;
using ModeTagStructType                 = Clusters::detail::Structs::ModeTagStruct::Type;

constexpr uint8_t kWaterHeaterModeOff    = 0;
constexpr uint8_t kWaterHeaterModeManual = 1;
constexpr uint8_t kWaterHeaterModeTimed  = 2;

const ModeTagStructType kWaterHeaterModeOffTags[]    = { { .value = to_underlying(WaterHeaterMode::ModeTag::kOff) } };
const ModeTagStructType kWaterHeaterModeManualTags[] = { { .value = to_underlying(WaterHeaterMode::ModeTag::kManual) } };
const ModeTagStructType kWaterHeaterModeTimedTags[]  = { { .value = to_underlying(WaterHeaterMode::ModeTag::kTimed) } };

struct WaterHeaterModeOption
{
    CharSpan label;
    uint8_t value;
    Span<const ModeTagStructType> tags;
};
const WaterHeaterModeOption kWaterHeaterModeOptions[] = {
    { "Manual"_span, kWaterHeaterModeManual, Span<const ModeTagStructType>(kWaterHeaterModeManualTags) },
    { "Off"_span, kWaterHeaterModeOff, Span<const ModeTagStructType>(kWaterHeaterModeOffTags) },
    { "Timed"_span, kWaterHeaterModeTimed, Span<const ModeTagStructType>(kWaterHeaterModeTimedTags) },
};
} // namespace

SimulatedWaterHeater::SimulatedWaterHeater(const Config & config) :
    WaterHeater(config, static_cast<Clusters::WaterHeaterManagement::Delegate &>(*this),
                static_cast<Clusters::ModeBase::AppDelegate &>(*this), static_cast<Clusters::Thermostat::Delegate &>(*this),
                static_cast<Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate &>(*this))
{}

SimulatedWaterHeater::~SimulatedWaterHeater()
{
    mConfig.timerDelegate.CancelTimer(this);
}

CHIP_ERROR SimulatedWaterHeater::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                          EndpointComposition composition)
{
    ReturnErrorOnFailure(WaterHeater::Register(endpoint, provider, composition));

    // Setup initial values
    mTemperature        = kInitialTemperature;
    mHeatingEnabled     = true;
    mBoostState         = BoostStateEnum::kInactive;
    mBoostRemainingTime = 0;
    mHeatDemand         = mHeaterTypes;

    WaterHeaterModeCluster().UpdateCurrentMode(kWaterHeaterModeManual);
    ThermostatCluster().SetLocalTemperature(DataModel::Nullable<temperature>(mTemperature));
    ThermostatCluster().SetSystemMode(SystemModeEnum::kHeat);
    ThermostatCluster().SetControlSequenceOfOperation(ControlSequenceOfOperationEnum::kHeatingOnly);
    bool changed = false;
    SetOccupiedHeatingSetpoint(kFinalTemperature, changed);

    CHIP_ERROR err = mConfig.timerDelegate.StartTimer(this, System::Clock::Seconds32(kStepDurationSeconds));
    if (err != CHIP_NO_ERROR)
    {
        Unregister(provider);
        return err;
    }
    return CHIP_NO_ERROR;
}

void SimulatedWaterHeater::Unregister(CodeDrivenDataModelProvider & provider)
{
    mConfig.timerDelegate.CancelTimer(this);
    WaterHeater::Unregister(provider);
}

void SimulatedWaterHeater::TimerFired()
{
    const bool modeOff   = WaterHeaterModeCluster().GetCurrentMode() == kWaterHeaterModeOff;
    const bool systemOff = mSystemMode == SystemModeEnum::kOff;

    if (mBoostState == BoostStateEnum::kInactive && (modeOff || systemOff) && mHeatingEnabled)
    {
        mHeatingEnabled = false;
        mHeatDemand.ClearAll();
        NotifyHeatDemandAndBoostStateChanged();
    }

    // Handle boost
    if (mBoostState == BoostStateEnum::kActive)
    {
        if (mBoostRemainingTime > kStepDurationSeconds)
        {
            mBoostRemainingTime -= kStepDurationSeconds;
        }
        else
        {
            mBoostRemainingTime = 0;
            ChipLogProgress(AppServer, "WaterHeater: Boost duration elapsed");
            EndBoost();
        }
    }

    // Handle heating
    if (mHeatingEnabled)
    {
        temperature temperatureStep = mBoostState == BoostStateEnum::kActive ? 200 : 100;
        mTemperature                = static_cast<temperature>(mTemperature + temperatureStep);
        ChipLogProgress(AppServer, "WaterHeater: Heating temperature=%" PRId16 "°C", static_cast<int16_t>(mTemperature / 100));
        ThermostatCluster().SetLocalTemperature(DataModel::Nullable<temperature>(mTemperature));
        if (mTemperature >= mOccupiedHeatingSetpoint)
        {
            if (mBoostState == BoostStateEnum::kActive)
            {
                EndBoost();
            }
            else
            {
                mHeatingEnabled = false;
                mHeatDemand.ClearAll();
                NotifyHeatDemandAndBoostStateChanged();
            }
        }
    }
    else
    {
        mTemperature = static_cast<temperature>(std::max(static_cast<temperature>(mTemperature - 100), kInitialTemperature));
        ChipLogProgress(AppServer, "WaterHeater: Cooling temperature=%" PRId16 "°C", static_cast<int16_t>(mTemperature / 100));
        ThermostatCluster().SetLocalTemperature(DataModel::Nullable<temperature>(mTemperature));
        if (mTemperature <= kInitialTemperature && !modeOff && !systemOff)
        {
            mHeatingEnabled = true;
            mHeatDemand     = mHeaterTypes;
            NotifyHeatDemandAndBoostStateChanged();
        }
    }

    LogErrorOnFailure(mConfig.timerDelegate.StartTimer(this, System::Clock::Seconds32(kStepDurationSeconds)));
}

Status SimulatedWaterHeater::HandleBoost(uint32_t duration, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                         Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                         Optional<Percent> targetReheat)
{
    ChipLogProgress(AppServer, "WaterHeater: Boost duration=%" PRIu32 "s", duration);

    mBoostState         = Clusters::WaterHeaterManagement::BoostStateEnum::kActive;
    mBoostRemainingTime = duration;
    mHeatingEnabled     = true;
    mHeatDemand         = mHeaterTypes;

    ThermostatCluster().SetSystemMode(SystemModeEnum::kHeat);

    CHIP_ERROR err =
        GenerateBoostStartedEvent(duration, oneShot, emergencyBoost, temporarySetpoint, targetPercentage, targetReheat);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "WaterHeater: Failed to generate BoostStarted event: %" CHIP_ERROR_FORMAT, err.Format());
    }

    NotifyHeatDemandAndBoostStateChanged();
    return Status::Success;
}

Status SimulatedWaterHeater::HandleCancelBoost()
{
    if (mBoostState == BoostStateEnum::kInactive)
    {
        return Status::Success;
    }

    ChipLogProgress(AppServer, "WaterHeater: CancelBoost");
    EndBoost();
    return Status::Success;
}

BitMask<WaterHeaterHeatSourceBitmap> SimulatedWaterHeater::GetHeaterTypes()
{
    return mHeaterTypes;
}

BitMask<WaterHeaterHeatSourceBitmap> SimulatedWaterHeater::GetHeatDemand()
{
    return mHeatDemand;
}

uint16_t SimulatedWaterHeater::GetTankVolume()
{
    return 0;
}

Energy_mWh SimulatedWaterHeater::GetEstimatedHeatRequired()
{
    return 0;
}

Percent SimulatedWaterHeater::GetTankPercentage()
{
    return 0;
}

BoostStateEnum SimulatedWaterHeater::GetBoostState()
{
    return mBoostState;
}

void SimulatedWaterHeater::EndBoost()
{
    mBoostState         = BoostStateEnum::kInactive;
    mBoostRemainingTime = 0;

    const bool modeOff   = WaterHeaterModeCluster().GetCurrentMode() == kWaterHeaterModeOff;
    const bool systemOff = mSystemMode == SystemModeEnum::kOff;

    if (modeOff || systemOff || mTemperature >= mOccupiedHeatingSetpoint)
    {
        mHeatingEnabled = false;
        mHeatDemand.ClearAll();
    }
    else
    {
        mHeatingEnabled = true;
        mHeatDemand     = mHeaterTypes;
    }

    CHIP_ERROR err = GenerateBoostEndedEvent();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "WaterHeater: Failed to generate BoostEnded event: %" CHIP_ERROR_FORMAT, err.Format());
    }

    NotifyHeatDemandAndBoostStateChanged();
}

void SimulatedWaterHeater::NotifyHeatDemandAndBoostStateChanged()
{
    VerifyOrReturn(mProvider != nullptr);
    mProvider->NotifyAttributeChanged({ SingleEndpoint::GetEndpointId(), WaterHeaterManagement::Id, HeatDemand::Id },
                                      DataModel::AttributeChangeType::kReportable);
    mProvider->NotifyAttributeChanged({ SingleEndpoint::GetEndpointId(), WaterHeaterManagement::Id, BoostState::Id },
                                      DataModel::AttributeChangeType::kReportable);
}

// Clusters::Thermostat::Delegate
FabricTable & SimulatedWaterHeater::GetFabricTable() const
{
    return mConfig.fabricTable;
}

SystemModeEnum SimulatedWaterHeater::GetSystemMode() const
{
    return mSystemMode;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::SetSystemMode(SystemModeEnum systemMode, bool & changed)
{
    changed = false;
    if (systemMode != SystemModeEnum::kOff && systemMode != SystemModeEnum::kHeat)
    {
        return Status::ConstraintError;
    }

    if (mSystemMode == systemMode)
    {
        return Status::Success;
    }

    mSystemMode = systemMode;
    changed     = true;

    if (systemMode == SystemModeEnum::kOff)
    {
        if (mBoostState == BoostStateEnum::kInactive && mHeatingEnabled)
        {
            mHeatingEnabled = false;
            mHeatDemand.ClearAll();
            NotifyHeatDemandAndBoostStateChanged();
        }
    }
    else if (systemMode == SystemModeEnum::kHeat)
    {
        const bool modeOff = WaterHeaterModeCluster().GetCurrentMode() == kWaterHeaterModeOff;
        if (!modeOff && mTemperature < mOccupiedHeatingSetpoint && !mHeatingEnabled)
        {
            mHeatingEnabled = true;
            mHeatDemand     = mHeaterTypes;
            NotifyHeatDemandAndBoostStateChanged();
        }
    }

    return Status::Success;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::GetRunningMode(ThermostatRunningModeEnum & runningMode) const
{
    return Status::UnsupportedAttribute;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::SetRunningMode(ThermostatRunningModeEnum runningMode, bool & changed)
{
    return Status::UnsupportedAttribute;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::GetRunningState(BitMask<RelayStateBitmap> & runningState) const
{
    return Status::UnsupportedAttribute;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::SetRunningState(BitMask<RelayStateBitmap> runningState, bool & changed)
{
    return Status::UnsupportedAttribute;
}

ControlSequenceOfOperationEnum SimulatedWaterHeater::GetControlSequenceOfOperation() const
{
    return mControlSequenceOfOperation;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::SetControlSequenceOfOperation(ControlSequenceOfOperationEnum seq,
                                                                                        bool & changed)
{
    changed = false;
    if (mControlSequenceOfOperation == seq)
    {
        return Status::Success;
    }

    mControlSequenceOfOperation = seq;
    changed                     = true;
    return Status::Success;
}

DataModel::Nullable<temperature> SimulatedWaterHeater::GetLocalTemperature() const
{
    return mLocalTemperature;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::SetLocalTemperature(DataModel::Nullable<temperature> temp, bool & changed)
{
    changed = false;
    if (mLocalTemperature == temp)
    {
        return Status::Success;
    }
    mLocalTemperature = temp;
    if (!temp.IsNull())
    {
        mTemperature = temp.Value();
    }
    changed = true;
    return Status::Success;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::SetRemoteSensing(BitMask<RemoteSensingBitmap> sensing, bool & changed)
{
    return Status::UnsupportedAttribute;
}

// Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate
Protocols::InteractionModel::Status SimulatedWaterHeater::GetOccupiedHeatingSetpoint(temperature & occupiedHeatingSetpoint) const
{
    occupiedHeatingSetpoint = mOccupiedHeatingSetpoint;
    return Status::Success;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::SetOccupiedHeatingSetpoint(temperature occupiedHeatingSetpoint,
                                                                                     bool & changed)
{
    changed = false;
    if (mOccupiedHeatingSetpoint == occupiedHeatingSetpoint)
    {
        return Status::Success;
    }

    mOccupiedHeatingSetpoint = occupiedHeatingSetpoint;
    changed                  = true;

    if (mHeatingEnabled && mTemperature >= mOccupiedHeatingSetpoint && mBoostState == BoostStateEnum::kInactive)
    {
        mHeatingEnabled = false;
        mHeatDemand.ClearAll();
        NotifyHeatDemandAndBoostStateChanged();
    }
    else if (!mHeatingEnabled && mTemperature < mOccupiedHeatingSetpoint && mSystemMode == SystemModeEnum::kHeat)
    {
        const bool modeOff = WaterHeaterModeCluster().GetCurrentMode() == kWaterHeaterModeOff;
        if (!modeOff)
        {
            mHeatingEnabled = true;
            mHeatDemand     = mHeaterTypes;
            NotifyHeatDemandAndBoostStateChanged();
        }
    }

    return Status::Success;
}

// Clusters::ModeBase::AppDelegate
CHIP_ERROR SimulatedWaterHeater::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedWaterHeater::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kWaterHeaterModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kWaterHeaterModeOptions[modeIndex].label, label);
}

CHIP_ERROR SimulatedWaterHeater::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kWaterHeaterModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = kWaterHeaterModeOptions[modeIndex].value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedWaterHeater::GetModeTagsByIndex(uint8_t modeIndex,
                                                    DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kWaterHeaterModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    const auto & tags = kWaterHeaterModeOptions[modeIndex].tags;
    VerifyOrReturnError(modeTags.size() >= tags.size(), CHIP_ERROR_INVALID_ARGUMENT);
    std::copy(tags.begin(), tags.end(), modeTags.begin());
    modeTags.reduce_size(tags.size());
    return CHIP_NO_ERROR;
}

void SimulatedWaterHeater::HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    bool modeFound = false;
    for (const auto & option : kWaterHeaterModeOptions)
    {
        if (option.value == newMode)
        {
            modeFound = true;
            break;
        }
    }
    if (!modeFound)
    {
        response.status = to_underlying(ModeBase::StatusCode::kUnsupportedMode);
        return;
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);

    if (newMode == kWaterHeaterModeOff)
    {
        if (mBoostState == BoostStateEnum::kInactive && mHeatingEnabled)
        {
            mHeatingEnabled = false;
            mHeatDemand.ClearAll();
            NotifyHeatDemandAndBoostStateChanged();
        }
    }
    else
    {
        if (mSystemMode == SystemModeEnum::kHeat && mTemperature < mOccupiedHeatingSetpoint && !mHeatingEnabled)
        {
            mHeatingEnabled = true;
            mHeatDemand     = mHeaterTypes;
            NotifyHeatDemandAndBoostStateChanged();
        }
    }
}
} // namespace chip::app
