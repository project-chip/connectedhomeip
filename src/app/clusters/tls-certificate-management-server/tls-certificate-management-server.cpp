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

#include "tls-certificate-management-server.h"

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

static constexpr uint16_t kSpecMaxCertBytes = 3000;

TlsCertificateManagementServer::TlsCertificateManagementServer(EndpointId endpointId, TlsCertificateManagementDelegate & delegate,
                                                               CertificateTable & certificateTable, uint8_t maxRootCertificates,
                                                               uint8_t maxClientCertificates) :
    AttributeAccessInterface(MakeOptional(endpointId), TlsCertificateManagement::Id),
    CommandHandlerInterface(MakeOptional(endpointId), TlsCertificateManagement::Id), mDelegate(delegate),
    mCertificateTable(certificateTable), mMaxRootCertificates(maxRootCertificates), mMaxClientCertificates(maxClientCertificates)
{
    VerifyOrDieWithMsg(mMaxRootCertificates >= 5, NotSpecified, "Spec requires MaxRootCertificates be >= 5");
    VerifyOrDieWithMsg(mMaxClientCertificates >= 5, NotSpecified, "Spec requires MaxClientCertificates be >= 5");
    mDelegate.SetTlsCertificateManagementServer(this);
}

TlsCertificateManagementServer::~TlsCertificateManagementServer()
{
    // null out the ref to us on the delegate
    mDelegate.SetTlsCertificateManagementServer(nullptr);

    // unregister
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR TlsCertificateManagementServer::Init()
{
    mCertificateTable.Init(Server::GetInstance().GetPersistentStorage());

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    return CHIP_NO_ERROR;
}

CHIP_ERROR TlsCertificateManagementServer::Finish()
{
    mCertificateTable.Finish();
    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR TlsCertificateManagementServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == TlsCertificateManagement::Id);

    switch (aPath.mAttributeId)
    {
    case MaxRootCertificates::Id:
        return aEncoder.Encode(mMaxRootCertificates);
    case ProvisionedRootCertificates::Id: {
        auto matterEndpoint = aPath.mEndpointId;
        auto fabric         = aEncoder.AccessingFabricIndex();
        return aEncoder.EncodeList([this, matterEndpoint, fabric](const auto & encoder) -> CHIP_ERROR {
            return EncodeProvisionedRootCertificates(matterEndpoint, fabric, encoder);
        });
    }
    case MaxClientCertificates::Id:
        return aEncoder.Encode(mMaxClientCertificates);
    case ProvisionedClientCertificates::Id: {
        auto matterEndpoint = aPath.mEndpointId;
        auto fabric         = aEncoder.AccessingFabricIndex();
        return aEncoder.EncodeList([this, matterEndpoint, fabric](const auto & encoder) -> CHIP_ERROR {
            return EncodeProvisionedClientCertificates(matterEndpoint, fabric, encoder);
        });
    }
    case ClusterRevision::Id:
        return aEncoder.Encode(kRevision);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
TlsCertificateManagementServer::EncodeProvisionedRootCertificates(EndpointId matterEndpoint, FabricIndex fabric,
                                                                  const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    return mDelegate.LoadedRootCerts(matterEndpoint, fabric,
                                     [&encoder](auto & cert) -> CHIP_ERROR { return encoder.Encode(cert); });
}

CHIP_ERROR
TlsCertificateManagementServer::EncodeProvisionedClientCertificates(EndpointId matterEndpoint, FabricIndex fabric,
                                                                    const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    return mDelegate.LoadedClientCerts(matterEndpoint, fabric, [&](auto & cert) -> CHIP_ERROR { return encoder.Encode(cert); });
}

CHIP_ERROR TlsCertificateManagementServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == TlsCertificateManagement::Id);

    switch (aPath.mAttributeId)
    {
    default:
        // Unknown attribute
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

void TlsCertificateManagementServer::InvokeCommand(HandlerContext & ctx)
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
    case TLSClientCSR::Id:
        CommandHandlerInterface::HandleCommand<TLSClientCSR::DecodableType>(
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

void TlsCertificateManagementServer::HandleProvisionRootCertificate(HandlerContext & ctx,
                                                                    const ProvisionRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: ProvisionRootCertificate");

    VerifyOrReturn(req.certificate.size() <= kSpecMaxCertBytes,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));
    VerifyOrReturn(req.caid.IsNull() || req.caid.Value() <= kMaxRootCertId,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    ProvisionRootCertificateResponse::Type response;
    auto status = mDelegate.ProvisionRootCert(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), req,
                                              response.caid);

    if (status.IsSuccess())
    {
        VerifyOrDieWithMsg(response.caid <= kMaxRootCertId, NotSpecified, "Spec requires CAID to be < kMaxRootCertId");
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    }
}

void TlsCertificateManagementServer::HandleFindRootCertificate(HandlerContext & ctx, const FindRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: FindRootCertificate");
}

void TlsCertificateManagementServer::HandleLookupRootCertificate(HandlerContext & ctx,
                                                                 const LookupRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: LookupRootCertificate");
}

void TlsCertificateManagementServer::HandleRemoveRootCertificate(HandlerContext & ctx,
                                                                 const RemoveRootCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: RemoveRootCertificate");
}

void TlsCertificateManagementServer::HandleGenerateClientCsr(HandlerContext & ctx, const TLSClientCSR::DecodableType & req)
{

    ChipLogDetail(Zcl, "TlsCertificateManagement: TLSClientCSR");

    VerifyOrReturn(req.nonce.size() <= 128, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    auto status = mDelegate.GenerateClientCsr(
        ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), req, [&](auto & response) -> Status {
            VerifyOrDieWithMsg(response.ccdid <= kMaxClientCertId, NotSpecified, "Spec requires CCDID to be <= kMaxClientCertId");
            VerifyOrDieWithMsg(response.csr.size() <= 3000, NotSpecified, "Spec requires csr.size() to be <= 3000");
            VerifyOrDieWithMsg(response.nonce.size() <= 128, NotSpecified, "Spec requires nonce.size() to be < kMaxClientCertId");
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
            return Status::Success;
        });

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    }
}

void TlsCertificateManagementServer::HandleProvisionClientCertificate(HandlerContext & ctx,
                                                                      const ProvisionClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: ProvisionClientCertificate");

    VerifyOrReturn(req.ccdid <= kMaxClientCertId, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));
    const auto & detail = req.clientCertificateDetails;
    VerifyOrReturn(!detail.clientCertificate.HasValue() || detail.clientCertificate.Value().size() <= kSpecMaxCertBytes,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    auto status = mDelegate.ProvisionClientCert(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), req);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void TlsCertificateManagementServer::HandleFindClientCertificate(HandlerContext & ctx,
                                                                 const FindClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: FindClientCertificate");
}

void TlsCertificateManagementServer::HandleLookupClientCertificate(HandlerContext & ctx,
                                                                   const LookupClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: LookupClientCertificate");
}

void TlsCertificateManagementServer::HandleRemoveClientCertificate(HandlerContext & ctx,
                                                                   const RemoveClientCertificate::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsCertificateManagement: RemoveRootCertificate");
}

/** @brief TlsCertificateManagement Cluster Server Init
 *
 * Server Init
 *
 */
void MatterTlsCertificateManagementPluginServerInitCallback() {}
