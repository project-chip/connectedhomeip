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
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

CommissioningProxyDevice::CommissioningProxyDevice(const Context & context,
                                                   const Clusters::CommissioningProxy::CommissioningProxyCluster::Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kCommissioningByProxy, 1)),
    mContext(context), mConfig(config)
{}

void CommissioningProxyDevice::AddTransport(Clusters::CommissioningProxy::CommissioningProxyTransport & transport)
{
    VerifyOrDie(mTransportCount < kMaxTransports);
    mTransports[mTransportCount++] = &transport;
}

CHIP_ERROR CommissioningProxyDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                              EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);

    // AddTransport() must have been called at least once: the cluster spec constrains the
    // Transport attribute to min 1, so a proxy with no transport would report a
    // non-conformant empty value and fail every connect and scan request.
    VerifyOrReturnError(mTransportCount > 0, CHIP_ERROR_INCORRECT_STATE);

    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // MaxSessions and MaxCachedResults are Fixed-quality attributes and come from
    // CHIP_CONFIG_COMMISSIONING_PROXY_MAX_SESSIONS / _MAX_CACHED_RESULTS.
    mCluster.Create(endpoint, mConfig, mContext.timerDelegate, &mContext.fabricTable);

    for (size_t i = 0; i < mTransportCount; i++)
    {
        mCluster.Cluster().RegisterTransport(*mTransports[i]);
    }

    ReturnErrorOnFailure(provider.AddCluster(mCluster.Registration()));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void CommissioningProxyDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mCluster.IsConstructed())
    {
        // The cluster destructor shuts down its subsystems (scan cache, session
        // manager, aggregator) and every registered transport, cancelling their
        // timers so nothing outlives the cluster.
        LogErrorOnFailure(provider.RemoveCluster(&mCluster.Cluster()));
        mCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
