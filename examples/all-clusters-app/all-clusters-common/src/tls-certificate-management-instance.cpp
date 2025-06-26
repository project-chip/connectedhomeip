/*
 *
 *    Copyright (c) 2025 Matter Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>
#include <app/clusters/tls-certificate-management-server/tls-certificate-management-server.h>
#include <clusters/TlsCertificateManagement/Commands.h>
#include <lib/support/Pool.h>
#include <tls-certificate-management-instance.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsCertificateManagement;
using namespace Protocols::InteractionModel;

static constexpr size_t kCertPoolSize = 5;

struct InlineBufferedRootCert : CertificateTable::BufferedRootCert
{
    PersistentStore<CHIP_CONFIG_TLS_PERSISTED_ROOT_CERT_BYTES> buffer;
    InlineBufferedRootCert() : CertificateTable::BufferedRootCert(buffer) {}
};

struct InlineBufferedClientCert : CertificateTable::BufferedClientCert
{
    PersistentStore<CHIP_CONFIG_TLS_PERSISTED_CLIENT_CERT_BYTES> buffer;
    InlineBufferedClientCert() : CertificateTable::BufferedClientCert(buffer) {}
};

using RootCertPool   = ObjectPool<InlineBufferedRootCert, kCertPoolSize>;
using ClientCertPool = ObjectPool<InlineBufferedClientCert, kCertPoolSize>;

static RootCertPool gRootCertPool;
static ClientCertPool gClientCertPool;

static constexpr uint8_t kMaxRootCerts   = 254;
static constexpr uint8_t kMaxClientCerts = 254;

ClusterStatusCode TlsCertificateManagementCommandDelegate::ProvisionRootCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                             const ProvisionRootCertificateType & provisionReq,
                                                                             Tls::TLSCAID & outCaid)
{
    outCaid = mNextRootId++;
    return ClusterStatusCode(Status::Failure);
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::LoadedRootCerts(EndpointId matterEndpoint, FabricIndex fabric,
                                                                    LoadedRootCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    auto * certBuffer = gRootCertPool.CreateObject();
    VerifyOrReturnError(certBuffer != nullptr, CHIP_ERROR_NO_MEMORY);

    PoolAutoRelease<InlineBufferedRootCert, RootCertPool> certBufferRelease(gRootCertPool, certBuffer);

    return mCertificateTable.IterateRootEntries(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->mCert))
        {
            ReturnErrorOnFailure(loadedCallback(certBuffer->mCert));
        }
        return CHIP_NO_ERROR;
    });
}

Status TlsCertificateManagementCommandDelegate::LookupRootCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                               const ByteSpan & fingerprint,
                                                               LoadedRootCertificateCallback loadedCallback) const
{
    return Status::Failure;
}

Status TlsCertificateManagementCommandDelegate::GenerateClientCsr(EndpointId matterEndpoint, FabricIndex fabric,
                                                                  const ClientCsrType & request,
                                                                  GeneratedCsrCallback loadedCallback) const
{
    return Status::Failure;
}

ClusterStatusCode TlsCertificateManagementCommandDelegate::ProvisionClientCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                               const ProvisionClientCertificateType & provisionReq)
{
    return ClusterStatusCode(Status::Failure);
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::LoadedClientCerts(EndpointId matterEndpoint, FabricIndex fabric,
                                                                      LoadedClientCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    auto * certBuffer = gClientCertPool.CreateObject();
    VerifyOrReturnError(certBuffer != nullptr, CHIP_ERROR_NO_MEMORY);

    PoolAutoRelease<InlineBufferedClientCert, ClientCertPool> certBufferRelease(gClientCertPool, certBuffer);

    return mCertificateTable.IterateClientEntries(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->mCert))
        {
            ReturnErrorOnFailure(loadedCallback(certBuffer->mCert));
        }
        return CHIP_NO_ERROR;
    });
}

Status TlsCertificateManagementCommandDelegate::LookupClientCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                 const ByteSpan & fingerprint,
                                                                 LoadedClientCertificateCallback loadedCallback) const
{
    return Status::Failure;
}

static CertificateTableImpl gCertificateTableInstance;
TlsCertificateManagementCommandDelegate TlsCertificateManagementCommandDelegate::instance(gCertificateTableInstance);
static TlsCertificateManagementServer gTlsCertificateManagementClusterServerInstance =
    TlsCertificateManagementServer(EndpointId(1), TlsCertificateManagementCommandDelegate::getInstance(), gCertificateTableInstance,
                                   kMaxRootCerts, kMaxClientCerts);

void emberAfTlsCertificateManagementClusterInitCallback(EndpointId matterEndpoint)
{
    gCertificateTableInstance.SetEndpoint(EndpointId(1));
    gTlsCertificateManagementClusterServerInstance.Init();
}

void emberAfTlsCertificateManagementClusterShutdownCallback(EndpointId matterEndpoint)
{
    gTlsCertificateManagementClusterServerInstance.Finish();
}
