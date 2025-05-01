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

// this file is ever only included IF software diagnostics is enabled and that MUST happen only on endpoint 0
static_assert(SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 1,
              "Exactly one softare diagnistics provider may exist on Endpoint 0");
static_assert(SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId,
              "Exactly one softare diagnistics provider may exist on Endpoint 0");

namespace {

RegisteredServerCluster<SoftwareDiagnosticsServerCluster<DeviceLayerSoftwareDiagnosticsLogic>> gServer;

} // namespace

void MatterSoftwareDiagnosticsPluginServerInitCallback()
{
    // We know Endpoint 0 contains this cluster
    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration()));
}
void MatterSoftwareDiagnosticsPluginServerShutdownCallback()
{
    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster()));
}
