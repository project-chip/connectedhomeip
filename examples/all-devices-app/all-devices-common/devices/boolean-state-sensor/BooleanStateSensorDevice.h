/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/boolean-state-server/boolean-state-cluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>
#include <memory>

namespace chip {
namespace app {

class BooleanStateSensorDevice : public SingleEndpointDevice
{
public:
    /// This is a general class for boolean state sensor devices. The device type passed here will
    /// determine the type of sensor it is (contact, water leak, etc.) This is meant to be a reusable
    /// class for the sensor types that share the same core functionality through the identify and
    /// boolean state clusters. The caller creating a BooleanStateSensorDevice MUST ensure that the underlying
    /// data for the Span of deviceTypes remains valid for the entire lifetime of the BooleanStateSensorDevice object instance.
    BooleanStateSensorDevice(TimerDelegate * timerDelegate, Span<const DataModel::DeviceTypeEntry> deviceType) :
        SingleEndpointDevice(deviceType), mTimerDelegate(timerDelegate)
    {}
    ~BooleanStateSensorDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void UnRegister(CodeDrivenDataModelProvider & provider) override;

    Clusters::BooleanStateCluster & BooleanState() { return mBooleanStateCluster.Cluster(); }

private:
    TimerDelegate * mTimerDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::BooleanStateCluster> mBooleanStateCluster;
};

} // namespace app
} // namespace chip
