/*
 *
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

#include <app/clusters/bridged-device-basic-information-server/BridgedDeviceBasicInformationCluster.h>
#include <app/clusters/bridged-device-basic-information-server/BridgedDeviceBasicInformationDelegate.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <functional>
#include <lib/support/TimerDelegate.h>
#include <string>

namespace chip {
namespace app {

class BridgedNodeDevice : public SingleEndpointDevice, public Clusters::BridgedDeviceBasicInformationDelegate
{
public:
    BridgedNodeDevice(TimerDelegate & timerDelegate, std::string uniqueId, std::string nodeLabel);
    ~BridgedNodeDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

protected:
    TimerDelegate & mTimerDelegate;
    std::string mUniqueId;
    std::string mNodeLabel;
    LazyRegisteredServerCluster<Clusters::BridgedDeviceBasicInformationCluster> mBridgedDeviceBasicInformationCluster;
};

} // namespace app
} // namespace chip
