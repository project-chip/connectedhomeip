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
#include <app/icd/server/ICDMonitoringTable.h>
#include <app/icd/server/ICDNotifier.h>
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
class IcdManagementAttributeAccess : public AttributeAccessInterface
{
public:
    IcdManagementAttributeAccess() : AttributeAccessInterface(MakeOptional(kRootEndpointId), IcdManagement::Id) {}

    void Init(PersistentStorageDelegate & storage, Crypto::SymmetricKeystore * symmetricKeystore, FabricTable & fabricTable,
              ICDConfigurationData & icdConfigurationData)
    {
        mStorage              = &storage;
        mSymmetricKeystore    = symmetricKeystore;
        mFabricTable          = &fabricTable;
        mICDConfigurationData = &icdConfigurationData;
    }

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadIdleModeDuration(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadActiveModeDuration(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadActiveModeThreshold(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadRegisteredClients(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadICDCounter(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadClientsSupportedPerFabric(EndpointId endpoint, AttributeValueEncoder & encoder);

    PersistentStorageDelegate * mStorage           = nullptr;
    Crypto::SymmetricKeystore * mSymmetricKeystore = nullptr;
    FabricTable * mFabricTable                     = nullptr;
    ICDConfigurationData * mICDConfigurationData   = nullptr;
};

CHIP_ERROR IcdManagementAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == IcdManagement::Id);

    switch (aPath.mAttributeId)
    {
    case IcdManagement::Attributes::IdleModeDuration::Id:
        return ReadIdleModeDuration(aPath.mEndpointId, aEncoder);

    case IcdManagement::Attributes::ActiveModeDuration::Id:
        return ReadActiveModeDuration(aPath.mEndpointId, aEncoder);

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

CHIP_ERROR IcdManagementAttributeAccess::ReadIdleModeDuration(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    return encoder.Encode(mICDConfigurationData->GetIdleModeDuration().count());
}

CHIP_ERROR IcdManagementAttributeAccess::ReadActiveModeDuration(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    return encoder.Encode(mICDConfigurationData->GetActiveModeDuration().count());
}

CHIP_ERROR IcdManagementAttributeAccess::ReadActiveModeThreshold(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    return encoder.Encode(mICDConfigurationData->GetActiveModeThreshold().count());
}

CHIP_ERROR IcdManagementAttributeAccess::ReadRegisteredClients(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    uint16_t supported_clients                    = mICDConfigurationData->GetClientsSupportedPerFabric();
    PersistentStorageDelegate * storage           = mStorage;
    Crypto::SymmetricKeystore * symmetricKeystore = mSymmetricKeystore;
    const FabricTable * fabricTable               = mFabricTable;

    return encoder.EncodeList([supported_clients, storage, symmetricKeystore, fabricTable](const auto & subEncoder) -> CHIP_ERROR {
        ICDMonitoringEntry e(symmetricKeystore);

        for (const auto & fabricInfo : *fabricTable)
        {
            ICDMonitoringTable table(*storage, fabricInfo.GetFabricIndex(), supported_clients, symmetricKeystore);
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
    return encoder.Encode(mICDConfigurationData->GetICDCounter().GetValue());
}

CHIP_ERROR IcdManagementAttributeAccess::ReadClientsSupportedPerFabric(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    return encoder.Encode(mICDConfigurationData->GetClientsSupportedPerFabric());
}

/**
 * @brief Implementation of Fabric Delegate for ICD Management cluster
 */
class IcdManagementFabricDelegate : public FabricTable::Delegate
{
public:
    void Init(PersistentStorageDelegate & storage, Crypto::SymmetricKeystore * symmetricKeystore,
              ICDConfigurationData & icdConfigurationData)
    {
        mStorage              = &storage;
        mSymmetricKeystore    = symmetricKeystore;
        mICDConfigurationData = &icdConfigurationData;
    }

    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        uint16_t supported_clients = mICDConfigurationData->GetClientsSupportedPerFabric();
        ICDMonitoringTable table(*mStorage, fabricIndex, supported_clients, mSymmetricKeystore);
        table.RemoveAll();
        ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDListener::ICDManagementEvents::kTableUpdated);
    }

private:
    PersistentStorageDelegate * mStorage           = nullptr;
    Crypto::SymmetricKeystore * mSymmetricKeystore = nullptr;
    ICDConfigurationData * mICDConfigurationData   = nullptr;
};

IcdManagementFabricDelegate gFabricDelegate;
IcdManagementAttributeAccess gAttribute;

/**
 * @brief Function checks if the client has admin permissions to the cluster in the commandPath
 *
 * @param[out] isClientAdmin True : Client has admin permissions
 *                           False : Client does not have admin permissions
 *                           If an error occurs, isClientAdmin is not changed
 * @return CHIP_ERROR
 */
CHIP_ERROR CheckAdmin(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, bool & isClientAdmin)
{
    RequestPath requestPath{ .cluster = commandPath.mClusterId, .endpoint = commandPath.mEndpointId };
    CHIP_ERROR err = GetAccessControl().Check(commandObj->GetSubjectDescriptor(), requestPath, Privilege::kAdminister);
    if (CHIP_NO_ERROR == err)
    {
        isClientAdmin = true;
    }
    else if (CHIP_ERROR_ACCESS_DENIED == err)
    {
        isClientAdmin = false;
        err           = CHIP_NO_ERROR;
    }
    return err;
}

} // namespace

/*
 * ICD Management Implementation
 */

PersistentStorageDelegate * ICDManagementServer::mStorage           = nullptr;
Crypto::SymmetricKeystore * ICDManagementServer::mSymmetricKeystore = nullptr;
ICDConfigurationData * ICDManagementServer::mICDConfigurationData   = nullptr;

Status ICDManagementServer::RegisterClient(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                           const Commands::RegisterClient::DecodableType & commandData, uint32_t & icdCounter)
{
    FabricIndex fabricIndex            = commandObj->GetAccessingFabricIndex();
    NodeId nodeId                      = commandData.checkInNodeID;
    uint64_t monitoredSubject          = commandData.monitoredSubject;
    ByteSpan key                       = commandData.key;
    Optional<ByteSpan> verificationKey = commandData.verificationKey;
    bool isClientAdmin                 = false;

    // Check if client is admin
    VerifyOrReturnError(CHIP_NO_ERROR == CheckAdmin(commandObj, commandPath, isClientAdmin), InteractionModel::Status::Failure);

    bool isFirstEntryForFabric = false;
    ICDMonitoringTable table(*mStorage, fabricIndex, mICDConfigurationData->GetClientsSupportedPerFabric(), mSymmetricKeystore);

    // Get current entry, if exists
    ICDMonitoringEntry entry(mSymmetricKeystore);
    CHIP_ERROR err = table.Find(nodeId, entry);
    if (CHIP_NO_ERROR == err)
    {
        // Existing entry: Validate Key if, and only if, the ISD does NOT have administrator permissions
        if (!isClientAdmin)
        {
            VerifyOrReturnError(verificationKey.HasValue(), InteractionModel::Status::Failure);
            VerifyOrReturnError(entry.IsKeyEquivalent(verificationKey.Value()), InteractionModel::Status::Failure);
        }
    }
    else if (CHIP_ERROR_NOT_FOUND == err)
    {
        // New entry
        VerifyOrReturnError(entry.index < table.Limit(), InteractionModel::Status::ResourceExhausted);

        // Check if it's going to be the first entry for fabric
        isFirstEntryForFabric = table.IsEmpty();
    }
    else
    {
        // Error
        return InteractionModel::Status::Failure;
    }

    // Save
    entry.checkInNodeID    = nodeId;
    entry.monitoredSubject = monitoredSubject;
    if (entry.keyHandleValid)
    {
        entry.DeleteKey();
    }

    err = entry.SetKey(key);
    VerifyOrReturnError(CHIP_ERROR_INVALID_ARGUMENT != err, InteractionModel::Status::ConstraintError);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);
    err = table.Set(entry.index, entry);

    // Delete key upon failure to prevent key storage leakage.
    if (err != CHIP_NO_ERROR)
    {
        entry.DeleteKey();
    }

    VerifyOrReturnError(CHIP_ERROR_INVALID_ARGUMENT != err, InteractionModel::Status::ConstraintError);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    if (isFirstEntryForFabric)
    {
        // Notify subscribers that the first entry for the fabric was successfully added
        TriggerICDMTableUpdatedEvent();
    }

    icdCounter = mICDConfigurationData->GetICDCounter().GetValue();
    return InteractionModel::Status::Success;
}

Status ICDManagementServer::UnregisterClient(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                             const Commands::UnregisterClient::DecodableType & commandData)
{
    FabricIndex fabricIndex            = commandObj->GetAccessingFabricIndex();
    NodeId nodeId                      = commandData.checkInNodeID;
    Optional<ByteSpan> verificationKey = commandData.verificationKey;
    bool isClientAdmin                 = false;

    // Check if client is admin
    VerifyOrReturnError(CHIP_NO_ERROR == CheckAdmin(commandObj, commandPath, isClientAdmin), InteractionModel::Status::Failure);

    ICDMonitoringTable table(*mStorage, fabricIndex, mICDConfigurationData->GetClientsSupportedPerFabric(), mSymmetricKeystore);

    // Get current entry, if exists
    ICDMonitoringEntry entry(mSymmetricKeystore);
    CHIP_ERROR err = table.Find(nodeId, entry);
    VerifyOrReturnError(CHIP_ERROR_NOT_FOUND != err, InteractionModel::Status::NotFound);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    // Existing entry: Validate Key if, and only if, the ISD has NOT administrator permissions
    if (!isClientAdmin)
    {
        VerifyOrReturnError(verificationKey.HasValue(), InteractionModel::Status::Failure);
        VerifyOrReturnError(entry.IsKeyEquivalent(verificationKey.Value()), InteractionModel::Status::Failure);
    }

    err = table.Remove(entry.index);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    if (table.IsEmpty())
    {
        TriggerICDMTableUpdatedEvent();
    }

    return InteractionModel::Status::Success;
}

Status ICDManagementServer::StayActiveRequest(FabricIndex fabricIndex)
{
    // TODO: Implementent stay awake logic for end device
    // https://github.com/project-chip/connectedhomeip/issues/24259
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDListener::ICDManagementEvents::kStayActiveRequestReceived);
    return InteractionModel::Status::UnsupportedCommand;
}

void ICDManagementServer::TriggerICDMTableUpdatedEvent()
{
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDListener::ICDManagementEvents::kTableUpdated);
}

void ICDManagementServer::Init(PersistentStorageDelegate & storage, Crypto::SymmetricKeystore * symmetricKeystore,
                               ICDConfigurationData & icdConfigurationData)
{
    mStorage              = &storage;
    mSymmetricKeystore    = symmetricKeystore;
    mICDConfigurationData = &icdConfigurationData;
}

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

/**
 * @brief ICD Management Cluster RegisterClient Command callback (from client)
 *
 */
bool emberAfIcdManagementClusterRegisterClientCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                       const Commands::RegisterClient::DecodableType & commandData)
{
    uint32_t icdCounter = 0;

    ICDManagementServer server;
    InteractionModel::Status status = server.RegisterClient(commandObj, commandPath, commandData, icdCounter);

    if (InteractionModel::Status::Success == status)
    {
        // Response
        IcdManagement::Commands::RegisterClientResponse::Type response{ .ICDCounter = icdCounter };
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
bool emberAfIcdManagementClusterUnregisterClientCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                         const Commands::UnregisterClient::DecodableType & commandData)
{
    ICDManagementServer server;
    InteractionModel::Status status = server.UnregisterClient(commandObj, commandPath, commandData);

    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief ICD Management Cluster StayActiveRequest Command callback (from client)
 */
bool emberAfIcdManagementClusterStayActiveRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                          const Commands::StayActiveRequest::DecodableType & commandData)
{
    ICDManagementServer server;
    InteractionModel::Status status = server.StayActiveRequest(commandObj->GetAccessingFabricIndex());

    commandObj->AddStatus(commandPath, status);
    return true;
}

void MatterIcdManagementPluginServerInitCallback()
{
    PersistentStorageDelegate & storage           = Server::GetInstance().GetPersistentStorage();
    FabricTable & fabricTable                     = Server::GetInstance().GetFabricTable();
    Crypto::SymmetricKeystore * symmetricKeystore = Server::GetInstance().GetSessionKeystore();
    ICDConfigurationData & icdConfigurationData   = ICDConfigurationData::GetInstance().GetInstance();

    // Configure and register Fabric delegate
    gFabricDelegate.Init(storage, symmetricKeystore, icdConfigurationData);
    fabricTable.AddFabricDelegate(&gFabricDelegate);

    // Configure and register Attribute Access Override
    gAttribute.Init(storage, symmetricKeystore, fabricTable, icdConfigurationData);
    registerAttributeAccessOverride(&gAttribute);

    // Configure ICD Management
    ICDManagementServer::Init(storage, symmetricKeystore, icdConfigurationData);
}
