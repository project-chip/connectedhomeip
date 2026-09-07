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

#include "CertificateTable.h"
#include <app/ConcreteAttributePath.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/TlsCertificateManagement/Commands.h>
#include <clusters/TlsCertificateManagement/Metadata.h>
#include <clusters/TlsCertificateManagement/Structs.h>
#include <credentials/FabricTable.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class TLSCertificateManagementDelegate;

class TLSCertificateManagementCluster : public DefaultServerCluster, private FabricTable::Delegate
{
public:
    struct Context
    {
        FabricTable & fabricTable;
    };

    /**
     * Creates a TlsCertificateManagement server instance.
     * @param context The context containing injected dependencies.
     *                           Note: the caller must ensure that the provided dependencies
     *                           outlive this instance.
     * @param endpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param delegate A reference to the delegate to be used by this server.
     * @param dependencyChecker A reference to a CertificateDependencyChecker which checks for transitive dependencies
     * @param certificateTable A reference to the certificate table for looking up certificates
     * @param maxRootCertificates The maximum number of root certificates which can be provisioned
     * @param maxClientCertificates The maximum number of client certificates which can be provisioned
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    TLSCertificateManagementCluster(const Context & context, EndpointId endpointId, TLSCertificateManagementDelegate & delegate,
                                    Tls::CertificateDependencyChecker & dependencyChecker, Tls::CertificateTable & certificateTable,
                                    uint8_t maxRootCertificates, uint8_t maxClientCertificates);
    ~TLSCertificateManagementCluster();

    // Attribute Getters

    /**
     * @return The MaxRootCertificates attribute.
     */
    uint8_t GetMaxRootCertificates() const;

    /**
     * @return The MaxClientCertificates attribute.
     */
    uint8_t GetMaxClientCertificates() const;

    /**
     * @return The endpoint ID.
     */
    EndpointId GetEndpointId() const { return mPath.mEndpointId; }

    Tls::CertificateTable & GetCertificateTable() { return mCertificateTable; }

    /**
     * @brief ServerClusterInterface methods.
     */
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

private:
    Context mContext;
    TLSCertificateManagementDelegate & mDelegate;
    Tls::CertificateDependencyChecker & mDependencyChecker;
    Tls::CertificateTable & mCertificateTable;

    // Attribute local storage
    uint8_t mMaxRootCertificates;
    uint8_t mMaxClientCertificates;

    // Command Handlers
    std::optional<DataModel::ActionReturnStatus>
    HandleProvisionRootCertificate(CommandHandler & commandHandler,
                                   const TlsCertificateManagement::Commands::ProvisionRootCertificate::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleFindRootCertificate(CommandHandler & commandHandler,
                              const TlsCertificateManagement::Commands::FindRootCertificate::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleLookupRootCertificate(CommandHandler & commandHandler,
                                const TlsCertificateManagement::Commands::LookupRootCertificate::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleRemoveRootCertificate(CommandHandler & commandHandler,
                                const TlsCertificateManagement::Commands::RemoveRootCertificate::DecodableType & req);

    std::optional<DataModel::ActionReturnStatus>
    HandleGenerateClientCsr(CommandHandler & commandHandler,
                            const TlsCertificateManagement::Commands::ClientCSR::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleProvisionClientCertificate(CommandHandler & commandHandler,
                                     const TlsCertificateManagement::Commands::ProvisionClientCertificate::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleFindClientCertificate(CommandHandler & commandHandler,
                                const TlsCertificateManagement::Commands::FindClientCertificate::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleLookupClientCertificate(CommandHandler & commandHandler,
                                  const TlsCertificateManagement::Commands::LookupClientCertificate::DecodableType & req);
    std::optional<DataModel::ActionReturnStatus>
    HandleRemoveClientCertificate(CommandHandler & commandHandler,
                                  const TlsCertificateManagement::Commands::RemoveClientCertificate::DecodableType & req);

    // Encodes all provisioned root certificates
    CHIP_ERROR EncodeProvisionedRootCertificates(EndpointId matterEndpoint, FabricIndex fabric, bool largePayload,
                                                 const AttributeValueEncoder::ListEncodeHelper & encoder);
    // Encodes all provisioned client certificates
    CHIP_ERROR EncodeProvisionedClientCertificates(EndpointId matterEndpoint, FabricIndex fabric, bool largePayload,
                                                   const AttributeValueEncoder::ListEncodeHelper & encoder);

    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;
};

/** @brief
 *  Defines methods for implementing application-specific logic for the TlsCertificateManagement Cluster.
 */
class TLSCertificateManagementDelegate
{
public:
    using RootCertStructType             = TlsCertificateManagement::Structs::TLSCertStruct::Type;
    using ClientCertStructType           = TlsCertificateManagement::Structs::TLSClientCertificateDetailStruct::Type;
    using ProvisionRootCertificateType   = TlsCertificateManagement::Commands::ProvisionRootCertificate::DecodableType;
    using ProvisionClientCertificateType = TlsCertificateManagement::Commands::ProvisionClientCertificate::DecodableType;
    using ClientCsrType                  = TlsCertificateManagement::Commands::ClientCSR::DecodableType;
    using ClientCsrResponseType          = TlsCertificateManagement::Commands::ClientCSRResponse::Type;

    TLSCertificateManagementDelegate() = default;

    virtual ~TLSCertificateManagementDelegate() = default;

    using RootCertificateListCallback     = std::function<CHIP_ERROR(DataModel::List<const RootCertStructType> & certs)>;
    using ClientCertificateListCallback   = std::function<CHIP_ERROR(DataModel::List<const ClientCertStructType> & certs)>;
    using LoadedRootCertificateCallback   = std::function<CHIP_ERROR(RootCertStructType & cert)>;
    using LoadedClientCertificateCallback = std::function<CHIP_ERROR(ClientCertStructType & certs)>;
    using GeneratedCsrCallback            = std::function<Protocols::InteractionModel::Status(ClientCsrResponseType & cert)>;

    /**
     * @brief Appends a root certificate to the provisioned root certificates list maintained by the delegate.
     *        The delegate must ensure it makes a copy of the provided request and data, if any.
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the certificate is associated with
     * @param[in] provisionReq The request data specifying the root certificate to be provisioned
     * @param[out] outCaid a reference to the TLSCAID variable that is to contain the ID of the provisioned root cert.
     *
     * @return Success if the certificate was provisioned successfully, or a failure status otherwise.
     */
    virtual Protocols::InteractionModel::Status ProvisionRootCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                  const ProvisionRootCertificateType & provisionReq,
                                                                  Tls::TLSCAID & outCaid) = 0;

    /**
     * @brief Executes loadedCallback for each root certificate with matching (matterEndpoint, fabric). The certificate passed to
     * loadedCallback has a guaranteed lifetime of the method call.
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the certificate is associated with
     * @param[in] loadedCallback lambda to execute for each certificate.  If this function returns an error result,
     * iteration stops and returns that same error result.
     */
    virtual CHIP_ERROR LoadedRootCerts(EndpointId matterEndpoint, FabricIndex fabric,
                                       LoadedRootCertificateCallback loadedCallback) const = 0;

    /**
     * @brief Executes loadedCallback on all matching certificates. The certificates passed to loadedCallback has a guaranteed
     * lifetime of the method call.
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric to load root certificates for
     * @param[in] loadedCallback lambda to execute on list of all matching certificates.  If this function returns an error result,
     * that error is propagated.
     */
    virtual CHIP_ERROR RootCertsForFabric(EndpointId matterEndpoint, FabricIndex fabric,
                                          RootCertificateListCallback loadedCallback) const = 0;

    /**
     * @brief Finds the TLSCertStruct with the given (matterEndpoint, fabric, id). The certificates passed to loadedCallback has a
     * guaranteed lifetime of the method call.
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the certificate is associated with
     * @param[in] id The id of the root certificate to find.
     * @param[out] loadedCallback The lambda to execute with the loaded root cert.
     * @return CHIP_ERROR_NOT_FOUND if no mapping is found.
     */
    virtual CHIP_ERROR FindRootCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id,
                                    LoadedRootCertificateCallback loadedCallback) const = 0;

    /**
     * @brief Finds the TLSCertStruct with the given (matterEndpoint, fabric, fingerprint). The certificates passed to
     * loadedCallback has a guaranteed lifetime of the method call.
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the certificate is associated with
     * @param[in] fingerprint The fingerprint of the root certificate to find.
     * @param[in] loadedCallback The lambda to execute with the loaded root cert.
     * @return CHIP_ERROR_NOT_FOUND if no mapping is found.
     */
    virtual CHIP_ERROR LookupRootCertByFingerprint(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & fingerprint,
                                                   LoadedRootCertificateCallback loadedCallback) const = 0;

    /**
     * @brief Like LookupRootCertByFingerprint except looks up by the certificate instead of fingerprint.
     * Implementations will generally fingerprint the certificate & delegate.
     */
    virtual CHIP_ERROR LookupRootCert(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & cert,
                                      LoadedRootCertificateCallback loadedCallback) const = 0;
    /**
     * @brief Removes the root certificate with the given (matterEndpoint, fabric, id)
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the certificate is associated with
     * @param[in] id The id of the root certificate to remove.
     * @return Success if the certificate was removed, NotFound if the certificate could not be found for (matterEndpoint, fabric),
     * InvalidInState if the certificate is attached/in-use
     */
    virtual Protocols::InteractionModel::Status RemoveRootCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id) = 0;

    /**
     * @brief Generates a client certificate signing request (CSR)
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fingerprint The fingerprint of the root certificate to find.
     * @param[in] loadedCallback The lambda to execute with the generated client CSR. This lambda will be called before this method
     * returns.
     * @return Success if the CSR was generated successfully, or a failure status otherwise
     */
    virtual Protocols::InteractionModel::Status GenerateClientCsr(EndpointId matterEndpoint, FabricIndex fabric,
                                                                  const ClientCsrType & request,
                                                                  GeneratedCsrCallback loadedCallback) const = 0;

    /**
     * @brief Appends a client certificate to the provisioned client certificates list maintained by the delegate.
     *        The delegate must ensure it makes a copy of the provided request and data, if any.
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the endpoint is associated with
     * @param[in] provisionReq The request data specifying the client certificate to be provisioned
     *
     * @return Success if the certificate was provisioned successfully, or a failure status otherwise
     */
    virtual Protocols::InteractionModel::Status ProvisionClientCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                    const ProvisionClientCertificateType & provisionReq) = 0;

    /**
     * @brief Executes loadedCallback for each client certificate matching (matterEndpoint, fabric).  The certificate passed to
     loadedCallback has a guaranteed lifetime of the method call.
     *
     * @param[in] loadedCallback lambda to execute with allocated memory for client certificate loading.  If this function returns
     an error result, returns that same error result.
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the certificate is associated with
     */
    virtual CHIP_ERROR LoadedClientCerts(EndpointId matterEndpoint, FabricIndex fabric,
                                         LoadedClientCertificateCallback loadedCallback) const = 0;

    /**
     * @brief Executes loadedCallback for all matching client certificates.  The certificates passed to loadedCallback has a
     guaranteed lifetime of the method call.
     *
     * @param[in] loadedCallback lambda to execute with allocated memory for client certificate loading.  If this function returns
     an error result, that error is propagated.

     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric to load client certificates for
     */
    virtual CHIP_ERROR ClientCertsForFabric(EndpointId matterEndpoint, FabricIndex fabric,
                                            ClientCertificateListCallback loadedCallback) const = 0;

    /**
     * @brief Finds the TLSClientCertificateDetailStruct with the given (matterEndpoint, fabric, id). The certificates passed to
     * loadedCallback has a guaranteed lifetime of the method call.
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the certificate is associated with
     * @param[in] id The id of the client certificate to find.
     * @param[in] loadedCallback The lambda to execute with the loaded client cert.
     * @return CHIP_ERROR_NOT_FOUND if no mapping is found.
     */
    virtual CHIP_ERROR FindClientCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCCDID id,
                                      LoadedClientCertificateCallback loadedCallback) const = 0;

    /**
     * @brief Finds the TLSClientCertificateDetailStruct with the given (matterEndpoint, fabric, fingerprint). The certificates
     * passed to loadedCallback has a guaranteed lifetime of the method call.
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the certificate is associated with
     * @param[in] fingerprint The fingerprint of the client certificate to find.
     * @param[in] loadedCallback The lambda to execute with the loaded client cert.
     * @return NotFound if no mapping is found.
     */
    virtual CHIP_ERROR LookupClientCertByFingerprint(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & fingerprint,
                                                     LoadedClientCertificateCallback loadedCallback) const = 0;

    /**
     * @brief Like LookupClientCertByFingerprint except looks up by the certificate instead of fingerprint.
     * Implementations will generally fingerprint the certificate & delegate.
     */
    virtual CHIP_ERROR LookupClientCert(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & certificate,
                                        LoadedClientCertificateCallback loadedCallback) const = 0;

    /**
     * @brief Removes the client certificate with the given (matterEndpoint, fabric, id)
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the certificate is associated with
     * @param[in] id The id of the client certificate to remove.
     * @return Success if the certificate was removed, NotFound if the certificate could not be found for (matterEndpoint, fabric),
     * InvalidInState if the certificate is attached/in-use
     */
    virtual Protocols::InteractionModel::Status RemoveClientCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                 Tls::TLSCCDID id) = 0;

    Tls::CertificateTable & GetCertificateTable() { return mTLSCertificateManagementCluster->GetCertificateTable(); }

protected:
    friend class TLSCertificateManagementCluster;

    TLSCertificateManagementCluster * mTLSCertificateManagementCluster = nullptr;

    // sets the TlsCertificateManagement Cluster pointer
    void SetTLSCertificateManagementCluster(TLSCertificateManagementCluster * tlsCertificateManagementServer)
    {
        mTLSCertificateManagementCluster = tlsCertificateManagementServer;
    }
    TLSCertificateManagementCluster * GetTLSCertificateManagementCluster() const { return mTLSCertificateManagementCluster; }
};

} // namespace Clusters
} // namespace app
} // namespace chip
