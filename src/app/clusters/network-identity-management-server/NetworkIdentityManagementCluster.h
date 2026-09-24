/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/network-identity-management-server/AuthenticatorDriver.h>
#include <app/clusters/network-identity-management-server/NetworkIdentityKeystore.h>
#include <app/clusters/network-identity-management-server/NetworkIdentityStorage.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/NetworkIdentityManagement/ClusterId.h>
#include <lib/core/DataModelTypes.h>

namespace chip::app::Clusters {

class NetworkIdentityManagementCluster : public DefaultServerCluster, public NetworkIdentityManagement::AuthenticatorDriverCallback
{
public:
    static constexpr auto Id = NetworkIdentityManagement::Id;

    NetworkIdentityManagementCluster(EndpointId endpoint, NetworkIdentityStorage & storage,
                                     Crypto::NetworkIdentityKeystore & keystore,
                                     NetworkIdentityManagement::AuthenticatorDriver & authenticator);

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    // AuthenticatorDriverCallback
    void OnClientAuthenticated(uint16_t clientIndex, uint16_t networkIdentityIndex) override;

private:
    std::optional<DataModel::ActionReturnStatus>
    HandleImportAdminSecret(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleExportAdminSecret(const DataModel::InvokeRequest & request,
                                                                         CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleQueryIdentity(const DataModel::InvokeRequest & request,
                                                                     TLV::TLVReader & input_arguments, CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleAddClient(const DataModel::InvokeRequest & request,
                                                                 TLV::TLVReader & input_arguments, CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleRemoveClient(const DataModel::InvokeRequest & request,
                                                                    TLV::TLVReader & input_arguments, CommandHandler * handler);

    DataModel::ActionReturnStatus ReadActiveNetworkIdentities(AttributeValueEncoder & encoder);
    DataModel::ActionReturnStatus ReadClients(AttributeValueEncoder & encoder);

    CHIP_ERROR FindRetirableNetworkIdentities(uint16_t & outTotalCount, Span<uint16_t> & outIndices);
    CHIP_ERROR RetireNetworkIdentity(uint16_t index);

    NetworkIdentityStorage & mStorage;
    Crypto::NetworkIdentityKeystore & mKeystore;
    NetworkIdentityManagement::AuthenticatorDriver & mAuthenticator;
};

} // namespace chip::app::Clusters
