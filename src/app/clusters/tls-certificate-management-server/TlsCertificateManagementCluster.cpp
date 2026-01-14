/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

/****************************************************************************'
 * @file
 * @brief Implementation for the TlsCertificateManagement Server Cluster
 ***************************************************************************/

#include "TlsCertificateManagementCluster.h"

#include <app/ConcreteAttributePath.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server/Server.h>
#include <clusters/TlsCertificateManagement/Attributes.h>
#include <clusters/TlsCertificateManagement/Commands.h>
#include <clusters/TlsCertificateManagement/Metadata.h>
#include <clusters/TlsCertificateManagement/Structs.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsCertificateManagement;
using namespace chip::app::Clusters::TlsCertificateManagement::Commands;
using namespace chip::app::Clusters::TlsCertificateManagement::Structs;
using namespace chip::app::Clusters::TlsCertificateManagement::Attributes;
using chip::Protocols::InteractionModel::Status;

static constexpr uint16_t kSpecMaxCertBytes            = 3000;
static constexpr uint16_t kSpecMaxFingerprintBytes     = 64;
static constexpr uint16_t kMaxIntermediateCertificates = 10;
static constexpr uint16_t kNonceBytes                  = 32;
static constexpr uint16_t kMaxSignatureBytes           = 128;

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    Commands::ProvisionRootCertificate::kMetadataEntry,
    Commands::FindRootCertificate::kMetadataEntry,
    Commands::LookupRootCertificate::kMetadataEntry,
    Commands::RemoveRootCertificate::kMetadataEntry,
    Commands::ClientCSR::kMetadataEntry,
    Commands::ProvisionClientCertificate::kMetadataEntry,
    Commands::FindClientCertificate::kMetadataEntry,
    Commands::LookupClientCertificate::kMetadataEntry,
    Commands::RemoveClientCertificate::kMetadataEntry,
};

TlsCertificateManagementCluster::TlsCertificateManagementCluster(EndpointId endpointId, TlsCertificateManagementDelegate & delegate,
                                                                 Tls::CertificateDependencyChecker & dependencyChecker,
                                                                 CertificateTable & certificateTable, uint8_t maxRootCertificates,
                                                                 uint8_t maxClientCertificates) :
    DefaultServerCluster(ConcreteClusterPath(endpointId, TlsCertificateManagement::Id)),
    mDelegate(delegate), mDependencyChecker(dependencyChecker), mCertificateTable(certificateTable),
    mMaxRootCertificates(maxRootCertificates), mMaxClientCertificates(maxClientCertificates)
{
    VerifyOrDieWithMsg(mMaxRootCertificates >= 5, NotSpecified, "Spec requires MaxRootCertificates be >= 5");
    VerifyOrDieWithMsg(mMaxClientCertificates >= 5, NotSpecified, "Spec requires MaxClientCertificates be >= 5");
    mDelegate.SetTlsCertificateManagementCluster(this);
}

TlsCertificateManagementCluster::~TlsCertificateManagementCluster()
{
    // null out the ref to us on the delegate
    mDelegate.SetTlsCertificateManagementCluster(nullptr);

    mCertificateTable.Finish();
    TEMPORARY_RETURN_IGNORED Server::GetInstance().GetFabricTable().RemoveFabricDelegate(this);
}

CHIP_ERROR TlsCertificateManagementCluster::Startup(ServerClusterContext & context)
{
    ChipLogProgress(DataManagement, "TlsCertificateManagementCluster: initializing");
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    ReturnErrorOnFailure(mCertificateTable.Init(Server::GetInstance().GetPersistentStorage()));

    return Server::GetInstance().GetFabricTable().AddFabricDelegate(this);
}

void TlsCertificateManagementCluster::Shutdown(ClusterShutdownType shutdownType)
{
    ChipLogProgress(DataManagement, "TlsCertificateManagementCluster: shutdown");

    mCertificateTable.Finish();
    Server::GetInstance().GetFabricTable().RemoveFabricDelegate(this);

    DefaultServerCluster::Shutdown(shutdownType);
}

// DefaultServerCluster override
DataModel::ActionReturnStatus TlsCertificateManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                             AttributeValueEncoder & encoder)
{
    VerifyOrDie(request.path.mClusterId == TlsCertificateManagement::Id);

    switch (request.path.mAttributeId)
    {
    case MaxRootCertificates::Id:
        return encoder.Encode(mMaxRootCertificates);
    case ProvisionedRootCertificates::Id: {
        TlsCertificateManagementCluster * server = this;
        auto matterEndpoint                      = request.path.mEndpointId;
        auto fabric                              = request.GetAccessingFabricIndex();
        return encoder.EncodeList([server, matterEndpoint, fabric](const auto & listEncoder) -> CHIP_ERROR {
            return server->EncodeProvisionedRootCertificates(matterEndpoint, fabric, false, listEncoder);
        });
    }
    case MaxClientCertificates::Id:
        return encoder.Encode(mMaxClientCertificates);
    case ProvisionedClientCertificates::Id: {
        TlsCertificateManagementCluster * server = this;
        auto matterEndpoint                      = request.path.mEndpointId;
        auto fabric                              = request.GetAccessingFabricIndex();
        return encoder.EncodeList([server, matterEndpoint, fabric](const auto & listEncoder) -> CHIP_ERROR {
            return server->EncodeProvisionedClientCertificates(matterEndpoint, fabric, false, listEncoder);
        });
    }
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR
TlsCertificateManagementCluster::EncodeProvisionedRootCertificates(EndpointId matterEndpoint, FabricIndex fabric, bool largePayload,
                                                                   const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    return mDelegate.LoadedRootCerts(matterEndpoint, fabric, [&](auto & cert) -> CHIP_ERROR { return encoder.Encode(cert); });
}

CHIP_ERROR
TlsCertificateManagementCluster::EncodeProvisionedClientCertificates(EndpointId matterEndpoint, FabricIndex fabric,
                                                                     bool largePayload,
                                                                     const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    return mDelegate.LoadedClientCerts(matterEndpoint, fabric, [&](auto & cert) -> CHIP_ERROR { return encoder.Encode(cert); });
}

std::optional<DataModel::ActionReturnStatus>
TlsCertificateManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                               CommandHandler * handler)
{
    VerifyOrDie(handler != nullptr);
    VerifyOrDie(request.path.mClusterId == TlsCertificateManagement::Id);

    FabricIndex accessingFabricIndex = handler->GetAccessingFabricIndex();

    switch (request.path.mCommandId)
    {
    case ProvisionRootCertificate::Id: {
        ProvisionRootCertificate::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleProvisionRootCertificate(*handler, req);
    }
    case FindRootCertificate::Id: {
        FindRootCertificate::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleFindRootCertificate(*handler, req);
    }
    case LookupRootCertificate::Id: {
        LookupRootCertificate::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleLookupRootCertificate(*handler, req);
    }
    case RemoveRootCertificate::Id: {
        RemoveRootCertificate::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleRemoveRootCertificate(*handler, req);
    }
    case ClientCSR::Id: {
        ClientCSR::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleGenerateClientCsr(*handler, req);
    }
    case ProvisionClientCertificate::Id: {
        ProvisionClientCertificate::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleProvisionClientCertificate(*handler, req);
    }
    case FindClientCertificate::Id: {
        FindClientCertificate::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleFindClientCertificate(*handler, req);
    }
    case LookupClientCertificate::Id: {
        LookupClientCertificate::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleLookupClientCertificate(*handler, req);
    }
    case RemoveClientCertificate::Id: {
        RemoveClientCertificate::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleRemoveClientCertificate(*handler, req);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

std::optional<DataModel::ActionReturnStatus>
TlsCertificateManagementCluster::HandleProvisionRootCertificate(CommandHandler & commandHandler,
                                                                const ProvisionRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: ProvisionRootCertificate");

    if (req.certificate.size() > kSpecMaxCertBytes)
    {
        return Status::ConstraintError;
    }

    auto fabric = commandHandler.GetAccessingFabricIndex();
    DataModel::Nullable<Tls::TLSCAID> foundId;
    auto lookupResult = mDelegate.LookupRootCert(mPath.mEndpointId, fabric, req.certificate, [&](auto & certificate) -> CHIP_ERROR {
        foundId = certificate.caid;
        return CHIP_NO_ERROR;
    });

    if (lookupResult != CHIP_ERROR_NOT_FOUND)
    {
        return Status::AlreadyExists;
    }

    if (req.caid.IsNull())
    {
        uint8_t numRootCerts;
        if (mCertificateTable.GetRootCertificateCount(fabric, numRootCerts) != CHIP_NO_ERROR)
        {
            return Status::Failure;
        }
        if (numRootCerts >= mMaxRootCertificates)
        {
            return Status::ResourceExhausted;
        }
    }
    else
    {
        auto caid = req.caid.Value();
        if (caid > kMaxRootCertId)
        {
            return Status::ConstraintError;
        }

        if (mCertificateTable.HasRootCertificateEntry(fabric, caid) != CHIP_NO_ERROR)
        {
            return Status::NotFound;
        }
    }

    if (Crypto::IsCertificateValidAtCurrentTime(req.certificate) != CHIP_NO_ERROR)
    {
        return Status::DynamicConstraintError;
    }

    ProvisionRootCertificateResponse::Type response;
    auto status = mDelegate.ProvisionRootCert(mPath.mEndpointId, fabric, req, response.caid);
    if (status != Status::Success)
    {
        return status;
    }

    VerifyOrDieWithMsg(response.caid <= kMaxRootCertId, NotSpecified, "Spec requires CAID to be < kMaxRootCertId");

    ConcreteCommandPath responsePath(mPath.mEndpointId, TlsCertificateManagement::Id, Commands::ProvisionRootCertificate::Id);
    commandHandler.AddResponse(responsePath, response);

    NotifyAttributeChanged(TlsCertificateManagement::Attributes::ProvisionedRootCertificates::Id);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
TlsCertificateManagementCluster::HandleFindRootCertificate(CommandHandler & commandHandler,
                                                           const FindRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: FindRootCertificate");
    CHIP_ERROR result;
    if (req.caid.IsNull())
    {
        result = mDelegate.RootCertsForFabric(mPath.mEndpointId, commandHandler.GetAccessingFabricIndex(),
                                              [&](auto & certs) -> CHIP_ERROR {
                                                  if (certs.size() == 0)
                                                  {
                                                      return CHIP_ERROR_NOT_FOUND;
                                                  }
                                                  FindRootCertificateResponse::Type response;
                                                  response.certificateDetails = certs;
                                                  ConcreteCommandPath responsePath(mPath.mEndpointId, TlsCertificateManagement::Id,
                                                                                   Commands::FindRootCertificate::Id);
                                                  commandHandler.AddResponse(responsePath, response);
                                                  return CHIP_NO_ERROR;
                                              });
    }
    else
    {
        if (req.caid.Value() > kMaxRootCertId)
        {
            return Status::ConstraintError;
        }

        result = mDelegate.FindRootCert(mPath.mEndpointId, commandHandler.GetAccessingFabricIndex(), req.caid.Value(),
                                        [&](auto & certificate) -> CHIP_ERROR {
                                            FindRootCertificateResponse::Type response;
                                            DataModel::List<const TLSCertStruct::Type> details(&certificate, 1);
                                            response.certificateDetails = details;
                                            ConcreteCommandPath responsePath(mPath.mEndpointId, TlsCertificateManagement::Id,
                                                                             Commands::FindRootCertificate::Id);
                                            commandHandler.AddResponse(responsePath, response);
                                            return CHIP_NO_ERROR;
                                        });
    }
    if (result == CHIP_ERROR_NOT_FOUND)
    {
        return Status::NotFound;
    }
    else if (result != CHIP_NO_ERROR)
    {
        return Status::Failure;
    }
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
TlsCertificateManagementCluster::HandleLookupRootCertificate(CommandHandler & commandHandler,
                                                             const LookupRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: LookupRootCertificate");
    if (req.fingerprint.size() > kSpecMaxFingerprintBytes)
    {
        return Status::ConstraintError;
    }

    auto result = mDelegate.LookupRootCertByFingerprint(
        mPath.mEndpointId, commandHandler.GetAccessingFabricIndex(), req.fingerprint, [&](auto & certificate) -> CHIP_ERROR {
            LookupRootCertificateResponse::Type response;
            response.caid = certificate.caid;
            ConcreteCommandPath responsePath(mPath.mEndpointId, TlsCertificateManagement::Id, Commands::LookupRootCertificate::Id);
            commandHandler.AddResponse(responsePath, response);
            return CHIP_NO_ERROR;
        });
    if (result == CHIP_ERROR_NOT_FOUND)
    {
        return Status::NotFound;
    }
    else if (result != CHIP_NO_ERROR)
    {
        return Status::Failure;
    }
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
TlsCertificateManagementCluster::HandleRemoveRootCertificate(CommandHandler & commandHandler,
                                                             const RemoveRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: RemoveRootCertificate");

    if (req.caid > kMaxRootCertId)
    {
        return Status::ConstraintError;
    }

    if (mDependencyChecker.RootCertCanBeRemoved(mPath.mEndpointId, commandHandler.GetAccessingFabricIndex(), req.caid) !=
        CHIP_NO_ERROR)
    {
        return Status::InvalidInState;
    }

    auto status = mDelegate.RemoveRootCert(mPath.mEndpointId, commandHandler.GetAccessingFabricIndex(), req.caid);

    if (status == Status::Success)
    {
        NotifyAttributeChanged(TlsCertificateManagement::Attributes::ProvisionedRootCertificates::Id);
    }

    return status;
}

std::optional<DataModel::ActionReturnStatus>
TlsCertificateManagementCluster::HandleGenerateClientCsr(CommandHandler & commandHandler, const ClientCSR::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: ClientCSR");

    if (req.nonce.size() != kNonceBytes)
    {
        return Status::ConstraintError;
    }

    auto fabric = commandHandler.GetAccessingFabricIndex();

    // If no CCDID is specified, ensure we have capacity for a new client certificate.
    if (req.ccdid.IsNull())
    {
        uint8_t numClientCerts;
        if (mCertificateTable.GetClientCertificateCount(fabric, numClientCerts) != CHIP_NO_ERROR)
        {
            return Status::Failure;
        }
        if (numClientCerts >= mMaxClientCertificates)
        {
            return Status::ResourceExhausted;
        }
    }

    auto status = mDelegate.GenerateClientCsr(mPath.mEndpointId, fabric, req, [&](auto & response) -> Status {
        VerifyOrDieWithMsg(response.ccdid <= kMaxClientCertId, NotSpecified, "Spec requires CCDID to be <= kMaxClientCertId");
        VerifyOrDieWithMsg(response.csr.size() <= 3000, NotSpecified, "Spec requires csr.size() to be <= 3000");
        VerifyOrDieWithMsg(response.nonceSignature.size() <= kMaxSignatureBytes, NotSpecified,
                           "Spec requires nonceSignature.size() to be < kMaxSignatureBytes");
        ConcreteCommandPath responsePath(mPath.mEndpointId, TlsCertificateManagement::Id, Commands::ClientCSR::Id);
        commandHandler.AddResponse(responsePath, response);
        return Status::Success;
    });

    return status;
}

std::optional<DataModel::ActionReturnStatus>
TlsCertificateManagementCluster::HandleProvisionClientCertificate(CommandHandler & commandHandler,
                                                                  const ProvisionClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: ProvisionClientCertificate");

    if (req.ccdid > kMaxClientCertId)
    {
        return Status::ConstraintError;
    }

    if (req.clientCertificate.size() > kSpecMaxCertBytes)
    {
        return Status::ConstraintError;
    }

    if (Crypto::IsCertificateValidAtCurrentTime(req.clientCertificate) != CHIP_NO_ERROR)
    {
        return Status::DynamicConstraintError;
    }

    size_t intermediateSize;
    if (req.intermediateCertificates.ComputeSize(&intermediateSize) != CHIP_NO_ERROR)
    {
        return Status::InvalidCommand;
    }

    if (intermediateSize > kMaxIntermediateCertificates)
    {
        return Status::ConstraintError;
    }

    auto srcIter = req.intermediateCertificates.begin();
    while (srcIter.Next())
    {
        auto & cert = srcIter.GetValue();
        if (cert.size() > kSpecMaxCertBytes)
        {
            return Status::ConstraintError;
        }

        if (Crypto::IsCertificateValidAtCurrentTime(cert) != CHIP_NO_ERROR)
        {
            return Status::DynamicConstraintError;
        }
    }

    if (srcIter.GetStatus() != CHIP_NO_ERROR)
    {
        return Status::InvalidAction;
    }

    auto fabric = commandHandler.GetAccessingFabricIndex();
    DataModel::Nullable<Tls::TLSCCDID> foundId;
    auto lookupResult =
        mDelegate.LookupClientCert(mPath.mEndpointId, fabric, req.clientCertificate, [&](auto & certificate) -> CHIP_ERROR {
            foundId = certificate.ccdid;
            return CHIP_NO_ERROR;
        });

    if (lookupResult != CHIP_ERROR_NOT_FOUND)
    {
        return Status::AlreadyExists;
    }

    if (mCertificateTable.HasClientCertificateEntry(fabric, req.ccdid) != CHIP_NO_ERROR)
    {
        return Status::NotFound;
    }

    auto status = mDelegate.ProvisionClientCert(mPath.mEndpointId, fabric, req);

    if (status == Status::Success)
    {
        NotifyAttributeChanged(TlsCertificateManagement::Attributes::ProvisionedClientCertificates::Id);
    }

    return status;
}

std::optional<DataModel::ActionReturnStatus>
TlsCertificateManagementCluster::HandleFindClientCertificate(CommandHandler & commandHandler,
                                                             const FindClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: FindClientCertificate");
    CHIP_ERROR result;
    if (req.ccdid.IsNull())
    {
        result = mDelegate.ClientCertsForFabric(
            mPath.mEndpointId, commandHandler.GetAccessingFabricIndex(), [&](auto & certs) -> CHIP_ERROR {
                if (certs.size() == 0)
                {
                    return CHIP_ERROR_NOT_FOUND;
                }
                FindClientCertificateResponse::Type response;
                response.certificateDetails = certs;
                ConcreteCommandPath responsePath(mPath.mEndpointId, TlsCertificateManagement::Id,
                                                 Commands::FindClientCertificate::Id);
                commandHandler.AddResponse(responsePath, response);
                return CHIP_NO_ERROR;
            });
    }
    else
    {
        if (req.ccdid.Value() > kMaxClientCertId)
        {
            return Status::ConstraintError;
        }

        result = mDelegate.FindClientCert(
            mPath.mEndpointId, commandHandler.GetAccessingFabricIndex(), req.ccdid.Value(), [&](auto & certificate) -> CHIP_ERROR {
                FindClientCertificateResponse::Type response;
                DataModel::List<const TLSClientCertificateDetailStruct::Type> details(&certificate, 1);
                response.certificateDetails = details;
                ConcreteCommandPath responsePath(mPath.mEndpointId, TlsCertificateManagement::Id,
                                                 Commands::FindClientCertificate::Id);
                commandHandler.AddResponse(responsePath, response);
                return CHIP_NO_ERROR;
            });
    }
    if (result == CHIP_ERROR_NOT_FOUND)
    {
        return Status::NotFound;
    }

    if (result != CHIP_NO_ERROR)
    {
        return Status::Failure;
    }

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
TlsCertificateManagementCluster::HandleLookupClientCertificate(CommandHandler & commandHandler,
                                                               const LookupClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: LookupClientCertificate");

    if (req.fingerprint.size() > kSpecMaxFingerprintBytes)
    {
        return Status::ConstraintError;
    }

    auto result = mDelegate.LookupClientCertByFingerprint(
        mPath.mEndpointId, commandHandler.GetAccessingFabricIndex(), req.fingerprint, [&](auto & certificate) -> CHIP_ERROR {
            LookupClientCertificateResponse::Type response;
            response.ccdid = certificate.ccdid;
            ConcreteCommandPath responsePath(mPath.mEndpointId, TlsCertificateManagement::Id,
                                             Commands::LookupClientCertificate::Id);
            commandHandler.AddResponse(responsePath, response);
            return CHIP_NO_ERROR;
        });
    if (result == CHIP_ERROR_NOT_FOUND)
    {
        return Status::NotFound;
    }

    if (result != CHIP_NO_ERROR)
    {
        return Status::Failure;
    }

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
TlsCertificateManagementCluster::HandleRemoveClientCertificate(CommandHandler & commandHandler,
                                                               const RemoveClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: RemoveClientCertificate");

    if (req.ccdid > kMaxClientCertId)
    {
        return Status::ConstraintError;
    }

    if (mDependencyChecker.ClientCertCanBeRemoved(mPath.mEndpointId, commandHandler.GetAccessingFabricIndex(), req.ccdid) !=
        CHIP_NO_ERROR)
    {
        return Status::InvalidInState;
    }

    auto status = mDelegate.RemoveClientCert(mPath.mEndpointId, commandHandler.GetAccessingFabricIndex(), req.ccdid);

    if (status == Status::Success)
    {
        NotifyAttributeChanged(TlsCertificateManagement::Attributes::ProvisionedClientCertificates::Id);
    }

    return status;
}

CHIP_ERROR TlsCertificateManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                             ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR TlsCertificateManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    // TlsCertificateManagement does not have optional attributes implemented yet,
    // so we just return mandatory ones.
    return listBuilder.Append(Span(kMandatoryMetadata), {});
}

void TlsCertificateManagementCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    ReturnAndLogOnFailure(mCertificateTable.RemoveFabric(fabricIndex), Zcl, "Failed to remove TLS certificate data for fabric 0x%x",
                          fabricIndex);
}
