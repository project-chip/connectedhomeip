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
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/static-cluster-config/SoftwareDiagnostics.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoftwareDiagnostics;
using namespace chip::app::Clusters::SoftwareDiagnostics::Attributes;

// for fixed endpoint, this file is ever only included IF software diagnostics is enabled and that MUST happen only on endpoint 0
// the static assert is skipped in case of dynamic endpoints.
static_assert((SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

namespace {

LazyRegisteredServerCluster<SoftwareDiagnosticsServerCluster> gServer;

} // namespace

void emberAfSoftwareDiagnosticsClusterInitCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);

    gServer.Create(SoftwareDiagnosticsLogic::OptionalAttributeSet()
                       .Set<ThreadMetrics::Id>(emberAfContainsAttribute(endpointId, SoftwareDiagnostics::Id, ThreadMetrics::Id))
                       .Set<CurrentHeapFree::Id>(emberAfContainsAttribute(endpointId, SoftwareDiagnostics::Id, CurrentHeapFree::Id))
                       .Set<CurrentHeapUsed::Id>(emberAfContainsAttribute(endpointId, SoftwareDiagnostics::Id, CurrentHeapUsed::Id))
                       .Set<CurrentHeapHighWatermark::Id>(
                           emberAfContainsAttribute(endpointId, SoftwareDiagnostics::Id, CurrentHeapHighWatermark::Id)));

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
