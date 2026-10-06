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
#include <limits>

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

const ModeTagStructType kWaterHeaterModeOffTags[]    = { { .value = to_underlying(WaterHeaterMode::ModeTag::kOff) } };
const ModeTagStructType kWaterHeaterModeManualTags[] = { { .value = to_underlying(WaterHeaterMode::ModeTag::kManual) } };

struct WaterHeaterModeOption
{
    CharSpan label;
    uint8_t value;
    Span<const ModeTagStructType> tags;
};
const WaterHeaterModeOption kWaterHeaterModeOptions[] = {
    { "Manual"_span, kWaterHeaterModeManual, Span<const ModeTagStructType>(kWaterHeaterModeManualTags) },
    { "Off"_span, kWaterHeaterModeOff, Span<const ModeTagStructType>(kWaterHeaterModeOffTags) },
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
    const uint8_t currentMode = WaterHeaterModeCluster().GetCurrentMode();
    ChipLogProgress(AppServer, "WaterHeater: Startup in mode %u", currentMode);

    // SystemMode is loaded from storage in Startup(), however we also have CurrentMode attribute stored (handled in the cluster
    // code) so we need to find a way to reconcile the two, in this implementation we set the SystemMode to the initial mode based
    // on the CurrentMode.
    const auto initialSystemMode = (currentMode == kWaterHeaterModeManual) ? SystemModeEnum::kHeat : SystemModeEnum::kOff;
    ThermostatCluster().SetSystemMode(initialSystemMode);

    EvaluateHeatingDemand();

    CHIP_ERROR err = mConfig.timerDelegate.StartTimer(this, System::Clock::Seconds32(kStepDurationSeconds));
    if (err != CHIP_NO_ERROR)
    {
        Unregister(provider);
        return err;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedWaterHeater::Startup(ServerClusterContext & context)
{
    VerifyOrReturnError(mAttributeStorage == nullptr, CHIP_NO_ERROR);
    mAttributeStorage = &context.attributeStorage;
    AttributePersistence persistence(*mAttributeStorage);
    persistence.LoadNativeEndianValue({ GetEndpointId(), Clusters::Thermostat::Id, Thermostat::Attributes::SystemMode::Id },
                                      mSystemMode, SystemModeEnum::kOff);
    if (mSystemMode != SystemModeEnum::kOff && mSystemMode != SystemModeEnum::kHeat)
    {
        mSystemMode = SystemModeEnum::kOff;
    }
    persistence.LoadNativeEndianValue(
        { GetEndpointId(), Clusters::Thermostat::Id, Thermostat::Attributes::OccupiedHeatingSetpoint::Id },
        mOccupiedHeatingSetpoint, kFinalTemperature);
    if (mOccupiedHeatingSetpoint < kMinTemperature || mOccupiedHeatingSetpoint > kMaxTemperature)
    {
        mOccupiedHeatingSetpoint = kFinalTemperature;
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
    if (mBoostState == BoostStateEnum::kInactive && !IsNormalHeatingPermitted())
    {
        SetHeatingEnabled(false);
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

    const temperature currentTemp = mLocalTemperature.ValueOr(kInitialTemperature);
    const temperature target      = GetHeatingTargetTemperature();

    // Handle heating
    if (mHeatingEnabled)
    {
        const int32_t temperatureStep = mBoostState == BoostStateEnum::kActive ? 200 : 100;
        const int32_t ceiling         = std::max(currentTemp, target);
        const int32_t nextTemp        = std::min(static_cast<int32_t>(currentTemp) + temperatureStep, ceiling);
        const temperature newTemp     = static_cast<temperature>(nextTemp);

        ChipLogProgress(AppServer, "WaterHeater: Heating temperature=%" PRId16 "°C", static_cast<int16_t>(newTemp / 100));
        ThermostatCluster().SetLocalTemperature(DataModel::MakeNullable(newTemp));
        if (newTemp >= target)
        {
            if (mBoostState == BoostStateEnum::kActive && mBoostOneShot)
            {
                EndBoost();
            }
            else
            {
                SetHeatingEnabled(false);
            }
        }
    }
    else
    {
        const int32_t nextTemp    = std::max(static_cast<int32_t>(currentTemp) - 100, static_cast<int32_t>(kInitialTemperature));
        const temperature newTemp = static_cast<temperature>(nextTemp);

        ChipLogProgress(AppServer, "WaterHeater: Cooling temperature=%" PRId16 "°C", static_cast<int16_t>(newTemp / 100));
        ThermostatCluster().SetLocalTemperature(DataModel::MakeNullable(newTemp));
        if (newTemp <= kInitialTemperature && newTemp < target &&
            (mBoostState == BoostStateEnum::kActive || IsNormalHeatingPermitted()))
        {
            SetHeatingEnabled(true);
        }
    }

    LogErrorOnFailure(mConfig.timerDelegate.StartTimer(this, System::Clock::Seconds32(kStepDurationSeconds)));
}

Status SimulatedWaterHeater::HandleBoost(uint32_t duration, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                         Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                         Optional<Percent> targetReheat)
{
    if (temporarySetpoint.HasValue())
    {
        if (temporarySetpoint.Value() < kInitialTemperature || temporarySetpoint.Value() > kMaxTemperature)
        {
            ChipLogError(AppServer, "WaterHeater: Boost temporarySetpoint out of range: %" PRId16, temporarySetpoint.Value());
            return Status::ConstraintError;
        }
    }

    ChipLogProgress(AppServer, "WaterHeater: Boost duration=%" PRIu32 "s", duration);

    mBoostState             = Clusters::WaterHeaterManagement::BoostStateEnum::kActive;
    mBoostRemainingTime     = duration;
    mBoostOneShot           = oneShot.ValueOr(false);
    mBoostTemporarySetpoint = temporarySetpoint.HasValue() ? std::make_optional(temporarySetpoint.Value()) : std::nullopt;
    SetHeatingEnabled(true);

    LogErrorOnFailure(
        GenerateBoostStartedEvent(duration, oneShot, emergencyBoost, temporarySetpoint, targetPercentage, targetReheat));

    NotifyBoostStateChanged();
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
    mBoostOneShot       = false;
    mBoostTemporarySetpoint.reset();

    EvaluateHeatingDemand();

    LogErrorOnFailure(GenerateBoostEndedEvent());

    NotifyBoostStateChanged();
}

void SimulatedWaterHeater::SetHeatingEnabled(bool enabled)
{
    if (mHeatingEnabled == enabled)
    {
        return;
    }
    mHeatingEnabled = enabled;
    mHeatDemand     = enabled ? mHeaterTypes : BitMask<WaterHeaterHeatSourceBitmap>();
    NotifyHeatDemandChanged();
}

void SimulatedWaterHeater::NotifyHeatDemandChanged()
{
    VerifyOrReturn(mProvider != nullptr);
    mProvider->NotifyAttributeChanged({ SingleEndpoint::GetEndpointId(), WaterHeaterManagement::Id, HeatDemand::Id },
                                      DataModel::AttributeChangeType::kReportable);
}

void SimulatedWaterHeater::NotifyBoostStateChanged()
{
    VerifyOrReturn(mProvider != nullptr);
    mProvider->NotifyAttributeChanged({ SingleEndpoint::GetEndpointId(), WaterHeaterManagement::Id, BoostState::Id },
                                      DataModel::AttributeChangeType::kReportable);
}

bool SimulatedWaterHeater::IsModeOff(std::optional<uint8_t> mode)
{
    return mode.value_or(WaterHeaterModeCluster().GetCurrentMode()) == kWaterHeaterModeOff;
}

bool SimulatedWaterHeater::IsNormalHeatingPermitted(std::optional<uint8_t> mode)
{
    return (mSystemMode == SystemModeEnum::kHeat) && !IsModeOff(mode);
}

temperature SimulatedWaterHeater::GetHeatingTargetTemperature() const
{
    if (mBoostState == BoostStateEnum::kActive && mBoostTemporarySetpoint.has_value())
    {
        return mBoostTemporarySetpoint.value();
    }
    return mOccupiedHeatingSetpoint;
}

void SimulatedWaterHeater::EvaluateHeatingDemand(std::optional<uint8_t> pendingMode)
{
    if (mBoostState == BoostStateEnum::kActive)
    {
        return;
    }

    const temperature currentTemp = mLocalTemperature.ValueOr(kInitialTemperature);
    const bool shouldHeat         = IsNormalHeatingPermitted(pendingMode) && (currentTemp < mOccupiedHeatingSetpoint);
    SetHeatingEnabled(shouldHeat);
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
    VerifyOrReturnError(systemMode == SystemModeEnum::kOff || systemMode == SystemModeEnum::kHeat, Status::ConstraintError);
    VerifyOrReturnError(mSystemMode != systemMode, Status::Success);
    VerifyOrReturnError(mAttributeStorage != nullptr, Status::Failure);
    AttributePersistence persistence(*mAttributeStorage);
    VerifyOrReturnError(
        persistence.StoreNativeEndianValue({ GetEndpointId(), Clusters::Thermostat::Id, Thermostat::Attributes::SystemMode::Id },
                                           systemMode) == CHIP_NO_ERROR,
        Status::Failure);

    mSystemMode = systemMode;
    changed     = true;

    if (!mIsSyncingMode)
    {
        mIsSyncingMode           = true;
        const uint8_t targetMode = (systemMode == SystemModeEnum::kHeat) ? kWaterHeaterModeManual : kWaterHeaterModeOff;
        WaterHeaterModeCluster().UpdateCurrentMode(targetMode);
        mIsSyncingMode = false;

        EvaluateHeatingDemand();
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
    return ControlSequenceOfOperationEnum::kHeatingOnly;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::SetControlSequenceOfOperation(ControlSequenceOfOperationEnum seq,
                                                                                        bool & changed)
{
    changed = false;
    // This device has only heating capability.
    return seq == ControlSequenceOfOperationEnum::kHeatingOnly ? Status::Success : Status::ConstraintError;
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
    changed           = true;
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
    VerifyOrReturnError(occupiedHeatingSetpoint >= kMinTemperature && occupiedHeatingSetpoint <= kMaxTemperature,
                        Status::ConstraintError);
    VerifyOrReturnError(mOccupiedHeatingSetpoint != occupiedHeatingSetpoint, Status::Success);
    VerifyOrReturnError(mAttributeStorage != nullptr, Status::Failure);

    AttributePersistence persistence(*mAttributeStorage);
    VerifyOrReturnError(persistence.StoreNativeEndianValue(
                            { GetEndpointId(), Clusters::Thermostat::Id, Thermostat::Attributes::OccupiedHeatingSetpoint::Id },
                            occupiedHeatingSetpoint) == CHIP_NO_ERROR,
                        Status::Failure);

    mOccupiedHeatingSetpoint = occupiedHeatingSetpoint;
    changed                  = true;

    EvaluateHeatingDemand();
    return Status::Success;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::GetAbsMinHeatSetpointLimit(temperature & absMinHeatSetpointLimit) const
{
    absMinHeatSetpointLimit = kMinTemperature;
    return Status::Success;
}

Protocols::InteractionModel::Status SimulatedWaterHeater::GetAbsMaxHeatSetpointLimit(temperature & absMaxHeatSetpointLimit) const
{
    absMaxHeatSetpointLimit = kMaxTemperature;
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

    if (!mIsSyncingMode)
    {
        mIsSyncingMode              = true;
        const auto targetSystemMode = (newMode == kWaterHeaterModeManual) ? SystemModeEnum::kHeat : SystemModeEnum::kOff;
        ThermostatCluster().SetSystemMode(targetSystemMode);
        mIsSyncingMode = false;
    }

    EvaluateHeatingDemand(newMode);
}
} // namespace chip::app
