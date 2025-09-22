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
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/Global.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DiagnosticLogs;
using namespace chip::Protocols::InteractionModel;

namespace {

// Diagnostic logs cluster is a device-wide cluster, so we only need one cluster instance.
Global<DiagnosticLogsCluster> gServer;

ServerClusterRegistration & ClusterRegistration()
{
    static ServerClusterRegistration gRegistration(gServer.get());
    return gRegistration;
}

} // namespace

void emberAfDiagnosticLogsClusterServerInitCallback(EndpointId endpoint)
{
    // This cluster is assumed to be and implemented as a singleton on the root endpoint
    VerifyOrReturn(endpoint == kRootEndpointId);
    (void) CodegenDataModelProvider::Instance().Registry().Register(ClusterRegistration());
}

void MatterDiagnosticLogsClusterServerShutdownCallback(EndpointId endpointId)
{
    // This cluster is assumed to be and implemented as a singleton on the root endpoint
    VerifyOrReturn(endpointId == kRootEndpointId);
    CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.get());
}

void MatterDiagnosticLogsPluginServerInitCallback() {}
void MatterDiagnosticLogsPluginServerShutdownCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

void DiagnosticLogsServer::SetDiagnosticLogsProviderDelegate(EndpointId endpoint, DiagnosticLogsProviderDelegate * delegate)
{
    gServer->SetDelegate(delegate);
}

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
