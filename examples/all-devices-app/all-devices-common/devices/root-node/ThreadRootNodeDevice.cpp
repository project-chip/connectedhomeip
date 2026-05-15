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

#include <app/clusters/network-commissioning/NetworkCommissioningCluster.h>
#include <devices/root-node/ThreadRootNodeDevice.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

CHIP_ERROR ThreadRootNodeDevice::Register(EndpointId endpointId, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(RootNodeDevice::Register(endpointId, provider, parentId));

    mNetworkCommissioningCluster.Create(endpointId, &mThreadContext.threadDriver,
                                        NetworkCommissioningCluster::Context{
                                            .breadcrumbTracker   = mGeneralCommissioningCluster.Cluster(),
                                            .failSafeContext     = mContext.failSafeContext,
                                            .platformManager     = mContext.platformManager,
                                            .deviceControlServer = mContext.deviceControlServer,
                                        });

    ReturnErrorOnFailure(mNetworkCommissioningCluster.Cluster().Init());
    ReturnErrorOnFailure(provider.AddCluster(mNetworkCommissioningCluster.Registration()));

    return CHIP_NO_ERROR;
}

void ThreadRootNodeDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    RootNodeDevice::Unregister(provider);

    if (mNetworkCommissioningCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mNetworkCommissioningCluster.Cluster()));
        mNetworkCommissioningCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
