/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "client-monitoring-server.h"

#include "app/server/Server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/ClientMonitoringRegistrationTable.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClientMonitoring;
using namespace Protocols;

namespace {

/************************************************************
 * ClientMonitoringAttributeAccess Implementation
 ************************************************************/

/**
 * @brief Implementation of attribute access for ClientMonitoring cluster
 */
class ClientMonitoringAttributeAccess : public app::AttributeAccessInterface
{
public:
    ClientMonitoringAttributeAccess() : AttributeAccessInterface(MakeOptional(kRootEndpointId), ClientMonitoring::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
    CHIP_ERROR ReadExpectedClients(EndpointId endpoint, AttributeValueEncoder & encoder);
};

CHIP_ERROR ClientMonitoringAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == ClientMonitoring::Id);

    switch (aPath.mAttributeId)
    {
    case ClientMonitoring::Attributes::ExpectedClients::Id:
        return ReadExpectedClients(aPath.mEndpointId, aEncoder);

    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClientMonitoringAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
}

CHIP_ERROR ClientMonitoringAttributeAccess::ReadExpectedClients(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    FabricIndex fabric = encoder.AccessingFabricIndex();

    return encoder.EncodeList([fabric](const auto & subEncoder) -> CHIP_ERROR {
        // TODO : https://github.com/project-chip/connectedhomeip/issues/24289
        ClientMonitoringRegistrationTable clientMonitoringRegistrationTable(chip::Server::GetInstance().GetPersistentStorage());
        CHIP_ERROR err = clientMonitoringRegistrationTable.LoadFromStorage(fabric);

        if (err == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(subEncoder.Encode(clientMonitoringRegistrationTable.GetClientRegistrationEntry()));
        }
        else if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            // No Entries saved for this fabric. Return empty list
            err = CHIP_NO_ERROR;
        }

        return err;
    });
}

ClientMonitoringAttributeAccess gAttribute;

} // namespace

/**********************************************************
 * ClientMonitoringServer Implementation
 *********************************************************/

InteractionModel::Status ClientMonitoringServer::StayAwakeRequestCommand(const ConcreteCommandPath & commandPath)
{
    // TODO: Implementent stay awake logic for end device
    return InteractionModel::Status::UnsupportedCommand;
}

InteractionModel::Status
ClientMonitoringServer::RegisterClientMonitoringCommand(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                        const Commands::RegisterClientMonitoring::DecodableType & commandData)
{
    FabricIndex fabric = commandObj->GetAccessingFabricIndex();

    // TODO : https://github.com/project-chip/connectedhomeip/issues/24289
    ClientMonitoringRegistrationTable table(chip::Server::GetInstance().GetPersistentStorage());

    VerifyOrReturnError(!table.HasValueForFabric(fabric), InteractionModel::Status::ResourceExhausted);

    table.GetClientRegistrationEntry().fabricIndex  = fabric;
    table.GetClientRegistrationEntry().clientNodeId = commandData.clientNodeId;
    table.GetClientRegistrationEntry().ICid         = commandData.ICid;

    VerifyOrReturnError(table.SaveToStorage() == CHIP_NO_ERROR, InteractionModel::Status::Failure);

    return InteractionModel::Status::Success;
}

InteractionModel::Status
ClientMonitoringServer::UnregisterClientMonitoringCommand(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                          const Commands::UnregisterClientMonitoring::DecodableType & commandData)
{
    FabricIndex fabric = commandObj->GetAccessingFabricIndex();

    // TODO : https://github.com/project-chip/connectedhomeip/issues/24289
    ClientMonitoringRegistrationTable table(chip::Server::GetInstance().GetPersistentStorage());

    CHIP_ERROR err = table.LoadFromStorage(fabric);

    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // Success if there was no key
        return InteractionModel::Status::Success;
    }

    VerifyOrReturnError(err == CHIP_NO_ERROR, InteractionModel::Status::Failure);

    // Check if initiator has the token and the correct node id
    VerifyOrReturnError(table.GetClientRegistrationEntry().clientNodeId == commandData.clientNodeId,
                        InteractionModel::Status::Failure);
    VerifyOrReturnError(table.GetClientRegistrationEntry().ICid == commandData.ICid, InteractionModel::Status::Failure);

    // Delete Key from storage afters checks
    VerifyOrReturnError(table.DeleteFromStorage(fabric) == CHIP_NO_ERROR, InteractionModel::Status::Failure);

    return InteractionModel::Status::Success;
}

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

/**
 * @brief Client Monitoring Cluster RegisterClientMonitoring Command callback (from client)
 *
 */
bool emberAfClientMonitoringClusterRegisterClientMonitoringCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::RegisterClientMonitoring::DecodableType & commandData)
{
    ClientMonitoringServer server;
    InteractionModel::Status status = server.RegisterClientMonitoringCommand(commandObj, commandPath, commandData);

    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Client Monitoring Cluster UregisterClientMonitoring Command callback (from client)
 *
 */
bool emberAfClientMonitoringClusterUnregisterClientMonitoringCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::UnregisterClientMonitoring::DecodableType & commandData)
{
    ClientMonitoringServer server;
    InteractionModel::Status status = server.UnregisterClientMonitoringCommand(commandObj, commandPath, commandData);

    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Client Monitoring Cluster StayAwakeRequest Command callback (from client)
 */
bool emberAfClientMonitoringClusterStayAwakeRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                            const Commands::StayAwakeRequest::DecodableType & commandData)
{
    ClientMonitoringServer server;
    InteractionModel::Status status = server.StayAwakeRequestCommand(commandPath);

    commandObj->AddStatus(commandPath, status);
    return true;
}

void MatterClientMonitoringPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttribute);
}
