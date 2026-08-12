/**
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include "SetpointRange.h"
#include "Setpoints.h"

#include "ThermostatClusterAtomic.h"
#include "ThermostatDelegate.h"

#include "app/ConcreteAttributePath.h"
#include "app/clusters/thermostat-server/Temperature.h"
#include "lib/core/DataModelTypes.h"
#include "lib/support/CodeUtils.h"
#include <app-common/zap-generated/callback.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <credentials/FabricTable.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatCluster : public DefaultServerCluster, public FabricTable::Delegate, public AtomicWriteSession::Delegate
{
public:
    struct OptionalAttributes
    {
        bool AbsMinHeatSetpointLimit = false;
        bool AbsMaxHeatSetpointLimit = false;
        bool AbsMinCoolSetpointLimit = false;
        bool AbsMaxCoolSetpointLimit = false;

        bool LocalTemperatureCalibration = false;

        bool MinHeatSetpointLimit = false;
        bool MaxHeatSetpointLimit = false;
        bool MinCoolSetpointLimit = false;
        bool MaxCoolSetpointLimit = false;

        bool RemoteSensing                   = false;
        bool ThermostatRunningMode           = false;
        bool TemperatureSetpointHold         = false;
        bool TemperatureSetpointHoldDuration = false;
        bool ThermostatRunningState          = false;
        bool SetpointChangeSource            = false;
        bool SetpointChangeAmount            = false;
        bool SetpointChangeSourceTimestamp   = false;

        bool SetpointHoldExpiryTimestamp = false;
        bool OutdoorTemperature          = false;

        OptionalAttributes() = default;
    };

    struct DefaultValues
    {
        temperature absMinHeatSetpointLimit;
        temperature absMaxHeatSetpointLimit;
        temperature absMinCoolSetpointLimit;
        temperature absMaxCoolSetpointLimit;
        int8_t localTemperatureCalibration;
        temperature occupiedCoolingSetpoint;
        temperature occupiedHeatingSetpoint;
        temperature unoccupiedCoolingSetpoint;
        temperature unoccupiedHeatingSetpoint;
        Optional<temperature> minHeatSetpointLimit;
        Optional<temperature> maxHeatSetpointLimit;
        Optional<temperature> minCoolSetpointLimit;
        Optional<temperature> maxCoolSetpointLimit;
        temperature minSetpointDeadBand;
        ControlSequenceOfOperationEnum controlSequenceOfOperation;
        SystemModeEnum systemMode;

        TemperatureSetpointHoldEnum temperatureSetpointHold;
        DataModel::Nullable<uint16_t> temperatureSetpointHoldDuration;
        DataModel::Nullable<uint32_t> setpointHoldExpiryTimestamp;

        DefaultValues() = default;
    };

    ThermostatCluster(EndpointId aEndpointId, BitFlags<Thermostat::Feature> features, const OptionalAttributes & optionalAttributes,
                      const DefaultValues & defaultValues, FabricTable & fabricTable);

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;

    // Exposing for feature clusters to be able to notify when they change an attribute
    using DefaultServerCluster::NotifyAttributeChanged;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    BitFlags<Thermostat::Feature> Features() const { return mFeatures; }
    void SetFeatures(BitFlags<Thermostat::Feature> features) { mFeatures = features; }

    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

    EndpointId Endpoint() const { return mPath.mEndpointId; }
    void SetDelegate(Thermostat::Delegate * delegate);
    Thermostat::Delegate * GetDelegate() const { return mDelegate; }

    Protocols::InteractionModel::Status OnAtomicWriteBegin(AttributeId attributeId) override;
    Protocols::InteractionModel::Status OnAtomicWritePrecommit(AttributeId attributeId) override;
    Protocols::InteractionModel::Status OnAtomicWriteCommit(AttributeId attributeId) override;
    Protocols::InteractionModel::Status OnAtomicWriteRollback(AttributeId attributeId) override;

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override;
    bool HasAttribute(chip::AttributeId attributeId) override;

    void OnAtomicWriteTimeout();

    SystemModeEnum GetSystemMode() const;
    Protocols::InteractionModel::Status SetSystemMode(SystemModeEnum systemMode);

    ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const;
    Protocols::InteractionModel::Status SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation);

    ThermostatRunningModeEnum GetRunningMode() const;
    Protocols::InteractionModel::Status SetRunningMode(ThermostatRunningModeEnum runningMode);

    BitMask<RelayStateBitmap> GetRunningState() const;
    Protocols::InteractionModel::Status SetRunningState(BitMask<RelayStateBitmap> runningState);

    DataModel::Nullable<temperature> GetLocalTemperature() const;
    Protocols::InteractionModel::Status
    SetLocalTemperature(DataModel::Nullable<temperature> localTemperature,
                        DataModel::AttributeChangeType changeType = DataModel::AttributeChangeType::kReportable);

    int8_t GetLocalTemperatureCalibration() const;
    Protocols::InteractionModel::Status SetLocalTemperatureCalibration(int8_t localTemperatureCalibration);

    DataModel::ActionReturnStatus ChangeSetpointAttribute(const AttributeId attributeId, temperature temp);

    AtomicWriteSession & GetAtomicWriteSession() { return mAtomicWriteSession; }
    const AtomicWriteSession & GetAtomicWriteSession() const { return mAtomicWriteSession; }
    Setpoints GetSetpoints();

    virtual bool IsOccupied() const { return true; }
    virtual bool IsActiveSetpoint(AttributeId attributeId) const;

protected:
    OptionalAttributes mOptionalAttributes;
    const DefaultValues mDefaultValues;
    FabricTable & mFabricTable;
    Thermostat::Delegate * mDelegate = nullptr;
    AtomicWriteSession mAtomicWriteSession;

    DataModel::ActionReturnStatus WriteNonAtomicAttribute(const DataModel::WriteAttributeRequest & request,
                                                          AttributeValueDecoder & decoder);
    DataModel::ActionReturnStatus HandleSetpointChange(Setpoints & setpoints, const AttributeId attributeId, temperature value,
                                                       SetpointAttributes & changedAttributes);
    DataModel::ActionReturnStatus SetpointRaiseLower(const Commands::SetpointRaiseLower::DecodableType & commandData);

    DataModel::ActionReturnStatus ReadSetpointAttribute(const DataModel::ReadAttributeRequest & request,
                                                        AttributeValueEncoder & encoder);

    Protocols::InteractionModel::Status SaveSetpoint(Setpoint & oldSetpoint, Setpoint & newSetpoint);
    DataModel::ActionReturnStatus SaveSetpoints(Setpoints & setpoints, SetpointAttributes changedAttributes);

    void GenerateSetpointEvent(AttributeId attributeId, temperature oldTemp, temperature newTemp);

    void GenerateSystemModeChangeEvent(chip::Optional<chip::app::Clusters::Thermostat::SystemModeEnum> previousSystemMode,
                                       chip::app::Clusters::Thermostat::SystemModeEnum currentSystemMode);

    void GenerateLocalTemperatureChangeEvent(chip::app::DataModel::Nullable<int16_t> currentLocalTemperature);

    void
    GenerateOccupancyChangeEvent(chip::Optional<chip::BitMask<chip::app::Clusters::Thermostat::OccupancyBitmap>> previousOccupancy,
                                 chip::BitMask<chip::app::Clusters::Thermostat::OccupancyBitmap> currentOccupancy);

    void GenerateSetpointChangeEvent(chip::app::Clusters::Thermostat::SystemModeEnum systemMode,
                                     chip::BitMask<chip::app::Clusters::Thermostat::OccupancyBitmap> occupancy,
                                     chip::Optional<temperature> previousSetpoint, temperature currentSetpoint);

    void GenerateRunningStateChangeEvent(
        chip::Optional<chip::BitMask<chip::app::Clusters::Thermostat::RelayStateBitmap>> previousRunningState,
        chip::BitMask<chip::app::Clusters::Thermostat::RelayStateBitmap> currentRunningState);

    void
    GenerateRunningModeChangeEvent(chip::Optional<chip::app::Clusters::Thermostat::ThermostatRunningModeEnum> previousRunningMode,
                                   chip::app::Clusters::Thermostat::ThermostatRunningModeEnum currentRunningMode);

    void GenerateActiveScheduleChangeEvent(chip::Optional<chip::app::DataModel::Nullable<chip::ByteSpan>> previousScheduleHandle,
                                           chip::app::DataModel::Nullable<chip::ByteSpan> currentScheduleHandle);

    void GenerateActivePresetChangeEvent(chip::Optional<chip::app::DataModel::Nullable<chip::ByteSpan>> previousPresetHandle,
                                         chip::app::DataModel::Nullable<chip::ByteSpan> currentPresetHandle);

    friend class ThermostatPresets;
};
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
