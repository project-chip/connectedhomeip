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

#include <devices/Types.h>
#include <devices/bridged-node/BridgedNodeDevice.h>
#include <lib/support/logging/CHIPLogging.h>

#include <string>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

BridgedNodeDevice::BridgedNodeDevice(TimerDelegate & timerDelegate, const std::string & uniqueId) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kBridgedNode, 1)), mTimerDelegate(timerDelegate),
    mUniqueId(uniqueId)
{}

CHIP_ERROR BridgedNodeDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    // Setup unique ID: use injected unique ID if specified, otherwise fall back to endpoint-based default
    std::string uniqueId = mUniqueId.empty() ? ("bridged-node-EP" + std::to_string(endpoint)) : mUniqueId;

    // Create the Bridged Device Basic Information cluster.
    mBridgedDeviceBasicInformationCluster.Create(
        endpoint,
        BridgedDeviceBasicInformationCluster::MutableData{
            .reachable = true,
            .nodeLabel = "Bridged Node",
        },
        BridgedDeviceBasicInformationCluster::FixedData{
            .uniqueId = uniqueId,
        },
        BridgedDeviceBasicInformationCluster::Context{
            .delegate      = *this,
            .timerDelegate = mTimerDelegate,
        });

    ReturnErrorOnFailure(provider.AddCluster(mBridgedDeviceBasicInformationCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void BridgedNodeDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
    if (mBridgedDeviceBasicInformationCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBridgedDeviceBasicInformationCluster.Cluster()));
        mBridgedDeviceBasicInformationCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
