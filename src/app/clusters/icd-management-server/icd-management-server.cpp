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

#include "app/server/Server.h"
#include <access/AccessControl.h>
#include <access/Privilege.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/IcdMonitoringTable.h>
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
    CHIP_ERROR ReadRegisteredClients(EndpointId endpoint, AttributeValueEncoder & encoder);
};

CHIP_ERROR IcdManagementAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == IcdManagement::Id);

    switch (aPath.mAttributeId)
    {
    case IcdManagement::Attributes::RegisteredClients::Id:
        return ReadRegisteredClients(aPath.mEndpointId, aEncoder);

    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR IcdManagementAttributeAccess::ReadRegisteredClients(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    uint16_t supported_clients = 0;
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == Attributes::ClientsSupportedPerFabric::Get(endpoint, &supported_clients),
                        CHIP_ERROR_NOT_FOUND);

    return encoder.EncodeList([supported_clients](const auto & subEncoder) -> CHIP_ERROR {
        IcdMonitoringEntry e;

        const auto & fabricTable = Server::GetInstance().GetFabricTable();
        for (const auto & fabricInfo : fabricTable)
        {
            PersistentStorageDelegate & storage = chip::Server::GetInstance().GetPersistentStorage();
            IcdMonitoringTable table(storage, fabricInfo.GetFabricIndex(), supported_clients);
            for (uint16_t i = 0; i < table.Limit(); ++i)
            {
                CHIP_ERROR err = table.Get(i, e);
                if (CHIP_ERROR_NOT_FOUND == err)
                {
                    // No more entries in the table
                    break;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(subEncoder.Encode(e));
            }
        }
        return CHIP_NO_ERROR;
    });
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
        uint16_t supported_clients = 0;
        if (EMBER_ZCL_STATUS_SUCCESS != Attributes::ClientsSupportedPerFabric::Get(kRootEndpointId, &supported_clients))
        {
            // Fallback to maximum, the remove function will loop until no more entries are found.
            supported_clients = UINT16_MAX;
        }
        IcdMonitoringTable table(chip::Server::GetInstance().GetPersistentStorage(), fabricIndex, supported_clients);
        table.RemoveAll();
    }
};

IcdManagementFabricDelegate gFabricDelegate;
IcdManagementAttributeAccess gAttribute;

} // namespace

InteractionModel::Status IcdManagementServer::RegisterClient(chip::app::CommandHandler * commandObj,
                                                             const chip::app::ConcreteCommandPath & commandPath,
                                                             const Commands::RegisterClient::DecodableType & commandData)
{
    uint16_t supported_clients = 0;
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS ==
                            Attributes::ClientsSupportedPerFabric::Get(commandPath.mEndpointId, &supported_clients),
                        InteractionModel::Status::Failure);
    IcdMonitoringTable table(chip::Server::GetInstance().GetPersistentStorage(), commandObj->GetAccessingFabricIndex(),
                             supported_clients);

    // Get current entry, if exists
    IcdMonitoringEntry entry;
    CHIP_ERROR err = table.Find(commandData.checkInNodeID, entry);
    if (CHIP_NO_ERROR == err)
    {
        // Existing entry: Validate Key if, and only if, the ISD has NOT administrator permissions
        bool is_admin = false;
        err           = CheckAdmin(commandObj, commandPath, is_admin);
        VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);
        if (!is_admin)
        {
            VerifyOrReturnError(commandData.verificationKey.HasValue(), InteractionModel::Status::Failure);
            VerifyOrReturnError(commandData.verificationKey.Value().data_equal(entry.key), InteractionModel::Status::Failure);
        }
    }
    else if (CHIP_ERROR_NOT_FOUND == err)
    {
        // New entry
        VerifyOrReturnError(entry.index < table.Limit(), InteractionModel::Status::ResourceExhausted);
    }
    else
    {
        // Error
        return InteractionModel::Status::Failure;
    }

    // Save
    entry.checkInNodeID    = commandData.checkInNodeID;
    entry.monitoredSubject = commandData.monitoredSubject;
    entry.key              = commandData.key;
    err                    = table.Set(entry.index, entry);
    VerifyOrReturnError(CHIP_ERROR_INVALID_ARGUMENT != err, InteractionModel::Status::ConstraintError);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    return InteractionModel::Status::Success;
}

InteractionModel::Status IcdManagementServer::UnregisterClient(chip::app::CommandHandler * commandObj,
                                                               const chip::app::ConcreteCommandPath & commandPath,
                                                               const Commands::UnregisterClient::DecodableType & commandData)
{
    uint16_t supported_clients = 0;
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS ==
                            Attributes::ClientsSupportedPerFabric::Get(commandPath.mEndpointId, &supported_clients),
                        InteractionModel::Status::Failure);
    IcdMonitoringTable table(chip::Server::GetInstance().GetPersistentStorage(), commandObj->GetAccessingFabricIndex(),
                             supported_clients);

    // Get current entry, if exists
    IcdMonitoringEntry entry;
    CHIP_ERROR err = table.Find(commandData.checkInNodeID, entry);
    VerifyOrReturnError(CHIP_ERROR_NOT_FOUND != err, InteractionModel::Status::NotFound);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    // Existing entry: Validate Key if, and only if, the ISD has NOT administrator permissions
    bool is_admin = false;
    err           = CheckAdmin(commandObj, commandPath, is_admin);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    if (!is_admin)
    {
        VerifyOrReturnError(commandData.key.HasValue(), InteractionModel::Status::Failure);
        VerifyOrReturnError(commandData.key.Value().data_equal(entry.key), InteractionModel::Status::Failure);
    }

    err = table.Remove(entry.index);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    return InteractionModel::Status::Success;
}

InteractionModel::Status IcdManagementServer::StayActiveRequest(const chip::app::ConcreteCommandPath & commandPath)
{

    // TODO: Implementent stay awake logic for end device
    return InteractionModel::Status::UnsupportedCommand;
}

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
    IcdManagementServer server;

    InteractionModel::Status status = server.RegisterClient(commandObj, commandPath, commandData);
    if (InteractionModel::Status::Success == status)
    {
        // Response
        IcdManagement::Commands::RegisterClientResponse::Type response;
        if (EMBER_ZCL_STATUS_SUCCESS == Attributes::ICDCounter::Get(commandPath.mEndpointId, &response.ICDCounter))
        {
            commandObj->AddResponse(commandPath, response);
            return true;
        }
        status = InteractionModel::Status::Failure;
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
    IcdManagementServer server;
    InteractionModel::Status status = server.UnregisterClient(commandObj, commandPath, commandData);

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
    IcdManagementServer server;
    InteractionModel::Status status = server.StayActiveRequest(commandPath);

    commandObj->AddStatus(commandPath, status);
    return true;
}

void MatterIcdManagementPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttribute);
}
