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

#pragma once

#include <app/clusters/tls-certificate-management-server/TlsCertificateManagementCluster.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * The application delegate to define the options & implement commands.
 */
class TlsCertificateManagementCommandDelegate : public TlsCertificateManagementDelegate
{
    static TlsCertificateManagementCommandDelegate instance;
    Tls::CertificateTable & mCertificateTable;

public:
    TlsCertificateManagementCommandDelegate(Tls::CertificateTable & certificateTable) : mCertificateTable(certificateTable) {}
    ~TlsCertificateManagementCommandDelegate() = default;

    Protocols::InteractionModel::Status ProvisionRootCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                          const ProvisionRootCertificateType & provisionReq,
                                                          Tls::TLSCAID & outCaid) override;

    CHIP_ERROR LoadedRootCerts(EndpointId matterEndpoint, FabricIndex fabric,
                               LoadedRootCertificateCallback loadedCallback) const override;
    CHIP_ERROR RootCertsForFabric(EndpointId matterEndpoint, FabricIndex fabric,
                                  RootCertificateListCallback loadedCallback) const override;

    CHIP_ERROR FindRootCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id,
                            LoadedRootCertificateCallback loadedCallback) const override;
    CHIP_ERROR LookupRootCertByFingerprint(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & fingerprint,
                                           LoadedRootCertificateCallback loadedCallback) const override;
    CHIP_ERROR LookupRootCert(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & certificate,
                              LoadedRootCertificateCallback loadedCallback) const override;
    Protocols::InteractionModel::Status RemoveRootCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id) override;

    Protocols::InteractionModel::Status GenerateClientCsr(EndpointId matterEndpoint, FabricIndex fabric,
                                                          const ClientCsrType & request,
                                                          GeneratedCsrCallback loadedCallback) const override;
    Protocols::InteractionModel::Status ProvisionClientCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                            const ProvisionClientCertificateType & provisionReq) override;

    CHIP_ERROR LoadedClientCerts(EndpointId matterEndpoint, FabricIndex fabric,
                                 LoadedClientCertificateCallback loadedCallback) const override;
    CHIP_ERROR ClientCertsForFabric(EndpointId matterEndpoint, FabricIndex fabric,
                                    ClientCertificateListCallback loadedCallback) const override;

    CHIP_ERROR FindClientCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCCDID id,
                              LoadedClientCertificateCallback loadedCallback) const override;
    CHIP_ERROR LookupClientCertByFingerprint(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & fingerprint,
                                             LoadedClientCertificateCallback loadedCallback) const override;
    CHIP_ERROR LookupClientCert(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & certificate,
                                LoadedClientCertificateCallback loadedCallback) const override;
    Protocols::InteractionModel::Status RemoveClientCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCCDID id) override;

    static inline TlsCertificateManagementCommandDelegate & GetInstance() { return instance; }
};

} // namespace Clusters
} // namespace app
} // namespace chip
