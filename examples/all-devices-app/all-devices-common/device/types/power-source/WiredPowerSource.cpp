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

#include "WiredPowerSource.h"

#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip::app {

WiredPowerSource::WiredPowerSource(CharSpan description, PowerSource::WiredCurrentTypeEnum wiredCurrentType, uint8_t order) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kPowerSource, 1)), mDescription(description),
    mWiredCurrentType(wiredCurrentType), mOrder(order)
{}

CHIP_ERROR WiredPowerSource::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                      EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    SimpleWiredPowerSourceCluster::Config config(mDescription, mWiredCurrentType);
    config.status    = PowerSource::PowerSourceStatusEnum::kActive;
    config.order     = mOrder;
    mEndpointList[0] = endpoint;

    mWiredPowerSourceCluster.Create(endpoint, config);
    ReturnErrorOnFailure(mWiredPowerSourceCluster.Cluster().SetEndpointList(Span<const EndpointId>(mEndpointList)));
    ReturnErrorOnFailure(provider.AddCluster(mWiredPowerSourceCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void WiredPowerSource::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mWiredPowerSourceCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mWiredPowerSourceCluster.Cluster()));
        mWiredPowerSourceCluster.Destroy();
    }
}

WiredPowerSource::SimpleWiredPowerSourceCluster & WiredPowerSource::WiredPowerSourceCluster()
{
    VerifyOrDie(mWiredPowerSourceCluster.IsConstructed());
    return mWiredPowerSourceCluster.Cluster();
}

} // namespace chip::app
