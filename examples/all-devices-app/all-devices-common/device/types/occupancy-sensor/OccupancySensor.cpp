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
#include <device/types/occupancy-sensor/OccupancySensor.h>
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

OccupancySensor::OccupancySensor(OccupancySensingConfig config, TimerDelegate & timerDelegate) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kOccupancySensor, 1)), mConfig(config),
    mTimerDelegate(timerDelegate)
{}

CHIP_ERROR OccupancySensor::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                     EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // Create the identify cluster.
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // Update the config with the actual endpoint ID
    mConfig.mEndpointId = endpoint;

    // Create the occupancy sensing cluster
    mOccupancySensingCluster.Create(mConfig);
    ReturnErrorOnFailure(provider.AddCluster(mOccupancySensingCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void OccupancySensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mOccupancySensingCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOccupancySensingCluster.Cluster()));
        mOccupancySensingCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
