/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <app/ConcreteAttributePath.h>
#include <app/MessageDef/StatusIB.h>
#include <app/clusters/energy-evse-server/Constants.h>
#include <app/clusters/energy-evse-server/EnergyEvseDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/EnergyEvse/Attributes.h>
#include <clusters/EnergyEvse/Commands.h>
#include <clusters/EnergyEvse/Enums.h>
#include <clusters/EnergyEvse/Events.h>
#include <clusters/EnergyEvse/Structs.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

enum class OptionalAttributes : uint32_t
{
    kSupportsUserMaximumChargingCurrent = 0x1,
    kSupportsRandomizationWindow        = 0x2,
    kSupportsApproximateEvEfficiency    = 0x4
};

enum class OptionalCommands : uint32_t
{
    kSupportsStartDiagnostics = 0x1
};

class EnergyEvseCluster : public DefaultServerCluster
{

public:
    struct Config
    {
        EndpointId endpointId;
        EnergyEvse::Delegate & delegate;
        BitMask<EnergyEvse::Feature> feature;
        BitMask<EnergyEvse::OptionalCommands> optionalCmds;
        BitMask<EnergyEvse::OptionalAttributes> optionalAttrs;

        Config(EndpointId aEndpointId, EnergyEvse::Delegate & aDelegate, BitMask<EnergyEvse::Feature> aFeature,
               BitMask<EnergyEvse::OptionalAttributes> aOptionalAttrs, BitMask<EnergyEvse::OptionalCommands> aOptionalCmds) :
            endpointId(aEndpointId),
            delegate(aDelegate), feature(aFeature), optionalCmds(aOptionalCmds), optionalAttrs(aOptionalAttrs)
        {}
    };
    // We don't want to allow the default constructor as this cluster requires a delegate to be set
    EnergyEvseCluster() = delete;

    EnergyEvseCluster(const Config & config) :
        DefaultServerCluster({ config.endpointId, EnergyEvse::Id }), mDelegate(config.delegate), mFeatureFlags(config.feature),
        mOptionalAttrs(config.optionalAttrs), mOptionalCmds(config.optionalCmds)
    {
        mDelegate.SetEndpointId(config.endpointId);
    }

    const BitFlags<EnergyEvse::Feature> & Features() const { return mFeatureFlags; }
    const BitFlags<EnergyEvse::OptionalAttributes> & OptionalAttrs() const { return mOptionalAttrs; }
    const BitFlags<EnergyEvse::OptionalCommands> & OptionalCmds() const { return mOptionalCmds; }

    // Attribute getters and setters - cluster owns the data
    StateEnum GetState() const { return mState; }
    CHIP_ERROR SetState(StateEnum newValue);

    SupplyStateEnum GetSupplyState() const { return mSupplyState; }
    CHIP_ERROR SetSupplyState(SupplyStateEnum newValue);

    FaultStateEnum GetFaultState() const { return mFaultState; }
    CHIP_ERROR SetFaultState(FaultStateEnum newValue);

    DataModel::Nullable<uint32_t> GetChargingEnabledUntil() const { return mChargingEnabledUntil; }
    CHIP_ERROR SetChargingEnabledUntil(DataModel::Nullable<uint32_t> newValue);

    DataModel::Nullable<uint32_t> GetDischargingEnabledUntil() const { return mDischargingEnabledUntil; }
    CHIP_ERROR SetDischargingEnabledUntil(DataModel::Nullable<uint32_t> newValue);

    int64_t GetCircuitCapacity() const { return mCircuitCapacity; }
    CHIP_ERROR SetCircuitCapacity(int64_t newValue);

    int64_t GetMinimumChargeCurrent() const { return mMinimumChargeCurrent; }
    CHIP_ERROR SetMinimumChargeCurrent(int64_t newValue);

    int64_t GetMaximumChargeCurrent() const { return mMaximumChargeCurrent; }
    CHIP_ERROR SetMaximumChargeCurrent(int64_t newValue);

    int64_t GetMaximumDischargeCurrent() const { return mMaximumDischargeCurrent; }
    CHIP_ERROR SetMaximumDischargeCurrent(int64_t newValue);

    int64_t GetUserMaximumChargeCurrent() const { return mUserMaximumChargeCurrent; }
    CHIP_ERROR SetUserMaximumChargeCurrent(int64_t newValue);

    uint32_t GetRandomizationDelayWindow() const { return mRandomizationDelayWindow; }
    CHIP_ERROR SetRandomizationDelayWindow(uint32_t newValue);

    DataModel::Nullable<uint32_t> GetNextChargeStartTime() const { return mNextChargeStartTime; }
    CHIP_ERROR SetNextChargeStartTime(DataModel::Nullable<uint32_t> newValue);

    DataModel::Nullable<uint32_t> GetNextChargeTargetTime() const { return mNextChargeTargetTime; }
    CHIP_ERROR SetNextChargeTargetTime(DataModel::Nullable<uint32_t> newValue);

    DataModel::Nullable<int64_t> GetNextChargeRequiredEnergy() const { return mNextChargeRequiredEnergy; }
    CHIP_ERROR SetNextChargeRequiredEnergy(DataModel::Nullable<int64_t> newValue);

    DataModel::Nullable<Percent> GetNextChargeTargetSoC() const { return mNextChargeTargetSoC; }
    CHIP_ERROR SetNextChargeTargetSoC(DataModel::Nullable<Percent> newValue);

    DataModel::Nullable<uint16_t> GetApproximateEVEfficiency() const { return mApproximateEVEfficiency; }
    CHIP_ERROR SetApproximateEVEfficiency(DataModel::Nullable<uint16_t> newValue);

    DataModel::Nullable<Percent> GetStateOfCharge() const { return mStateOfCharge; }
    CHIP_ERROR SetStateOfCharge(DataModel::Nullable<Percent> newValue);

    DataModel::Nullable<int64_t> GetBatteryCapacity() const { return mBatteryCapacity; }
    CHIP_ERROR SetBatteryCapacity(DataModel::Nullable<int64_t> newValue);

    DataModel::Nullable<CharSpan> GetVehicleID() const;
    CHIP_ERROR SetVehicleID(DataModel::Nullable<CharSpan> newValue);

    DataModel::Nullable<uint32_t> GetSessionID() const { return mSessionID; }
    CHIP_ERROR SetSessionID(DataModel::Nullable<uint32_t> newValue);

    DataModel::Nullable<uint32_t> GetSessionDuration() const { return mSessionDuration; }
    CHIP_ERROR SetSessionDuration(DataModel::Nullable<uint32_t> newValue);

    DataModel::Nullable<int64_t> GetSessionEnergyCharged() const { return mSessionEnergyCharged; }
    CHIP_ERROR SetSessionEnergyCharged(DataModel::Nullable<int64_t> newValue);

    DataModel::Nullable<int64_t> GetSessionEnergyDischarged() const { return mSessionEnergyDischarged; }
    CHIP_ERROR SetSessionEnergyDischarged(DataModel::Nullable<int64_t> newValue);

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

private:
    DataModel::ActionReturnStatus HandleDisable(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                CommandHandler * handler);
    DataModel::ActionReturnStatus HandleEnableCharging(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                       CommandHandler * handler);
    DataModel::ActionReturnStatus HandleEnableDischarging(const DataModel::InvokeRequest & request,
                                                          TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleStartDiagnostics(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                         CommandHandler * handler);
    DataModel::ActionReturnStatus HandleSetTargets(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                   CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleGetTargets(const DataModel::InvokeRequest & request,
                                                                  TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleClearTargets(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                     CommandHandler * handler);

    Protocols::InteractionModel::Status
    ValidateTargets(const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules);

    EnergyEvse::Delegate & mDelegate;
    const BitFlags<EnergyEvse::Feature> mFeatureFlags;
    const BitFlags<EnergyEvse::OptionalAttributes> mOptionalAttrs;
    const BitFlags<EnergyEvse::OptionalCommands> mOptionalCmds;

    // Attribute storage
    StateEnum mState                                       = StateEnum::kNotPluggedIn;
    SupplyStateEnum mSupplyState                           = SupplyStateEnum::kDisabled;
    FaultStateEnum mFaultState                             = FaultStateEnum::kNoError;
    DataModel::Nullable<uint32_t> mChargingEnabledUntil    = DataModel::NullNullable; // TODO Default to 0 to indicate disabled
    DataModel::Nullable<uint32_t> mDischargingEnabledUntil = DataModel::NullNullable; // TODO Default to 0 to indicate disabled
    int64_t mCircuitCapacity                               = 0;
    int64_t mMinimumChargeCurrent                          = kMinimumChargeCurrent;
    int64_t mMaximumChargeCurrent                          = 0;
    int64_t mMaximumDischargeCurrent                       = 0;
    int64_t mUserMaximumChargeCurrent                      = 0;
    uint32_t mRandomizationDelayWindow                     = 600; // Default 600s per spec
    // PREF attributes
    DataModel::Nullable<uint32_t> mNextChargeStartTime     = DataModel::NullNullable;
    DataModel::Nullable<uint32_t> mNextChargeTargetTime    = DataModel::NullNullable;
    DataModel::Nullable<int64_t> mNextChargeRequiredEnergy = DataModel::NullNullable;
    DataModel::Nullable<Percent> mNextChargeTargetSoC      = DataModel::NullNullable;
    DataModel::Nullable<uint16_t> mApproximateEVEfficiency = DataModel::NullNullable;
    // SOC attributes
    DataModel::Nullable<Percent> mStateOfCharge   = DataModel::NullNullable;
    DataModel::Nullable<int64_t> mBatteryCapacity = DataModel::NullNullable;
    // PNC attributes
    char mVehicleIDBuffer[kMaxVehicleIDBufSize] = { 0 };
    DataModel::Nullable<CharSpan> mVehicleID    = DataModel::NullNullable;
    // Session attributes
    DataModel::Nullable<uint32_t> mSessionID              = DataModel::NullNullable;
    DataModel::Nullable<uint32_t> mSessionDuration        = DataModel::NullNullable;
    DataModel::Nullable<int64_t> mSessionEnergyCharged    = DataModel::NullNullable;
    DataModel::Nullable<int64_t> mSessionEnergyDischarged = DataModel::NullNullable;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
