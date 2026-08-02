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
#include "ThermostatClusterAtomic.h"
#include "ThermostatClusterOccupancy.h"
#include "ThermostatClusterPresets.h"
#include "ThermostatClusterSetpoints.h"
#include "ThermostatClusterSuggestions.h"
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

    ThermostatCluster(EndpointId aEndpointId, BitFlags<Thermostat::Feature> features,
                      const OptionalAttributes & optionalAttributes);

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

    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

    EndpointId Endpoint() { return mPath.mEndpointId; }
    void SetDelegate(Thermostat::Delegate * delegate) { mDelegate = delegate; }
    Thermostat::Delegate * GetDelegate() const { return mDelegate; }

    Protocols::InteractionModel::Status OnAtomicWriteBegin(AttributeId attributeId) override;
    Protocols::InteractionModel::Status OnAtomicWritePrecommit(AttributeId attributeId) override;
    Protocols::InteractionModel::Status OnAtomicWriteCommit(AttributeId attributeId) override;
    Protocols::InteractionModel::Status OnAtomicWriteRollback(AttributeId attributeId) override;

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override;
    bool HasAttribute(chip::AttributeId attributeId) override;

    void OnAtomicWriteTimeout();

    SystemModeEnum GetSystemMode() const { return mSystemMode; }
    Protocols::InteractionModel::Status SetSystemMode(SystemModeEnum systemMode);

    ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const { return mControlSequenceOfOperation; }
    Protocols::InteractionModel::Status SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation);

    ThermostatRunningModeEnum GetRunningMode() const { return mRunningMode; }
    Protocols::InteractionModel::Status SetRunningMode(ThermostatRunningModeEnum runningMode);

    BitMask<RelayStateBitmap> GetRunningState() const { return mRunningState; }
    Protocols::InteractionModel::Status SetRunningState(BitMask<RelayStateBitmap> runningState);

    DataModel::Nullable<int16_t> GetLocalTemperature() const { return mLocalTemperature; }
    Protocols::InteractionModel::Status SetLocalTemperature(DataModel::Nullable<int16_t> localTemperature);

    DataModel::ActionReturnStatus ChangeSetpointAttribute(const AttributeId attributeId, temperature temp);
    DataModel::ActionReturnStatus SetpointRaiseLower(const SetpointRaiseLowerModeEnum mode, const int16_t amount);

    BitFlags<Thermostat::Feature> mFeatures;
    Setpoints mSetpoints;

    AtomicWriteSession & GetAtomicWriteSession() { return mAtomicWriteSession; }
    const AtomicWriteSession & GetAtomicWriteSession() const { return mAtomicWriteSession; }
    Setpoints & GetSetpoints() { return mSetpoints; }

    virtual bool IsOccupied() const { return true; }
    virtual bool IsActiveSetpoint(AttributeId attributeId) const;

protected:
    OptionalAttributes mOptionalAttributes;
    ControlSequenceOfOperationEnum mControlSequenceOfOperation;
    Thermostat::Delegate * mDelegate = nullptr;
    AtomicWriteSession mAtomicWriteSession;

    BitMask<RemoteSensingBitmap> mRemoteSensing;
    BitMask<OccupancyBitmap> mOccupancy;

    SystemModeEnum mSystemMode;
    ThermostatRunningModeEnum mRunningMode;
    BitMask<RelayStateBitmap> mRunningState;
    DataModel::Nullable<int16_t> mLocalTemperature;
    int8_t mLocalTemperatureCalibration;

    TemperatureSetpointHoldEnum mTemperatureSetpointHold;
    DataModel::Nullable<uint16_t> mTemperatureSetpointHoldDuration;
    DataModel::Nullable<uint32_t> mSetpointHoldExpiryTimestamp;

    DataModel::ActionReturnStatus WriteNonAtomicAttribute(const DataModel::WriteAttributeRequest & request,
                                                          AttributeValueDecoder & decoder);
    DataModel::ActionReturnStatus HandleSetpointChange(Setpoints & setpoints, const AttributeId attributeId, temperature value,
                                                       SetpointAttributes & changedAttributes);
    DataModel::ActionReturnStatus SetpointRaiseLower(const Commands::SetpointRaiseLower::DecodableType & commandData);

    Protocols::InteractionModel::Status LoadSetpoints(Setpoints & setpoints, AttributePersistence & persistence);
    Protocols::InteractionModel::Status SaveSetpoint(Setpoint & oldSetpoint, Setpoint & newSetpoint);
    DataModel::ActionReturnStatus SaveSetpoints(Setpoints & setpoints, SetpointAttributes changedAttributes);

    void GenerateSetpointEvent(AttributeId attributeId, temperature oldTemp, temperature newTemp);
};

ThermostatCluster * FindClusterOnEndpoint(EndpointId endpointId);
Protocols::InteractionModel::Status SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
