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
#include <device/types/root-node/features/ThreadFeature.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

CHIP_ERROR ThreadFeature::RegisterFeatureClusters(EndpointId endpointId, CodeDrivenDataModelProvider & provider,
                                                  RootNode::Context & rootContext,
                                                  Clusters::GeneralCommissioningCluster & generalCommissioning)
{
    mNetworkCommissioningCluster.Create(endpointId, &mContext.threadDriver,
                                        NetworkCommissioningCluster::Context{
                                            .breadcrumbTracker   = generalCommissioning,
                                            .failSafeContext     = rootContext.failSafeContext,
                                            .platformManager     = rootContext.platformManager,
                                            .deviceControlServer = rootContext.deviceControlServer,
                                        });

    ReturnErrorOnFailure(mNetworkCommissioningCluster.Cluster().Init());
    ReturnErrorOnFailure(provider.AddCluster(mNetworkCommissioningCluster.Registration()));

    return CHIP_NO_ERROR;
}

void ThreadFeature::UnregisterFeatureClusters(CodeDrivenDataModelProvider & provider)
{
    if (mNetworkCommissioningCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mNetworkCommissioningCluster.Cluster()));
        mNetworkCommissioningCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
