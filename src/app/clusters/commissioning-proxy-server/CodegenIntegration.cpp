/*
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

#include <app/clusters/commissioning-proxy-server/CodegenIntegration.h>

#include <app/util/generic-callbacks.h>
#include <clusters/CommissioningProxy/Metadata.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

Instance::Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature) :
    mCluster(CommissioningProxyCluster::Config(aEndpointId, aFeature, aDelegate))
{}

CHIP_ERROR Instance::Init()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, ChipLogValueMEI(CommissioningProxy::Id), err.Format());
    }
    return err;
}

void Instance::Shutdown()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, ChipLogValueMEI(CommissioningProxy::Id), err.Format());
    }
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mCluster.Cluster().Features().Has(aFeature);
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip

// The current implementation already manually instantiates and initializes the cluster, so no need for the codegen integration.
void MatterCommissioningProxyClusterInitCallback(chip::EndpointId) {}
void MatterCommissioningProxyClusterShutdownCallback(chip::EndpointId, MatterClusterShutdownType) {}

// Legacy callback stubs
void MatterCommissioningProxyPluginServerInitCallback() {}
void MatterCommissioningProxyPluginServerShutdownCallback() {}
