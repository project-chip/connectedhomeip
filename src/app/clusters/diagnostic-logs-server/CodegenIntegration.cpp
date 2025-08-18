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

#include "CodegenIntegration.h"

#include <app/clusters/diagnostic-logs-server/DiagnosticLogsCluster.h>
#include <app/static-cluster-config/DiagnosticLogs.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <protocols/bdx/DiagnosticLogs.h>

#include "BDXDiagnosticLogsProvider.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DiagnosticLogs;
using namespace chip::Protocols::InteractionModel;

namespace {
static constexpr size_t kDiagnosticLogsFixedClusterCount = DiagnosticLogs::StaticApplicationConfig::kFixedClusterConfig.size();

static constexpr size_t kDiagnosticLogsMaxClusterCount =
    kDiagnosticLogsFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<DiagnosticLogsCluster> gServers[kDiagnosticLogsMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned emberEndpointIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServers[emberEndpointIndex].Create(endpointId);
        return gServers[emberEndpointIndex].Registration();
    }

    ServerClusterInterface & FindRegistration(unsigned emberEndpointIndex) override
    {
        return gServers[emberEndpointIndex].Cluster();
    }
    void ReleaseRegistration(unsigned emberEndpointIndex) override { gServers[emberEndpointIndex].Destroy(); }
};

} // namespace

void emberAfDiagnosticLogsClusterServerInitCallback(EndpointId endpoint)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                      = endpoint,
            .clusterId                       = DiagnosticLogs::Id,
            .fixedClusterServerEndpointCount = kDiagnosticLogsFixedClusterCount,
            .maxEndpointCount                = kDiagnosticLogsMaxClusterCount,
            .fetchFeatureMap                 = false,
            .fetchOptionalAttributes         = false,
        },
        integrationDelegate);
}

void MatterDiagnosticLogsClusterServerShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = DiagnosticLogs::Id,
            .fixedClusterServerEndpointCount = kDiagnosticLogsFixedClusterCount,
            .maxEndpointCount                = kDiagnosticLogsMaxClusterCount,
        },
        integrationDelegate);
}

void MatterDiagnosticLogsPluginServerInitCallback() {}
void MatterDiagnosticLogsPluginServerShutdownCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

DiagnosticLogsServer DiagnosticLogsServer::sInstance;

DiagnosticLogsServer & DiagnosticLogsServer::Instance()
{
    return sInstance;
}

void DiagnosticLogsServer::SetDiagnosticLogsProviderDelegate(EndpointId endpoint, DiagnosticLogsProviderDelegate * delegate)
{
    uint16_t arrayIndex = emberAfGetClusterServerEndpointIndex(endpoint, DiagnosticLogs::Id, kDiagnosticLogsFixedClusterCount);
    VerifyOrReturn(arrayIndex < kDiagnosticLogsMaxClusterCount);

    gServers[arrayIndex].Cluster().SetDelegate(endpoint, delegate);
}

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
