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

#include "SetpointRange.h"
#include "Setpoints.h"

#include "ThermostatClusterAtomic.h"
<<<<<<< HEAD
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
=======
#include "ThermostatClusterBase.h"
#include "ThermostatClusterHold.h"
#include "ThermostatClusterOccupancy.h"
#include "ThermostatClusterPresets.h"
#include "ThermostatClusterSensors.h"
#include "ThermostatClusterSetpoints.h"
#include "ThermostatClusterSuggestions.h"
#include <clusters/Thermostat/Metadata.h>
#include <type_traits>
#include <variant>
>>>>>>> 3888116 ([HVAC] Initial implementation of Thermostat Sensors (#73484))

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatCluster : public DefaultServerCluster, private chip::FabricTable::Delegate, private AtomicWriteSession::Delegate
{

public:
<<<<<<< HEAD
    struct OptionalAttributes
=======
    static constexpr bool kHasHeating          = detail::kArgsHasDelegate<ThermostatHeatingSetpoints::Delegate, Delegates...>;
    static constexpr bool kHasCooling          = detail::kArgsHasDelegate<ThermostatCoolingSetpoints::Delegate, Delegates...>;
    static constexpr bool kHasPresets          = detail::kArgsHasDelegate<ThermostatPresets::Delegate, Delegates...>;
    static constexpr bool kHasHold             = detail::kArgsHasDelegate<ThermostatHold::Delegate, Delegates...>;
    static constexpr bool kHasSuggestions      = detail::kArgsHasDelegate<ThermostatSuggestions::Delegate, Delegates...>;
    static constexpr bool kHasOccupancy        = detail::kArgsHasDelegate<ThermostatOccupancy::Delegate, Delegates...>;
    static constexpr bool kHasSensors          = detail::kArgsHasDelegate<ThermostatSensors::Delegate, Delegates...>;
    static constexpr bool kRequiresAtomicWrite = kHasPresets || kHasSensors;

    static_assert(!kHasSuggestions || kHasPresets, "Suggestions feature requires Presets feature");
    static_assert(kHasHeating || kHasCooling, "Thermostat cluster must implement either heating or cooling");

    ThermostatCluster(EndpointId aEndpointId, BitFlags<Thermostat::Feature> features, const Config & config,
                      Delegates &... delegates) :
        ThermostatClusterBase(aEndpointId, features, config, detail::FindDelegate<Thermostat::Delegate>(delegates...)),
        mSetpoints(*this, delegates...), mAtomicWriteSession(detail::MakeAtomicWriteSession<kRequiresAtomicWrite>(
                                             *this, config.mTimerDelegate, mDelegate.GetFabricTable())),
        mHold(detail::MakeFeature<kHasHold, ThermostatHold>(*this, std::forward_as_tuple(delegates...))),
        mPresets(
            detail::MakeFeature<kHasPresets, ThermostatPresets>(*this, mAtomicWriteSession, std::forward_as_tuple(delegates...))),
        mSuggestions(
            detail::MakeFeature<kHasSuggestions, ThermostatSuggestions>(*this, mPresets, std::forward_as_tuple(delegates...))),
        mOccupancy(detail::MakeFeature<kHasOccupancy, ThermostatOccupancy>(*this, std::forward_as_tuple(delegates...))),
        mSensors(
            detail::MakeFeature<kHasSensors, ThermostatSensors>(*this, mAtomicWriteSession, std::forward_as_tuple(delegates...)))
>>>>>>> 3888116 ([HVAC] Initial implementation of Thermostat Sensors (#73484))
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
        temperature absMinHeatSetpointLimit   = kDefaultAbsMinHeatSetpointLimit;
        temperature absMaxHeatSetpointLimit   = kDefaultAbsMaxHeatSetpointLimit;
        temperature absMinCoolSetpointLimit   = kDefaultAbsMinCoolSetpointLimit;
        temperature absMaxCoolSetpointLimit   = kDefaultAbsMaxCoolSetpointLimit;
        int8_t localTemperatureCalibration    = kDefaultLocalTemperatureCalibration;
        temperature occupiedCoolingSetpoint   = kDefaultCoolingSetpoint;
        temperature occupiedHeatingSetpoint   = kDefaultHeatingSetpoint;
        temperature unoccupiedCoolingSetpoint = kDefaultCoolingSetpoint;
        temperature unoccupiedHeatingSetpoint = kDefaultHeatingSetpoint;
        Optional<temperature> minHeatSetpointLimit;
        Optional<temperature> maxHeatSetpointLimit;
        Optional<temperature> minCoolSetpointLimit;
        Optional<temperature> maxCoolSetpointLimit;

        temperature minSetpointDeadBand                           = kDefaultDeadBand;
        ControlSequenceOfOperationEnum controlSequenceOfOperation = ControlSequenceOfOperationEnum::kCoolingAndHeating;
        SystemModeEnum systemMode                                 = SystemModeEnum::kOff;

        TemperatureSetpointHoldEnum temperatureSetpointHold = TemperatureSetpointHoldEnum::kSetpointHoldOff;
        DataModel::Nullable<uint16_t> temperatureSetpointHoldDuration;
        DataModel::Nullable<uint32_t> setpointHoldExpiryTimestamp;

        DefaultValues() = default;
    };

    ThermostatCluster(EndpointId aEndpointId, BitFlags<Thermostat::Feature> features, const OptionalAttributes & optionalAttributes,
                      const DefaultValues & defaultValues, FabricTable & fabricTable);

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
<<<<<<< HEAD
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
=======
                                                AttributeValueEncoder & encoder) override
    {
        if (auto status = mSetpoints.ReadAttribute(request, encoder))
        {
            return *status;
        }
        if constexpr (kHasHold)
        {
            if (auto status = mHold.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        if constexpr (kHasOccupancy)
        {
            if (auto status = mOccupancy.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        if constexpr (kHasSuggestions)
        {
            if (auto status = mSuggestions.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        if constexpr (kHasSensors)
        {
            if (auto status = mSensors.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        return ThermostatClusterBase::ReadAttribute(request, encoder);
    }

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.WriteAttribute(request, decoder))
            {
                return *status;
            }
        }
        if constexpr (kHasSensors)
        {
            if (auto status = mSensors.WriteAttribute(request, decoder))
            {
                return *status;
            }
        }
        if constexpr (kRequiresAtomicWrite)
        {
            auto & subjectDescriptor = decoder.GetSubjectDescriptor();
            if (mAtomicWriteSession.InAtomicWrite(subjectDescriptor))
            {
                ChipLogError(Zcl, "Can not write to non-atomic attribute " ChipLogFormatMEI " during atomic write",
                             ChipLogValueMEI(request.path.mAttributeId));
                return Protocols::InteractionModel::Status::InvalidInState;
            }
        }
        if (auto status = mSetpoints.WriteAttribute(request, decoder))
        {
            if constexpr (kHasPresets)
            {
                if (status->IsSuccess() && IsActiveSetpoint(request.path.mAttributeId))
                {
                    ChipLogProgress(Zcl, "Setting active preset to null");
                    mPresets.SetActivePreset(DataModel::NullNullable);
                }
            }
            return *status;
        }
        if constexpr (kHasHold)
        {
            if (auto status = mHold.WriteAttribute(request, decoder))
            {
                return *status;
            }
        }
        return ThermostatClusterBase::WriteAttribute(request, decoder);
    }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override
    {
        if (auto status = mSetpoints.InvokeCommand(request, input_arguments, handler))
        {
            return status;
        }
        if constexpr (kRequiresAtomicWrite)
        {
            bool handled = false;
            if (auto status = mAtomicWriteSession.InvokeCommand(request, input_arguments, handler, handled))
            {
                return status;
            }
            if (handled)
            {
                return std::nullopt;
            }
        }
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.InvokeCommand(request, input_arguments, handler))
            {
                return status;
            }
        }
        if constexpr (kHasSuggestions)
        {
            bool handled = false;
            if (auto status = mSuggestions.InvokeCommand(request, input_arguments, handler, handled))
            {
                return status;
            }
            if (handled)
            {
                return std::nullopt;
            }
        }
        return ThermostatClusterBase::InvokeCommand(request, input_arguments, handler);
    }
>>>>>>> 3888116 ([HVAC] Initial implementation of Thermostat Sensors (#73484))

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

<<<<<<< HEAD
    BitFlags<Thermostat::Feature> Features() const { return mFeatures; }
    void SetFeatures(BitFlags<Thermostat::Feature> features) { mFeatures = features; }
=======
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        ReturnErrorOnFailure(ThermostatClusterBase::Attributes(path, builder));
        ReturnErrorOnFailure(mSetpoints.Attributes(path, builder));
        if constexpr (kHasHold)
        {
            ReturnErrorOnFailure(mHold.Attributes(path, builder));
        }
        if constexpr (kHasPresets)
        {
            ReturnErrorOnFailure(mPresets.Attributes(path, builder));
        }
        if constexpr (kHasSuggestions)
        {
            ReturnErrorOnFailure(mSuggestions.Attributes(path, builder));
        }
        if constexpr (kHasOccupancy)
        {
            ReturnErrorOnFailure(mOccupancy.Attributes(path, builder));
        }
        if constexpr (kHasSensors)
        {
            ReturnErrorOnFailure(mSensors.Attributes(path, builder));
        }
        return CHIP_NO_ERROR;
    }
>>>>>>> 3888116 ([HVAC] Initial implementation of Thermostat Sensors (#73484))

    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

<<<<<<< HEAD
    EndpointId Endpoint() const { return mPath.mEndpointId; }
    void SetDelegate(Thermostat::Delegate * delegate) { mDelegate = delegate; }

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
    Protocols::InteractionModel::Status
    SetLocalTemperature(DataModel::Nullable<int16_t> localTemperature,
                        DataModel::AttributeChangeType changeType = DataModel::AttributeChangeType::kReportable);

    DataModel::ActionReturnStatus ChangeSetpointAttribute(const AttributeId attributeId, temperature temp);

    Setpoints mSetpoints;

private:
    BitFlags<Thermostat::Feature> mFeatures;
    OptionalAttributes mOptionalAttributes;
    const DefaultValues mDefaultValues;
    FabricTable & mFabricTable;

    ControlSequenceOfOperationEnum mControlSequenceOfOperation = ControlSequenceOfOperationEnum::kCoolingOnly;

    Thermostat::Delegate * mDelegate = nullptr;

    AtomicWriteSession mAtomicWriteSession;

    BitMask<RemoteSensingBitmap> mRemoteSensing = 0;
    BitMask<OccupancyBitmap> mOccupancy         = 0;

    SystemModeEnum mSystemMode              = SystemModeEnum::kOff;
    ThermostatRunningModeEnum mRunningMode  = ThermostatRunningModeEnum::kOff;
    BitMask<RelayStateBitmap> mRunningState = 0;
    DataModel::Nullable<int16_t> mLocalTemperature;
    int8_t mLocalTemperatureCalibration = 0;

    TemperatureSetpointHoldEnum mTemperatureSetpointHold           = TemperatureSetpointHoldEnum::kSetpointHoldOff;
    DataModel::Nullable<uint16_t> mTemperatureSetpointHoldDuration = DataModel::Nullable<uint16_t>(0);
    DataModel::Nullable<uint32_t> mSetpointHoldExpiryTimestamp     = DataModel::Nullable<uint32_t>(0);

    DataModel::ActionReturnStatus WriteNonAtomicAttribute(const DataModel::WriteAttributeRequest & request,
                                                          AttributeValueDecoder & decoder);

    DataModel::ActionReturnStatus HandleSetpointChange(Setpoints & setpoints, const AttributeId attributeId, temperature value,
                                                       SetpointAttributes & changedAttributes);
    DataModel::ActionReturnStatus SetpointRaiseLower(const Commands::SetpointRaiseLower::DecodableType & commandData);

    Protocols::InteractionModel::Status LoadSetpoints(Setpoints & setpoints, AttributePersistence & persistence);
    Protocols::InteractionModel::Status SaveSetpoint(Setpoint & oldSetpoint, Setpoint & newSetpoint);
    DataModel::ActionReturnStatus SaveSetpoints(Setpoints & setpoints, SetpointAttributes changedAttributes);

    /**
     * @brief Set the Active Preset to a given preset handle, or null
     *
     * @param presetHandle The handle of the preset to set active, or null to clear the active preset
     * @return Success if the active preset was updated, an error code if not
     */
    Protocols::InteractionModel::Status SetActivePreset(DataModel::Nullable<ByteSpan> presetHandle);

    /**
     * @brief Apply a preset to the pending lists of presets during an atomic write
     *
     * @param preset The preset to append
     * @return CHIP_NO_ERROR if successful, an error code if not
     */
    CHIP_ERROR AppendPendingPreset(const Structs::PresetStruct::Type & preset);

    chip::Protocols::InteractionModel::Status PrecommitPresets();

    void GenerateSetpointEvent(AttributeId attributeId, temperature oldTemp, temperature newTemp);

    std::optional<DataModel::ActionReturnStatus>
    AddThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                            const Commands::AddThermostatSuggestion::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    RemoveThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                               const Commands::RemoveThermostatSuggestion::DecodableType & commandData);

    void ReEvaluateCurrentSuggestion();
};

=======
    Protocols::InteractionModel::Status OnAtomicWriteBegin(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWriteBegin(attributeId))
            {
                return *status;
            }
        }
        if constexpr (kHasSensors)
        {
            if (auto status = mSensors.OnAtomicWriteBegin(attributeId))
            {
                return *status;
            }
        }
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status OnAtomicWritePrecommit(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWritePrecommit(attributeId))
            {
                return *status;
            }
        }
        if constexpr (kHasSensors)
        {
            if (auto status = mSensors.OnAtomicWritePrecommit(attributeId))
            {
                return *status;
            }
        }
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status OnAtomicWriteCommit(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWriteCommit(attributeId))
            {
                return *status;
            }
        }
        if constexpr (kHasSensors)
        {
            if (auto status = mSensors.OnAtomicWriteCommit(attributeId))
            {
                return *status;
            }
        }
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status OnAtomicWriteRollback(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWriteRollback(attributeId))
            {
                return *status;
            }
        }
        if constexpr (kHasSensors)
        {
            if (auto status = mSensors.OnAtomicWriteRollback(attributeId))
            {
                return *status;
            }
        }
        return Protocols::InteractionModel::Status::Success;
    }

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto timeout = mPresets.GetMaxAtomicWriteTimeout(attributeId))
            {
                return timeout;
            }
        }
        if constexpr (kHasSensors)
        {
            if (auto timeout = mSensors.GetMaxAtomicWriteTimeout(attributeId))
            {
                return timeout;
            }
        }
        return std::nullopt;
    }

    bool HasAttribute(chip::AttributeId attributeId) override
    {
        switch (attributeId)
        {
        case Attributes::PresetTypes::Id:
        case Attributes::NumberOfPresets::Id:
        case Attributes::ActivePresetHandle::Id:
        case Attributes::Presets::Id:
            return mFeatures.Has(Feature::kPresets);
        case Attributes::ScheduleTypes::Id:
        case Attributes::NumberOfSchedules::Id:
        case Attributes::NumberOfScheduleTransitions::Id:
        case Attributes::NumberOfScheduleTransitionPerDay::Id:
        case Attributes::ActiveScheduleHandle::Id:
        case Attributes::Schedules::Id:
            return mFeatures.Has(Feature::kMatterScheduleConfiguration);
        case Attributes::MaxThermostatSuggestions::Id:
        case Attributes::ThermostatSuggestions::Id:
        case Attributes::CurrentThermostatSuggestion::Id:
        case Attributes::ThermostatSuggestionNotFollowingReason::Id:
            return mFeatures.Has(Feature::kThermostatSuggestions);
        case Attributes::Sensors::Id:
        case Attributes::AvailableSensors::Id:
        case Attributes::EnabledSensors::Id:
        case Attributes::NumberOfSensorScheduleTransitions::Id:
        case Attributes::SensorSchedule::Id:
            return mFeatures.Has(Feature::kThermostatSensors);
        default:
            return ThermostatClusterBase::HasAttribute(attributeId);
        }
    }

private:
    ThermostatSetpoints<Delegates...> mSetpoints;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kRequiresAtomicWrite, AtomicWriteSession, std::monostate> mAtomicWriteSession;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasHold, ThermostatHold, std::monostate> mHold;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasPresets, ThermostatPresets, std::monostate> mPresets;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasSuggestions, ThermostatSuggestions, std::monostate> mSuggestions;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasOccupancy, ThermostatOccupancy, std::monostate> mOccupancy;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasSensors, ThermostatSensors, std::monostate> mSensors;
};

/**
 * Deduce the template parameters for ThermostatCluster from the arguments.
 */
template <typename... DelegateArgs>
ThermostatCluster(EndpointId, BitFlags<Thermostat::Feature>, const ThermostatClusterBase::Config &, DelegateArgs &...)
    -> ThermostatCluster<std::decay_t<DelegateArgs>...>;

/**
 * An alias for a ThermostatCluster with all features enabled and all delegate types implemented.
 */
using FullFeaturedThermostatCluster =
    ThermostatCluster<Thermostat::Delegate, ThermostatHeatingSetpoints::Delegate, ThermostatCoolingSetpoints::Delegate,
                      ThermostatAutoSetpoints::Delegate, ThermostatHold::Delegate, ThermostatPresets::Delegate,
                      ThermostatSuggestions::Delegate, ThermostatOccupancy::Delegate, ThermostatSensors::Delegate>;

>>>>>>> 3888116 ([HVAC] Initial implementation of Thermostat Sensors (#73484))
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
