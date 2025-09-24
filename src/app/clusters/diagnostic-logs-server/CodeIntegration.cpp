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

#include "CodeIntegration.h"

#include <app/clusters/diagnostic-logs-server/DiagnosticLogsCluster.h>
#include <app/static-cluster-config/DiagnosticLogs.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <protocols/bdx/DiagnosticLogs.h>

#include "BDXDiagnosticLogsProvider.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Protocols::InteractionModel;

namespace {
static constexpr size_t kDiagnosticLogsFixedClusterCount = DiagnosticLogs::StaticApplicationConfig::kFixedClusterConfig.size();

static constexpr size_t kDiagnosticLogsMaxClusterCount =
    kDiagnosticLogsFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<DiagnosticLogsCluster> gServers[kDiagnosticLogsMaxClusterCount];

// Find the 0-based array index corresponding to the given endpoint id.
// Log an error if not found.
bool findEndpointWithLog(EndpointId endpointId, uint16_t & outArrayIndex)
{
    uint16_t arrayIndex = emberAfGetClusterServerEndpointIndex(endpointId, DiagnosticLogs::Id, kDiagnosticLogsFixedClusterCount);

    if (arrayIndex >= kDiagnosticLogsMaxClusterCount)
    {
        ChipLogError(AppServer, "Cound not find endpoint index for endpoint %u", endpointId);
        return false;
    }
    return true;
}
} // namespace

void emberAfDiagnosticLogsClusterInitCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }
    gServers[arrayIndex].Create();
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServers[arrayIndex].Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register Diagnostic Logs on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void emberAfDiagnosticLogsClusterShutdownCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServers[arrayIndex].Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister Diagnostic Logs on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServers[arrayIndex].Destroy();
}

bool emberAfDiagnosticLogsClusterRetrieveLogsRequestCallback(chip::app::CommandHandler * commandObj,
                                                             const chip::app::ConcreteCommandPath & commandPath,
                                                             const Commands::RetrieveLogsRequest::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpoint, arrayIndex))
    {
        return false;
    }

    // If the Intent and/or the RequestedProtocol arguments contain invalid (out of range) values the command SHALL fail with a
    // Status Code of INVALID_COMMAND.
    auto intent   = commandData.intent;
    auto protocol = commandData.requestedProtocol;
    if (intent == IntentEnum::kUnknownEnumValue || protocol == TransferProtocolEnum::kUnknownEnumValue)
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    auto instance = gServers[arrayIndex].Cluster().Instance();
    if (protocol == TransferProtocolEnum::kResponsePayload)
    {
        instance.HandleLogRequestForResponsePayload(commandObj, commandPath, intent);
    }
    else
    {
        instance.HandleLogRequestForBdx(commandObj, commandPath, intent, commandData.transferFileDesignator);
    }

    return true;
}

void MatterDiagnosticLogsPluginServerInitCallback() {}
void MatterDiagnosticLogsPluginServerShutdownCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

void SetDelegate(EndpointId endpoint, DiagnosticLogsProviderDelegate * delegate)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpoint, arrayIndex))
    {
        return;
    }
    gServers[arrayIndex].Cluster().SetDelegate(endpoint, delegate);
}

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
