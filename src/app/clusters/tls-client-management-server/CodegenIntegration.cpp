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

#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/server/Server.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::DataModel;

namespace {

CertificateTableImpl gDefaultCertificateTable;

TLSClientManagementDelegate * gDelegate           = nullptr;
CertificateTableImpl * gCertificateTable          = &gDefaultCertificateTable;
constexpr uint8_t kDefaultMaxProvisionedEndpoints = CHIP_CONFIG_TLS_MAX_PROVISIONED_ENDPOINTS;

LazyRegisteredServerCluster<TLSClientManagementCluster> gClusterInstance;

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {

void MatterTlsClientManagementSetDelegate(TLSClientManagementDelegate & delegate)
{
    gDelegate = &delegate;
}

void MatterTlsClientManagementSetCertificateTable(Tls::CertificateTableImpl & certificateTable)
{
    gCertificateTable = &certificateTable;
}

} // namespace Clusters
} // namespace app
} // namespace chip

void MatterTlsClientManagementPluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initializing TLS Client Management cluster.");
}

void MatterTlsClientManagementClusterInitCallback(EndpointId endpointId)
{
    // TODO:(#42638) This cluster SHALL be present on the root node endpoint (endpoint 0) when required, and SHALL NOT be present on
    // any other endpoint. if (endpointId != kRootEndpointId)
    // {
    //     ChipLogError(Zcl,
    //                  "TLS Client Management Cluster can only be initialized on root endpoint (0). "
    //                  "Ignoring initialization on endpoint %u.",
    //                  endpointId);
    //     return;
    // }

    // Only create once - avoid double initialization if callback is called multiple times
    if (gClusterInstance.IsConstructed())
    {
        ChipLogError(Zcl, "TLS Client Management Cluster already initialized on endpoint 0. Ignoring duplicate initialization.");
        return;
    }

    // Require a proper delegate - the cluster cannot function without one
    if (gDelegate == nullptr)
    {
        ChipLogError(Zcl,
                     "TLS Client Management Cluster cannot be initialized without a delegate. "
                     "Call MatterTlsClientManagementSetDelegate() before ServerInit().");
        return;
    }

    LogErrorOnFailure(gCertificateTable->SetEndpoint(endpointId));

    TLSClientManagementCluster::Context context{ Server::GetInstance().GetFabricTable() };
    gClusterInstance.Create(context, endpointId, *gDelegate, *gCertificateTable, kDefaultMaxProvisionedEndpoints);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gClusterInstance.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to register TLS Client Management Cluster on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

void MatterTlsClientManagementClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    VerifyOrReturn(gClusterInstance.IsConstructed());
    VerifyOrReturn(gClusterInstance.Cluster().GetEndpointId() == endpointId);

    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&gClusterInstance.Cluster()));
    gClusterInstance.Destroy();
}
