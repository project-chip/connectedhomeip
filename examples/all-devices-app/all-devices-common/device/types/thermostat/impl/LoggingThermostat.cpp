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
} // namespace

LoggingThermostat::LoggingThermostat(const Context & context) :
    Thermostat(context, *this, *this, *this, *this, *this), mFabricTable(context.fabricTable)
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
    return Status::Success;
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
