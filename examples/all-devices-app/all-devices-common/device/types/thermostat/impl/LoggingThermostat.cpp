/*
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

#include <device/types/thermostat/impl/LoggingThermostat.h>

#include <app/persistence/AttributePersistence.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

namespace {
namespace thermostat = Clusters::Thermostat;
using Protocols::InteractionModel::Status;

bool IsSupportedMode(thermostat::SystemModeEnum mode)
{
    return mode == thermostat::SystemModeEnum::kOff || mode == thermostat::SystemModeEnum::kHeat ||
        mode == thermostat::SystemModeEnum::kCool;
}

Thermostat::Context MakeLoggingContext(const Thermostat::Context & context)
{
    Thermostat::Context loggingContext                            = context;
    loggingContext.optionalAttributes.ThermostatRunningState      = true;
    loggingContext.optionalAttributes.RemoteSensing               = true;
    loggingContext.optionalAttributes.LocalTemperatureCalibration = true;
    return loggingContext;
}
} // namespace

LoggingThermostat::LoggingThermostat(const Context & context) :
    Thermostat(MakeLoggingContext(context), *this, *this, *this, *this, *this), mFabricTable(context.fabricTable)
{}

CHIP_ERROR LoggingThermostat::Startup(ServerClusterContext & context)
{
    // Startup is called for the main delegate and both setpoint delegates.
    VerifyOrReturnError(mAttributeStorage == nullptr, CHIP_NO_ERROR);
    mAttributeStorage = &context.attributeStorage;
    AttributePersistence persistence(*mAttributeStorage);
    persistence.LoadNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::SystemMode::Id }, mSystemMode,
                                      thermostat::SystemModeEnum::kOff);
    persistence.LoadNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::OccupiedHeatingSetpoint::Id },
                                      mHeatingSetpoint, thermostat::kDefaultHeatingSetpoint);
    persistence.LoadNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::OccupiedCoolingSetpoint::Id },
                                      mCoolingSetpoint, thermostat::kDefaultCoolingSetpoint);
    if (!IsSupportedMode(mSystemMode))
    {
        mSystemMode = thermostat::SystemModeEnum::kOff;
    }
    if (mHeatingSetpoint < thermostat::kDefaultAbsMinHeatSetpointLimit ||
        mHeatingSetpoint > thermostat::kDefaultAbsMaxHeatSetpointLimit)
    {
        mHeatingSetpoint = thermostat::kDefaultHeatingSetpoint;
    }
    if (mCoolingSetpoint < thermostat::kDefaultAbsMinCoolSetpointLimit ||
        mCoolingSetpoint > thermostat::kDefaultAbsMaxCoolSetpointLimit)
    {
        mCoolingSetpoint = thermostat::kDefaultCoolingSetpoint;
    }
    UpdateSimulatedRunningState();
    return CHIP_NO_ERROR;
}

void LoggingThermostat::Shutdown(ClusterShutdownType type)
{
    mAttributeStorage = nullptr;
}

void LoggingThermostat::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(AppServer, "Thermostat: Identify started");
}

void LoggingThermostat::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(AppServer, "Thermostat: Identify stopped");
}

Status LoggingThermostat::SetLocalTemperature(DataModel::Nullable<int16_t> value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value.IsNull() || value.Value() >= -27315, Status::ConstraintError);
    changed           = mLocalTemperature != value;
    mLocalTemperature = value;
    if (changed)
    {
        UpdateSimulatedRunningState();
    }
    return Status::Success;
}

Status LoggingThermostat::SetSystemMode(thermostat::SystemModeEnum value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(IsSupportedMode(value), Status::InvalidValue);
    VerifyOrReturnValue(mSystemMode != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    CHIP_ERROR err =
        AttributePersistence(*mAttributeStorage)
            .StoreNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::SystemMode::Id }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Thermostat: persisting SystemMode failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    mSystemMode = value;
    changed     = true;
    ChipLogProgress(AppServer, "Thermostat: SystemMode changed to %u", static_cast<unsigned>(value));
    UpdateSimulatedRunningState();
    return Status::Success;
}

Status LoggingThermostat::SetControlSequenceOfOperation(thermostat::ControlSequenceOfOperationEnum value, bool & changed)
{
    changed = false;
    return value == GetControlSequenceOfOperation() ? Status::Success : Status::ConstraintError;
}

Status LoggingThermostat::GetOccupiedHeatingSetpoint(int16_t & value) const
{
    value = mHeatingSetpoint;
    return Status::Success;
}

Status LoggingThermostat::SetOccupiedHeatingSetpoint(int16_t value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value >= thermostat::kDefaultAbsMinHeatSetpointLimit &&
                            value <= thermostat::kDefaultAbsMaxHeatSetpointLimit,
                        Status::ConstraintError);
    VerifyOrReturnValue(mHeatingSetpoint != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    CHIP_ERROR err = AttributePersistence(*mAttributeStorage)
                         .StoreNativeEndianValue(
                             { GetEndpointId(), thermostat::Id, thermostat::Attributes::OccupiedHeatingSetpoint::Id }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Thermostat: persisting heating setpoint failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    mHeatingSetpoint = value;
    changed          = true;
    ChipLogProgress(AppServer, "Thermostat: heating setpoint changed to %d", value);
    UpdateSimulatedRunningState();
    return Status::Success;
}

Status LoggingThermostat::GetOccupiedCoolingSetpoint(int16_t & value) const
{
    value = mCoolingSetpoint;
    return Status::Success;
}

Status LoggingThermostat::SetOccupiedCoolingSetpoint(int16_t value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value >= thermostat::kDefaultAbsMinCoolSetpointLimit &&
                            value <= thermostat::kDefaultAbsMaxCoolSetpointLimit,
                        Status::ConstraintError);
    VerifyOrReturnValue(mCoolingSetpoint != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    CHIP_ERROR err = AttributePersistence(*mAttributeStorage)
                         .StoreNativeEndianValue(
                             { GetEndpointId(), thermostat::Id, thermostat::Attributes::OccupiedCoolingSetpoint::Id }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Thermostat: persisting cooling setpoint failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    mCoolingSetpoint = value;
    changed          = true;
    ChipLogProgress(AppServer, "Thermostat: cooling setpoint changed to %d", value);
    UpdateSimulatedRunningState();
    return Status::Success;
}

Status LoggingThermostat::GetRunningMode(thermostat::ThermostatRunningModeEnum & value) const
{
    value = mRunningMode;
    return Status::Success;
}

Status LoggingThermostat::SetRunningMode(thermostat::ThermostatRunningModeEnum value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value == thermostat::ThermostatRunningModeEnum::kOff ||
                            value == thermostat::ThermostatRunningModeEnum::kCool ||
                            value == thermostat::ThermostatRunningModeEnum::kHeat,
                        Status::InvalidValue);
    changed      = (mRunningMode != value);
    mRunningMode = value;
    if (changed)
    {
        ChipLogProgress(AppServer, "Thermostat: RunningMode set to %u", static_cast<unsigned>(value));
    }
    return Status::Success;
}

Status LoggingThermostat::GetRunningState(BitMask<thermostat::RelayStateBitmap> & value) const
{
    value = mRunningState;
    return Status::Success;
}

Status LoggingThermostat::SetRunningState(BitMask<thermostat::RelayStateBitmap> value, bool & changed)
{
    changed       = (mRunningState != value);
    mRunningState = value;
    if (changed)
    {
        ChipLogProgress(AppServer, "Thermostat: RunningState set to 0x%04x", value.Raw());
    }
    return Status::Success;
}

int8_t LoggingThermostat::GetLocalTemperatureCalibration() const
{
    return mCalibration;
}

Status LoggingThermostat::SetLocalTemperatureCalibration(int8_t value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value >= -30 && value <= 30, Status::ConstraintError);
    changed      = (mCalibration != value);
    mCalibration = value;
    if (changed)
    {
        ChipLogProgress(AppServer, "Thermostat: LocalTemperatureCalibration set to %d", value);
    }
    return Status::Success;
}

Status LoggingThermostat::GetRemoteSensing(BitMask<thermostat::RemoteSensingBitmap> & value) const
{
    value = mRemoteSensing;
    return Status::Success;
}

Status LoggingThermostat::SetRemoteSensing(BitMask<thermostat::RemoteSensingBitmap> value, bool & changed)
{
    changed        = (mRemoteSensing != value);
    mRemoteSensing = value;
    if (changed)
    {
        ChipLogProgress(AppServer, "Thermostat: RemoteSensing set to 0x%02x", value.Raw());
    }
    return Status::Success;
}

void LoggingThermostat::UpdateSimulatedRunningState()
{
    thermostat::ThermostatRunningModeEnum newRunningMode = thermostat::ThermostatRunningModeEnum::kOff;
    BitMask<thermostat::RelayStateBitmap> newRunningState;

    if (!mLocalTemperature.IsNull())
    {
        int16_t currentTemp = mLocalTemperature.Value();
        if (mSystemMode == thermostat::SystemModeEnum::kHeat && currentTemp < mHeatingSetpoint)
        {
            newRunningMode = thermostat::ThermostatRunningModeEnum::kHeat;
            newRunningState.Set(thermostat::RelayStateBitmap::kHeat);
        }
        else if (mSystemMode == thermostat::SystemModeEnum::kCool && currentTemp > mCoolingSetpoint)
        {
            newRunningMode = thermostat::ThermostatRunningModeEnum::kCool;
            newRunningState.Set(thermostat::RelayStateBitmap::kCool);
        }
    }

    if (HasThermostatCluster())
    {
        ThermostatCluster().SetRunningMode(newRunningMode);
        ThermostatCluster().SetRunningState(newRunningState);
    }
    else
    {
        mRunningMode  = newRunningMode;
        mRunningState = newRunningState;
    }
}

void LoggingThermostat::OnTemperatureDisplayModeChanged(
    Clusters::ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum value)
{
    ChipLogProgress(AppServer, "Thermostat: TemperatureDisplayMode changed to %u", static_cast<unsigned>(value));
}

void LoggingThermostat::OnKeypadLockoutChanged(Clusters::ThermostatUserInterfaceConfiguration::KeypadLockoutEnum value)
{
    ChipLogProgress(AppServer, "Thermostat: KeypadLockout changed to %u", static_cast<unsigned>(value));
}

} // namespace chip::app
