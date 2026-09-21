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
    { "Off"_span, kWaterHeaterModeOff, Span<const ModeTagStructType>(kWaterHeaterModeOffTags) },
    { "Manual"_span, kWaterHeaterModeManual, Span<const ModeTagStructType>(kWaterHeaterModeManualTags) },
    { "Timed"_span, kWaterHeaterModeTimed, Span<const ModeTagStructType>(kWaterHeaterModeTimedTags) },
};
} // namespace

SimulatedWaterHeater::SimulatedWaterHeater(const Config & config) :
    SimulatedWaterHeaterDelegates(config.fabricTable),
    WaterHeater(config, *this, *this, thermostatDelegate, thermostatSetpointsDelegate)
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
    mHeatDemand.ClearAll();

    ThermostatCluster().SetLocalTemperature(DataModel::Nullable<temperature>(mTemperature));
    ThermostatCluster().SetSystemMode(SystemModeEnum::kHeat);
    ThermostatCluster().SetControlSequenceOfOperation(ControlSequenceOfOperationEnum::kHeatingOnly);
    bool changed = false;
    GetDelegate<ThermostatSetpointsDelegate>().SetOccupiedHeatingSetpoint(kFinalTemperature, changed);

    ReturnErrorOnFailure(mConfig.timerDelegate.StartTimer(this, System::Clock::Seconds32(kStepDurationSeconds)));
    return CHIP_NO_ERROR;
}

void SimulatedWaterHeater::Unregister(CodeDrivenDataModelProvider & provider)
{
    mConfig.timerDelegate.CancelTimer(this);
    WaterHeater::Unregister(provider);
}

void SimulatedWaterHeater::TimerFired()
{
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
        if (mTemperature >= kFinalTemperature)
        {
            ThermostatCluster().SetSystemMode(SystemModeEnum::kOff);
            mHeatingEnabled = false;
        }
    }
    else
    {
        mTemperature = static_cast<temperature>(mTemperature - 100);
        ChipLogProgress(AppServer, "WaterHeater: Cooling temperature=%" PRId16 "°C", static_cast<int16_t>(mTemperature / 100));
        ThermostatCluster().SetLocalTemperature(DataModel::Nullable<temperature>(mTemperature));
        if (mTemperature <= kInitialTemperature)
        {
            ThermostatCluster().SetSystemMode(SystemModeEnum::kHeat);
            mHeatingEnabled = true;
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
    mHeatDemand         = mHeaterTypes;

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
    mHeatDemand.ClearAll();

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
    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}
} // namespace chip::app
