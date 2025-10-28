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

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/tls-certificate-management-server/CertificateTable.h>
#include <app/reporting/reporting.h>
#include <clusters/TlsClientManagement/Commands.h>
#include <clusters/TlsClientManagement/Structs.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
static constexpr uint16_t kSpecMaxHostname = 253;

class TlsClientManagementDelegate;

class TlsClientManagementServer : private AttributeAccessInterface,
                                  private CommandHandlerInterface,
                                  private chip::FabricTable::Delegate
{
public:
    /**
     * Creates a TLSClientManagement server instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param endpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param delegate A reference to the delegate to be used by this server.
     * @param certificateTable A reference to the certificate table for looking up certiciates
     * @param maxProvisioned The maximum number of endpoints which can be provisioned
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    TlsClientManagementServer(EndpointId endpointId, TlsClientManagementDelegate & delegate,
                              Tls::CertificateTable & certificateTable, uint8_t maxProvisioned);
    ~TlsClientManagementServer();

    /**
     * Initialise the TLS Client Management server instance.
     * @return Returns an error  if the CommandHandler or AttributeHandler registration fails.
     */
    CHIP_ERROR Init();

    /**
     * Shuts down the TLS Client Management server instance.
     * @return Returns an error if the destruction fails.
     */
    CHIP_ERROR Finish();

    // Attribute Getters

    /**
     * @return The MaxProvisioned attribute.
     */
    uint8_t GetMaxProvisioned() const;

    /**
     * @return The endpoint ID.
     */
    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

private:
    TlsClientManagementDelegate & mDelegate;
    Tls::CertificateTable & mCertificateTable;

    // Attribute local storage
    uint8_t mMaxProvisioned;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    void HandleProvisionEndpoint(HandlerContext & ctx, const TlsClientManagement::Commands::ProvisionEndpoint::DecodableType & req);
    void HandleFindEndpoint(HandlerContext & ctx, const TlsClientManagement::Commands::FindEndpoint::DecodableType & req);
    void HandleRemoveEndpoint(HandlerContext & ctx, const TlsClientManagement::Commands::RemoveEndpoint::DecodableType & req);

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
    friend class TlsClientManagementServer;

    TlsClientManagementServer * mTlsClientManagementServer = nullptr;

    // sets the TlsClientManagement Server pointer
    void SetTlsClientManagementServer(TlsClientManagementServer * tlsClientManagementServer)
    {
        mTlsClientManagementServer = tlsClientManagementServer;
    }
    TlsClientManagementServer * GetTlsClientManagementServer() const { return mTlsClientManagementServer; }
};

} // namespace Clusters
} // namespace app
} // namespace chip
