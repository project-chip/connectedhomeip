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

#include <device/types/room-air-conditioner/impl/LoggingRoomAirConditioner.h>

#include <app/MessageDef/StatusIB.h>
#include <app/persistence/AttributePersistence.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip::app {

using namespace Clusters::Thermostat;
using Protocols::InteractionModel::Status;

LoggingRoomAirConditioner::LoggingRoomAirConditioner(TimerDelegate & timerDelegate, FabricTable & fabricTable,
                                                     std::optional<EndpointComposition::SemanticTag> tag) :
    RoomAirConditioner(RoomAirConditioner::Context{ timerDelegate, *this, *this, *this, *this }),
    mFabricTable(fabricTable), mTag(tag)
{}

CHIP_ERROR LoggingRoomAirConditioner::RegisterDescriptor(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                         EndpointComposition composition)
{
    if (composition.tagList.empty() && mTag.has_value())
    {
        composition.tagList = Span(&mTag.value(), 1);
    }
    return RoomAirConditioner::RegisterDescriptor(endpoint, provider, composition);
}

CHIP_ERROR LoggingRoomAirConditioner::RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    mUserInterfaceCluster.Create(endpoint);
    mUserInterfaceCluster.Cluster().SetDelegate(this);
    return provider.AddCluster(mUserInterfaceCluster.Registration());
}

void LoggingRoomAirConditioner::UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mUserInterfaceCluster.IsConstructed())
    {
        mUserInterfaceCluster.Cluster().SetDelegate(nullptr);
        LogErrorOnFailure(provider.RemoveCluster(&mUserInterfaceCluster.Cluster()));
        mUserInterfaceCluster.Destroy();
    }
}

CHIP_ERROR LoggingRoomAirConditioner::Startup(ServerClusterContext & context)
{
    // ThermostatCluster starts both delegate interfaces implemented by this object.
    VerifyOrReturnError(mAttributeStorage == nullptr, CHIP_NO_ERROR);
    mAttributeStorage = &context.attributeStorage;
    AttributePersistence persistence(*mAttributeStorage);
    persistence.LoadNativeEndianValue({ GetEndpointId(), Clusters::Thermostat::Id, Attributes::SystemMode::Id }, mSystemMode,
                                      SystemModeEnum::kOff);
    if (mSystemMode != SystemModeEnum::kOff && mSystemMode != SystemModeEnum::kCool)
    {
        mSystemMode = SystemModeEnum::kOff;
    }
    persistence.LoadNativeEndianValue({ GetEndpointId(), Clusters::Thermostat::Id, Attributes::OccupiedCoolingSetpoint::Id },
                                      mCoolingSetpoint, kDefaultCoolingSetpoint);
    if (mCoolingSetpoint < kDefaultAbsMinCoolSetpointLimit || mCoolingSetpoint > kDefaultAbsMaxCoolSetpointLimit)
    {
        mCoolingSetpoint = kDefaultCoolingSetpoint;
    }
    return CHIP_NO_ERROR;
}

void LoggingRoomAirConditioner::Shutdown(ClusterShutdownType type)
{
    mAttributeStorage = nullptr;
}

void LoggingRoomAirConditioner::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(AppServer, "RoomAirConditioner: identification started");
}

void LoggingRoomAirConditioner::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(AppServer, "RoomAirConditioner: identification stopped");
}

void LoggingRoomAirConditioner::OnOffStartup(bool on)
{
    // Schedule the startup logic to run on the event loop.
    // This ensures that all clusters (including ThermostatCluster) are fully
    // initialized and their states are loaded from KVS before we attempt to sync them.
    CHIP_ERROR err = DeviceLayer::SystemLayer().ScheduleLambda([this, on]() {
        ChipLogProgress(AppServer, "RoomAirConditioner: starting %s", on ? "on" : "off");
        LogErrorOnFailure(StatusIB(ThermostatCluster().SetLocalTemperature(
                                       on ? DataModel::MakeNullable(kDefaultLocalTemperatureCentiCelsius) : DataModel::NullNullable))
                              .ToChipError());
        if (!on && mSystemMode != SystemModeEnum::kOff)
        {
            LogErrorOnFailure(StatusIB(ThermostatCluster().SetSystemMode(SystemModeEnum::kOff)).ToChipError());
        }
    });
    LogErrorOnFailure(err);
}

void LoggingRoomAirConditioner::OnOnOffChanged(bool on)
{
    ChipLogProgress(AppServer, "RoomAirConditioner: turned %s", on ? "on" : "off");
    LogErrorOnFailure(StatusIB(ThermostatCluster().SetLocalTemperature(
                                   on ? DataModel::MakeNullable(kDefaultLocalTemperatureCentiCelsius) : DataModel::NullNullable))
                          .ToChipError());
    if (!on && mSystemMode != SystemModeEnum::kOff)
    {
        LogErrorOnFailure(StatusIB(ThermostatCluster().SetSystemMode(SystemModeEnum::kOff)).ToChipError());
    }
    else if (on && mSystemMode == SystemModeEnum::kOff)
    {
        // Cool is the only supported operational mode for this simulator.
        LogErrorOnFailure(StatusIB(ThermostatCluster().SetSystemMode(SystemModeEnum::kCool)).ToChipError());
    }
}

Status LoggingRoomAirConditioner::SetLocalTemperature(DataModel::Nullable<int16_t> value, bool & changed)
{
    changed                       = mLocalTemperatureCentiCelsius != value;
    mLocalTemperatureCentiCelsius = value;
    return Status::Success;
}

Status LoggingRoomAirConditioner::SetSystemMode(SystemModeEnum value, bool & changed)
{
    changed = false;
    VerifyOrReturnError(value == SystemModeEnum::kOff || value == SystemModeEnum::kCool, Status::ConstraintError);
    VerifyOrReturnError(mSystemMode != value, Status::Success);
    VerifyOrReturnError(mAttributeStorage != nullptr, Status::Failure);
    AttributePersistence persistence(*mAttributeStorage);
    VerifyOrReturnError(persistence.StoreNativeEndianValue(
                            { GetEndpointId(), Clusters::Thermostat::Id, Attributes::SystemMode::Id }, value) == CHIP_NO_ERROR,
                        Status::Failure);
    mSystemMode = value;
    changed     = true;
    ChipLogProgress(AppServer, "RoomAirConditioner: thermostat mode %s", value == SystemModeEnum::kCool ? "Cool" : "Off");
    return Status::Success;
}

ControlSequenceOfOperationEnum LoggingRoomAirConditioner::GetControlSequenceOfOperation() const
{
    return ControlSequenceOfOperationEnum::kCoolingOnly;
}

Status LoggingRoomAirConditioner::SetControlSequenceOfOperation(ControlSequenceOfOperationEnum value, bool & changed)
{
    changed = false;
    // This device has no heater; changing the sequence must not advertise heating capability.
    return value == ControlSequenceOfOperationEnum::kCoolingOnly ? Status::Success : Status::ConstraintError;
}

Status LoggingRoomAirConditioner::GetOccupiedCoolingSetpoint(int16_t & value) const
{
    value = mCoolingSetpoint;
    return Status::Success;
}

Status LoggingRoomAirConditioner::SetOccupiedCoolingSetpoint(int16_t value, bool & changed)
{
    changed = false;
    VerifyOrReturnError(value >= kDefaultAbsMinCoolSetpointLimit && value <= kDefaultAbsMaxCoolSetpointLimit,
                        Status::ConstraintError);
    VerifyOrReturnError(mCoolingSetpoint != value, Status::Success);
    VerifyOrReturnError(mAttributeStorage != nullptr, Status::Failure);
    AttributePersistence persistence(*mAttributeStorage);
    VerifyOrReturnError(
        persistence.StoreNativeEndianValue({ GetEndpointId(), Clusters::Thermostat::Id, Attributes::OccupiedCoolingSetpoint::Id },
                                           value) == CHIP_NO_ERROR,
        Status::Failure);
    mCoolingSetpoint = value;
    changed          = true;
    ChipLogProgress(AppServer, "RoomAirConditioner: cooling setpoint %d hundredths of a degree Celsius", value);
    return Status::Success;
}

void LoggingRoomAirConditioner::OnTemperatureDisplayModeChanged(
    Clusters::ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum value)
{
    using Clusters::ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum;
    ChipLogProgress(AppServer, "RoomAirConditioner: display unit %s",
                    value == TemperatureDisplayModeEnum::kCelsius ? "Celsius" : "Fahrenheit");
}

void LoggingRoomAirConditioner::OnKeypadLockoutChanged(Clusters::ThermostatUserInterfaceConfiguration::KeypadLockoutEnum value)
{
    ChipLogProgress(AppServer, "RoomAirConditioner: keypad lockout %u", static_cast<unsigned>(value));
}

} // namespace chip::app
