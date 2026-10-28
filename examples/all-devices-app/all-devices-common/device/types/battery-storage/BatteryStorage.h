/*
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

#include <app/TestEventTriggerDelegate.h>
#include <device/api/Interface.h>
#include <device/types/device-energy-management/EnergyManagement.h>
#include <device/types/electrical-sensor/impl/SimulatedElectricalSensor.h>
#include <device/types/power-source/BatteryPowerSource.h>
#include <device/types/power-source/WiredPowerSource.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

/// Battery Storage device type (0x0018) rev 2 parent with mandatory composed endpoints.
class BatteryStorage : public DeviceInterface
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
        TestEventTriggerDelegate & testEventTriggerDelegate;
    };

    ~BatteryStorage() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    EndpointId GetEndpointId() const { return mEndpointId; }

    EnergyManagement & GetEnergyManagement() { return mEnergyManagement; }
    SimulatedElectricalSensor & GetGridElectricalSensor() { return mGridElectricalSensor; }
    SimulatedElectricalSensor & GetBatteryElectricalSensor() { return mBatteryElectricalSensor; }
    WiredPowerSource & GetGridPowerSource() { return mGridPowerSource; }
    BatteryPowerSource & GetBatteryPowerSource() { return mBatteryPowerSource; }

protected:
    explicit BatteryStorage(const Context & context);

    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

private:
    void UnregisterComposedEndpoints(CodeDrivenDataModelProvider & provider);

    EndpointId mEndpointId = kInvalidEndpointId;

    EnergyManagement mEnergyManagement;
    SimulatedElectricalSensor mGridElectricalSensor;
    SimulatedElectricalSensor mBatteryElectricalSensor;
    WiredPowerSource mGridPowerSource;
    BatteryPowerSource mBatteryPowerSource;
};

} // namespace chip::app
