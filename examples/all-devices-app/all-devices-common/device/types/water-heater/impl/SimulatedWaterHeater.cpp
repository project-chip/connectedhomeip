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

using ModeTagStructType = Clusters::detail::Structs::ModeTagStruct::Type;

// Simulation tuning. Temperatures are in units of 0.01 °C.
constexpr uint32_t kStepDurationSeconds = 3;
constexpr temperature kHeatingStep      = 100; // per step, normal heating
constexpr temperature kBoostHeatingStep = 200; // per step, while boosting
constexpr temperature kCoolingStep      = 100; // per step, while not heating
constexpr temperature kReheatHysteresis = 200; // re-heat once this far below the target
constexpr BitMask<WaterHeaterHeatSourceBitmap> kHeaterTypes(WaterHeaterHeatSourceBitmap::kImmersionElement1);

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
    { "Off"_span, kWaterHeaterModeOff, Span<const ModeTagStructType>(kWaterHeaterModeOffTags) },
    { "Manual"_span, kWaterHeaterModeManual, Span<const ModeTagStructType>(kWaterHeaterModeManualTags) },
};

// This cluster implementation does not support any Optional features for the WaterHeaterManagement cluster.
constexpr BitMask<Clusters::WaterHeaterManagement::Feature> kWhmFeatures = {};
// Only the Heating feature is supported for the Thermostat cluster.
constexpr Clusters::Thermostat::OptionalAttributes GetThermostatOptionalAttributes()
{
    Clusters::Thermostat::OptionalAttributes attrs;
    attrs.AbsMinHeatSetpointLimit = true;
    attrs.AbsMaxHeatSetpointLimit = true;
    return attrs;
}
constexpr Clusters::Thermostat::OptionalAttributes kThermostatOptionalAttributes = GetThermostatOptionalAttributes();

bool IsWithinAbsoluteHeatSetpointLimits(temperature value)
{
    return value >= SimulatedWaterHeater::kMinTemperature && value <= SimulatedWaterHeater::kMaxTemperature;
}
} // namespace

SimulatedWaterHeater::SimulatedWaterHeater(TimerDelegate & timerDelegate, FabricTable & fabricTable,
                                           DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider) :
    WaterHeater(WaterHeater::Config{ .timerDelegate                 = timerDelegate,
                                     .diagnosticDataProvider        = diagnosticDataProvider,
                                     .whmFeatures                   = kWhmFeatures,
                                     .waterHeaterManagementDelegate = *this,
                                     .thermostatOptionalAttributes  = kThermostatOptionalAttributes,
                                     .thermostatDelegate            = *this,
                                     .heatingDelegate               = *this,
                                     .waterHeaterModeDelegate       = *this }),
    mFabricTable(fabricTable)
{}

SimulatedWaterHeater::~SimulatedWaterHeater()
{
    GetTimerDelegate().CancelTimer(this);
}

CHIP_ERROR SimulatedWaterHeater::Startup(ServerClusterContext & context)
{
    // ThermostatCluster starts both delegate interfaces implemented by this object (Thermostat::Delegate and
    // ThermostatHeatingSetpoints::Delegate), so this is invoked twice per cluster startup. Only the first call
    // initializes; mAttributeStorage is cleared again in Shutdown().
    if (mAttributeStorage != nullptr)
    {
        return CHIP_NO_ERROR;
    }
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
        mOccupiedHeatingSetpoint, kDefaultHeatingSetpoint);
    if (!IsWithinAbsoluteHeatSetpointLimits(mOccupiedHeatingSetpoint))
    {
        mOccupiedHeatingSetpoint = kDefaultHeatingSetpoint;
    }
    // SystemMode is loaded from storage in Startup(), however we also have CurrentMode attribute stored (handled in the cluster
    // code) so we need to find a way to reconcile the two. Defer mode reconciliation until all clusters on the endpoint
    // are constructed and started.
    mStartupReconciliationPending = true;
    return GetTimerDelegate().StartTimer(this, System::Clock::Milliseconds32(0));
}

void SimulatedWaterHeater::Shutdown(ClusterShutdownType type)
{
    GetTimerDelegate().CancelTimer(this);
    mStartupReconciliationPending = false;
    mAttributeStorage             = nullptr;
}

void SimulatedWaterHeater::Unregister(CodeDrivenDataModelProvider & provider)
{
    GetTimerDelegate().CancelTimer(this);
    WaterHeater::Unregister(provider);
}

void SimulatedWaterHeater::TimerFired()
{
    if (mStartupReconciliationPending)
    {
        mStartupReconciliationPending = false;

        const uint8_t currentMode = WaterHeaterModeCluster().GetCurrentMode();
        ChipLogProgress(AppServer, "WaterHeater: Startup in mode %u", currentMode);

        const auto initialSystemMode = (currentMode == kWaterHeaterModeManual) ? SystemModeEnum::kHeat : SystemModeEnum::kOff;
        ThermostatCluster().SetSystemMode(initialSystemMode);

        EvaluateHeatingDemand();

        LogErrorOnFailure(GetTimerDelegate().StartTimer(this, System::Clock::Seconds32(kStepDurationSeconds)));
        return;
    }

    if (mBoostState == BoostStateEnum::kInactive && !IsNormalHeatingPermitted())
    {
        SetHeatingEnabled(false);
    }

    // Handle boost
    if (mBoostState == BoostStateEnum::kActive)
    {
        if (mBoostOneShot && mLocalTemperature.ValueOr(kAmbientTemperature) >= GetHeatingTargetTemperature())
        {
            ChipLogProgress(AppServer, "WaterHeater: One-shot boost target reached");
            EndBoost();
        }
        else if (mBoostRemainingTime > kStepDurationSeconds)
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

    const temperature currentTemp = mLocalTemperature.ValueOr(kAmbientTemperature);
    const temperature target      = GetHeatingTargetTemperature();

    // Only report (and log) actual changes, so an idle heater at ambient temperature stays quiet.
    auto updateTemperature = [&](const char * phase, temperature newTemp) {
        VerifyOrReturn(newTemp != currentTemp);
        ChipLogProgress(AppServer, "WaterHeater: %s temperature=%" PRId16 "°C", phase, static_cast<int16_t>(newTemp / 100));
        ThermostatCluster().SetLocalTemperature(DataModel::MakeNullable(newTemp));
    };

    // Handle heating
    if (mHeatingEnabled)
    {
        const int32_t temperatureStep = (mBoostState == BoostStateEnum::kActive) ? kBoostHeatingStep : kHeatingStep;
        const int32_t ceiling         = std::max(currentTemp, target);
        const int32_t nextTemp        = std::min(static_cast<int32_t>(currentTemp) + temperatureStep, ceiling);
        const temperature newTemp     = static_cast<temperature>(nextTemp);

        updateTemperature("Heating", newTemp);
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
        const int32_t nextTemp =
            std::max(static_cast<int32_t>(currentTemp) - kCoolingStep, static_cast<int32_t>(kAmbientTemperature));
        const temperature newTemp = static_cast<temperature>(nextTemp);

        updateTemperature("Cooling", newTemp);
        // Water never cools below ambient, so clamp the re-heat threshold to it.
        const int32_t reheatThreshold =
            std::max(static_cast<int32_t>(target) - kReheatHysteresis, static_cast<int32_t>(kAmbientTemperature));
        if (newTemp < target && newTemp <= reheatThreshold &&
            (mBoostState == BoostStateEnum::kActive || IsNormalHeatingPermitted()))
        {
            SetHeatingEnabled(true);
        }
    }

    LogErrorOnFailure(GetTimerDelegate().StartTimer(this, System::Clock::Seconds32(kStepDurationSeconds)));
}

Status SimulatedWaterHeater::HandleBoost(uint32_t duration, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                         Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                         Optional<Percent> targetReheat)
{
    // Use the same absolute limits as OccupiedHeatingSetpoint (advertised via AbsMin/AbsMaxHeatSetpointLimit).
    if (temporarySetpoint.HasValue() && !IsWithinAbsoluteHeatSetpointLimits(temporarySetpoint.Value()))
    {
        ChipLogError(AppServer, "WaterHeater: Boost temporarySetpoint out of range: %" PRId16, temporarySetpoint.Value());
        return Status::ConstraintError;
    }

    ChipLogProgress(AppServer, "WaterHeater: Boost duration=%" PRIu32 "s", duration);

    const bool wasBoostActive = (mBoostState == BoostStateEnum::kActive);

    mBoostState             = Clusters::WaterHeaterManagement::BoostStateEnum::kActive;
    mBoostRemainingTime     = duration;
    mBoostOneShot           = oneShot.ValueOr(false);
    mBoostTemporarySetpoint = temporarySetpoint.std_optional();
    EvaluateHeatingDemand();

    LogErrorOnFailure(
        GenerateBoostStartedEvent(duration, oneShot, emergencyBoost, temporarySetpoint, targetPercentage, targetReheat));

    if (!wasBoostActive)
    {
        NotifyBoostStateChanged();
    }
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
    return kHeaterTypes;
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
    mHeatDemand     = enabled ? kHeaterTypes : BitMask<WaterHeaterHeatSourceBitmap>();
    NotifyHeatDemandChanged();
}

void SimulatedWaterHeater::NotifyHeatDemandChanged()
{
    WaterHeaterManagementCluster().NotifyAttributeChanged(HeatDemand::Id);
}

void SimulatedWaterHeater::NotifyBoostStateChanged()
{
    WaterHeaterManagementCluster().NotifyAttributeChanged(BoostState::Id);
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
    const temperature currentTemp = mLocalTemperature.ValueOr(kAmbientTemperature);
    const bool heatingPermitted   = (mBoostState == BoostStateEnum::kActive) || IsNormalHeatingPermitted(pendingMode);
    SetHeatingEnabled(heatingPermitted && (currentTemp < GetHeatingTargetTemperature()));
}

// Clusters::Thermostat::Delegate
FabricTable & SimulatedWaterHeater::GetFabricTable() const
{
    return mFabricTable;
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
    VerifyOrReturnError(IsWithinAbsoluteHeatSetpointLimits(occupiedHeatingSetpoint), Status::ConstraintError);
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
    VerifyOrReturnError(modeTags.size() >= tags.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    std::copy(tags.begin(), tags.end(), modeTags.begin());
    modeTags.reduce_size(tags.size());
    return CHIP_NO_ERROR;
}

void SimulatedWaterHeater::HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    // ModeBaseCluster only calls this for supported modes that differ from CurrentMode.
    if (!mIsSyncingMode)
    {
        mIsSyncingMode              = true;
        const auto targetSystemMode = (newMode == kWaterHeaterModeManual) ? SystemModeEnum::kHeat : SystemModeEnum::kOff;
        const auto status           = ThermostatCluster().SetSystemMode(targetSystemMode);
        mIsSyncingMode              = false;

        if (status != Protocols::InteractionModel::Status::Success)
        {
            response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
            return;
        }
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
    EvaluateHeatingDemand(newMode);
}
} // namespace chip::app
