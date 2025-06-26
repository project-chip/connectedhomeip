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

#include <app/clusters/tls-client-management-server/tls-client-management-server.h>
#include <app/util/config.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

/**
 * The application delegate to define the options & implement commands.
 */
class TlsClientManagementCommandDelegate : public TlsClientManagementDelegate
{
    struct Provisioned
    {
        FabricIndex fabric;
        EndpointStructType payload;
    };

    static TlsClientManagementCommandDelegate instance;
    Tls::CertificateTable & mCertificateTable;
    std::vector<Provisioned> mProvisioned;
    uint16_t mNextId = 1;

public:
    TlsClientManagementCommandDelegate(Tls::CertificateTable & certificateTable) : mCertificateTable(certificateTable) {}
    ~TlsClientManagementCommandDelegate() = default;

    CHIP_ERROR GetProvisionedEndpointByIndex(EndpointId matterEndpoint, FabricIndex fabric, size_t index,
                                             EndpointStructType & endpoint) const override;

    Protocols::InteractionModel::ClusterStatusCode
    ProvisionEndpoint(EndpointId matterEndpoint, FabricIndex fabric,
                      const TlsClientManagement::Commands::ProvisionEndpoint::DecodableType & provisionReq,
                      uint16_t & endpointID) override;

    Protocols::InteractionModel::Status FindProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric,
                                                                    uint16_t endpointID,
                                                                    EndpointStructType & endpoint) const override;

    Protocols::InteractionModel::ClusterStatusCode RemoveProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric,
                                                                                 uint16_t endpointID) override;

    static inline TlsClientManagementCommandDelegate & GetInstance() { return instance; }
};

} // namespace Clusters
} // namespace app
} // namespace chip
