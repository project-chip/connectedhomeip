/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/clusters/administrator-commissioning-server/AdministratorCommissioningCluster.h>
#include <app/static-cluster-config/AdministratorCommissioning.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

// AdministraotrCommissioningCluster implementation is specifically implemented
// only for the root endpoint (endpoint 0)
// So either:
//   - we have a fixed config and it is endpoint 0 OR
//   - we have a fully dynamic config

static constexpr size_t kAdministratorCommissioningFixedClusterCount =
    AdministratorCommissioning::StaticApplicationConfig::kFixedClusterConfig.size();

static_assert((kAdministratorCommissioningFixedClusterCount == 0) ||
                  ((kAdministratorCommissioningFixedClusterCount == 1) &&
                   AdministratorCommissioning::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId),
              "Fixed administrator commissioning MUST be on endpoint 0");

#ifdef ADMINISTRATOR_COMMISSIONING_ENABLE_OPEN_BASIC_COMMISSIONING_WINDOW_CMD
using ClusterImpl = AdministratorCommissioningWithBasicCommissioningWindowCluster;
#else
using ClusterImpl = AdministratorCommissioningCluster;
#endif

LazyRegisteredServerCluster<ClusterImpl> gServer;

} // namespace

void emberAfAdministratorCommissioningClusterInitCallback(EndpointId endpointId)
{
    if (endpointId != kRootEndpointId)
    {
        return;
    }

    uint32_t rawFeatureMap;
    if (FeatureMap::Get(endpointId, &rawFeatureMap) != Status::Success)
    {
        ChipLogError(AppServer, "Failed to get feature map for endpoint %u", endpointId);
        rawFeatureMap = 0;
    }

    gServer.Create(endpointId, BitFlags<AdministratorCommissioning::Feature>(rawFeatureMap));
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Admin Commissioning register error: endpoint %u, %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void emberAfAdministratorCommissioningClusterShutdownCallback(EndpointId endpointId)
{
    if (endpointId != kRootEndpointId)
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Admin Commissioning unregister error: endpoint %u, %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    gServer.Destroy();
}

void MatterAdministratorCommissioningPluginServerInitCallback() {}
void MatterAdministratorCommissioningPluginServerShutdownCallback() {}
