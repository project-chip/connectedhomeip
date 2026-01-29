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

enum class FeatureAttributes : uint32_t
{
    kDischargingEnabledUntil  = 0x1,   // V2x feature
    kMaximumDischargeCurrent  = 0x2,   // V2x feature
    kNextChargeStartTime      = 0x4,   // ChargingPreferences feature
    kNextChargeTargetTime     = 0x8,   // ChargingPreferences feature
    kNextChargeRequiredEnergy = 0x10,  // ChargingPreferences feature
    kNextChargeTargetSoC      = 0x20,  // ChargingPreferences feature
    kApproximateEvEfficiency  = 0x40,  // ChargingPreferences feature & kSupportsApproximateEvEfficiency
    kStateOfCharge            = 0x80,  // SoCReporting feature
    kBatteryCapacity          = 0x100, // SoCReporting feature
    kVehicleID                = 0x200, // PlugAndCharge feature
    kSessionEnergyDischarged  = 0x400  // V2x feature
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
    const BitMask<EnergyEvse::OptionalAttributes> & OptionalAttrs() const { return mOptionalAttrs; }
    const BitMask<EnergyEvse::OptionalCommands> & OptionalCmds() const { return mOptionalCmds; }

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
    DataModel::ActionReturnStatus HandleGetTargets(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                   CommandHandler * handler);
    DataModel::ActionReturnStatus HandleClearTargets(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                     CommandHandler * handler);

    Protocols::InteractionModel::Status
    ValidateTargets(const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules);

    EnergyEvse::Delegate & mDelegate;
    const BitMask<EnergyEvse::Feature> mFeatureFlags;
    const BitMask<EnergyEvse::OptionalAttributes> mOptionalAttrs;
    const BitMask<EnergyEvse::OptionalCommands> mOptionalCmds;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
