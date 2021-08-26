/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************/
/**
 * @file
 * @brief This file contains a function that processes
 *cluster-specific ZCL message.
 *******************************************************************************
 ******************************************************************************/

// this file contains all the common includes for clusters in the zcl-util
#include "app/util/common.h"

// for pulling in defines dealing with EITHER server or client
#include <app/util/af-main.h>

#include <app-common/zap-generated/cluster-id.h>

// the EM260 host needs to include the config file
#ifdef EZSP_HOST
#include <app/util/config.h>
#endif

//------------------------------------------------------------------------------

bool emAfProcessClusterSpecificCommand(EmberAfClusterCommand * cmd)
{
    // if we are disabled then we can only respond to read or write commands
    // or identify cluster (see device enabled attr of basic cluster)
    if (!emberAfIsDeviceEnabled(cmd->apsFrame->destinationEndpoint) && cmd->apsFrame->clusterId != ZCL_IDENTIFY_CLUSTER_ID)
    {
        emberAfCorePrintln("%pd, dropping ep 0x%x clus 0x%2x cmd 0x%x", "disable", cmd->apsFrame->destinationEndpoint,
                           cmd->apsFrame->clusterId, cmd->commandId);
        emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

#ifdef ZCL_USING_KEY_ESTABLISHMENT_CLUSTER_CLIENT
    if (cmd->apsFrame->clusterId == ZCL_KEY_ESTABLISHMENT_CLUSTER_ID && cmd->direction == ZCL_DIRECTION_SERVER_TO_CLIENT &&
        emberAfKeyEstablishmentClusterClientCommandReceivedCallback(cmd))
    {
        return true;
    }
#endif
#ifdef ZCL_USING_KEY_ESTABLISHMENT_CLUSTER_SERVER
    if (cmd->apsFrame->clusterId == ZCL_KEY_ESTABLISHMENT_CLUSTER_ID && cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER &&
        emberAfKeyEstablishmentClusterServerCommandReceivedCallback(cmd))
    {
        return true;
    }
#endif

#ifdef ZCL_USING_OTA_BOOTLOAD_CLUSTER_CLIENT
    if (cmd->apsFrame->clusterId == ZCL_OTA_BOOTLOAD_CLUSTER_ID && cmd->direction == ZCL_DIRECTION_SERVER_TO_CLIENT &&
        emberAfOtaRequestorIncomingMessageRawCallback(cmd))
    {
        return true;
    }
#endif
#ifdef ZCL_USING_OTA_BOOTLOAD_CLUSTER_SERVER
    if (cmd->apsFrame->clusterId == ZCL_OTA_BOOTLOAD_CLUSTER_ID && cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER &&
        emberAfOtaProviderIncomingMessageRawCallback(cmd))
    {
        return true;
    }
#endif

    // All cluster messages (commands) will go through the IM now, return false to indicate an error.
    return false;
}
