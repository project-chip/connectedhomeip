/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "icd-management-server.h"

#include <access/AccessControl.h>
#include <access/Privilege.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/icd/ICDManagementServer.h>
#include <app/icd/ICDMonitoringTable.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;
using namespace Protocols;
using namespace chip::Access;

namespace {

/**
 * @brief Implementation of attribute access for IcdManagement cluster
 */
class IcdManagementAttributeAccess : public app::AttributeAccessInterface
{
public:
    IcdManagementAttributeAccess() : AttributeAccessInterface(MakeOptional(kRootEndpointId), IcdManagement::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadIdleModeInterval(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadActiveModeInterval(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadActiveModeThreshold(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadRegisteredClients(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadICDCounter(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadClientsSupportedPerFabric(EndpointId endpoint, AttributeValueEncoder & encoder);
};

CHIP_ERROR IcdManagementAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == IcdManagement::Id);

    switch (aPath.mAttributeId)
    {
    case IcdManagement::Attributes::IdleModeInterval::Id:
        return ReadIdleModeInterval(aPath.mEndpointId, aEncoder);

    case IcdManagement::Attributes::ActiveModeInterval::Id:
        return ReadActiveModeInterval(aPath.mEndpointId, aEncoder);

    case IcdManagement::Attributes::ActiveModeThreshold::Id:
        return ReadActiveModeThreshold(aPath.mEndpointId, aEncoder);

    case IcdManagement::Attributes::RegisteredClients::Id:
        return ReadRegisteredClients(aPath.mEndpointId, aEncoder);

    case IcdManagement::Attributes::ICDCounter::Id:
        return ReadICDCounter(aPath.mEndpointId, aEncoder);

    case IcdManagement::Attributes::ClientsSupportedPerFabric::Id:
        return ReadClientsSupportedPerFabric(aPath.mEndpointId, aEncoder);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR IcdManagementAttributeAccess::ReadIdleModeInterval(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    return encoder.Encode(ICDManagementServer::GetInstance().GetIdleModeIntervalSec());
}

CHIP_ERROR IcdManagementAttributeAccess::ReadActiveModeInterval(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    return encoder.Encode(ICDManagementServer::GetInstance().GetActiveModeIntervalMs());
}

CHIP_ERROR IcdManagementAttributeAccess::ReadActiveModeThreshold(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    return encoder.Encode(ICDManagementServer::GetInstance().GetActiveModeThresholdMs());
}

CHIP_ERROR IcdManagementAttributeAccess::ReadRegisteredClients(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    uint16_t supported_clients = ICDManagementServer::GetInstance().GetClientsSupportedPerFabric();

    return encoder.EncodeList([supported_clients](const auto & subEncoder) -> CHIP_ERROR {
        Crypto::SessionKeystore * keyStore = Server::GetInstance().GetSessionKeystore();
        ICDMonitoringEntry e(keyStore);

        const auto & fabricTable            = Server::GetInstance().GetFabricTable();
        PersistentStorageDelegate & storage = Server::GetInstance().GetPersistentStorage();
        for (const auto & fabricInfo : fabricTable)
        {
            ICDMonitoringTable table(storage, fabricInfo.GetFabricIndex(), supported_clients, keyStore);
            for (uint16_t i = 0; i < table.Limit(); ++i)
            {
                CHIP_ERROR err = table.Get(i, e);
                if (CHIP_ERROR_NOT_FOUND == err)
                {
                    // No more entries in the table
                    break;
                }
                ReturnErrorOnFailure(err);

                Structs::MonitoringRegistrationStruct::Type s{ .checkInNodeID    = e.checkInNodeID,
                                                               .monitoredSubject = e.monitoredSubject,
                                                               .fabricIndex      = e.fabricIndex };
                ReturnErrorOnFailure(subEncoder.Encode(s));
            }
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR IcdManagementAttributeAccess::ReadICDCounter(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    return encoder.Encode(ICDManagementServer::GetInstance().GetICDCounter());
}

CHIP_ERROR IcdManagementAttributeAccess::ReadClientsSupportedPerFabric(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    return encoder.Encode(ICDManagementServer::GetInstance().GetClientsSupportedPerFabric());
}

CHIP_ERROR CheckAdmin(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath, bool & is_admin)
{
    RequestPath requestPath{ .cluster = commandPath.mClusterId, .endpoint = commandPath.mEndpointId };
    CHIP_ERROR err = GetAccessControl().Check(commandObj->GetSubjectDescriptor(), requestPath, Privilege::kAdminister);
    if (CHIP_NO_ERROR == err)
    {
        is_admin = true;
    }
    else if (CHIP_ERROR_ACCESS_DENIED == err)
    {
        is_admin = false;
        err      = CHIP_NO_ERROR;
    }
    return err;
}

class IcdManagementFabricDelegate : public chip::FabricTable::Delegate
{
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        uint16_t supported_clients = ICDManagementServer::GetInstance().GetClientsSupportedPerFabric();
        ICDMonitoringTable table(Server::GetInstance().GetPersistentStorage(), fabricIndex, supported_clients,
                                 Server::GetInstance().GetSessionKeystore());
        table.RemoveAll();
    }
};

IcdManagementFabricDelegate gFabricDelegate;
IcdManagementAttributeAccess gAttribute;

} // namespace

void emberAfIcdManagementClusterInitCallback()
{
    Server::GetInstance().GetFabricTable().AddFabricDelegate(&gFabricDelegate);
}

/**
 * @brief ICD Management Cluster RegisterClient Command callback (from client)
 *
 */
bool emberAfIcdManagementClusterRegisterClientCallback(chip::app::CommandHandler * commandObj,
                                                       const chip::app::ConcreteCommandPath & commandPath,
                                                       const Commands::RegisterClient::DecodableType & commandData)
{
    InteractionModel::Status status = InteractionModel::Status::Failure;
    bool is_admin                   = false;
    if (CHIP_NO_ERROR == CheckAdmin(commandObj, commandPath, is_admin))
    {
        PersistentStorageDelegate & storage = chip::Server::GetInstance().GetPersistentStorage();
        FabricIndex fabric                  = commandObj->GetAccessingFabricIndex();
        status = ICDManagementServer::GetInstance().RegisterClient(storage, fabric, commandData.checkInNodeID,
                                                                   commandData.monitoredSubject, commandData.key,
                                                                   commandData.verificationKey, is_admin);
    }

    if (InteractionModel::Status::Success == status)
    {
        // Response
        IcdManagement::Commands::RegisterClientResponse::Type response{ .ICDCounter =
                                                                            ICDManagementServer::GetInstance().GetICDCounter() };
        commandObj->AddResponse(commandPath, response);
        return true;
    }

    // Error
    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief ICD Management Cluster UregisterClient Command callback (from client)
 *
 */
bool emberAfIcdManagementClusterUnregisterClientCallback(chip::app::CommandHandler * commandObj,
                                                         const chip::app::ConcreteCommandPath & commandPath,
                                                         const Commands::UnregisterClient::DecodableType & commandData)
{
    InteractionModel::Status status = InteractionModel::Status::Failure;
    bool is_admin                   = false;
    if (CHIP_NO_ERROR == CheckAdmin(commandObj, commandPath, is_admin))
    {
        PersistentStorageDelegate & storage = chip::Server::GetInstance().GetPersistentStorage();
        FabricIndex fabric                  = commandObj->GetAccessingFabricIndex();
        status = ICDManagementServer::GetInstance().UnregisterClient(storage, fabric, commandData.checkInNodeID,
                                                                     commandData.verificationKey, is_admin);
    }

    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief ICD Management Cluster StayActiveRequest Command callback (from client)
 */
bool emberAfIcdManagementClusterStayActiveRequestCallback(chip::app::CommandHandler * commandObj,
                                                          const chip::app::ConcreteCommandPath & commandPath,
                                                          const Commands::StayActiveRequest::DecodableType & commandData)
{
    InteractionModel::Status status = ICDManagementServer::GetInstance().StayActiveRequest(commandObj->GetAccessingFabricIndex());

    commandObj->AddStatus(commandPath, status);
    return true;
}

void MatterIcdManagementPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttribute);
}
