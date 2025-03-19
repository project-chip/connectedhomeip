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

#include "tls-client-managemnet-server.h"

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::TlsClientManagement;
using namespace chip::app::Clusters::TlsClientManagement::Attributes;
using chip::Protocols::InteractionModel::Status;

TlsClientManagementServer::TlsClientManagementServer(EndpointId endpointId, TlsClientManagementDelegate & delegate, CertificateTable & certificateTable) :
    AttributeAccessInterface(MakeOptional(endpointId), TlsClientManagement::Id),
    CommandHandlerInterface(MakeOptional(endpointId), TlsClientManagement::Id), mDelegate(delegate), mCertificateTable(certificateTable), mMaxProvisioned(0),
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
    LoadPersistentAttributes();

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

void TlsClientManagementServer::LoadPersistentAttributes()
{
    // Load MaxProvisioned
    uint8_t storedMaxProvisioned = 5;
    CHIP_ERROR err               = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(GetEndpointId(), TlsClientManagement::Id, MaxProvisioned::Id), storedActiveTlsClientManagementID);
    if (err == CHIP_NO_ERROR)
    {
        mMaxProvisioned = storedMaxProvisioned;
    }
    else
    {
        // otherwise defaults
        ChipLogDetail(
            Zcl, "TlsClientManagement: Unable to load the ActiveTlsClientManagementID attribute from the KVS. Defaulting to %u",
            mActiveTlsClientManagementID);
    }
}

// AttributeAccessInterface
CHIP_ERROR TlsClientManagementServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == TlsClientManagement::Id);

    switch (aPath.mAttributeId)
    {
    case MaxProvisioned::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mMaxProvisioned));
        break;
    case ProvisionedEndpoints::Id:
        TlsClientManagementServer * server = this;
        CHIP_ERROR err =
            aEncoder.EncodeList([server](const auto & encoder) -> CHIP_ERROR { return server->EncodeProvisionedEndpoints(encoder); });
        return err;
    }
    break;

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
        TLSEndpointStruct endpoint;

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
    case Commands::PlayTlsClientManagementSound::Id:
        CommandHandlerInterface::HandleCommand<Commands::PlayTlsClientManagementSound::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandlePlayTlsClientManagementSound(innerCtx, req); });
        break;
    }
}

void TlsClientManagementServer::HandlePlayTlsClientManagementSound(
    HandlerContext & ctx, const Commands::PlayTlsClientManagementSound::DecodableType & req)
{

    ChipLogDetail(Zcl, "TlsClientManagement: PlayTlsClientManagementSound");

    // call the delegate to play the chime
    Status status = mDelegate.PlayTlsClientManagementSound();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

/** @brief TlsClientManagement Cluster Server Init
 *
 * Server Init
 *
 */
void MatterTlsClientManagementPluginServerInitCallback() {}
