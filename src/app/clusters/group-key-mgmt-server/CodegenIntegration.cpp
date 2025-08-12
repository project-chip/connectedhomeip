/**
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

#include <app/clusters/group-key-mgmt-server/group-key-mgmt-cluster.h>
#include <app/static-cluster-config/GroupKeyManagement.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GroupKeyManagement;

static_assert((GroupKeyManagement::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               GroupKeyManagement::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              GroupKeyManagement::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

LazyRegisteredServerCluster<GroupKeyManagementCluster> gServer;

void emberAfGroupKeyManagementClusterServerInitCallback(EndpointId endpointId)
{
    VerifyOrDie(endpointId == kRootEndpointId);
    gServer.Create();

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register GroupKeyManagement on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

void MatterGroupKeyManagementClusterServerShutdownCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister GroupKeyManagement on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServer.Destroy();
}

void MatterGroupKeyManagementPluginServerInitCallback() {}

void MatterGroupKeyManagementPluginServerShutdownCallback() {}
