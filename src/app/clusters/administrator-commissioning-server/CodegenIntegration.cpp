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

static constexpr size_t kAdministratorCommissioningFixedClusterCount =
    AdministratorCommissioning::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kAdministratorCommissioningMaxClusterCount =
    kAdministratorCommissioningFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

#ifdef ADMINISTRATOR_COMMISSIONING_ENABLE_OPEN_BASIC_COMMISSIONING_WINDOW_CMD
using ClusterImpl = AdministratorCommissioningWithBasicCommissioningWindowCluster;
#else
using ClusterImpl = AdministratorCommissioningCluster;
#endif

LazyRegisteredServerCluster<ClusterImpl> gServers[kAdministratorCommissioningMaxClusterCount];

} // namespace

void emberAfAdministratorCommissioningClusterInitCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = emberAfGetClusterServerEndpointIndex(endpointId, AdministratorCommissioning::Id,
                                                               kAdministratorCommissioningFixedClusterCount);
    if (arrayIndex >= kAdministratorCommissioningMaxClusterCount)
    {
        return;
    }

    uint32_t rawFeatureMap;
    if (FeatureMap::Get(endpointId, &rawFeatureMap) != Status::Success)
    {
        ChipLogError(AppServer, "Failed to get feature map for endpoint %u", endpointId);
        rawFeatureMap = 0;
    }

    gServers[arrayIndex].Create(endpointId, BitFlags<AdministratorCommissioning::Feature>(rawFeatureMap));
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServers[arrayIndex].Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Admin Commissioning register error: endpoint %u, %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void emberAfAdministratorCommissioningClusterShutdownCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = emberAfGetClusterServerEndpointIndex(endpointId, AdministratorCommissioning::Id,
                                                               kAdministratorCommissioningFixedClusterCount);
    if (arrayIndex >= kAdministratorCommissioningMaxClusterCount)
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServers[arrayIndex].Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Admin Commissioning unregister error: endpoint %u, %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    gServers[arrayIndex].Destroy();
}

void MatterAdministratorCommissioningPluginServerInitCallback() {}
void MatterAdministratorCommissioningPluginServerShutdownCallback() {}
