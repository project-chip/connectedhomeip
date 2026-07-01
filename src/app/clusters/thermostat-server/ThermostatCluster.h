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

enum class OptionalAttributesBits : uint32_t
{
    kOutdoorTemperature                     = 0x1,
    kAbsMinHeatSetpointLimit                = 0x2,
    kAbsMaxHeatSetpointLimit                = 0x4,
    kAbsMinCoolSetpointLimit                = 0x8,
    kAbsMaxCoolSetpointLimit                = 0x10,
    kLocalTemperatureCalibration            = 0x20,
    kMinHeatSetpointLimit                   = 0x40,
    kMaxHeatSetpointLimit                   = 0x80,
    kMinCoolSetpointLimit                   = 0x100,
    kMaxCoolSetpointLimit                   = 0x200,
    kRemoteSensing                          = 0x400,
    kThermostatRunningMode                  = 0x800,
    kTemperatureSetpointHold                = 0x1000,
    kTemperatureSetpointHoldDuration        = 0x2000,
    kThermostatRunningState                 = 0x4000,
    kSetpointChangeSource                   = 0x8000,
    kSetpointChangeAmount                   = 0x10000,
    kSetpointChangeSourceTimestamp          = 0x20000,
    kEmergencyHeatDelta                     = 0x40000,
    kACType                                 = 0x80000,
    kACCapacity                             = 0x100000,
    kACRefrigerantType                      = 0x200000,
    kACCompressorType                       = 0x400000,
    kACErrorCode                            = 0x800000,
    kACLouverPosition                       = 0x1000000,
    kACCoilTemperature                      = 0x2000000,
    kACCapacityFormat                       = 0x4000000,
    kSetpointHoldExpiryTimestamp            = 0x8000000,
};

/**
 * @brief Code-driven Thermostat cluster server.
 *
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
        uint8_t numberOfSchedules;
        uint8_t numberOfScheduleTransitions;
        DataModel::Nullable<uint8_t> numberOfScheduleTransitionPerDay;
    };

    struct Config
    {
        Config(FabricTable & fabricTable, uint32_t featureMap, BitFlags<Thermostat::OptionalAttributesBits> optionalAttributes) 
        : fabricTable(fabricTable), features(featureMap), optionalAttributes(optionalAttributes) {};

        FabricTable & fabricTable;
        BitFlags<Thermostat::Feature> features;
        BitFlags<Thermostat::OptionalAttributesBits> optionalAttributes;
    };

    ThermostatCluster(EndpointId endpointId, const StartupConfiguration & config, const Config & gConfig);

    void SetDelegate(Thermostat::Delegate * delegate) { mDelegate = delegate; }
    Thermostat::Delegate * GetDelegate() const { return mDelegate; }

    EndpointId GetEndpointId() const { return mPath.mEndpointId; }

    // Setters
    // LocalTemperature
    void SetLocalTemperature(DataModel::Nullable<int16_t> value);
    DataModel::Nullable<int16_t> GetLocalTemperature();

    // OutdoorTemperature
    void SetOutdoorTemperature(DataModel::Nullable<int16_t> value);
    DataModel::Nullable<int16_t> GetOutdoorTemperature();

    // Occupancy
    void SetOccupancy(BitMask<OccupancyBitmap> value);
    BitMask<chip::app::Clusters::Thermostat::OccupancyBitmap> GetOccupancy();

    // LocalTemperatureCalibration
    void SetLocalTemperatureCalibration(int8_t value);
    int8_t GetLocalTemperatureCalibration();

    // OccupiedCoolingSetpoint
    Protocols::InteractionModel::Status SetOccupiedCoolingSetpoint(int16_t value);
    int16_t GetOccupiedCoolingSetpoint();

    // OccupiedHeatingSetpoint
    Protocols::InteractionModel::Status SetOccupiedHeatingSetpoint(int16_t value);
    int16_t GetOccupiedHeatingSetpoint();

    // UnoccupiedCoolingSetpoint
    Protocols::InteractionModel::Status SetUnoccupiedCoolingSetpoint(int16_t value);
    int16_t GetUnoccupiedCoolingSetpoint();

    // UnoccupiedHeatingSetpoint
    Protocols::InteractionModel::Status SetUnoccupiedHeatingSetpoint(int16_t value);
    int16_t GetUnoccupiedHeatingSetpoint();

    // MinHeatSetpointLimit
    Protocols::InteractionModel::Status SetMinHeatSetpointLimit(int16_t value);
    int16_t GetMinHeatSetpointLimit();

    // MaxHeatSetpointLimit
    Protocols::InteractionModel::Status SetMaxHeatSetpointLimit(int16_t value);
    int16_t GetMaxHeatSetpointLimit();

    // MinCoolSetpointLimit
    Protocols::InteractionModel::Status SetMinCoolSetpointLimit(int16_t value);
    int16_t GetMinCoolSetpointLimit();

    // MaxCoolSetpointLimit
    Protocols::InteractionModel::Status SetMaxCoolSetpointLimit(int16_t value);
    int16_t GetMaxCoolSetpointLimit();

    // MinSetpointDeadbanc
    Protocols::InteractionModel::Status SetMinSetpointDeadbanc(int8_t value);
    int8_t GetMinSetpointDeadbanc();

    // RemoteSensing
    Protocols::InteractionModel::Status SetRemoteSensing(BitMask<RemoteSensingBitmap> value);
    BitMask<RemoteSensingBitmap> GetRemoteSensing();

    // ControlSequenceOfOperation
    Protocols::InteractionModel::Status SetControlSequenceOfOperation(ControlSequenceOfOperationEnum value);
    ControlSequenceOfOperationEnum GetControlSequenceOfOperation();

    // SystemMode
    Protocols::InteractionModel::Status SetSystemMode(SystemModeEnum value);
    SystemModeEnum GetSystemMode();

    // ThermostatRunningMode
    void SetThermostatRunningMode(ThermostatRunningModeEnum value);
    ThermostatRunningModeEnum GetThermostatRunningMode();

    // TemperatureSetpointHold
    void SetTemperatureSetpointHold(TemperatureSetpointHoldEnum value);
    TemperatureSetpointHoldEnum GetTemperatureSetpointHold();

    // TemperatureSetpointHoldDuration
    void SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> value);
    DataModel::Nullable<uint16_t> GetTemperatureSetpointHoldDuration();

    // ThermostatRunningState
    void SetThermostatRunningState(BitMask<RelayStateBitmap> value);
    BitMask<RelayStateBitmap> GetThermostatRunningState();

    // SetpointChangeSource
    void SetSetpointChangeSource(SetpointChangeSourceEnum value);
    SetpointChangeSourceEnum GetSetpointChangeSource();

    // SetpointChangeAmount
    void SetSetpointChangeAmount(DataModel::Nullable<int16_t> value);
    DataModel::Nullable<int16_t> GetSetpointChangeAmount();

    // SetpointChangeSourceTimestamp
    void SetSetpointChangeSourceTimestamp(uint32_t value);
    uint32_t GetSetpointChangeSourceTimestamp();

    // EmergencyHeatDelta
    void SetEmergencyHeatDelta(uint8_t value);
    uint8_t GetEmergencyHeatDelta();

    // ACType
    void SetACType(ACTypeEnum value);
    ACTypeEnum GetACType();

    // ACCapacity
    void SetACCapacity(uint16_t value);
    uint16_t GetACCapacity();

    // ACCapacity
    void SetACRefrigerantType(ACRefrigerantTypeEnum value);
    ACRefrigerantTypeEnum GetACRefrigerantType();

    // ACCompressorType
    void SetACCompressorType(ACCompressorTypeEnum value);
    ACCompressorTypeEnum GetACCompressorType();

    // ACErrorCode
    void SetACErrorCode(BitMask<chip::app::Clusters::Thermostat::ACErrorCodeBitmap> value);
    BitMask<chip::app::Clusters::Thermostat::ACErrorCodeBitmap> GetACErrorCode();

    // ACLouverPosition
    void SetACLouverPosition(ACLouverPositionEnum value);
    ACLouverPositionEnum GetACLouverPosition();

    // ACCoilTemperature
    void SetACCoilTemperature(DataModel::Nullable<int16_t> value);
    DataModel::Nullable<int16_t> GetACCoilTemperature();

    // ACCapacityFormat
    void SetACCapacityFormat(ACCapacityFormatEnum value);
    ACCapacityFormatEnum GetACCapacityFormat();

    // NumberOfSchedules
    void SetNumberOfSchedules(uint8_t value);
    uint8_t GetNumberOfSchedules();

    // NumberOfScheduleTransitions
    void SetNumberOfScheduleTransitions(uint8_t value);
    uint8_t GetNumberOfScheduleTransitions();

    // NumberOfScheduleTransitionPerDay
    void SetNumberOfScheduleTransitionPerDay(DataModel::Nullable<uint8_t> value);
    DataModel::Nullable<uint8_t> GetNumberOfScheduleTransitionPerDay();

    // SetpointHoldExpiryTimestamp
    void SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> value);
    DataModel::Nullable<uint32_t> GetSetpointHoldExpiryTimestamp();

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
    /// @brief Gets the scoped node id of the originator of the open atomic write, or ScopedNodeId() if none.
    ScopedNodeId GetAtomicWriteOriginatorScopedNodeId(EndpointId endpoint);

    /// @brief Sets the atomic write state for the given originatorNodeId.
    bool
    SetAtomicWrite(EndpointId endpoint, ScopedNodeId originatorNodeId, AtomicWriteState state,
                   Platform::ScopedMemoryBufferWithSize<Globals::Structs::AtomicAttributeStatusStruct::Type> & attributeStatuses);

    /// @brief Resets (closes) the atomic write, clears the pending preset list and cancels the timeout timer.
    void ResetAtomicWrite(EndpointId endpoint);


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
    Protocols::InteractionModel::Status HandleOccupancyHeatingSetpoint(int16_t value, AttributeId attributeId);
    Protocols::InteractionModel::Status HandleOccupancyCoolingSetpoint(int16_t value, AttributeId attributeId);
    Protocols::InteractionModel::Status HandleMinHeatSetpointLimit(int16_t value);
    Protocols::InteractionModel::Status HandleMaxHeatSetpointLimit(int16_t value);
    Protocols::InteractionModel::Status HandleMinCoolSetpointLimit(int16_t value);
    Protocols::InteractionModel::Status HandleMaxCoolSetpointLimit(int16_t value);
    Protocols::InteractionModel::Status HandleMinSetpointDeadband(int8_t value);
    Protocols::InteractionModel::Status HandleRemoteSensing(BitMask<RemoteSensingBitmap> value);
    Protocols::InteractionModel::Status HandleControlSequenceOfOperation(ControlSequenceOfOperationEnum value);
    Protocols::InteractionModel::Status HandleSystemMode(SystemModeEnum value);

    // Sets member to value and, if it changed, persists it to attribute storage. Returns true if changed.
    bool SetAndPersistSetpoint(int16_t & member, int16_t value, AttributeId attributeId);

    // Emits the relevant change event for a just-written scalar attribute, if the Events feature is supported.
    void GenerateScalarChangeEvent(AttributeId attributeId);
    void LoadPersistentAttributes();

    FabricTable & mFabricTable;
    Thermostat::Delegate * mDelegate = nullptr;
    const BitFlags<Thermostat::Feature> mFeatures;
    const BitFlags<Thermostat::OptionalAttributesBits> mOptionalAttributes;

    struct AtomicWriteSession
    {
        AtomicWriteState state = AtomicWriteState::Closed;
        Platform::ScopedMemoryBufferWithSize<AttributeId> attributeIds;
        ScopedNodeId nodeId;
        EndpointId endpointId = kInvalidEndpointId;
    };
    AtomicWriteSession mAtomicWriteSession;

    // Scalar attribute state (formerly Ember RAM-backed). Typed through the generated TypeInfo.
    DataModel::Nullable<int16_t> mLocalTemperature{};
    DataModel::Nullable<int16_t> mOutdoorTemperature{};
    BitMask<chip::app::Clusters::Thermostat::OccupancyBitmap> mOccupancy{};
    int16_t mAbsMinHeatSetpointLimit;
    int16_t mAbsMaxHeatSetpointLimit;
    int16_t mAbsMinCoolSetpointLimit;
    int16_t mAbsMaxCoolSetpointLimit;
    int8_t mLocalTemperatureCalibration{ 0 };
    int16_t mOccupiedCoolingSetpoint { 2600 };
    int16_t mOccupiedHeatingSetpoint { 2000 };
    int16_t mUnoccupiedCoolingSetpoint { 2600 };
    int16_t mUnoccupiedHeatingSetpoint { 2000 };
    int16_t mMinHeatSetpointLimit;
    int16_t mMaxHeatSetpointLimit;
    int16_t mMinCoolSetpointLimit;
    int16_t mMaxCoolSetpointLimit;
    int8_t mMinSetpointDeadBand { 20 };
    BitMask<chip::app::Clusters::Thermostat::RemoteSensingBitmap> mRemoteSensing{ 0 };
    ControlSequenceOfOperationEnum mControlSequenceOfOperation;
    SystemModeEnum mSystemMode;
    ThermostatRunningModeEnum mThermostatRunningMode{};
    TemperatureSetpointHoldEnum mTemperatureSetpointHold{};
    DataModel::Nullable<uint16_t> mTemperatureSetpointHoldDuration{};
    BitMask<chip::app::Clusters::Thermostat::RelayStateBitmap> mThermostatRunningState{};
    SetpointChangeSourceEnum mSetpointChangeSource{};
    DataModel::Nullable<int16_t> mSetpointChangeAmount{};
    uint32_t mSetpointChangeSourceTimestamp{};
    uint8_t mEmergencyHeatDelta{};
    ACTypeEnum mACType{};
    uint16_t mACCapacity{};
    ACRefrigerantTypeEnum mACRefrigerantType{};
    ACCompressorTypeEnum mACCompressorType{};
    BitMask<chip::app::Clusters::Thermostat::ACErrorCodeBitmap> mACErrorCode{};
    // ACLouverPositionEnum has no 0 value (0 == kUnknownEnumValue, which must never be transmitted), so
    // default to the first valid value rather than {} to avoid emitting a constraint-invalid enum.
    ACLouverPositionEnum mACLouverPosition{ ACLouverPositionEnum::kClosed };
    DataModel::Nullable<int16_t> mACCoilTemperature{};
    ACCapacityFormatEnum mACCapacityFormat{};
    uint8_t mNumberOfSchedules{};
    uint8_t mNumberOfScheduleTransitions{};
    DataModel::Nullable<uint8_t> mNumberOfScheduleTransitionPerDay{};
    // mActivePresetHandle and mActiveScheduleHandle hold non-owning ByteSpans; the backing
    // buffers below own the bytes so the spans remain valid for the lifetime of the cluster.
    uint8_t mActivePresetHandleBuffer[kPresetHandleSize]{};
    uint8_t mActiveScheduleHandleBuffer[kPresetHandleSize]{};
    Attributes::ActivePresetHandle::TypeInfo::Type mActivePresetHandle{};
    Attributes::ActiveScheduleHandle::TypeInfo::Type mActiveScheduleHandle{};
    DataModel::Nullable<uint32_t> mSetpointHoldExpiryTimestamp{};
};

/**
 * @brief Sets the default delegate for the  specific thermostat features.
 *
 * @param[in] endpoint The endpoint to set the default delegate on.
 * @param[in] delegate The default delegate.
 */
void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

Delegate * GetDelegate(EndpointId endpoint);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
