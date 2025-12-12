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
 * @brief Implementation for the TlsClientManagement Server Cluster
 ***************************************************************************/

#include "TlsClientManagementCluster.h"

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/server/Server.h>
#include <clusters/TlsClientManagement/Attributes.h>
#include <clusters/TlsClientManagement/Commands.h>
#include <clusters/TlsClientManagement/Metadata.h>
#include <clusters/TlsClientManagement/Structs.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsClientManagement;
using namespace chip::app::Clusters::TlsClientManagement::Structs;
using namespace chip::app::Clusters::TlsClientManagement::Attributes;
using namespace Protocols::InteractionModel;

static constexpr uint16_t kMaxTlsEndpointId = 65534;

TlsClientManagementCluster::TlsClientManagementCluster(EndpointId endpointId, TlsClientManagementDelegate & delegate,
                                                       CertificateTable & certificateTable, uint8_t maxProvisioned) :
    AttributeAccessInterface(MakeOptional(endpointId), TlsClientManagement::Id),
    CommandHandlerInterface(MakeOptional(endpointId), TlsClientManagement::Id), mDelegate(delegate),
    mCertificateTable(certificateTable), mMaxProvisioned(maxProvisioned)
{
    VerifyOrDieWithMsg(mMaxProvisioned >= 5, NotSpecified, "Spec requires MaxProvisioned be >= 5");
    VerifyOrDieWithMsg(mMaxProvisioned <= 254, NotSpecified, "Spec requires MaxProvisioned be <= 254");
    mDelegate.SetTlsClientManagementCluster(this);
}

TlsClientManagementCluster::~TlsClientManagementCluster()
{
    // null out the ref to us on the delegate
    mDelegate.SetTlsClientManagementCluster(nullptr);

    // unregister
    TEMPORARY_RETURN_IGNORED CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR TlsClientManagementCluster::Init()
{
    ReturnErrorOnFailure(mCertificateTable.Init(Server::GetInstance().GetPersistentStorage()));
    ReturnErrorOnFailure(mDelegate.Init(Server::GetInstance().GetPersistentStorage()));

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    return Server::GetInstance().GetFabricTable().AddFabricDelegate(this);
}

CHIP_ERROR TlsClientManagementCluster::Finish()
{
    TEMPORARY_RETURN_IGNORED mCertificateTable.Finish();

    TEMPORARY_RETURN_IGNORED Server::GetInstance().GetFabricTable().RemoveFabricDelegate(this);
    TEMPORARY_RETURN_IGNORED CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);

    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR TlsClientManagementCluster::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == TlsClientManagement::Id);

    switch (aPath.mAttributeId)
    {
    case MaxProvisioned::Id:
        return aEncoder.Encode(mMaxProvisioned);
    case ProvisionedEndpoints::Id: {
        TlsClientManagementCluster * server = this;
        auto matterEndpoint                 = aPath.mEndpointId;
        auto fabric                         = aEncoder.AccessingFabricIndex();
        CHIP_ERROR err = aEncoder.EncodeList([server, matterEndpoint, fabric](const auto & encoder) -> CHIP_ERROR {
            return server->EncodeProvisionedEndpoints(matterEndpoint, fabric, encoder);
        });
        return err;
    }
    case ClusterRevision::Id:
        return aEncoder.Encode(kRevision);
    }

    return CHIP_NO_ERROR;
}

uint8_t TlsClientManagementCluster::GetMaxProvisioned() const
{
    return mMaxProvisioned;
}

// helper method to get the TlsClientManagement provisioned endpoints encoded into a list
CHIP_ERROR
TlsClientManagementCluster::EncodeProvisionedEndpoints(EndpointId matterEndpoint, FabricIndex fabric,
                                                       const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    return mDelegate.ForEachEndpoint(matterEndpoint, fabric,
                                     [&](auto & endpoint) -> CHIP_ERROR { return encoder.Encode(endpoint); });
}

void TlsClientManagementCluster::InvokeCommand(HandlerContext & ctx)
{
    switch (ctx.mRequestPath.mCommandId)
    {
    case Commands::ProvisionEndpoint::Id:
        CommandHandlerInterface::HandleCommand<Commands::ProvisionEndpoint::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleProvisionEndpoint(innerCtx, req); });
        break;
    case Commands::FindEndpoint::Id:
        CommandHandlerInterface::HandleCommand<Commands::FindEndpoint::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleFindEndpoint(innerCtx, req); });
        break;
    case Commands::RemoveEndpoint::Id:
        CommandHandlerInterface::HandleCommand<Commands::RemoveEndpoint::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandleRemoveEndpoint(innerCtx, req); });
        break;
    }
}

void TlsClientManagementCluster::HandleProvisionEndpoint(HandlerContext & ctx,
                                                         const Commands::ProvisionEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: ProvisionEndpoint");

    VerifyOrReturn(req.hostname.size() >= 4 && req.hostname.size() <= kSpecMaxHostname,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));
    VerifyOrReturn(req.caid <= kMaxRootCertId, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    auto fabric = ctx.mCommandHandler.GetAccessingFabricIndex();

    ReturnOnFailure(mCertificateTable.HasRootCertificateEntry(fabric, req.caid),
                    ctx.mCommandHandler.AddStatus(
                        ctx.mRequestPath, ClusterStatusCode::ClusterSpecificFailure(StatusCodeEnum::kRootCertificateNotFound)));
    VerifyOrReturn(req.ccdid.IsNull() || mCertificateTable.HasClientCertificateEntry(fabric, req.ccdid.Value()) == CHIP_NO_ERROR,
                   ctx.mCommandHandler.AddStatus(
                       ctx.mRequestPath, ClusterStatusCode::ClusterSpecificFailure(StatusCodeEnum::kClientCertificateNotFound)));

    Commands::ProvisionEndpointResponse::Type response;
    auto status = mDelegate.ProvisionEndpoint(ctx.mRequestPath.mEndpointId, fabric, req, response.endpointID);

    if (status.IsSuccess())
    {
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);

        MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, TlsClientManagement::Id,
                                               TlsClientManagement::Attributes::ProvisionedEndpoints::Id);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    }
}

void TlsClientManagementCluster::HandleFindEndpoint(HandlerContext & ctx, const Commands::FindEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: FindEndpoint");

    VerifyOrReturn(req.endpointID <= kMaxTlsEndpointId, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    CHIP_ERROR result =
        mDelegate.FindProvisionedEndpointByID(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                              req.endpointID, [&](auto & endpoint) -> CHIP_ERROR {
                                                  Commands::FindEndpointResponse::Type response;
                                                  response.endpoint = endpoint;
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

void TlsClientManagementCluster::HandleRemoveEndpoint(HandlerContext & ctx, const Commands::RemoveEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: RemoveEndpoint");

    VerifyOrReturn(req.endpointID <= kMaxTlsEndpointId, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    auto status = mDelegate.RemoveProvisionedEndpointByID(ctx.mRequestPath.mEndpointId,
                                                          ctx.mCommandHandler.GetAccessingFabricIndex(), req.endpointID);

    if (status == Status::Success)
    {
        MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, TlsClientManagement::Id,
                                               TlsClientManagement::Attributes::ProvisionedEndpoints::Id);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void TlsClientManagementCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    TEMPORARY_RETURN_IGNORED mDelegate.RemoveFabric(fabricIndex);
}

/** @brief TlsClientManagement Cluster Server Init
 *
 * Server Init
 *
 */
void MatterTlsClientManagementPluginServerInitCallback() {}
