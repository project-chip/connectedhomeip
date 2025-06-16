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
#include <app/clusters/software-diagnostics-server/software-diagnostics-cluster.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-logic.h>
#include <app/static-cluster-config/SoftwareDiagnostics.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoftwareDiagnostics;

// for fixed endpoint, this file is ever only included IF software diagnostics is enabled and that MUST happen only on endpoint 0
// the static assert is skipped in case of dynamic endpoints.
static_assert((SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

namespace {

LazyRegisteredServerCluster<SoftwareDiagnosticsServerCluster<DeviceLayerSoftwareDiagnosticsLogic>> gServer;

// compile-time evaluated method if "is <EP>::SoftwareDiagnostics::<ATTR>" enabled
constexpr bool IsAttributeEnabled(EndpointId endpointId, AttributeId attributeId)
{
    for (auto & config : SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig)
    {
        if (config.endpointNumber != endpointId)
        {
            continue;
        }
        for (auto & attr : config.enabledAttributes)
        {
            if (attr == attributeId)
            {
                return true;
            }
        }
    }
    return false;
}

} // namespace

void MatterSoftwareDiagnosticsPluginServerInitCallback()
{
    // NOTE: Code already asserts that only kRootEndpointId is registered.
    const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
        .enableThreadMetrics     = IsAttributeEnabled(kRootEndpointId, Attributes::ThreadMetrics::Id),
        .enableCurrentHeapFree   = IsAttributeEnabled(kRootEndpointId, Attributes::CurrentHeapFree::Id),
        .enableCurrentHeapUsed   = IsAttributeEnabled(kRootEndpointId, Attributes::CurrentHeapUsed::Id),
        .enableCurrentWatermarks = IsAttributeEnabled(kRootEndpointId, Attributes::CurrentHeapHighWatermark::Id),
    };
    gServer.Create(enabledAttributes);
    (void) CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
}
void MatterSoftwareDiagnosticsPluginServerShutdownCallback()
{
    (void) CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    gServer.Destroy();
}
