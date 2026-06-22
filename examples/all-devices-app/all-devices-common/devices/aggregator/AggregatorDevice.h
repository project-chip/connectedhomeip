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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/**
 * @brief Represents an Aggregator (Bridge) device type.
 *
 * NOTE: The aggregated parts/endpoints list (Descriptor PartsList) is handled dynamically
 * by the CodeDrivenDataModelProvider and the DescriptorCluster. Individual child devices
 * establish their parent-child relationship at registration time by specifying this aggregator's
 * endpoint ID as their `parentId` (e.g. via RegisterDescriptor). The descriptor cluster's
 * PartsList attribute queries the registered endpoints from the provider at runtime and dynamically
 * includes any descendant endpoints that reference this aggregator as their ancestor.
 * Consequently, the AggregatorDevice class itself does not need to manually maintain a list of parts.
 */
class AggregatorDevice : public SingleEndpointDevice
{
public:
    AggregatorDevice(TimerDelegate & timerDelegate);
    ~AggregatorDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

protected:
    TimerDelegate & mTimerDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
};

} // namespace app
} // namespace chip
