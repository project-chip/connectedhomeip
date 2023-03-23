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

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/ClientMonitoringRegistrationTable.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClientMonitoring;

namespace {

/**
 * @brief Implementation of attribute access for ClientMonitoring cluster
 */
class ClientMonitoringAttributeAccess : public app::AttributeAccessInterface
{
public:
    ClientMonitoringAttributeAccess() : AttributeAccessInterface(MakeOptional(kRootEndpointId), ClientMonitoring::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        VerifyOrDie(aPath.mClusterId == ClientMonitoring::Id);

        switch (aPath.mAttributeId)
        {
        case ClientMonitoring::Attributes::ExpectedClients::Id:
            // TODO : Implement Client monitoring registration table
            return CHIP_IM_GLOBAL_STATUS(UnsupportedRead);

        default:
            break;
        }

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
    }
};

ClientMonitoringAttributeAccess gAttribute;

} // namespace

/**
 * @brief Client Monitoring Cluster RegisterClientMonitoring Command callback (from client)
 */
bool emberAfClientMonitoringClusterRegisterClientMonitoringCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::RegisterClientMonitoring::DecodableType & commandData)
{
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    return false;
}
/**
 * @brief Client Monitoring Cluster StayAwakeRequest Command callback (from client)
 */
bool emberAfClientMonitoringClusterStayAwakeRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                            const Commands::StayAwakeRequest::DecodableType & commandData)
{
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    return false;
}

void MatterClientMonitoringPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttribute);
}
