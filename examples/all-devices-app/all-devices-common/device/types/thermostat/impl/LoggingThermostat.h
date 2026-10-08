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

#include <app/clusters/groups-server/GroupsCluster.h>
#include <app/clusters/thermostat-user-interface-configuration-server/ThermostatUserInterfaceConfigurationCluster.h>
#include <device/types/thermostat/Thermostat.h>

#include <array>

namespace chip::app {

class LoggingThermostat : public Clusters::IdentifyDelegate,
                          public Clusters::Thermostat::Delegate,
                          public Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate,
                          public Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate,
                          public Clusters::Thermostat::ThermostatAutoSetpoints::Delegate,
                          public Clusters::Thermostat::ThermostatPresets::Delegate,
                          public Clusters::Thermostat::ThermostatHold::Delegate,
                          public Clusters::ThermostatUserInterfaceConfiguration::Delegate,
                          public Thermostat
{
public:
    struct Context
    {
        FabricTable & fabricTable;
        Credentials::GroupDataProvider & groupDataProvider;
        TimerDelegate & timerDelegate;
        BitFlags<Clusters::Thermostat::Feature> features = {
            Clusters::Thermostat::Feature::kHeating,
            Clusters::Thermostat::Feature::kCooling,
            Clusters::Thermostat::Feature::kAutoMode,
            Clusters::Thermostat::Feature::kPresets,
        };
        Clusters::Thermostat::OptionalAttributes optionalAttributes = {};
    };

    explicit LoggingThermostat(const Context & context);
    ~LoggingThermostat() override = default;

    // IdentifyDelegate
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override {}
    bool IsTriggerEffectEnabled() const override { return false; }

    // Thermostat and setpoint delegates share startup/shutdown and persistence.
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;
    FabricTable & GetFabricTable() const override { return mFabricTable; }

    DataModel::Nullable<int16_t> GetLocalTemperature() const override { return mLocalTemperature; }
    Protocols::InteractionModel::Status SetLocalTemperature(DataModel::Nullable<int16_t> value, bool & changed) override;

    Clusters::Thermostat::SystemModeEnum GetSystemMode() const override { return mSystemMode; }
    Protocols::InteractionModel::Status SetSystemMode(Clusters::Thermostat::SystemModeEnum value, bool & changed) override;

    Clusters::Thermostat::ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override;
    Protocols::InteractionModel::Status SetControlSequenceOfOperation(Clusters::Thermostat::ControlSequenceOfOperationEnum value,
                                                                      bool & changed) override;

    Protocols::InteractionModel::Status GetOccupiedHeatingSetpoint(int16_t & value) const override;
    Protocols::InteractionModel::Status SetOccupiedHeatingSetpoint(int16_t value, bool & changed) override;
    Protocols::InteractionModel::Status GetOccupiedCoolingSetpoint(int16_t & value) const override;
    Protocols::InteractionModel::Status SetOccupiedCoolingSetpoint(int16_t value, bool & changed) override;

    // Optional Setpoint Limit Getters/Setters
    Protocols::InteractionModel::Status GetAbsMinHeatSetpointLimit(int16_t & value) const override;
    Protocols::InteractionModel::Status GetAbsMaxHeatSetpointLimit(int16_t & value) const override;
    Protocols::InteractionModel::Status GetMinHeatSetpointLimit(int16_t & value) const override;
    Protocols::InteractionModel::Status SetMinHeatSetpointLimit(int16_t value, bool & changed) override;
    Protocols::InteractionModel::Status GetMaxHeatSetpointLimit(int16_t & value) const override;
    Protocols::InteractionModel::Status SetMaxHeatSetpointLimit(int16_t value, bool & changed) override;

    Protocols::InteractionModel::Status GetAbsMinCoolSetpointLimit(int16_t & value) const override;
    Protocols::InteractionModel::Status GetAbsMaxCoolSetpointLimit(int16_t & value) const override;
    Protocols::InteractionModel::Status GetMinCoolSetpointLimit(int16_t & value) const override;
    Protocols::InteractionModel::Status SetMinCoolSetpointLimit(int16_t value, bool & changed) override;
    Protocols::InteractionModel::Status GetMaxCoolSetpointLimit(int16_t & value) const override;
    Protocols::InteractionModel::Status SetMaxCoolSetpointLimit(int16_t value, bool & changed) override;

    // Optional Deadband Override
    Protocols::InteractionModel::Status GetMinDeadband(int16_t & value) const override;

    Protocols::InteractionModel::Status GetCriticalFreezeProtection(bool & value) const override;
    Protocols::InteractionModel::Status GetCriticalOverheatProtection(bool & value) const override;

    // ThermostatPresets::Delegate
    CHIP_ERROR GetPresetTypeAtIndex(size_t index, Clusters::Thermostat::Structs::PresetTypeStruct::Type & value) override;
    uint8_t GetNumberOfPresets() override { return kPresetCapacity; }
    CHIP_ERROR GetPresetAtIndex(size_t index, Clusters::Thermostat::PresetStructWithOwnedMembers & value) override;
    CHIP_ERROR GetPendingPresetAtIndex(size_t index, Clusters::Thermostat::PresetStructWithOwnedMembers & value) override;
    CHIP_ERROR GetActivePresetHandle(DataModel::Nullable<MutableByteSpan> & value) override;
    CHIP_ERROR SetActivePresetHandle(const DataModel::Nullable<ByteSpan> & value) override;
    void InitializePendingPresets() override;
    void ClearPendingPresetList() override { mPendingPresetCount = 0; }
    CHIP_ERROR AppendToPendingPresetList(const Clusters::Thermostat::PresetStructWithOwnedMembers & value) override;
    CHIP_ERROR CommitPendingPresets() override;
    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(AttributeId attributeId) override;

    // ThermostatHold::Delegate
    Clusters::Thermostat::TemperatureSetpointHoldEnum GetTemperatureSetpointHold() const override { return mSetpointHold; }
    Protocols::InteractionModel::Status SetTemperatureSetpointHold(Clusters::Thermostat::TemperatureSetpointHoldEnum value,
                                                                   bool & changed) override;
    DataModel::Nullable<uint16_t> GetTemperatureSetpointHoldDuration() const override { return mSetpointHoldDuration; }
    Protocols::InteractionModel::Status SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> value,
                                                                           bool & changed) override;
    DataModel::Nullable<uint32_t> GetSetpointHoldExpiryTimestamp() const override { return mSetpointHoldExpiryTimestamp; }
    Protocols::InteractionModel::Status SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> value,
                                                                       bool & changed) override;

    Protocols::InteractionModel::Status GetRunningMode(Clusters::Thermostat::ThermostatRunningModeEnum & value) const override;
    Protocols::InteractionModel::Status SetRunningMode(Clusters::Thermostat::ThermostatRunningModeEnum value,
                                                       bool & changed) override;
    Protocols::InteractionModel::Status GetRunningState(BitMask<Clusters::Thermostat::RelayStateBitmap> & value) const override;
    Protocols::InteractionModel::Status SetRunningState(BitMask<Clusters::Thermostat::RelayStateBitmap> value,
                                                        bool & changed) override;

    int8_t GetLocalTemperatureCalibration() const override;
    Protocols::InteractionModel::Status SetLocalTemperatureCalibration(int8_t value, bool & changed) override;

    Protocols::InteractionModel::Status GetRemoteSensing(BitMask<Clusters::Thermostat::RemoteSensingBitmap> & value) const override;
    Protocols::InteractionModel::Status SetRemoteSensing(BitMask<Clusters::Thermostat::RemoteSensingBitmap> value,
                                                         bool & changed) override;

    // ThermostatUserInterfaceConfiguration::Delegate
    void OnTemperatureDisplayModeChanged(Clusters::ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum value) override;
    void OnKeypadLockoutChanged(Clusters::ThermostatUserInterfaceConfiguration::KeypadLockoutEnum value) override;

protected:
    CHIP_ERROR RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    void UpdateSimulatedRunningState();

    FabricTable & mFabricTable;
    Credentials::GroupDataProvider & mGroupDataProvider;
    AttributePersistenceProvider * mAttributeStorage             = nullptr;
    DataModel::Nullable<int16_t> mLocalTemperature               = DataModel::MakeNullable<int16_t>(2500);
    Clusters::Thermostat::SystemModeEnum mSystemMode             = Clusters::Thermostat::SystemModeEnum::kOff;
    int16_t mHeatingSetpoint                                     = Clusters::Thermostat::kDefaultHeatingSetpoint;
    int16_t mCoolingSetpoint                                     = Clusters::Thermostat::kDefaultCoolingSetpoint;
    int16_t mMinHeatSetpointLimit                                = Clusters::Thermostat::kDefaultAbsMinHeatSetpointLimit;
    int16_t mMaxHeatSetpointLimit                                = Clusters::Thermostat::kDefaultAbsMaxHeatSetpointLimit;
    int16_t mMinCoolSetpointLimit                                = Clusters::Thermostat::kDefaultAbsMinCoolSetpointLimit;
    int16_t mMaxCoolSetpointLimit                                = Clusters::Thermostat::kDefaultAbsMaxCoolSetpointLimit;
    Clusters::Thermostat::ThermostatRunningModeEnum mRunningMode = Clusters::Thermostat::ThermostatRunningModeEnum::kOff;
    BitMask<Clusters::Thermostat::RelayStateBitmap> mRunningState;
    BitMask<Clusters::Thermostat::RemoteSensingBitmap> mRemoteSensing;
    int8_t mCalibration = 0;

    // One preset per scenario, with bounded storage for committed and pending lists.
    static constexpr uint8_t kPresetCapacity = 3;
    std::array<Clusters::Thermostat::PresetStructWithOwnedMembers, kPresetCapacity> mPresets;
    std::array<Clusters::Thermostat::PresetStructWithOwnedMembers, kPresetCapacity> mPendingPresets;
    uint8_t mPresetCount        = 0;
    uint8_t mPendingPresetCount = 0;
    DataModel::Nullable<uint8_t> mActivePresetHandle;
    Clusters::Thermostat::TemperatureSetpointHoldEnum mSetpointHold =
        Clusters::Thermostat::TemperatureSetpointHoldEnum::kSetpointHoldOff;
    DataModel::Nullable<uint16_t> mSetpointHoldDuration;
    DataModel::Nullable<uint32_t> mSetpointHoldExpiryTimestamp;

    LazyRegisteredServerCluster<Clusters::GroupsCluster> mGroupsCluster;
    LazyRegisteredServerCluster<Clusters::ThermostatUserInterfaceConfigurationCluster> mUserInterfaceCluster;
};

} // namespace chip::app
