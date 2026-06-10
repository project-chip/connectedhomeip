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
#include <devices/aggregator/AggregatorDevice.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

AggregatorDevice::AggregatorDevice(TimerDelegate & timerDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kAggregator, 1)), mTimerDelegate(timerDelegate)
{}

CHIP_ERROR AggregatorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    CHIP_ERROR err = SingleEndpointRegistration(endpoint, provider, parentId);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    // Create the identify cluster.
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    err = provider.AddCluster(mIdentifyCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        Unregister(provider);
        return err;
    }

    err = provider.AddEndpoint(mEndpointRegistration);
    if (err != CHIP_NO_ERROR)
    {
        Unregister(provider);
        return err;
    }

    return CHIP_NO_ERROR;
}

void AggregatorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
