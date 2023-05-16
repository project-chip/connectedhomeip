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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;
using namespace Protocols;

//==============================================================================

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
    return encoder.EncodeEmptyList();
}

IcdManagementAttributeAccess gAttribute;

} // namespace

//==============================================================================

InteractionModel::Status
IcdManagementServer::RegisterClient(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                    const chip::app::Clusters::IcdManagement::Commands::RegisterClient::DecodableType & commandData)
{
    // TODO: Implementent logic for end device
    return InteractionModel::Status::UnsupportedCommand;
}

InteractionModel::Status IcdManagementServer::UnregisterClient(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::IcdManagement::Commands::UnregisterClient::DecodableType & commandData)
{
    // TODO: Implementent logic for end device
    return InteractionModel::Status::UnsupportedCommand;
}

InteractionModel::Status IcdManagementServer::StayActiveRequest(const chip::app::ConcreteCommandPath & commandPath)
{
    // TODO: Implementent logic for end device
    return InteractionModel::Status::UnsupportedCommand;
}

//==============================================================================

/**
 * @brief ICD Management Cluster RegisterClient Command callback (from client)
 *
 */
bool emberAfIcdManagementClusterRegisterClientCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::IcdManagement::Commands::RegisterClient::DecodableType & commandData)
{
    IcdManagementServer server;
    InteractionModel::Status status = server.RegisterClient(commandObj, commandPath, commandData);

    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief ICD Management Cluster UnregisterClient Command callback (from client)
 *
 */
bool emberAfIcdManagementClusterUnregisterClientCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::IcdManagement::Commands::UnregisterClient::DecodableType & commandData)
{
    IcdManagementServer server;
    InteractionModel::Status status = server.UnregisterClient(commandObj, commandPath, commandData);

    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief ICD Management Cluster StayActiveRequest Command callback (from client)
 */
bool emberAfIcdManagementClusterStayActiveRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::IcdManagement::Commands::StayActiveRequest::DecodableType & commandData)
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
