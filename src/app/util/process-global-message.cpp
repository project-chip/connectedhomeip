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
 * @brief This file contains function that processes
 *global ZCL message.
 *******************************************************************************
 ******************************************************************************/

#include <app/util/af.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/reporting/reporting.h>
#include <app/util/common.h>

#ifdef EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_SUB_GHZ
#include "app/framework/plugin/comms-hub-function-sub-ghz/comms-hub-function-sub-ghz.h"
#endif

#include <lib/support/CodeUtils.h>

using namespace chip;

bool emAfProcessGlobalCommand(EmberAfClusterCommand * cmd)
{
    uint8_t frameControl;
    // This is a little clumsy but easier to read and port
    // from earlier implementation.
    ClusterId clusterId      = cmd->apsFrame->clusterId;
    CommandId zclCmd         = cmd->commandId;
    uint8_t * message        = cmd->buffer;
    uint16_t msgLen          = cmd->bufLen;
    uint16_t msgIndex        = cmd->payloadStartIndex;
    uint8_t clientServerMask = (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER ? CLUSTER_MASK_SERVER : CLUSTER_MASK_CLIENT);

    // If we are disabled then we can only respond to read or write commands
    // or identify cluster (see device enabled attr of basic cluster)
    // Since read and write is handled by interaction model, we only need to handle identify command.
    if (!emberAfIsDeviceEnabled(cmd->apsFrame->destinationEndpoint) &&
        /* zclCmd != ZCL_READ_ATTRIBUTES_COMMAND_ID &&
        zclCmd != ZCL_WRITE_ATTRIBUTES_COMMAND_ID && zclCmd != ZCL_WRITE_ATTRIBUTES_UNDIVIDED_COMMAND_ID &&
        zclCmd != ZCL_WRITE_ATTRIBUTES_NO_RESPONSE_COMMAND_ID && */
        clusterId != ZCL_IDENTIFY_CLUSTER_ID)
    {
        emberAfCorePrintln("disabled");
        emberAfDebugPrintln("%pd, dropping global cmd:" ChipLogFormatMEI, "disable", ChipLogValueMEI(zclCmd));
        emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    // If a manufacturer-specific command arrives using our special internal "not
    // manufacturer specific" code, we need to reject it outright without letting
    // it pass through to the rest of the code.  The internal read and write APIs
    // would interpret it as a standard attribute or cluster and return incorrect
    // results.
    if (cmd->mfgSpecific && cmd->mfgCode == EMBER_AF_NULL_MANUFACTURER_CODE)
    {
        goto kickout;
    }

    // Clear out the response buffer by setting its length to zero
    appResponseLength = 0;

    // Make the ZCL header for the response
    // note: cmd byte is set below
    frameControl = static_cast<uint8_t>(ZCL_GLOBAL_COMMAND |
                                        (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER
                                             ? ZCL_FRAME_CONTROL_SERVER_TO_CLIENT | EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES
                                             : ZCL_FRAME_CONTROL_CLIENT_TO_SERVER | EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES));
    if (cmd->mfgSpecific)
    {
        frameControl |= ZCL_MANUFACTURER_SPECIFIC_MASK;
    }
    emberAfPutInt8uInResp(frameControl);
    if (cmd->mfgSpecific)
    {
        emberAfPutInt16uInResp(cmd->mfgCode);
    }
    emberAfPutInt8uInResp(cmd->seqNum);

    switch (zclCmd)
    {
    // [command id:4] [status:1]
    case ZCL_DEFAULT_RESPONSE_COMMAND_ID: {
        EmberAfStatus status;
        CommandId commandId;
        commandId = emberAfGetInt32u(message, msgIndex, msgLen);
        status    = (EmberAfStatus) emberAfGetInt8u(message, msgIndex + 4, msgLen);

        emberAfClusterDefaultResponseWithMfgCodeCallback(cmd->apsFrame->destinationEndpoint, clusterId, commandId, status,
                                                         clientServerMask, cmd->mfgCode);
        emberAfDefaultResponseCallback(clusterId, commandId, status);
        return true;
    }

    default:
        // MISRA requires default case.
        break;
    }

kickout:
    emberAfSendDefaultResponse(
        cmd, (cmd->mfgSpecific ? EMBER_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND : EMBER_ZCL_STATUS_UNSUP_GENERAL_COMMAND));
    return true;
}
