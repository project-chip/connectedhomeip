/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief APIs for the  Thermostat cluster.
 *
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "ThermostatDelegate.h"

#include <app-common/zap-generated/callback.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/Thermostat/Attributes.h>
#include <clusters/Thermostat/Ids.h>
#include <clusters/Thermostat/Metadata.h>
#include <credentials/FabricTable.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

enum class AtomicWriteState
{
    Closed = 0,
    Open,
};

static constexpr size_t kThermostatEndpointCount =
    MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

/**
 * @brief Retained helper that owns the legacy preset / schedule / suggestion / atomic-write logic.
 *
 * During the code-driven migration (PR 3a) this class is NO LONGER registered as an
 * AttributeAccessInterface; instead the new ThermostatCluster (a DefaultServerCluster) forwards the
 * complex list/preset/suggestion attribute reads & writes and the preset/atomic/suggestion commands
 * to this helper. It remains registered as a FabricTable::Delegate so that atomic-write sessions are
 * cleaned up when a fabric is removed.
 *
 * A follow-up PR (3b) re-houses this state into per-endpoint ThermostatCluster instances and removes
 * this helper entirely.
 */
class ThermostatAttrAccess : public chip::FabricTable::Delegate
{

public:
    ThermostatAttrAccess() {}

    // Formerly AttributeAccessInterface overrides; now plain methods invoked by ThermostatCluster.
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder);

    /**
     * @brief Set the Active Preset to a given preset handle, or null
     *
     * @param endpoint The endpoint
     * @param presetHandle The handle of the preset to set active, or null to clear the active preset
     * @return Success if the active preset was updated, an error code if not
     */
    Protocols::InteractionModel::Status SetActivePreset(EndpointId endpoint, DataModel::Nullable<ByteSpan> presetHandle);

    /**
     * @brief Apply a preset to the pending lists of presets during an atomic write
     *
     * @param delegate The current ThermostatDelegate
     * @param preset The preset to append
     * @return CHIP_NO_ERROR if successful, an error code if not
     */
    CHIP_ERROR AppendPendingPreset(Thermostat::Delegate * delegate, const Structs::PresetStruct::Type & preset);

    /**
     * @brief Verifies if the pending presets for a given endpoint are valid
     *
     * @param endpoint The endpoint
     * @return Success if the list of pending presets is valid, an error code if not
     */
    Protocols::InteractionModel::Status PrecommitPresets(EndpointId endpoint);

    /**
     * @brief Callback for when the server is removed from a given fabric; all associated atomic writes are reset
     *
     * @param fabricTable The fabric table
     * @param fabricIndex The fabric index
     */
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

    /**
     * @brief Gets the scoped node id of the originator that sent the last successful
     *        AtomicRequest of type BeginWrite for the given endpoint.
     *
     * @param[in] endpoint The endpoint.
     *
     * @return the scoped node id for the given endpoint if set. Otherwise returns ScopedNodeId().
     */
    ScopedNodeId GetAtomicWriteOriginatorScopedNodeId(EndpointId endpoint);

    /**
     * @brief Sets the atomic write state for the given endpoint and originatorNodeId
     *
     * @param[in] endpoint The endpoint.
     * @param[in] originatorNodeId The originator scoped node id.
     * @param[in] state Whether or not an atomic write is open or closed.
     * @param attributeStatuses The set of attribute status structs the atomic write should be associated with
     * @return true if it was able to update the atomic write state
     * @return false if it was unable to update the atomic write state
     */
    bool
    SetAtomicWrite(EndpointId endpoint, ScopedNodeId originatorNodeId, AtomicWriteState state,
                   Platform::ScopedMemoryBufferWithSize<Globals::Structs::AtomicAttributeStatusStruct::Type> & attributeStatuses);

    /**
     * @brief Sets the atomic write state for the given endpoint and originatorNodeId
     *
     */
    /**
     * @brief Resets the atomic write for a given endpoint
     *
     * @param endpoint The endpoint
     */
    void ResetAtomicWrite(EndpointId endpoint);

    /**
     * @brief Checks if a given endpoint has an atomic write open, optionally filtered by an attribute ID
     *
     * @param endpoint The endpoint
     * @param attributeId The optional attribute ID to filter on
     * @return true if the endpoint has an open atomic write
     * @return false if the endpoint does not have an open atomic write
     */
    bool InAtomicWrite(EndpointId endpoint, Optional<AttributeId> attributeId = NullOptional);

    /**
     * @brief Checks if a given endpoint has an atomic write open for a given subject descriptor, optionally filtered by an
     * attribute ID
     *
     * @param endpoint The endpoint
     * @param subjectDescriptor The subject descriptor for the client making a read or write request
     * @param attributeId The optional attribute ID to filter on
     * @return true if the endpoint has an open atomic write
     * @return false if the endpoint does not have an open atomic write
     */
    bool InAtomicWrite(EndpointId endpoint, const Access::SubjectDescriptor & subjectDescriptor,
                       Optional<AttributeId> attributeId = NullOptional);

    /**
     * @brief Checks if a given endpoint has an atomic write open for a given command invocation, optionally filtered by an
     * attribute ID
     *
     * @param endpoint The endpoint
     * @param commandObj The CommandHandler for the invoked command
     * @param attributeId The optional attribute ID to filter on
     * @return true if the endpoint has an open atomic write
     * @return false if the endpoint does not have an open atomic write
     */
    bool InAtomicWrite(EndpointId endpoint, CommandHandler * commandObj, Optional<AttributeId> attributeId = NullOptional);

    /**
     * @brief Checks if a given endpoint has an atomic write open for a given command invocation and a list of attributes
     *
     * @param endpoint The endpoint
     * @param commandObj The CommandHandler for the invoked command
     * @param attributeStatuses The list of attribute statuses whose attributeIds must match the open atomic write
     * @return true if the endpoint has an open atomic write
     * @return false if the endpoint does not have an open atomic write
     */
    bool
    InAtomicWrite(EndpointId endpoint, CommandHandler * commandObj,
                  Platform::ScopedMemoryBufferWithSize<Globals::Structs::AtomicAttributeStatusStruct::Type> & attributeStatuses);

    /**
     * @brief Handles an AtomicRequest of type BeginWrite
     *
     * @param commandObj The AtomicRequest command handler
     * @param commandPath The path for the Atomic Request command
     * @param commandData The payload data for the Atomic Request
     */
    void BeginAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                          const Commands::AtomicRequest::DecodableType & commandData);

    /**
     * @brief Handles an AtomicRequest of type CommitWrite
     *
     * @param commandObj The AtomicRequest command handler
     * @param commandPath The path for the Atomic Request command
     * @param commandData The payload data for the Atomic Request
     */
    void CommitAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                           const Commands::AtomicRequest::DecodableType & commandData);

    /**
     * @brief Handles an AtomicRequest of type RollbackWrite
     *
     * @param commandObj The AtomicRequest command handler
     * @param commandPath The path for the Atomic Request command
     * @param commandData The payload data for the Atomic Request
     */
    void RollbackAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                             const Commands::AtomicRequest::DecodableType & commandData);

    friend void TimerExpiredCallback(System::Layer * systemLayer, void * callbackContext);

    friend bool emberAfThermostatClusterSetActivePresetRequestCallback(
        CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
        const Clusters::Thermostat::Commands::SetActivePresetRequest::DecodableType & commandData);

    friend bool
    emberAfThermostatClusterAtomicRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                  const Clusters::Thermostat::Commands::AtomicRequest::DecodableType & commandData);

    friend bool emberAfThermostatClusterAddThermostatSuggestionCallback(
        CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
        const Clusters::Thermostat::Commands::AddThermostatSuggestion::DecodableType & commandData);

    friend bool emberAfThermostatClusterRemoveThermostatSuggestionCallback(
        CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
        const Clusters::Thermostat::Commands::RemoveThermostatSuggestion::DecodableType & commandData);

    struct AtomicWriteSession
    {
        AtomicWriteState state = AtomicWriteState::Closed;
        Platform::ScopedMemoryBufferWithSize<AttributeId> attributeIds;
        ScopedNodeId nodeId;
        EndpointId endpointId = kInvalidEndpointId;
    };

    AtomicWriteSession mAtomicWriteSessions[kThermostatEndpointCount];
};

/// Retained singleton owning atomic-write/preset/suggestion state during the migration.
/// Defined in ThermostatCluster.cpp; registered as a FabricTable::Delegate by the codegen integration.
extern ThermostatAttrAccess gThermostatAttrAccess;

/**
 * @brief Code-driven Thermostat cluster server.
 *
 * One instance is created per endpoint by the codegen integration layer (see CodegenIntegration.cpp).
 * It owns the scalar (formerly Ember RAM-backed) attributes directly and handles the
 * SetpointRaiseLower command. The complex list/preset/schedule/suggestion attributes and the
 * preset/atomic/suggestion commands are forwarded to the retained ThermostatAttrAccess helper for now.
 */
class ThermostatCluster : public DefaultServerCluster
{
public:
    /// Scalar attribute default values read from the Ember/ZAP configuration by the codegen
    /// integration layer. Members are typed via the generated TypeInfo so they always track the spec.
    struct StartupConfiguration
    {
        Attributes::AbsMinHeatSetpointLimit::TypeInfo::Type absMinHeatSetpointLimit   = 700;
        Attributes::AbsMaxHeatSetpointLimit::TypeInfo::Type absMaxHeatSetpointLimit   = 3000;
        Attributes::AbsMinCoolSetpointLimit::TypeInfo::Type absMinCoolSetpointLimit   = 1600;
        Attributes::AbsMaxCoolSetpointLimit::TypeInfo::Type absMaxCoolSetpointLimit   = 3200;
        Attributes::MinHeatSetpointLimit::TypeInfo::Type minHeatSetpointLimit         = 700;
        Attributes::MaxHeatSetpointLimit::TypeInfo::Type maxHeatSetpointLimit         = 3000;
        Attributes::MinCoolSetpointLimit::TypeInfo::Type minCoolSetpointLimit         = 1600;
        Attributes::MaxCoolSetpointLimit::TypeInfo::Type maxCoolSetpointLimit         = 3200;
        Attributes::OccupiedHeatingSetpoint::TypeInfo::Type occupiedHeatingSetpoint   = 2000;
        Attributes::OccupiedCoolingSetpoint::TypeInfo::Type occupiedCoolingSetpoint   = 2600;
        Attributes::UnoccupiedHeatingSetpoint::TypeInfo::Type unoccupiedHeatingSetpoint = 2000;
        Attributes::UnoccupiedCoolingSetpoint::TypeInfo::Type unoccupiedCoolingSetpoint = 2600;
        Attributes::MinSetpointDeadBand::TypeInfo::Type minSetpointDeadBand           = 20;
        Attributes::ControlSequenceOfOperation::TypeInfo::Type controlSequenceOfOperation =
            ControlSequenceOfOperationEnum::kCoolingAndHeating;
        Attributes::SystemMode::TypeInfo::Type systemMode = SystemModeEnum::kAuto;
    };

    ThermostatCluster(EndpointId endpointId, uint32_t featureMap, const StartupConfiguration & config);

    void SetDelegate(Delegate * delegate) { mDelegate = delegate; }
    Delegate * GetDelegate() const { return mDelegate; }

    // ServerClusterInterface
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    uint32_t GetFeatureMap() const { return mFeatureMap; }

private:
    EndpointId GetEndpointId() const { return mPath.mEndpointId; }

    bool FeatureSupported(Feature feature) const { return (mFeatureMap & static_cast<uint32_t>(feature)) != 0; }

    // SetpointRaiseLower handling (ported from emberAfThermostatClusterSetpointRaiseLowerCallback).
    Protocols::InteractionModel::Status HandleSetpointRaiseLower(const Commands::SetpointRaiseLower::DecodableType & commandData);

    // Setpoint limit enforcement (ported from the free functions in the legacy implementation), reading
    // the authoritative limits from this instance's members.
    int16_t EnforceHeatingSetpointLimits(int16_t heatingSetpoint) const;
    int16_t EnforceCoolingSetpointLimits(int16_t coolingSetpoint) const;

    // Deadband, in 0.01C units, or 0 when AutoMode is not supported.
    int16_t DeadBandTemp() const
    {
        return FeatureSupported(Feature::kAutoMode) ? static_cast<int16_t>(mMinSetpointDeadBand * 10) : 0;
    }

    // Post-write side effects for a setpoint attribute (replaces MatterThermostatClusterServerAttributeChangedCallback):
    // shifts the paired setpoint to maintain the deadband, emits change events, and clears the active preset.
    void HandleSetpointPostWrite(AttributeId attributeId);

    // Emits the relevant change event for a just-written scalar attribute, if the Events feature is supported.
    void GenerateScalarChangeEvent(AttributeId attributeId);

    Delegate * mDelegate = nullptr;
    const uint32_t mFeatureMap;

    // Scalar attribute state (formerly Ember RAM-backed). Typed through the generated TypeInfo.
    Attributes::LocalTemperature::TypeInfo::Type mLocalTemperature{};
    Attributes::OutdoorTemperature::TypeInfo::Type mOutdoorTemperature{};
    Attributes::Occupancy::TypeInfo::Type mOccupancy{};
    Attributes::AbsMinHeatSetpointLimit::TypeInfo::Type mAbsMinHeatSetpointLimit;
    Attributes::AbsMaxHeatSetpointLimit::TypeInfo::Type mAbsMaxHeatSetpointLimit;
    Attributes::AbsMinCoolSetpointLimit::TypeInfo::Type mAbsMinCoolSetpointLimit;
    Attributes::AbsMaxCoolSetpointLimit::TypeInfo::Type mAbsMaxCoolSetpointLimit;
    Attributes::PICoolingDemand::TypeInfo::Type mPICoolingDemand{};
    Attributes::PIHeatingDemand::TypeInfo::Type mPIHeatingDemand{};
    Attributes::HVACSystemTypeConfiguration::TypeInfo::Type mHVACSystemTypeConfiguration{};
    Attributes::LocalTemperatureCalibration::TypeInfo::Type mLocalTemperatureCalibration{};
    Attributes::OccupiedCoolingSetpoint::TypeInfo::Type mOccupiedCoolingSetpoint;
    Attributes::OccupiedHeatingSetpoint::TypeInfo::Type mOccupiedHeatingSetpoint;
    Attributes::UnoccupiedCoolingSetpoint::TypeInfo::Type mUnoccupiedCoolingSetpoint;
    Attributes::UnoccupiedHeatingSetpoint::TypeInfo::Type mUnoccupiedHeatingSetpoint;
    Attributes::MinHeatSetpointLimit::TypeInfo::Type mMinHeatSetpointLimit;
    Attributes::MaxHeatSetpointLimit::TypeInfo::Type mMaxHeatSetpointLimit;
    Attributes::MinCoolSetpointLimit::TypeInfo::Type mMinCoolSetpointLimit;
    Attributes::MaxCoolSetpointLimit::TypeInfo::Type mMaxCoolSetpointLimit;
    Attributes::MinSetpointDeadBand::TypeInfo::Type mMinSetpointDeadBand;
    Attributes::RemoteSensing::TypeInfo::Type mRemoteSensing{};
    Attributes::ControlSequenceOfOperation::TypeInfo::Type mControlSequenceOfOperation;
    Attributes::SystemMode::TypeInfo::Type mSystemMode;
    Attributes::ThermostatRunningMode::TypeInfo::Type mThermostatRunningMode{};
    Attributes::StartOfWeek::TypeInfo::Type mStartOfWeek{};
    Attributes::NumberOfWeeklyTransitions::TypeInfo::Type mNumberOfWeeklyTransitions{};
    Attributes::NumberOfDailyTransitions::TypeInfo::Type mNumberOfDailyTransitions{};
    Attributes::TemperatureSetpointHold::TypeInfo::Type mTemperatureSetpointHold{};
    Attributes::TemperatureSetpointHoldDuration::TypeInfo::Type mTemperatureSetpointHoldDuration{};
    Attributes::ThermostatProgrammingOperationMode::TypeInfo::Type mThermostatProgrammingOperationMode{};
    Attributes::ThermostatRunningState::TypeInfo::Type mThermostatRunningState{};
    Attributes::SetpointChangeSource::TypeInfo::Type mSetpointChangeSource{};
    Attributes::SetpointChangeAmount::TypeInfo::Type mSetpointChangeAmount{};
    Attributes::SetpointChangeSourceTimestamp::TypeInfo::Type mSetpointChangeSourceTimestamp{};
    Attributes::OccupiedSetback::TypeInfo::Type mOccupiedSetback{};
    Attributes::OccupiedSetbackMin::TypeInfo::Type mOccupiedSetbackMin{};
    Attributes::OccupiedSetbackMax::TypeInfo::Type mOccupiedSetbackMax{};
    Attributes::UnoccupiedSetback::TypeInfo::Type mUnoccupiedSetback{};
    Attributes::UnoccupiedSetbackMin::TypeInfo::Type mUnoccupiedSetbackMin{};
    Attributes::UnoccupiedSetbackMax::TypeInfo::Type mUnoccupiedSetbackMax{};
    Attributes::EmergencyHeatDelta::TypeInfo::Type mEmergencyHeatDelta{};
    Attributes::ACType::TypeInfo::Type mACType{};
    Attributes::ACCapacity::TypeInfo::Type mACCapacity{};
    Attributes::ACRefrigerantType::TypeInfo::Type mACRefrigerantType{};
    Attributes::ACCompressorType::TypeInfo::Type mACCompressorType{};
    Attributes::ACErrorCode::TypeInfo::Type mACErrorCode{};
    // ACLouverPositionEnum has no 0 value (0 == kUnknownEnumValue, which must never be transmitted), so
    // default to the first valid value rather than {} to avoid emitting a constraint-invalid enum.
    Attributes::ACLouverPosition::TypeInfo::Type mACLouverPosition{ ACLouverPositionEnum::kClosed };
    Attributes::ACCoilTemperature::TypeInfo::Type mACCoilTemperature{};
    Attributes::ACCapacityformat::TypeInfo::Type mACCapacityformat{};
    Attributes::NumberOfSchedules::TypeInfo::Type mNumberOfSchedules{};
    Attributes::NumberOfScheduleTransitions::TypeInfo::Type mNumberOfScheduleTransitions{};
    Attributes::NumberOfScheduleTransitionPerDay::TypeInfo::Type mNumberOfScheduleTransitionPerDay{};
    Attributes::ActiveScheduleHandle::TypeInfo::Type mActiveScheduleHandle{};
    Attributes::SetpointHoldExpiryTimestamp::TypeInfo::Type mSetpointHoldExpiryTimestamp{};
};

/**
 * @brief Sets the default delegate for the  specific thermostat features.
 *
 * @param[in] endpoint The endpoint to set the default delegate on.
 * @param[in] delegate The default delegate.
 */
void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

Delegate * GetDelegate(EndpointId endpoint);

// These functions were removed from the ember headers.
 bool emberAfThermostatClusterSetActivePresetRequestCallback(
        CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
        const Clusters::Thermostat::Commands::SetActivePresetRequest::DecodableType & commandData);

 bool
    emberAfThermostatClusterAtomicRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                  const Clusters::Thermostat::Commands::AtomicRequest::DecodableType & commandData);

 bool emberAfThermostatClusterAddThermostatSuggestionCallback(
        CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
        const Clusters::Thermostat::Commands::AddThermostatSuggestion::DecodableType & commandData);

 bool emberAfThermostatClusterRemoveThermostatSuggestionCallback(
        CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
        const Clusters::Thermostat::Commands::RemoveThermostatSuggestion::DecodableType & commandData);
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
