/*
 *
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

#include <app/clusters/general-commissioning-server/general-commissioning-cluster.h>
#include <app/static-cluster-config/GeneralCommissioning.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;
using chip::Protocols::InteractionModel::Status;
using namespace chip::DeviceLayer;

namespace {

// GeneralCommissioning implementation is specifically implemented
// only for the root endpoint (endpoint 0)
// So either:
//   - we have a fixed config and it is endpoint 0 OR
//   - we have a fully dynamic config

static constexpr size_t kGeneralCommissioningFixedClusterCount =
    GeneralCommissioning::StaticApplicationConfig::kFixedClusterConfig.size();

static_assert((kGeneralCommissioningFixedClusterCount == 0) ||
                  ((kGeneralCommissioningFixedClusterCount == 1) &&
                   GeneralCommissioning::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId),
              "Fixed general commissioning MUST be on endpoint 0");

LazyRegisteredServerCluster<GeneralCommissioningCluster> gServer;
static GeneralCommissioningFabricTableDelegate fabricDelegate;

} // namespace

void emberAfGeneralCommissioningClusterInitCallback(EndpointId endpointId)
{
    if (endpointId != kRootEndpointId)
    {
        return;
    }
    printf("Inside Init callback");
    Breadcrumb::Set(0, 0);
    uint32_t rawFeatureMap;
    if (FeatureMap::Get(endpointId, &rawFeatureMap) != Status::Success)
    {
        ChipLogError(AppServer, "Failed to get feature map for endpoint %u", endpointId);
        rawFeatureMap = 0;
    }

    gServer.Create(endpointId, BitFlags<GeneralCommissioning::Feature>(rawFeatureMap));
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "General Commissioning register error: endpoint %u, %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    DeviceLayer::PlatformMgrImpl().AddEventHandler(chip::app::Clusters::GeneralCommissioningLogic::OnPlatformEventHandler);
    chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&fabricDelegate);
}

void emberAfGeneralCommissioningClusterShutdownCallback(EndpointId endpointId)
{
    if (endpointId != kRootEndpointId)
    {
        return;
    }
    Server::GetInstance().GetFabricTable().RemoveFabricDelegate(&fabricDelegate);
    DeviceLayer::PlatformMgrImpl().RemoveEventHandler(chip::app::Clusters::GeneralCommissioningLogic::OnPlatformEventHandler);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "General Commissioning unregister error: endpoint %u, %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServer.Destroy();
}

void MatterGeneralCommissioningPluginServerInitCallback() {}
void MatterGeneralCommissioningPluginServerShutdownCallback() {}
