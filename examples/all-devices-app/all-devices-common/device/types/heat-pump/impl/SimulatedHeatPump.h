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

#pragma once

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/bindings/BindingCluster.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <binding-handler/BindingCommand.h>
#include <binding-handler/BindingHandler.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <device/types/device-energy-management/EnergyManagement.h>
#include <device/types/electrical-sensor/impl/SimulatedElectricalSensor.h>
#include <device/types/heat-pump/HeatPump.h>
#include <device/types/power-source/WiredPowerSource.h>
#include <device/types/thermostat/impl/LoggingThermostat.h>
#include <lib/support/TimerDelegate.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/PlatformManager.h>

#include <memory>

namespace chip {
namespace app {

/// Owns the mandatory Heat Pump parts. A base class of SimulatedHeatPump so the parts are constructed
/// before HeatPump stores references to them.
class SimulatedHeatPumpParts
{
protected:
    SimulatedHeatPumpParts(TimerDelegate & timerDelegate, TestEventTriggerDelegate & testEventTriggerDelegate);

    WiredPowerSource mPowerSource;
    EnergyManagement mEnergyManagement;
    SimulatedElectricalSensor mElectricalSensor;
};

/// Heat Pump with a simulated grid electrical sensor and Identify on the heat pump endpoint.
/// ThermostatControl selects how the device meets the Thermostat requirement of the spec.
class SimulatedHeatPump : private SimulatedHeatPumpParts, public HeatPump
{
public:
    enum class ThermostatControl : uint8_t
    {
        /// A Thermostat device on a child endpoint.
        kThermostatEndpoint,
        /// A Thermostat client with a Binding cluster on the heat pump endpoint, to control
        /// third-party thermostats.
        kThermostatClient,
    };

    struct Context
    {
        TimerDelegate & timerDelegate;
        TestEventTriggerDelegate & testEventTriggerDelegate;
        Clusters::IdentifyDelegate & identifyDelegate;
        FabricTable & fabricTable;
        DeviceLayer::DeviceInfoProvider & deviceInfoProvider;
        Credentials::GroupDataProvider & groupDataProvider;
        DeviceLayer::PlatformManager & platformManager;
        Clusters::Binding::Table & bindingTable;
        Clusters::Binding::Manager & bindingManager;
        BindingHandler & bindingHandler;
    };

    SimulatedHeatPump(const Context & context, ThermostatControl thermostatControl);
    ~SimulatedHeatPump() override = default;

    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

    /// Sends a Thermostat command to the bound thermostats. Returns CHIP_ERROR_INCORRECT_STATE if the
    /// device is not registered, and CHIP_ERROR_INVALID_ARGUMENT if the command is null or the
    /// device has no client for the command cluster. Otherwise follows BindingHandler::Dispatch.
    CHIP_ERROR Dispatch(Platform::UniquePtr<BindingCommand> command);

    Clusters::IdentifyCluster & IdentifyCluster()
    {
        VerifyOrDie(mIdentifyCluster.IsConstructed());
        return mIdentifyCluster.Cluster();
    }

    Clusters::BindingCluster & BindingCluster()
    {
        VerifyOrDie(mBindingCluster.IsConstructed());
        return mBindingCluster.Cluster();
    }

protected:
    CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) override;

    CHIP_ERROR RegisterOptionalEndpoints(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                         EndpointId parent) override;
    void UnregisterOptionalEndpoints(CodeDrivenDataModelProvider & provider) override;

private:
    const Context mContext;
    const ThermostatControl mThermostatControl;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::BindingCluster> mBindingCluster;
    /// Only constructed for ThermostatControl::kThermostatEndpoint.
    std::unique_ptr<LoggingThermostat> mThermostat;
};

} // namespace app
} // namespace chip
