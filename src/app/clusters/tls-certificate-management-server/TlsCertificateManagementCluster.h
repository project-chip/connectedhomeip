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
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/reporting/reporting.h>
#include <clusters/TlsCertificateManagement/Commands.h>
#include <clusters/TlsCertificateManagement/Structs.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class TlsCertificateManagementDelegate;

class TlsCertificateManagementCluster : private AttributeAccessInterface,
                                        private CommandHandlerInterface,
                                        private FabricTable::Delegate
{
public:
    /**
     * Creates a TlsCertificateManagement server instance. The Init() function needs to be called for this instance to be registered
     * and called by the interaction model at the appropriate times.
     * @param endpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param delegate A reference to the delegate to be used by this server.
     * @param dependencyChecker A reference to a CertificateDependencyChecker which checks for transitive dependencies
     * @param certificateTable A reference to the certificate table for looking up certificates
     * @param maxRootCertificates The maximum number of root certificates which can be provisioned
     * @param maxClientCertificates The maximum number of client certificates which can be provisioned
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    TlsCertificateManagementCluster(EndpointId endpointId, TlsCertificateManagementDelegate & delegate,
                                    Tls::CertificateDependencyChecker & dependencyChecker, Tls::CertificateTable & certificateTable,
                                    uint8_t maxRootCertificates, uint8_t maxClientCertificates);
    ~TlsCertificateManagementCluster();

    /**
     * Initializes the TLS Certificate Management server instance.
     * @return Returns an error  if the CommandHandler or AttributeHandler registration fails.
     */
    CHIP_ERROR Init();

    /**
     * Shuts down the TLS Certificate Management server instance.
     * @return Returns an error if the destruction fails.
     */
    CHIP_ERROR Finish();

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
    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

    Tls::CertificateTable & GetCertificateTable() { return mCertificateTable; }

private:
    TlsCertificateManagementDelegate & mDelegate;
    Tls::CertificateDependencyChecker & mDependencyChecker;
    Tls::CertificateTable & mCertificateTable;

    // Attribute local storage
    uint8_t mMaxRootCertificates;
    uint8_t mMaxClientCertificates;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        ChipLogError(NotSpecified, "DataModel::ReadAttributeRequest overload should be called");
        chipAbort();
    }

    CHIP_ERROR Read(const DataModel::ReadAttributeRequest & aRequest, AttributeValueEncoder & aEncoder) override;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    void HandleProvisionRootCertificate(HandlerContext & ctx,
                                        const TlsCertificateManagement::Commands::ProvisionRootCertificate::DecodableType & req);
    void HandleFindRootCertificate(HandlerContext & ctx,
                                   const TlsCertificateManagement::Commands::FindRootCertificate::DecodableType & req);
    void HandleLookupRootCertificate(HandlerContext & ctx,
                                     const TlsCertificateManagement::Commands::LookupRootCertificate::DecodableType & req);
    void HandleRemoveRootCertificate(HandlerContext & ctx,
                                     const TlsCertificateManagement::Commands::RemoveRootCertificate::DecodableType & req);

    void HandleGenerateClientCsr(HandlerContext & ctx, const TlsCertificateManagement::Commands::ClientCSR::DecodableType & req);
    void
    HandleProvisionClientCertificate(HandlerContext & ctx,
                                     const TlsCertificateManagement::Commands::ProvisionClientCertificate::DecodableType & req);
    void HandleFindClientCertificate(HandlerContext & ctx,
                                     const TlsCertificateManagement::Commands::FindClientCertificate::DecodableType & req);
    void HandleLookupClientCertificate(HandlerContext & ctx,
                                       const TlsCertificateManagement::Commands::LookupClientCertificate::DecodableType & req);
    void HandleRemoveClientCertificate(HandlerContext & ctx,
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
class TlsCertificateManagementDelegate
{
public:
    using RootCertStructType             = TlsCertificateManagement::Structs::TLSCertStruct::Type;
    using ClientCertStructType           = TlsCertificateManagement::Structs::TLSClientCertificateDetailStruct::Type;
    using ProvisionRootCertificateType   = TlsCertificateManagement::Commands::ProvisionRootCertificate::DecodableType;
    using ProvisionClientCertificateType = TlsCertificateManagement::Commands::ProvisionClientCertificate::DecodableType;
    using ClientCsrType                  = TlsCertificateManagement::Commands::ClientCSR::DecodableType;
    using ClientCsrResponseType          = TlsCertificateManagement::Commands::ClientCSRResponse::Type;

    TlsCertificateManagementDelegate() = default;

    virtual ~TlsCertificateManagementDelegate() = default;

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

    Tls::CertificateTable & GetCertificateTable() { return mTlsCertificateManagementCluster->GetCertificateTable(); }

protected:
    friend class TlsCertificateManagementCluster;

    TlsCertificateManagementCluster * mTlsCertificateManagementCluster = nullptr;

    // sets the TlsCertificateManagement Cluster pointer
    void SetTlsCertificateManagementCluster(TlsCertificateManagementCluster * tlsCertificateManagementServer)
    {
        mTlsCertificateManagementCluster = tlsCertificateManagementServer;
    }
    TlsCertificateManagementCluster * GetTlsCertificateManagementCluster() const { return mTlsCertificateManagementCluster; }
};

} // namespace Clusters
} // namespace app
} // namespace chip
