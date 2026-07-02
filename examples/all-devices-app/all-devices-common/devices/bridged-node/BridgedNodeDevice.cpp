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
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <string>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

BridgedNodeDevice::BridgedNodeDevice(TimerDelegate & timerDelegate, std::string uniqueId, std::string nodeLabel) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kBridgedNode, 1)), mTimerDelegate(timerDelegate),
    mUniqueId(std::move(uniqueId)), mNodeLabel(std::move(nodeLabel))
{}

CHIP_ERROR BridgedNodeDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // Create the Bridged Device Basic Information cluster.
    mBridgedDeviceBasicInformationCluster.Create(endpoint,
                                                 BridgedDeviceBasicInformationCluster::MutableData{
                                                     .reachable = true,
                                                     .nodeLabel = mNodeLabel,
                                                 },
                                                 BridgedDeviceBasicInformationCluster::FixedData{
                                                     .uniqueId = mUniqueId,
                                                 },
                                                 BridgedDeviceBasicInformationCluster::Context{
                                                     .delegate      = *this,
                                                     .timerDelegate = mTimerDelegate,
                                                 });

    ReturnErrorOnFailure(provider.AddCluster(mBridgedDeviceBasicInformationCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void BridgedNodeDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mBridgedDeviceBasicInformationCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBridgedDeviceBasicInformationCluster.Cluster()));
        mBridgedDeviceBasicInformationCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
