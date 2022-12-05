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
#include <app/CommandHandler.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClientMonitoring;

/**
 * @brief Client Monitoring Cluster RegisterClientMonitoring Command callback (from client)
 */
bool emberAfClientMonitoringClusterRegisterClientMonitoringCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::RegisterClientMonitoring::DecodableType & commandData)
{
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
    return false;
}
/**
 * @brief Client Monitoring Cluster StayAwakeRequest Command callback (from client)
 */
bool emberAfClientMonitoringClusterStayAwakeRequestCallback(app::CommandHandler * commandObj,
                                                            const chip::app::ConcreteCommandPath & commandPath,
                                                            const Commands::StayAwakeRequest::DecodableType & commandData)
{
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
    return false;
}

void MatterClientMonitoringPluginServerInitCallback() {}
