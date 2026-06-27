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

/**
 * @brief Code-driven Thermostat cluster server.
 *
 * One instance is created per endpoint by the codegen integration layer (see CodegenIntegration.cpp).
 * It owns the scalar (formerly Ember RAM-backed) attributes directly, handles the SetpointRaiseLower
 * command, and (since PR 3b-1) owns the atomic-write state machine and registers itself as a
 * FabricTable::Delegate so open atomic writes are rolled back when their fabric is removed. The complex
 * list/preset/schedule/suggestion attributes and the preset/suggestion commands are still forwarded to the
 * retained ThermostatAttrAccess helper.
 */
class ThermostatCluster : public DefaultServerCluster, public chip::FabricTable::Delegate
{
public:
    /// External dependencies injected at construction (see CodegenIntegration.cpp). Kept as a struct so
    /// additional dependencies can be threaded in by later migration steps without churning call sites.
    struct Context
    {
        chip::FabricTable & fabricTable;
    };

    /// Scalar attribute default values read from the Ember/ZAP configuration by the codegen
    /// integration layer. Members are typed via the generated TypeInfo so they always track the spec.
    struct StartupConfiguration
    {
        int16_t absMinHeatSetpointLimit   = 700; // 7C (44.5 F) is the default
        int16_t absMaxHeatSetpointLimit   = 3000; // 30C (86 F) is the default
        int16_t absMinCoolSetpointLimit   = 1600; // 16C (61 F) is the default
        int16_t absMaxCoolSetpointLimit   = 3200; // 32C (90 F) is the default
        int16_t minHeatSetpointLimit         = 700; // 7C (44.5 F) is the default
        int16_t maxHeatSetpointLimit         = 3000; // 30C (86 F) is the default
        int16_t minCoolSetpointLimit         = 1600; // 16C (61 F) is the default
        int16_t maxCoolSetpointLimit         = 3200; // 32C (90 F) is the default
        Attributes::OccupiedHeatingSetpoint::TypeInfo::Type occupiedHeatingSetpoint   = 2000;
        Attributes::OccupiedCoolingSetpoint::TypeInfo::Type occupiedCoolingSetpoint   = 2600;
        Attributes::UnoccupiedHeatingSetpoint::TypeInfo::Type unoccupiedHeatingSetpoint = 2000;
        Attributes::UnoccupiedCoolingSetpoint::TypeInfo::Type unoccupiedCoolingSetpoint = 2600;
        int8_t minSetpointDeadBand           = 20; // 2.0C is the default
        Attributes::ControlSequenceOfOperation::TypeInfo::Type controlSequenceOfOperation =
            ControlSequenceOfOperationEnum::kCoolingAndHeating;
        Attributes::SystemMode::TypeInfo::Type systemMode = SystemModeEnum::kAuto;
    };

    ThermostatCluster(EndpointId endpointId, uint32_t featureMap, const StartupConfiguration & config, const Context & context);

    void SetDelegate(Thermostat::Delegate * delegate) { mDelegate = delegate; }
    Thermostat::Delegate * GetDelegate() const { return mDelegate; }

    EndpointId GetEndpointId() const { return mPath.mEndpointId; }

    // Setters for device-set read-only attributes. Each notifies subscribers and, when the Events
    // feature is enabled, emits the corresponding Matter event — matching the behaviour of the legacy
    // MatterThermostatClusterServerAttributeChangedCallback / GenerateEvents path.
    void SetLocalTemperature(DataModel::Nullable<int16_t> value);
    void SetOccupancy(BitMask<OccupancyBitmap> value);
    void SetThermostatRunningState(BitMask<RelayStateBitmap> value);
    void SetThermostatRunningMode(ThermostatRunningModeEnum value);

    // Setters for device-set read-only attributes that do not generate events.
    void SetOutdoorTemperature(DataModel::Nullable<int16_t> value);
    void SetSetpointChangeSource(SetpointChangeSourceEnum value);
    void SetSetpointChangeAmount(DataModel::Nullable<int16_t> value);
    void SetSetpointChangeSourceTimestamp(uint32_t value);
    void SetACCoilTemperature(DataModel::Nullable<int16_t> value);
    // Handle setters also persist their values across reboots.
    void SetActivePresetHandle(DataModel::Nullable<ByteSpan> value);
    void SetActiveScheduleHandle(DataModel::Nullable<ByteSpan> value);

    // ServerClusterInterface
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;
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


    uint32_t GetFeatureMap() const { return mFeatures.Raw(); }

    //
    // Atomic-write state machine (moved from ThermostatAttrAccess in PR 3b-1). The session is per-instance;
    // every method operates on this cluster's endpoint (the `endpoint` parameters must equal GetEndpointId()).
    //

    /// @brief Checks if an atomic write is open, optionally filtered by an attribute ID.
    bool InAtomicWrite(EndpointId endpoint, Optional<AttributeId> attributeId = NullOptional);

    /// @brief Checks if an atomic write is open for the given subject descriptor, optionally filtered by an attribute ID.
    bool InAtomicWrite(EndpointId endpoint, const Access::SubjectDescriptor & subjectDescriptor,
                       Optional<AttributeId> attributeId = NullOptional);

    /// @brief Checks if an atomic write is open for the given command invocation, optionally filtered by an attribute ID.
    bool InAtomicWrite(EndpointId endpoint, CommandHandler * commandObj, Optional<AttributeId> attributeId = NullOptional);

    /// @brief Checks if an atomic write is open for the given command invocation and a list of attributes.
    bool
    InAtomicWrite(EndpointId endpoint, CommandHandler * commandObj,
                  Platform::ScopedMemoryBufferWithSize<Globals::Structs::AtomicAttributeStatusStruct::Type> & attributeStatuses);

    /// @brief Sets the atomic write state for the given originatorNodeId.
    bool
    SetAtomicWrite(EndpointId endpoint, ScopedNodeId originatorNodeId, AtomicWriteState state,
                   Platform::ScopedMemoryBufferWithSize<Globals::Structs::AtomicAttributeStatusStruct::Type> & attributeStatuses);

    /// @brief Resets (closes) the atomic write, clears the pending preset list and cancels the timeout timer.
    void ResetAtomicWrite(EndpointId endpoint);

    /// @brief Gets the scoped node id of the originator of the open atomic write, or ScopedNodeId() if none.
    ScopedNodeId GetAtomicWriteOriginatorScopedNodeId(EndpointId endpoint);

    void BeginAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                          const Commands::AtomicRequest::DecodableType & commandData);
    void CommitAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                           const Commands::AtomicRequest::DecodableType & commandData);
    void RollbackAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                             const Commands::AtomicRequest::DecodableType & commandData);

private:
    // FabricTable::Delegate: roll back an open atomic write originated by a removed fabric.
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;
    // @brief Sets the ActivePresetHandle to the given handle (or null), validating it against the Presets list.
    Protocols::InteractionModel::Status SetActivePreset(DataModel::Nullable<ByteSpan> presetHandle);

    // @brief Validates that the delegate's pending-preset list satisfies the spec constraints before commit.
    Protocols::InteractionModel::Status PrecommitPresets();

    // @brief Validates and appends a preset to the delegate's pending-preset list during an atomic write.
    CHIP_ERROR AppendPendingPreset(const Structs::PresetStruct::Type & preset);

    // Delegate-backed attribute encode/decode (presets / schedules / suggestions). Scalar attributes are
    // handled directly by ReadAttribute / WriteAttribute.
    CHIP_ERROR ReadDelegateAttribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteDelegateAttribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);

    // Suggestion command handlers (each writes its response/status onto commandObj directly).
    bool HandleAddThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                       const Commands::AddThermostatSuggestion::DecodableType & commandData);
    bool HandleRemoveThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                          const Commands::RemoveThermostatSuggestion::DecodableType & commandData);

    // SetpointRaiseLower handling (ported from emberAfThermostatClusterSetpointRaiseLowerCallback).
    Protocols::InteractionModel::Status HandleSetpointRaiseLower(const Commands::SetpointRaiseLower::DecodableType & commandData);

    // Setpoint limit enforcement (ported from the free functions in the legacy implementation), reading
    // the authoritative limits from this instance's members.
    int16_t EnforceHeatingSetpointLimits(int16_t heatingSetpoint) const;
    int16_t EnforceCoolingSetpointLimits(int16_t coolingSetpoint) const;

    // Deadband, in 0.01C units, or 0 when AutoMode is not supported.
    int16_t DeadBandTemp() const
    {
        return mFeatures.Has(Feature::kAutoMode) ? static_cast<int16_t>(mMinSetpointDeadBand * 10) : 0;
    }

    // Post-write side effects for a setpoint attribute (replaces MatterThermostatClusterServerAttributeChangedCallback):
    // shifts the paired setpoint to maintain the deadband, emits change events, and clears the active preset.
    void HandleSetpointPostWrite(AttributeId attributeId);

    // Emits the relevant change event for a just-written scalar attribute, if the Events feature is supported.
    void GenerateScalarChangeEvent(AttributeId attributeId);
    void LoadPersistentAttributes();

    Context mContext;
    Thermostat::Delegate * mDelegate = nullptr;
    const BitFlags<Thermostat::Feature> mFeatures;

    struct AtomicWriteSession
    {
        AtomicWriteState state = AtomicWriteState::Closed;
        Platform::ScopedMemoryBufferWithSize<AttributeId> attributeIds;
        ScopedNodeId nodeId;
        EndpointId endpointId = kInvalidEndpointId;
    };
    AtomicWriteSession mAtomicWriteSession;

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
    // mActivePresetHandle and mActiveScheduleHandle hold non-owning ByteSpans; the backing
    // buffers below own the bytes so the spans remain valid for the lifetime of the cluster.
    uint8_t mActivePresetHandleBuffer[kPresetHandleSize]{};
    uint8_t mActiveScheduleHandleBuffer[kPresetHandleSize]{};
    Attributes::ActivePresetHandle::TypeInfo::Type mActivePresetHandle{};
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

// These ember command-callback declarations were removed from the generated headers once the cluster
// became code-driven; they are still defined in the suggestion source file and invoked (by name)
// from ThermostatCluster::InvokeCommand.
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
