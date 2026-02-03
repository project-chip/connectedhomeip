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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/MessageDef/StatusIB.h>
#include <app/clusters/energy-evse-server/Constants.h>
#include <app/clusters/energy-evse-server/EnergyEvseDelegate.h>
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

class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature, OptionalAttributes aOptionalAttrs,
             OptionalCommands aOptionalCmds) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id),
        CommandHandlerInterface(MakeOptional(aEndpointId), Id), mDelegate(aDelegate), mFeature(aFeature),
        mOptionalAttrs(aOptionalAttrs), mOptionalCmds(aOptionalCmds)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
        mDelegate.SetInstance(this);
    }
    ~Instance()
    {
        mDelegate.SetInstance(nullptr);
        Shutdown();
    }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;
    bool SupportsOptAttr(OptionalAttributes aOptionalAttrs) const;
    bool SupportsOptCmd(OptionalCommands aOptionalCmds) const;

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeature;
    BitMask<OptionalAttributes> mOptionalAttrs;
    BitMask<OptionalCommands> mOptionalCmds;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;
    CHIP_ERROR RetrieveAcceptedCommands(const ConcreteClusterPath & cluster,
                                        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    void HandleDisable(HandlerContext & ctx, const Commands::Disable::DecodableType & commandData);
    void HandleEnableCharging(HandlerContext & ctx, const Commands::EnableCharging::DecodableType & commandData);
    void HandleEnableDischarging(HandlerContext & ctx, const Commands::EnableDischarging::DecodableType & commandData);
    void HandleStartDiagnostics(HandlerContext & ctx, const Commands::StartDiagnostics::DecodableType & commandData);
    void HandleSetTargets(HandlerContext & ctx, const Commands::SetTargets::DecodableType & commandData);
    void HandleGetTargets(HandlerContext & ctx, const Commands::GetTargets::DecodableType & commandData);
    void HandleClearTargets(HandlerContext & ctx, const Commands::ClearTargets::DecodableType & commandData);

    // Check that the targets are valid
    Protocols::InteractionModel::Status
    ValidateTargets(const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules);
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
