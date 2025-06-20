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
#include <app/util/attribute-storage.h>
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

void emberAfSoftwareDiagnosticsClusterInitCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);
    const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
        .enableThreadMetrics     = IsAttributeEnabled(kRootEndpointId, Attributes::ThreadMetrics::Id),
        .enableCurrentHeapFree   = IsAttributeEnabled(kRootEndpointId, Attributes::CurrentHeapFree::Id),
        .enableCurrentHeapUsed   = IsAttributeEnabled(kRootEndpointId, Attributes::CurrentHeapUsed::Id),
        .enableCurrentWatermarks = IsAttributeEnabled(kRootEndpointId, Attributes::CurrentHeapHighWatermark::Id),
    };

    gServer.Create(enabledAttributes);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register SoftwareDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

void emberAfSoftwareDiagnosticsClusterShutdownCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister SoftwareDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServer.Destroy();
}

void MatterSoftwareDiagnosticsPluginServerInitCallback() {}

void MatterSoftwareDiagnosticsPluginServerShutdownCallback() {}
