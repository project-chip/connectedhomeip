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
#pragma once

#include <app/clusters/thermostat-user-interface-configuration-server/ThermostatUserInterfaceConfigurationCluster.h>
#include <device/types/room-air-conditioner/RoomAirConditioner.h>

namespace chip::app {

class LoggingRoomAirConditioner : public Clusters::IdentifyDelegate,
                                  public Clusters::OnOffDelegate,
                                  public Clusters::Thermostat::Delegate,
                                  public Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate,
                                  public Clusters::ThermostatUserInterfaceConfiguration::Delegate,
                                  public RoomAirConditioner
{
public:
    LoggingRoomAirConditioner(TimerDelegate & timerDelegate, FabricTable & fabricTable);
    ~LoggingRoomAirConditioner() override = default;

    Clusters::ThermostatUserInterfaceConfigurationCluster & UserInterfaceCluster() { return mUserInterfaceCluster.Cluster(); }

    // IdentifyDelegate
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster &) override {}
    bool IsTriggerEffectEnabled() const override { return false; }

    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

    // Thermostat delegates
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;
    FabricTable & GetFabricTable() const override { return mFabricTable; }
    DataModel::Nullable<int16_t> GetLocalTemperature() const override { return mLocalTemperatureCentiCelsius; }
    Protocols::InteractionModel::Status SetLocalTemperature(DataModel::Nullable<int16_t> value, bool & changed) override;
    Clusters::Thermostat::SystemModeEnum GetSystemMode() const override { return mSystemMode; }
    Protocols::InteractionModel::Status SetSystemMode(Clusters::Thermostat::SystemModeEnum value, bool & changed) override;
    Clusters::Thermostat::ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override;
    Protocols::InteractionModel::Status SetControlSequenceOfOperation(Clusters::Thermostat::ControlSequenceOfOperationEnum value,
                                                                      bool & changed) override;
    Protocols::InteractionModel::Status GetOccupiedCoolingSetpoint(int16_t & value) const override;
    Protocols::InteractionModel::Status SetOccupiedCoolingSetpoint(int16_t value, bool & changed) override;

    Protocols::InteractionModel::Status GetRunningMode(Clusters::Thermostat::ThermostatRunningModeEnum &) const override
    {
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
    Protocols::InteractionModel::Status SetRunningMode(Clusters::Thermostat::ThermostatRunningModeEnum, bool & changed) override
    {
        changed = false;
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
    Protocols::InteractionModel::Status GetRunningState(BitMask<Clusters::Thermostat::RelayStateBitmap> &) const override
    {
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
    Protocols::InteractionModel::Status SetRunningState(BitMask<Clusters::Thermostat::RelayStateBitmap>, bool & changed) override
    {
        changed = false;
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
    Protocols::InteractionModel::Status SetRemoteSensing(BitMask<Clusters::Thermostat::RemoteSensingBitmap>,
                                                         bool & changed) override
    {
        changed = false;
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    // ThermostatUserInterfaceConfiguration::Delegate
    void OnTemperatureDisplayModeChanged(Clusters::ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum value) override;
    void OnKeypadLockoutChanged(Clusters::ThermostatUserInterfaceConfiguration::KeypadLockoutEnum value) override;

protected:
    CHIP_ERROR RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    static constexpr int16_t kDefaultLocalTemperatureCentiCelsius = 2500;

    FabricTable & mFabricTable;
    LazyRegisteredServerCluster<Clusters::ThermostatUserInterfaceConfigurationCluster> mUserInterfaceCluster;
    AttributePersistenceProvider * mAttributeStorage = nullptr;
    DataModel::Nullable<int16_t> mLocalTemperatureCentiCelsius{ kDefaultLocalTemperatureCentiCelsius };
    Clusters::Thermostat::SystemModeEnum mSystemMode = Clusters::Thermostat::SystemModeEnum::kOff;
    int16_t mCoolingSetpoint                         = Clusters::Thermostat::kDefaultCoolingSetpoint;
};

} // namespace chip::app
