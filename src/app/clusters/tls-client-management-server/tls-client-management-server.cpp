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

TlsClientManagementServer::TlsClientManagementServer(EndpointId endpointId, TlsClientManagementDelegate & delegate, CertificateTable & certificateTable) :
    AttributeAccessInterface(MakeOptional(endpointId), TlsClientManagement::Id),
    CommandHandlerInterface(MakeOptional(endpointId), TlsClientManagement::Id), mDelegate(delegate), mCertificateTable(certificateTable), mMaxProvisioned(0)
{
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

    LoadPersistentAttributes();

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    return CHIP_NO_ERROR;
}


CHIP_ERROR TlsClientManagementServer::Finish()
{
    mCertificateTable.Finish();
    return CHIP_NO_ERROR;
}

void TlsClientManagementServer::LoadPersistentAttributes()
{
    // Load MaxProvisioned
    uint8_t storedMaxProvisioned = 5;
    CHIP_ERROR err               = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(GetEndpointId(), TlsClientManagement::Id, MaxProvisioned::Id), storedMaxProvisioned);
    if (err != CHIP_NO_ERROR)
    {
        mMaxProvisioned = storedMaxProvisioned;
    }
    else
    {
        // otherwise defaults
        ChipLogDetail(
            Zcl, "TlsClientManagement: Unable to load the MaxProvisioned attribute from the KVS. Defaulting to %u",
            mMaxProvisioned);
    }
}

// AttributeAccessInterface
CHIP_ERROR TlsClientManagementServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == TlsClientManagement::Id);

    switch (aPath.mAttributeId)
    {
    case MaxProvisioned::Id:
        return aEncoder.Encode(mMaxProvisioned);
    case ProvisionedEndpoints::Id:
        TlsClientManagementServer * server = this;
        CHIP_ERROR err =
            aEncoder.EncodeList([server](const auto & encoder) -> CHIP_ERROR { return server->EncodeProvisionedEndpoints(encoder); });
        return err;
    }

    return CHIP_NO_ERROR;
}

uint8_t TlsClientManagementServer::GetMaxProvisioned() const
{
    return mMaxProvisioned;
}

// helper method to get the TlsClientManagement Sounds one by one and encode into a list
CHIP_ERROR
TlsClientManagementServer::EncodeProvisionedEndpoints(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (uint8_t i = 0; true; i++)
    {
        TLSEndpointStruct::Type endpoint;

        auto err = mDelegate.GetProvisionedEndpointByIndex(i, endpoint);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(err);

        ReturnErrorOnFailure(encoder.Encode(endpoint));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TlsClientManagementServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == TlsClientManagement::Id);
    Status status;

    switch (aPath.mAttributeId)
    {
    case MaxProvisioned::Id: {
        uint8_t newValue;
        ReturnErrorOnFailure(aDecoder.Decode(newValue));
        status = SetMaxProvisioned(newValue);
        return StatusIB(status).ToChipError();
    }
    default:
        // Unknown attribute
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

Status TlsClientManagementServer::SetMaxProvisioned(uint8_t maxProvisioned)
{
    bool enableChanged = mMaxProvisioned != maxProvisioned;

    if (enableChanged)
    {
        mMaxProvisioned = maxProvisioned;

        // Write new value to persistent storage.
        auto endpointId            = GetEndpointId();
        ConcreteAttributePath path = ConcreteAttributePath(endpointId, TlsClientManagement::Id, MaxProvisioned::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mMaxProvisioned);

        // and mark as dirty
        MatterReportingAttributeChangeCallback(path);
    }

    return Protocols::InteractionModel::Status::Success;
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


void TlsClientManagementServer::HandleProvisionEndpoint(HandlerContext & ctx, const Commands::ProvisionEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: ProvisionEndpoint");

    Commands::ProvisionEndpointResponse::Type response;
    Status status = mDelegate.ProvisionEndpoint(req, response.endpointID);

    if (status == Protocols::InteractionModel::Status::Success) {
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    } else {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        }
}

void TlsClientManagementServer::HandleFindEndpoint(HandlerContext & ctx, const Commands::FindEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: FindEndpoint");

    Commands::FindEndpointResponse::Type response;
    Status status = mDelegate.FindProvisionedEndpointByID(req.endpointID, response.endpoint);

    if (status == Protocols::InteractionModel::Status::Success) {
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    } else {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        }
}

void TlsClientManagementServer::HandleRemoveEndpoint(HandlerContext & ctx, const Commands::RemoveEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: RemoveEndpoint");

    Status status = mDelegate.RemoveProvisionedEndpointByID(req.endpointID);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

/** @brief TlsClientManagement Cluster Server Init
 *
 * Server Init
 *
 */
void MatterTlsClientManagementPluginServerInitCallback() {}
