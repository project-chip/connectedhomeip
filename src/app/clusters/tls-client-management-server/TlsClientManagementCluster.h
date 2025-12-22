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

#include <app/clusters/tls-certificate-management-server/CertificateTable.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/TlsClientManagement/Commands.h>
#include <clusters/TlsClientManagement/Metadata.h>
#include <clusters/TlsClientManagement/Structs.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
static constexpr uint16_t kSpecMaxHostname = 253;

class TlsClientManagementDelegate;

class TlsClientManagementCluster : public DefaultServerCluster, private chip::FabricTable::Delegate
{
public:
    /**
     * Creates a TLSClientManagement server instance.
     * @param endpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param delegate A reference to the delegate to be used by this server.
     * @param certificateTable A reference to the certificate table for looking up certificates
     * @param maxProvisioned The maximum number of endpoints which can be provisioned
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    TlsClientManagementCluster(EndpointId endpointId, TlsClientManagementDelegate & delegate,
                               Tls::CertificateTable & certificateTable, uint8_t maxProvisioned);
    ~TlsClientManagementCluster();

    // Attribute Getters

    /**
     * @return The MaxProvisioned attribute.
     */
    uint8_t GetMaxProvisioned() const;

    /**
     * @return The endpoint ID.
     */
    EndpointId GetEndpointId() { return mPath.mEndpointId; }

    /**
     * @brief ServerClusterInterface methods.
     */
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

private:
    TlsClientManagementDelegate & mDelegate;
    Tls::CertificateTable & mCertificateTable;

    // Attribute local storage
    uint8_t mMaxProvisioned;

    // Command handlers
    std::optional<DataModel::ActionReturnStatus>
    HandleProvisionEndpoint(CommandHandler & commandHandler,
                            const TlsClientManagement::Commands::ProvisionEndpoint::DecodableType & req);

    std::optional<DataModel::ActionReturnStatus>
    HandleFindEndpoint(CommandHandler & commandHandler, const TlsClientManagement::Commands::FindEndpoint::DecodableType & req);

    std::optional<DataModel::ActionReturnStatus>
    HandleRemoveEndpoint(CommandHandler & commandHandler, const TlsClientManagement::Commands::RemoveEndpoint::DecodableType & req);

    // Encodes all provisioned endpoints
    CHIP_ERROR EncodeProvisionedEndpoints(EndpointId matterEndpoint, FabricIndex fabric,
                                          const AttributeValueEncoder::ListEncodeHelper & encoder);

    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;
};

/** @brief
 *  Defines methods for implementing application-specific logic for the TLSClientManagement Cluster.
 */
class TlsClientManagementDelegate : public Tls::CertificateDependencyChecker
{
public:
    using EndpointStructType     = TlsClientManagement::Structs::TLSEndpointStruct::DecodableType;
    using LoadedEndpointCallback = std::function<CHIP_ERROR(EndpointStructType & endpoint)>;

    TlsClientManagementDelegate() = default;

    virtual ~TlsClientManagementDelegate() = default;

    virtual CHIP_ERROR Init(PersistentStorageDelegate & storage) = 0;

    /**
     * @brief Executes callback for each TLSEndpointStruct matching (matterEndpoint, fabric). The endpoint passed to
     * callback has a guaranteed lifetime of the method call.
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the endpoint is associated with
     * @param[in] callback lambda to execute for each endpoint.  If this function returns an error result,
     * iteration stops and returns that same error result.
     */
    virtual CHIP_ERROR ForEachEndpoint(EndpointId matterEndpoint, FabricIndex fabric, LoadedEndpointCallback callback) = 0;

    /**
     * @brief Finds the TLSEndpointStruct with the given EndpointID
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] endpointID The EndpoitnID to find.
     * @param[in] callback lambda to execute for found endpoint.  If this function returns an error result,
     * iteration stops and returns that same error result.
     * @return CHIP_ERROR_NOT_FOUND if not found
     */
    virtual CHIP_ERROR FindProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric, uint16_t endpointID,
                                                   LoadedEndpointCallback callback) = 0;

    /**
     * @brief Appends a TLSEndpointStruct to the provisioned endpoints list maintained by the delegate.
     *        The delegate must ensure it makes a copy of the provided preset and the data
     *        of its preset handle, if any.
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the endpoint is associated with
     * @param[in] provisionReq The request data specifying the endpoint to be provisioned
     * @param[out] endpointID a reference to the uint16_t variable that is to contain the ID of the provisioned endpoint.
     *
     * @return Success if the endpoint was appended to the list successfully, a failure code otherwise.
     */
    virtual Protocols::InteractionModel::ClusterStatusCode
    ProvisionEndpoint(EndpointId matterEndpoint, FabricIndex fabric,
                      const TlsClientManagement::Commands::ProvisionEndpoint::DecodableType & provisionReq,
                      uint16_t & endpointID) = 0;

    /**
     * @brief Removes the TLSEndpointStruct with the given EndpointID
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric to query against
     * @param[in] endpointID The ID of the endpoint to remove.
     * @return Success if the endpoint was removed, a failure Status otherwise.
     */
    virtual Protocols::InteractionModel::Status RemoveProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric,
                                                                              uint16_t endpointID) = 0;

    /**
     * @brief Removes all associated endpoints for the given fabricIndex.
     */
    virtual void RemoveFabric(FabricIndex fabricIndex) = 0;

    /**
     * @brief Mutates the referenceCount of the TLSEndpointStruct by delta with the given EndpointID
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric to query against
     * @param[in] endpointID The ID of the endpoint to remove.
     * @param[in] delta The amount to mutate the reference count by.
     * @return CHIP_NO_ERROR if the mutation was successful, CHIP_ERROR_NOT_FOUND if not found, a failure code otherwise.
     */
    virtual CHIP_ERROR MutateEndpointReferenceCount(EndpointId matterEndpoint, FabricIndex fabric, uint16_t endpointID,
                                                    int8_t delta) = 0;

protected:
    friend class TlsClientManagementCluster;

    TlsClientManagementCluster * mTlsClientManagementCluster = nullptr;

    // sets the TlsClientManagement Cluster pointer
    void SetTlsClientManagementCluster(TlsClientManagementCluster * tlsClientManagementServer)
    {
        mTlsClientManagementCluster = tlsClientManagementServer;
    }
    TlsClientManagementCluster * GetTlsClientManagementCluster() const { return mTlsClientManagementCluster; }
};

/**
 * Set the delegate for the TLS Client Management cluster.
 * MUST be called before server initialization (e.g. in main() before ServerInit()).
 * If not called, a default delegate with minimal functionality will be used.
 */
void MatterTlsClientManagementSetDelegate(TlsClientManagementDelegate & delegate);

/**
 * Set the certificate table for the TLS Client Management cluster.
 * MUST be called before server initialization (e.g. in main() before ServerInit()).
 * If not called, a default certificate table will be used.
 */
void MatterTlsClientManagementSetCertificateTable(Tls::CertificateTable & certificateTable);

} // namespace Clusters
} // namespace app
} // namespace chip
