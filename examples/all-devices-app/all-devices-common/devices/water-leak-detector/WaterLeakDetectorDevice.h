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

#include <app/TimerDelegates.h>
#include <app/clusters/boolean-state-server/boolean-state-cluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <devices/base-device/Device.h>

namespace chip {
namespace app {

constexpr DeviceTypeId kWaterLeakDetectorDeviceTypeRevision = 1;

class WaterLeakDetectorDevice : public Device
{
public:
    WaterLeakDetectorDevice() :
        Device(
            Device::DeviceType{ .deviceType = static_cast<DeviceTypeId>(0x0043), .revision = kWaterLeakDetectorDeviceTypeRevision })
    {}
    ~WaterLeakDetectorDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void UnRegister(CodeDrivenDataModelProvider & provider) override;

    Clusters::BooleanStateCluster & Cluster() { return mBooleanStateCluster.Cluster(); }

private:
    DefaultTimerDelegate mTimerDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::BooleanStateCluster> mBooleanStateCluster;
};

} // namespace app
} // namespace chip
