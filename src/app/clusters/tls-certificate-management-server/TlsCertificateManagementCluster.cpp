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

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/SafeAttributePersistenceProvider.h>
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

TlsCertificateManagementCluster::TlsCertificateManagementCluster(EndpointId endpointId, TlsCertificateManagementDelegate & delegate,
                                                                 Tls::CertificateDependencyChecker & dependencyChecker,
                                                                 CertificateTable & certificateTable, uint8_t maxRootCertificates,
                                                                 uint8_t maxClientCertificates) :
    AttributeAccessInterface(MakeOptional(endpointId), TlsCertificateManagement::Id),
    CommandHandlerInterface(MakeOptional(endpointId), TlsCertificateManagement::Id), mDelegate(delegate),
    mDependencyChecker(dependencyChecker), mCertificateTable(certificateTable), mMaxRootCertificates(maxRootCertificates),
    mMaxClientCertificates(maxClientCertificates)
{
    VerifyOrDieWithMsg(mMaxRootCertificates >= 5, NotSpecified, "Spec requires MaxRootCertificates be >= 5");
    VerifyOrDieWithMsg(mMaxClientCertificates >= 5, NotSpecified, "Spec requires MaxClientCertificates be >= 5");
    mDelegate.SetTlsCertificateManagementCluster(this);
}

TlsCertificateManagementCluster::~TlsCertificateManagementCluster()
{
    // null out the ref to us on the delegate
    mDelegate.SetTlsCertificateManagementCluster(nullptr);

    // unregister
    TEMPORARY_RETURN_IGNORED CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR TlsCertificateManagementCluster::Init()
{
    ReturnErrorOnFailure(mCertificateTable.Init(Server::GetInstance().GetPersistentStorage()));

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    return Server::GetInstance().GetFabricTable().AddFabricDelegate(this);
}

CHIP_ERROR TlsCertificateManagementCluster::Finish()
{
    mCertificateTable.Finish();

    TEMPORARY_RETURN_IGNORED Server::GetInstance().GetFabricTable().RemoveFabricDelegate(this);
    TEMPORARY_RETURN_IGNORED CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);

    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR TlsCertificateManagementCluster::Read(const DataModel::ReadAttributeRequest & aRequest, AttributeValueEncoder & aEncoder)
{
    const auto & aPath = aRequest.path;
    bool largePayload  = aRequest.readFlags.Has(DataModel::ReadFlags::kAllowsLargePayload);
    VerifyOrDie(aPath.mClusterId == TlsCertificateManagement::Id);

    switch (aPath.mAttributeId)
    {
    case MaxRootCertificates::Id:
        return aEncoder.Encode(mMaxRootCertificates);
    case ProvisionedRootCertificates::Id: {
        auto matterEndpoint = aPath.mEndpointId;
        auto fabric         = aEncoder.AccessingFabricIndex();
        return aEncoder.EncodeList([this, matterEndpoint, fabric, largePayload](const auto & encoder) -> CHIP_ERROR {
            return EncodeProvisionedRootCertificates(matterEndpoint, fabric, largePayload, encoder);
        });
    }
    case MaxClientCertificates::Id:
        return aEncoder.Encode(mMaxClientCertificates);
    case ProvisionedClientCertificates::Id: {
        auto matterEndpoint = aPath.mEndpointId;
        auto fabric         = aEncoder.AccessingFabricIndex();
        return aEncoder.EncodeList([this, matterEndpoint, fabric, largePayload](const auto & encoder) -> CHIP_ERROR {
            return EncodeProvisionedClientCertificates(matterEndpoint, fabric, largePayload, encoder);
        });
    }
    case ClusterRevision::Id:
        return aEncoder.Encode(kRevision);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
TlsCertificateManagementCluster::EncodeProvisionedRootCertificates(EndpointId matterEndpoint, FabricIndex fabric, bool largePayload,
                                                                   const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    return mDelegate.LoadedRootCerts(matterEndpoint, fabric, [&](auto & cert) -> CHIP_ERROR {
        if (largePayload)
        {
            return encoder.Encode(cert);
        }

        // Drop the certificate payload if transport doesn't support large payload
        TLSCertStruct::Type idOnlyCert;
        idOnlyCert.fabricIndex = cert.fabricIndex;
        idOnlyCert.caid        = cert.caid;
        return encoder.Encode(idOnlyCert);
    });
}

CHIP_ERROR
TlsCertificateManagementCluster::EncodeProvisionedClientCertificates(EndpointId matterEndpoint, FabricIndex fabric,
                                                                     bool largePayload,
                                                                     const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    return mDelegate.LoadedClientCerts(matterEndpoint, fabric, [&](auto & cert) -> CHIP_ERROR {
        if (largePayload)
        {
            return encoder.Encode(cert);
        }

        // Drop the certificate payload if transport doesn't support large payload
        TLSClientCertificateDetailStruct::Type idOnlyCert;
        idOnlyCert.fabricIndex = cert.fabricIndex;
        idOnlyCert.ccdid       = cert.ccdid;
        return encoder.Encode(idOnlyCert);
    });
}

void TlsCertificateManagementCluster::InvokeCommand(HandlerContext & ctx)
{
    switch (ctx.mRequestPath.mCommandId)
    {
    case ProvisionRootCertificate::Id:
        CommandHandlerInterface::HandleCommand<ProvisionRootCertificate::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleProvisionRootCertificate(innerCtx, req); });
        break;
    case FindRootCertificate::Id:
        CommandHandlerInterface::HandleCommand<FindRootCertificate::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleFindRootCertificate(innerCtx, req); });
        break;
    case LookupRootCertificate::Id:
        CommandHandlerInterface::HandleCommand<LookupRootCertificate::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleLookupRootCertificate(innerCtx, req); });
        break;
    case RemoveRootCertificate::Id:
        CommandHandlerInterface::HandleCommand<RemoveRootCertificate::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleRemoveRootCertificate(innerCtx, req); });
        break;
    case ClientCSR::Id:
        CommandHandlerInterface::HandleCommand<ClientCSR::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleGenerateClientCsr(innerCtx, req); });
        break;
    case ProvisionClientCertificate::Id:
        CommandHandlerInterface::HandleCommand<ProvisionClientCertificate::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleProvisionClientCertificate(innerCtx, req); });
        break;
    case FindClientCertificate::Id:
        CommandHandlerInterface::HandleCommand<FindClientCertificate::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleFindClientCertificate(innerCtx, req); });
        break;
    case LookupClientCertificate::Id:
        CommandHandlerInterface::HandleCommand<LookupClientCertificate::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleLookupClientCertificate(innerCtx, req); });
        break;
    case RemoveClientCertificate::Id:
        CommandHandlerInterface::HandleCommand<RemoveClientCertificate::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleRemoveClientCertificate(innerCtx, req); });
        break;
    }
}

void TlsCertificateManagementCluster::HandleProvisionRootCertificate(HandlerContext & ctx,
                                                                     const ProvisionRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: ProvisionRootCertificate");

    VerifyOrReturn(req.certificate.size() <= kSpecMaxCertBytes,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));
    auto fabric = ctx.mCommandHandler.GetAccessingFabricIndex();
    DataModel::Nullable<Tls::TLSCAID> foundId;
    auto lookupResult =
        mDelegate.LookupRootCert(ctx.mRequestPath.mEndpointId, fabric, req.certificate, [&](auto & certificate) -> CHIP_ERROR {
            foundId = certificate.caid;
            return CHIP_NO_ERROR;
        });
    if (lookupResult != CHIP_ERROR_NOT_FOUND)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::AlreadyExists);
        return;
    }

    if (req.caid.IsNull())
    {
        uint8_t numRootCerts;
        ReturnOnFailure(mCertificateTable.GetRootCertificateCount(fabric, numRootCerts),
                        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure));
        VerifyOrReturn(numRootCerts < mMaxRootCertificates,
                       ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted));
    }
    else
    {
        auto caid = req.caid.Value();
        VerifyOrReturn(caid <= kMaxRootCertId, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

        ReturnOnFailure(mCertificateTable.HasRootCertificateEntry(fabric, caid),
                        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound));
    }

    ReturnOnFailure(Crypto::IsCertificateValidAtCurrentTime(req.certificate),
                    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError));

    ProvisionRootCertificateResponse::Type response;
    auto status = mDelegate.ProvisionRootCert(ctx.mRequestPath.mEndpointId, fabric, req, response.caid);
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    VerifyOrDieWithMsg(response.caid <= kMaxRootCertId, NotSpecified, "Spec requires CAID to be < kMaxRootCertId");

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);

    MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, TlsCertificateManagement::Id,
                                           TlsCertificateManagement::Attributes::ProvisionedRootCertificates::Id);
}

void TlsCertificateManagementCluster::HandleFindRootCertificate(HandlerContext & ctx,
                                                                const FindRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: FindRootCertificate");
    CHIP_ERROR result;
    if (req.caid.IsNull())
    {
        result = mDelegate.RootCertsForFabric(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                              [&](auto & certs) -> CHIP_ERROR {
                                                  if (certs.size() == 0)
                                                  {
                                                      return CHIP_ERROR_NOT_FOUND;
                                                  }
                                                  FindRootCertificateResponse::Type response;
                                                  response.certificateDetails = certs;
                                                  ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
                                                  return CHIP_NO_ERROR;
                                              });
    }
    else
    {
        VerifyOrReturn(req.caid.Value() <= kMaxRootCertId,
                       ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

        result = mDelegate.FindRootCert(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                        req.caid.Value(), [&](auto & certificate) -> CHIP_ERROR {
                                            FindRootCertificateResponse::Type response;
                                            DataModel::List<const TLSCertStruct::Type> details(&certificate, 1);
                                            response.certificateDetails = details;
                                            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
                                            return CHIP_NO_ERROR;
                                        });
    }
    if (result == CHIP_ERROR_NOT_FOUND)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
    }
    else if (result != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
    }
}

void TlsCertificateManagementCluster::HandleLookupRootCertificate(HandlerContext & ctx,
                                                                  const LookupRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: LookupRootCertificate");
    VerifyOrReturn(req.fingerprint.size() <= kSpecMaxFingerprintBytes,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    auto result = mDelegate.LookupRootCertByFingerprint(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                        req.fingerprint, [&](auto & certificate) -> CHIP_ERROR {
                                                            LookupRootCertificateResponse::Type response;
                                                            response.caid = certificate.caid;
                                                            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
                                                            return CHIP_NO_ERROR;
                                                        });
    if (result == CHIP_ERROR_NOT_FOUND)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
    }
    else if (result != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
    }
}

void TlsCertificateManagementCluster::HandleRemoveRootCertificate(HandlerContext & ctx,
                                                                  const RemoveRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: RemoveRootCertificate");

    VerifyOrReturn(req.caid <= kMaxRootCertId, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    ReturnOnFailure(mDependencyChecker.RootCertCanBeRemoved(ctx.mRequestPath.mEndpointId,
                                                            ctx.mCommandHandler.GetAccessingFabricIndex(), req.caid),
                    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState));

    auto result = mDelegate.RemoveRootCert(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), req.caid);

    if (result == Status::Success)
    {
        MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, TlsCertificateManagement::Id,
                                               TlsCertificateManagement::Attributes::ProvisionedRootCertificates::Id);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, result);
}

void TlsCertificateManagementCluster::HandleGenerateClientCsr(HandlerContext & ctx, const ClientCSR::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: ClientCSR");

    VerifyOrReturn(req.nonce.size() == kNonceBytes, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    auto fabric = ctx.mCommandHandler.GetAccessingFabricIndex();

    // If no CCDID is specified, ensure we have capacity for a new client certificate.
    if (req.ccdid.IsNull())
    {
        uint8_t numClientCerts;
        VerifyOrReturn(mCertificateTable.GetClientCertificateCount(fabric, numClientCerts) == CHIP_NO_ERROR,
                       ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure));
        VerifyOrReturn(numClientCerts < mMaxClientCertificates,
                       ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted));
    }

    auto status = mDelegate.GenerateClientCsr(ctx.mRequestPath.mEndpointId, fabric, req, [&](auto & response) -> Status {
        VerifyOrDieWithMsg(response.ccdid <= kMaxClientCertId, NotSpecified, "Spec requires CCDID to be <= kMaxClientCertId");
        VerifyOrDieWithMsg(response.csr.size() <= 3000, NotSpecified, "Spec requires csr.size() to be <= 3000");
        VerifyOrDieWithMsg(response.nonceSignature.size() <= kMaxSignatureBytes, NotSpecified,
                           "Spec requires nonceSignature.size() to be < kMaxSignatureBytes");
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return Status::Success;
    });

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    }
}

void TlsCertificateManagementCluster::HandleProvisionClientCertificate(HandlerContext & ctx,
                                                                       const ProvisionClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: ProvisionClientCertificate");

    VerifyOrReturn(req.ccdid <= kMaxClientCertId, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));
    VerifyOrReturn(req.clientCertificate.size() <= kSpecMaxCertBytes,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));
    ReturnOnFailure(Crypto::IsCertificateValidAtCurrentTime(req.clientCertificate),
                    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError));
    size_t intermediateSize;
    ReturnOnFailure(req.intermediateCertificates.ComputeSize(&intermediateSize),
                    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand));
    VerifyOrReturn(intermediateSize <= kMaxIntermediateCertificates,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));
    auto srcIter = req.intermediateCertificates.begin();
    while (srcIter.Next())
    {
        auto & cert = srcIter.GetValue();
        VerifyOrReturn(cert.size() <= kSpecMaxCertBytes, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

        ReturnOnFailure(Crypto::IsCertificateValidAtCurrentTime(cert),
                        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError));
    }
    ReturnOnFailure(srcIter.GetStatus(), ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidAction));

    auto fabric = ctx.mCommandHandler.GetAccessingFabricIndex();
    DataModel::Nullable<Tls::TLSCCDID> foundId;
    auto lookupResult = mDelegate.LookupClientCert(ctx.mRequestPath.mEndpointId, fabric, req.clientCertificate,
                                                   [&](auto & certificate) -> CHIP_ERROR {
                                                       foundId = certificate.ccdid;
                                                       return CHIP_NO_ERROR;
                                                   });
    if (lookupResult != CHIP_ERROR_NOT_FOUND)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::AlreadyExists);
        return;
    }

    ReturnOnFailure(mCertificateTable.HasClientCertificateEntry(fabric, req.ccdid),
                    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound));

    auto status = mDelegate.ProvisionClientCert(ctx.mRequestPath.mEndpointId, fabric, req);

    if (status == Status::Success)
    {
        MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, TlsCertificateManagement::Id,
                                               TlsCertificateManagement::Attributes::ProvisionedClientCertificates::Id);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void TlsCertificateManagementCluster::HandleFindClientCertificate(HandlerContext & ctx,
                                                                  const FindClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: FindClientCertificate");
    CHIP_ERROR result;
    if (req.ccdid.IsNull())
    {
        result = mDelegate.ClientCertsForFabric(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                [&](auto & certs) -> CHIP_ERROR {
                                                    if (certs.size() == 0)
                                                    {
                                                        return CHIP_ERROR_NOT_FOUND;
                                                    }
                                                    FindClientCertificateResponse::Type response;
                                                    response.certificateDetails = certs;
                                                    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
                                                    return CHIP_NO_ERROR;
                                                });
    }
    else
    {
        VerifyOrReturn(req.ccdid.Value() <= kMaxRootCertId,
                       ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

        result =
            mDelegate.FindClientCert(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), req.ccdid.Value(),
                                     [&](auto & certificate) -> CHIP_ERROR {
                                         FindClientCertificateResponse::Type response;
                                         DataModel::List<const TLSClientCertificateDetailStruct::Type> details(&certificate, 1);
                                         response.certificateDetails = details;
                                         ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
                                         return CHIP_NO_ERROR;
                                     });
    }
    if (result == CHIP_ERROR_NOT_FOUND)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
    }
    else if (result != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
    }
}

void TlsCertificateManagementCluster::HandleLookupClientCertificate(HandlerContext & ctx,
                                                                    const LookupClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: LookupClientCertificate");

    VerifyOrReturn(req.fingerprint.size() <= kSpecMaxFingerprintBytes,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    auto result =
        mDelegate.LookupClientCertByFingerprint(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                req.fingerprint, [&](auto & certificate) -> CHIP_ERROR {
                                                    LookupClientCertificateResponse::Type response;
                                                    response.ccdid = certificate.ccdid;
                                                    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
                                                    return CHIP_NO_ERROR;
                                                });
    if (result == CHIP_ERROR_NOT_FOUND)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
    }
    else if (result != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
    }
}

void TlsCertificateManagementCluster::HandleRemoveClientCertificate(HandlerContext & ctx,
                                                                    const RemoveClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: RemoveClientCertificate");

    VerifyOrReturn(req.ccdid <= kMaxClientCertId, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    ReturnOnFailure(mDependencyChecker.ClientCertCanBeRemoved(ctx.mRequestPath.mEndpointId,
                                                              ctx.mCommandHandler.GetAccessingFabricIndex(), req.ccdid),
                    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState));

    auto result =
        mDelegate.RemoveClientCert(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), req.ccdid);

    if (result == Status::Success)
    {
        MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, TlsCertificateManagement::Id,
                                               TlsCertificateManagement::Attributes::ProvisionedClientCertificates::Id);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, result);
}

void TlsCertificateManagementCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    ReturnAndLogOnFailure(mCertificateTable.RemoveFabric(fabricIndex), Zcl, "Failed to remove TLS certificate data for fabric 0x%x",
                          fabricIndex);
}

/** @brief TlsCertificateManagement Cluster Server Init
 *
 * Server Init
 *
 */
void MatterTlsCertificateManagementPluginServerInitCallback() {}
