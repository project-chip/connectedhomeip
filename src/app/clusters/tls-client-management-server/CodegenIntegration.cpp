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

#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>
#include <app/clusters/tls-client-management-server/TlsClientManagementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
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

// Default implementations - applications can override via SetDelegate/SetCertificateTable
class DefaultTlsClientManagementDelegate : public TlsClientManagementDelegate
{
public:
    CHIP_ERROR Init(PersistentStorageDelegate & storage) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ForEachEndpoint(EndpointId matterEndpoint, FabricIndex fabric, LoadedEndpointCallback callback) override
    {
        // No endpoints provisioned in default implementation
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FindProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric, uint16_t endpointID,
                                           LoadedEndpointCallback callback) override
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    Protocols::InteractionModel::ClusterStatusCode
    ProvisionEndpoint(EndpointId matterEndpoint, FabricIndex fabric,
                      const TlsClientManagement::Commands::ProvisionEndpoint::DecodableType & provisionReq,
                      uint16_t & endpointID) override
    {
        return Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::UnsupportedCommand);
    }

    Protocols::InteractionModel::Status RemoveProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric,
                                                                      uint16_t endpointID) override
    {
        return Protocols::InteractionModel::Status::NotFound;
    }

    void RemoveFabric(FabricIndex fabricIndex) override {}

    CHIP_ERROR MutateEndpointReferenceCount(EndpointId matterEndpoint, FabricIndex fabric, uint16_t endpointID,
                                            int8_t delta) override
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR RootCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id) override
    {
        // Default: allow removal (no dependencies tracked)
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ClientCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCCDID id) override
    {
        // Default: allow removal (no dependencies tracked)
        return CHIP_NO_ERROR;
    }
};

DefaultTlsClientManagementDelegate gDefaultDelegate;
CertificateTableImpl gDefaultCertificateTable;

TlsClientManagementDelegate * gDelegate           = &gDefaultDelegate;
CertificateTable * gCertificateTable              = &gDefaultCertificateTable;
constexpr uint8_t kDefaultMaxProvisionedEndpoints = 254;

LazyRegisteredServerCluster<TlsClientManagementCluster> gClusterInstance;

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {

void MatterTlsClientManagementSetDelegate(TlsClientManagementDelegate & delegate)
{
    VerifyOrDie(!gClusterInstance.IsConstructed());
    gDelegate = &delegate;
}

void MatterTlsClientManagementSetCertificateTable(Tls::CertificateTable & certificateTable)
{
    VerifyOrDie(!gClusterInstance.IsConstructed());
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
    // Only create once - avoid double initialization if callback is called multiple times
    VerifyOrReturn(!gClusterInstance.IsConstructed());

    // SetEndpoint is only available on CertificateTableImpl.
    // Both the default and application-provided tables are expected to be CertificateTableImpl instances.
    CertificateTableImpl * impl = static_cast<CertificateTableImpl *>(gCertificateTable);
    VerifyOrDie(impl != nullptr);
    LogErrorOnFailure(impl->SetEndpoint(endpointId));

    gClusterInstance.Create(endpointId, *gDelegate, *gCertificateTable, kDefaultMaxProvisionedEndpoints);
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

    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&gClusterInstance.Cluster()));
    gClusterInstance.Destroy();
}
