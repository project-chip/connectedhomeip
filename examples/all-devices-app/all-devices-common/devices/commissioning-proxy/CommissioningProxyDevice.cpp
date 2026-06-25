/*
 *
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

#include "CommissioningProxyDevice.h"

#include <devices/Types.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters::CommissioningProxy;

namespace chip {
namespace app {

CommissioningProxyDevice::CommissioningProxyDevice() :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kCommissioningByProxy, 1))
{}

CHIP_ERROR CommissioningProxyDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                              EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    BitMask<Feature> features(Feature::kBackgroundScan);

    mCluster.Create(Clusters::CommissioningProxy::CommissioningProxyCluster::Config(endpoint, features, mDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mCluster.Registration()));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    return CHIP_NO_ERROR;
}

void CommissioningProxyDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mCluster.Cluster()));
        mCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
