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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <device/types/battery-storage/BatteryStorage.h>
#include <device/types/electrical-sensor/impl/SimulatedElectricalSensor.h>

namespace chip::app {

class SimulatedBatteryStorageDelegates
{
protected:
    SimulatedBatteryStorageDelegates(TimerDelegate & timerDelegate, TestEventTriggerDelegate & testEventTriggerDelegate) :
        mGridSensorDelegate(timerDelegate, testEventTriggerDelegate, SimulatedElectricalSensorDelegate::Role::kGrid),
        mBatterySensorDelegate(timerDelegate, testEventTriggerDelegate, SimulatedElectricalSensorDelegate::Role::kBattery)
    {}

    SimulatedElectricalSensorDelegate mGridSensorDelegate;
    SimulatedElectricalSensorDelegate mBatterySensorDelegate;
};

/// Simulated Battery Storage with optional Identify on the parent endpoint.
class SimulatedBatteryStorage : private SimulatedBatteryStorageDelegates, public BatteryStorage
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
        TestEventTriggerDelegate & testEventTriggerDelegate;
    };

    explicit SimulatedBatteryStorage(const Context & context);
    ~SimulatedBatteryStorage() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }

protected:
    CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    TimerDelegate & mTimerDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
};

} // namespace chip::app
