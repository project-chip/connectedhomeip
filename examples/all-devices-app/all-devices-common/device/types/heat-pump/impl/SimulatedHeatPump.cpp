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

#include <device/types/heat-pump/impl/SimulatedHeatPump.h>

#include <clusters/Thermostat/ClusterId.h>
#include <device/types/device-energy-management/EnergyManagement.h>
#include <device/types/electrical-sensor/impl/SimulatedElectricalSensor.h>
#include <device/types/power-source/WiredPowerSource.h>

namespace chip {
namespace app {

namespace {

const ClusterId kThermostatClientClusters[] = { Clusters::Thermostat::Id };

HeatPump::Parts MakeParts(const SimulatedHeatPump::Context & context)
{
    HeatPump::Parts parts;
    parts.powerSource =
        std::make_unique<WiredPowerSource>("Heat pump grid connection"_span, Clusters::PowerSource::WiredCurrentTypeEnum::kAc);
    parts.energyManagement =
        std::make_unique<EnergyManagement>(context.timerDelegate,
                                           EnergyManagement::Config{
                                               .featureFlags = BitMask<Clusters::DeviceEnergyManagement::Feature>(
                                                   Clusters::DeviceEnergyManagement::Feature::kPowerAdjustment),
                                               .esaType        = Clusters::DeviceEnergyManagement::ESATypeEnum::kSpaceHeating,
                                               .esaCanGenerate = false,
                                           });
    parts.electricalSensor = std::make_unique<SimulatedElectricalSensor>(
        context.timerDelegate, context.testEventTriggerDelegate, Span<const Clusters::Globals::Structs::SemanticTagStruct::Type>(),
        SimulatedElectricalSensorDelegate::Role::kGrid);
    return parts;
}

} // namespace

SimulatedHeatPump::SimulatedHeatPump(const Context & context, ThermostatControl thermostatControl) :
    HeatPump(Config{}, MakeParts(context)), mContext(context), mThermostatControl(thermostatControl)
{
    if (mThermostatControl == ThermostatControl::kThermostatEndpoint)
    {
        mThermostat = std::make_unique<LoggingThermostat>(LoggingThermostat::Context{
            .fabricTable       = context.fabricTable,
            .groupDataProvider = context.groupDataProvider,
            .timerDelegate     = context.timerDelegate,
        });
    }
}

CHIP_ERROR SimulatedHeatPump::ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const
{
    VerifyOrReturnError(mThermostatControl == ThermostatControl::kThermostatClient, HeatPump::ClientClusters(out));
    return out.ReferenceExisting(Span<const ClusterId>(kThermostatClientClusters));
}

CHIP_ERROR SimulatedHeatPump::Dispatch(Platform::UniquePtr<BindingCommand> command)
{
    VerifyOrReturnError(GetEndpointId() != kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mThermostatControl == ThermostatControl::kThermostatClient, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(command->GetClusterId() == Clusters::Thermostat::Id, CHIP_ERROR_INVALID_ARGUMENT);
    return mContext.bindingHandler.Dispatch(GetEndpointId(), std::move(command));
}

CHIP_ERROR SimulatedHeatPump::RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    Clusters::IdentifyCluster::Config identifyConfig(endpoint, mContext.timerDelegate);
    identifyConfig.WithDelegate(&mContext.identifyDelegate);
    mIdentifyCluster.Create(identifyConfig);
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    if (mThermostatControl == ThermostatControl::kThermostatClient)
    {
        mBindingCluster.Create(
            Clusters::BindingCluster::Context{
                .bindingTable    = mContext.bindingTable,
                .bindingManager  = mContext.bindingManager,
                .platformManager = mContext.platformManager,
            },
            endpoint);
        ReturnErrorOnFailure(provider.AddCluster(mBindingCluster.Registration()));
    }
    return CHIP_NO_ERROR;
}

void SimulatedHeatPump::UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mBindingCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBindingCluster.Cluster()));
        mBindingCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

CHIP_ERROR SimulatedHeatPump::RegisterOptionalEndpoints(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                                        EndpointId parent)
{
    VerifyOrReturnError(mThermostat != nullptr, CHIP_NO_ERROR);
    return mThermostat->Register(allocator.Allocate(), provider, EndpointComposition::WithParent(parent));
}

void SimulatedHeatPump::UnregisterOptionalEndpoints(CodeDrivenDataModelProvider & provider)
{
    if (mThermostat != nullptr && mThermostat->GetEndpointId() != kInvalidEndpointId)
    {
        mThermostat->Unregister(provider);
    }
}

} // namespace app
} // namespace chip
