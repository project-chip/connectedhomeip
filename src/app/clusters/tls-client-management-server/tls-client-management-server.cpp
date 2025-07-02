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

#include "tls-client-management-server.h"

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
using chip::Protocols::InteractionModel::Status;

static constexpr uint16_t kSpecMaxHostname = 253;

TlsClientManagementServer::TlsClientManagementServer(EndpointId endpointId, TlsClientManagementDelegate & delegate,
                                                     CertificateTable & certificateTable, uint8_t maxProvisioned) :
    AttributeAccessInterface(MakeOptional(endpointId), TlsClientManagement::Id),
    CommandHandlerInterface(MakeOptional(endpointId), TlsClientManagement::Id), mDelegate(delegate),
    mCertificateTable(certificateTable), mMaxProvisioned(maxProvisioned)
{
    VerifyOrDieWithMsg(mMaxProvisioned >= 5, NotSpecified, "Spec requires MaxProvisioned be >= 5");
    VerifyOrDieWithMsg(mMaxProvisioned <= 254, NotSpecified, "Spec requires MaxProvisioned be <= 254");
    mDelegate.SetTlsClientManagementServer(this);
}

TlsClientManagementServer::~TlsClientManagementServer()
{
    // null out the ref to us on the delegate
    mDelegate.SetTlsClientManagementServer(nullptr);

    // unregister
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR TlsClientManagementServer::Init()
{
    mCertificateTable.Init(Server::GetInstance().GetPersistentStorage());

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    return CHIP_NO_ERROR;
}

CHIP_ERROR TlsClientManagementServer::Finish()
{
    mCertificateTable.Finish();
    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR TlsClientManagementServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == TlsClientManagement::Id);

    switch (aPath.mAttributeId)
    {
    case MaxProvisioned::Id:
        return aEncoder.Encode(mMaxProvisioned);
    case ProvisionedEndpoints::Id: {
        TlsClientManagementServer * server = this;
        auto matterEndpoint                = aPath.mEndpointId;
        auto fabric                        = aEncoder.AccessingFabricIndex();
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

uint8_t TlsClientManagementServer::GetMaxProvisioned() const
{
    return mMaxProvisioned;
}

// helper method to get the TlsClientManagement provisioned endpoints encoded into a list
CHIP_ERROR
TlsClientManagementServer::EncodeProvisionedEndpoints(EndpointId matterEndpoint, FabricIndex fabric,
                                                      const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (uint8_t i = 0; true; i++)
    {
        TlsClientManagementDelegate::EndpointStructType endpoint;

        auto err = mDelegate.GetProvisionedEndpointByIndex(matterEndpoint, fabric, i, endpoint);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(err);

        ReturnErrorOnFailure(encoder.Encode(endpoint));
    }
    return CHIP_NO_ERROR;
}

void TlsClientManagementServer::InvokeCommand(HandlerContext & ctx)
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

void TlsClientManagementServer::HandleProvisionEndpoint(HandlerContext & ctx,
                                                        const Commands::ProvisionEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: ProvisionEndpoint");

    VerifyOrReturn(req.hostname.size() >= 4 && req.hostname.size() <= kSpecMaxHostname,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    Commands::ProvisionEndpointResponse::Type response;
    auto status = mDelegate.ProvisionEndpoint(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), req,
                                              response.endpointID);

    if (status.IsSuccess())
    {
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    }
}

void TlsClientManagementServer::HandleFindEndpoint(HandlerContext & ctx, const Commands::FindEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: FindEndpoint");

    Commands::FindEndpointResponse::Type response;
    TlsClientManagementDelegate::EndpointStructType endpoint;
    Status status     = mDelegate.FindProvisionedEndpointByID(ctx.mRequestPath.mEndpointId,
                                                              ctx.mCommandHandler.GetAccessingFabricIndex(), req.endpointID, endpoint);
    response.endpoint = endpoint;
    if (status == Protocols::InteractionModel::Status::Success)
    {
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    }
}

void TlsClientManagementServer::HandleRemoveEndpoint(HandlerContext & ctx, const Commands::RemoveEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: RemoveEndpoint");

    auto status = mDelegate.RemoveProvisionedEndpointByID(ctx.mRequestPath.mEndpointId,
                                                          ctx.mCommandHandler.GetAccessingFabricIndex(), req.endpointID);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

/** @brief TlsClientManagement Cluster Server Init
 *
 * Server Init
 *
 */
void MatterTlsClientManagementPluginServerInitCallback() {}
