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
/***************************************************************************//**
 * @file
 * @brief Routines for the Gas Proxy Function plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/gbz-message-controller/gbz-message-controller.h"
#include "app/framework/plugin/gas-proxy-function/gas-proxy-function.h"
#include "app/framework/plugin/price-server/price-server.h"
#include "app/framework/plugin/messaging-server/messaging-server.h"
#include "app/framework/plugin/calendar-common/calendar-common.h"
#include "app/framework/plugin/device-management-server/device-management-server.h"
#include "app/framework/plugin/meter-mirror/meter-mirror.h"
#include "gpf-structured-data.h"
#include "app/framework/plugin/events-server/events-server.h"

#define fieldLength(field) \
  (emberAfCurrentCommand()->bufLen - (field - emberAfCurrentCommand()->buffer));

// default configurations
#define DEFAULT_TABLE_SET_INDEX (0)
static bool nonTomHandlingActive = false;
static EmberAfGbzMessageParserState nonTomGbzRequestParser;
static EmberAfGbzMessageCreatorState nonTomGbzResponseCreator;
static uint8_t nonTomExpectedZclResps = 0;  // per command
static uint8_t nonTomResponsesCount = 0;    // per command
static uint32_t nonTomGbzStartTime = 0;
static EmberAfGpfMessage currentGpfMessage;

void hideEndpoint(uint8_t endpoint)
{
#ifdef EZSP_HOST
  EzspStatus status = ezspSetEndpointFlags(endpoint,
                                           EZSP_ENDPOINT_DISABLED);
  emberAfPluginGasProxyFunctionPrintln("GPF: hiding endpoint status: 0x%X", status);
#else
  // we need to standarize disabling/hiding/reset endpoints.
  emberAfPluginGasProxyFunctionPrintln("GPF: hiding endpoint not supported on Soc!");
#endif
}

void emberAfPluginGasProxyFunctionInitCallback(uint8_t endpoint)
{
  emberAfPluginGasProxyFunctionInitStructuredData();
  // try to hide hidden endpoint from service discovery.
  hideEndpoint(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT);

  MEMSET(&currentGpfMessage, 0x00, sizeof(EmberAfGpfMessage));
}

/*  @brief  Log all GPF commands to GPF Event Log.
 *
 *  In GBCS v0.81, the GPF shall be capable of logging all Commands
 *  received and Outcomes in the GPF Event Log(4.6.3.8). If the message is
 *  too long, the data will be truncated.
 *  */
void emAfPluginGasProxyFunctionLogEvent(uint8_t * gbzCmd,
                                        uint16_t  gbzCmdLen,
                                        uint8_t * gbzResp,
                                        uint16_t  gbzRespLen,
                                        uint16_t eventId,
                                        EmberAfGPFMessageType cmdType,
                                        uint16_t messageCode)
{
  EmberAfEvent event;
  uint8_t * logMsg = event.eventData;
  uint8_t endpoint;

  logMsg[0] = 0x00;

  // EMAPPFWKV2-1315 - "For any Event Log entries relating to Event Codes
  // 0x0054 and 0x0055, the Device shall record the Commands received on the
  // Network Interface by including the Message Code in the Event Log"
  if (eventId == GBCS_EVENT_ID_IMM_HAN_CMD_RXED_ACTED
      || eventId == GBCS_EVENT_ID_IMM_HAN_CMD_RXED_NOT_ACTED) {
    logMsg[0] = 0x02;
    logMsg[1] = HIGH_BYTE(messageCode);
    logMsg[2] = LOW_BYTE(messageCode);
  }

  event.eventId = eventId;
  event.eventTime = emberAfGetCurrentTime();

  if ((messageCode == GCS01a_MESSAGE_CODE)
      || (messageCode == GCS01b_MESSAGE_CODE)
      || (messageCode == GCS05_MESSAGE_CODE)
      || (messageCode == GCS07_MESSAGE_CODE)
      || (messageCode == GCS23_MESSAGE_CODE)
      || (messageCode == GCS25_MESSAGE_CODE)
      || (messageCode == GCS44_MESSAGE_CODE)
      ) {
    endpoint = EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT;
  } else {
    endpoint = EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT;
  }
  emberAfEventsServerAddEvent(endpoint,
                              EMBER_ZCL_EVENT_LOG_ID_GENERAL_EVENT_LOG,
                              &event);
}

static EmberStatus emberAfPluginGasProxyFunctionReplayZclCommand(uint8_t srcEndpoint,
                                                                 uint8_t dstEndpoint,
                                                                 EmberAfGbzZclCommand * cmd)
{
  EmberNodeId dstId = emberAfGetNodeId();
  EmberStatus status;

  emberAfPluginGasProxyFunctionPrintln("GPF: Replaying following command from endpoint(%d) to (%d)", srcEndpoint, dstEndpoint);
  emberAfPluginGbzMessageControllerPrintCommandInfo(cmd);

  if (dstEndpoint == EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT
      && !emberAfIsDeviceEnabled(dstEndpoint)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: ERR: Cannot replay command. Mirror is not available.");
    emberAfFillExternalBuffer(((cmd->direction == ZCL_DIRECTION_SERVER_TO_CLIENT)
                               ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                              cmd->clusterId,                  \
                              ZCL_DEFAULT_RESPONSE_COMMAND_ID, \
                              "uu",                            \
                              cmd->commandId,                  \
                              EMBER_ZCL_STATUS_FAILURE);
    emberAfSetCommandEndpoints(srcEndpoint, srcEndpoint);
  } else {
    emberAfFillExternalBuffer(cmd->frameControl, \
                              cmd->clusterId,    \
                              cmd->commandId,    \
                              "");
    emberAfAppendToExternalBuffer(cmd->payload, cmd->payloadLength);
    emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  }

  appResponseData[1] = cmd->transactionSequenceNumber;

  // We assume that one response will be sent per command.  If more than one
  // response is to be received the expected response count will be updated.
  // Refer to emberAfPluginCalendarServerPublishInfoCallback.
  nonTomExpectedZclResps = 1;
  nonTomResponsesCount = 0;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dstId);
  return status;
}

/* @brief check if a GBZ ZCL command matches the corresponding command
 * response
 *
 * return true if pair matches. Otherwise, false.
 * */
static bool matchingZclCommands(EmberAfGbzZclCommand * cmd, EmberAfGbzZclCommand  * cmdResp)
{
  bool result = true;

  // check command direction
  if ((cmd->frameControl & ZCL_FRAME_CONTROL_DIRECTION_MASK) == (cmdResp->frameControl & ZCL_FRAME_CONTROL_DIRECTION_MASK)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: ERR: No matching ZCL command direction (0x%X/0x%X) in ZCL cmd/cmd resp pair!",
                                         cmd->frameControl & ZCL_FRAME_CONTROL_DIRECTION_MASK,
                                         cmdResp->frameControl & ZCL_FRAME_CONTROL_DIRECTION_MASK);
    result = false;
  }

  if (cmd->transactionSequenceNumber != cmdResp->transactionSequenceNumber) {
    emberAfPluginGasProxyFunctionPrintln("GPF: ERR: No matching ZCL tran seq number (0x%X/0x%X) in ZCL cmd/cmd resp pair!", cmd->transactionSequenceNumber, cmdResp->transactionSequenceNumber);
    result = false;
  }

  if (cmd->clusterId != cmdResp->clusterId) {
    emberAfPluginGasProxyFunctionPrintln("GPF: ERR: No matching ZCL cluster id (%2X/%2X) in ZCL cmd/cmd resp pair!", cmd->clusterId, cmdResp->clusterId);
    result = false;
  }

  // the command id doesn't always matches in value (e.g. read attr(0x00) vs read attr resp (0x01)).
  // this is not being checked for now.
  /*if (cmd->commandId != cmdResp->commandId) {*/
  /*emberAfPluginGasProxyFunctionPrintln("GPF: ERR: No matching ZCL command id (%x/%x) in ZCL cmd/cmd resp pair!", cmd->commandId, cmdResp->commandId);*/
  /*result = false;*/
  /*}*/

  return result;
}

/*
 * @brief Send a Tap Off Message (TOM) to the GPF
 *
 * This function can be used to send a Tap Off Message (TOM) to the Gas
 * Proxy Function (GPF) for processing. The message has been tapped off
 * and validated by the gas meter (GSME), so that now it can be applied to the GPF.
 *
 * @param gbzCommands  a pointer to the complete GBZ command data.
 * @param gbzCommandsLength the length of the GBZ command.
 * @param gbzCommandsResponse a pointer to all GBZ responses generated by the GSME.
 * @param gbzCommandsResponseLength the length of the GBZ responses.
 *
 * @return EMBER_SUCCESS if all messages have been successfully parsed, a non-zero
 *   status code on error.
 */
EmberStatus emberAfPluginGasProxyFunctionTapOffMessageHandler(uint8_t * gbzCommands,
                                                              uint16_t  gbzCommandsLength,
                                                              uint8_t * gbzCommandsResponse,
                                                              uint16_t  gbzCommandsResponseLength,
                                                              uint16_t  messageCode)
{
  EmberAfGbzMessageParserState gbzCommandHandler;
  EmberAfGbzMessageParserState gbzCommandResponseHandler;
  EmberStatus status;

  emberAfPluginGbzMessageControllerParserInit(&gbzCommandHandler,
                                              EMBER_AF_GBZ_MESSAGE_COMMAND,
                                              gbzCommands,
                                              gbzCommandsLength,
                                              false,
                                              messageCode);
  emberAfPluginGbzMessageControllerParserInit(&gbzCommandResponseHandler,
                                              EMBER_AF_GBZ_MESSAGE_RESPONSE,
                                              gbzCommandsResponse,
                                              gbzCommandsResponseLength,
                                              false,
                                              messageCode);

  if (emberAfPluginGbzMessageControllerGetComponentSize(&gbzCommandHandler)
      != emberAfPluginGbzMessageControllerGetComponentSize(&gbzCommandResponseHandler)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: ERR: GBZ command / command response does not have same number of components!");
    status = EMBER_BAD_ARGUMENT;
    goto kickout;
  }

  currentGpfMessage.gbzCommands = gbzCommands;
  currentGpfMessage.gbzCommandsLength = gbzCommandsLength;
  currentGpfMessage.gbzCommandsResponse = gbzCommandsResponse;
  currentGpfMessage.gbzCommandsResponseLength = gbzCommandsResponseLength;
  currentGpfMessage.messageCode = messageCode;

  while (emberAfPluginGbzMessageControllerHasNextCommand(&gbzCommandHandler)) {
    EmberAfGbzZclCommand cmd;
    EmberAfGbzZclCommand cmdResp;
    emberAfPluginGbzMessageControllerNextCommand(&gbzCommandHandler, &cmd);
    emberAfPluginGbzMessageControllerNextCommand(&gbzCommandResponseHandler, &cmdResp);

    if (matchingZclCommands(&cmd, &cmdResp)
        && emberAfPluginGbzMessageControllerGetZclDefaultResponse(&cmdResp) == EMBER_ZCL_STATUS_SUCCESS) {
      EmberApsFrame apsFrame;
      EmberAfClusterCommand clusterCmd;
      MEMSET(&apsFrame, 0x00, sizeof(EmberApsFrame));
      MEMSET(&clusterCmd, 0x00, sizeof(EmberAfClusterCommand));
      emberAfPluginGasProxyFunctionPrintln("GPF: Updating ESI endpoint with following info: ");
      emberAfPluginGbzMessageControllerPrintCommandInfo(&cmd);

      apsFrame.profileId = gbzCommandHandler.profileId;
      apsFrame.clusterId = cmd.clusterId;
      clusterCmd.commandId = cmd.commandId;

      // we mangle the zcl dst ep to a proxy endpoint to intercept the server to client side
      // commands. however, certain commands (Events: ClearEventLogRequest)
      // doesn't need to be redirected to the proxy ep thanks to the correct
      // zcl direction.
      if ((cmd.clusterId == ZCL_EVENTS_CLUSTER_ID)
          && (cmd.commandId == ZCL_CLEAR_EVENT_LOG_REQUEST_COMMAND_ID)) {
        // this TOM cmd is directed at the GSME
        apsFrame.destinationEndpoint = (EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT);
      } else {
        apsFrame.destinationEndpoint = (EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT);
      }

      clusterCmd.apsFrame = &apsFrame;
      clusterCmd.seqNum = cmd.transactionSequenceNumber;
      clusterCmd.clusterSpecific = cmd.clusterSpecific;
      clusterCmd.mfgSpecific = cmd.mfgSpecific;
      clusterCmd.direction = cmd.direction;
      clusterCmd.buffer = cmd.payload;
      clusterCmd.bufLen = cmd.payloadLength;
      clusterCmd.payloadStartIndex = 0;

      status = emberAfPluginGasProxyFunctionValidateIncomingZclCommandCallback(&clusterCmd,
                                                                               messageCode);
      if (status == EMBER_AF_GPF_ZCL_COMMAND_PERMISSION_ALLOWED) {
        EmberAfStatus parsingStatus;
        emberAfCurrentCommand() = &clusterCmd;
        emberAfPluginGasProxyFunctionPrintln("GPF: Passing cmd to cluster parser");
        parsingStatus = emberAfClusterSpecificCommandParse(&clusterCmd);
        if (parsingStatus != EMBER_ZCL_STATUS_SUCCESS) {
          emberAfPluginGasProxyFunctionPrintln("GPF: ERR: Unable to apply ZCL command (error: 0x%X)!", parsingStatus);
          status = EMBER_ERR_FATAL;
          goto kickout;
        }
      } else if (status == EMBER_AF_GPF_ZCL_COMMAND_PERMISSION_IGNORED) {
        emberAfPluginGasProxyFunctionPrintln("GPF: Info: Command ignored: ZCL command(clus 0x%2X, cmd 0x%x) embedded within Tap Off Message.", clusterCmd.apsFrame->clusterId, clusterCmd.commandId);
      } else {
        // only EMBER_AF_GPF_ZCL_COMMAND_PERMISSION_NOT_ALLOWED is expected to be
        // here.
        emberAfPluginGasProxyFunctionPrintln("GPF: ERR: Unsupported ZCL command(clus 0x%2X, cmd 0x%x) embedded within Tap Off Message!", clusterCmd.apsFrame->clusterId, clusterCmd.commandId);
        status = EMBER_ERR_FATAL;
        goto kickout;
      }
    } else {
      emberAfPluginGasProxyFunctionPrintln("GPF: ERR: Unable to process ZCL cmd/resp pair with trans seq number (%d)",
                                           cmd.transactionSequenceNumber);
      status = EMBER_ERR_FATAL;
      goto kickout;
    }
  }

  status = EMBER_SUCCESS;

  kickout:

  emberAfPluginGbzMessageControllerParserCleanup(&gbzCommandHandler);
  emberAfPluginGbzMessageControllerParserCleanup(&gbzCommandResponseHandler);

  MEMSET(&currentGpfMessage, 0x00, sizeof(EmberAfGpfMessage));

  if (status == EMBER_SUCCESS) {
    emberAfPluginGasProxyFunctionPrintln("GPF: TOM message has been successfully processed and will be logged to Event cluster on ep(%d).",
                                         EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT);
  }

  // time to log these lovely TOM commands to the Event log.
  emAfPluginGasProxyFunctionLogEvent(gbzCommands,
                                     gbzCommandsLength,
                                     gbzCommandsResponse,
                                     gbzCommandsResponseLength,
                                     (status == EMBER_SUCCESS)
                                     ?  GBCS_EVENT_ID_IMM_HAN_CMD_RXED_ACTED
                                     : GBCS_EVENT_ID_IMM_HAN_CMD_RXED_NOT_ACTED,
                                     EMBER_AF_GPF_MESSAGE_TYPE_TOM,
                                     messageCode);

  emberAfCurrentCommand() = NULL;
  return status;
}

static bool sendNextNonTomZclCmd(void)
{
  EmberAfStatus status;
  EmberAfGbzZclCommand cmd;

  if (!emberAfPluginGbzMessageControllerHasNextCommand(&nonTomGbzRequestParser)) {
    return false;
  }

  emberAfPluginGbzMessageControllerNextCommand(&nonTomGbzRequestParser, &cmd);
  nonTomGbzStartTime = (cmd.hasFromDateTime) ? cmd.fromDateTime : 0;
  if (cmd.clusterId == ZCL_PRICE_CLUSTER_ID
      || cmd.clusterId == ZCL_EVENTS_CLUSTER_ID
      || cmd.clusterId == ZCL_CALENDAR_CLUSTER_ID) {
    status = emberAfPluginGasProxyFunctionReplayZclCommand(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                           EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                                           &cmd);
  } else if (cmd.clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID
             || cmd.clusterId == ZCL_PREPAYMENT_CLUSTER_ID) {
    status = emberAfPluginGasProxyFunctionReplayZclCommand(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                           EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT,
                                                           &cmd);
  } else if (cmd.clusterId == ZCL_BASIC_CLUSTER_ID) {
    // EMAPPFWKV2-1308 - GCS21e (messageCode = 0x009E) wants us to read the
    // Basic cluster data from the mirror.
    status = emberAfPluginGasProxyFunctionReplayZclCommand(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                           (nonTomGbzRequestParser.messageCode == 0x009E
                                                            ? EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT
                                                            : EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT),
                                                           &cmd);
  } else if (cmd.clusterId == ZCL_DEVICE_MANAGEMENT_CLUSTER_ID ) {
    if ((cmd.commandId == ZCL_PUBLISH_CHANGE_OF_TENANCY_COMMAND_ID)
        && (cmd.direction == ZCL_DIRECTION_SERVER_TO_CLIENT)) {
      status = emberAfPluginGasProxyFunctionReplayZclCommand(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                             EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                             &cmd);
    } else {
      status = emberAfPluginGasProxyFunctionReplayZclCommand(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                             EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                                             &cmd);
    }
  } else {
    emberAfPluginGasProxyFunctionPrintln("GPF: ERR: Replaying following command");
    emberAfPluginGbzMessageControllerPrintCommandInfo(&cmd);
    status = EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
  }

  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPluginGasProxyFunctionPrintln("GPF: ERR: Unable to replay ZCL command (error: 0x%x)!", status);
    return false;
  }

  return true;
}

uint32_t emAfGasProxyFunctionGetGbzStartTime(void)
{
  return (!nonTomHandlingActive) ? 0 : nonTomGbzStartTime;
}

void emAfGasProxyFunctionAlert(uint16_t alertCode,
                               EmberAfClusterCommand *cmd,
                               uint16_t messageCode)
{
  EmberAfGbzZclCommand gbzZclCmd = { 0 };
  EmberAfGbzMessageCreatorState creator;
  EmberAfGbzMessageCreatorResult *gbzResponse;

  gbzZclCmd.clusterId = cmd->apsFrame->clusterId;
  gbzZclCmd.frameControl = cmd->buffer[0]; // first byte of the aps payload is the frame control
  gbzZclCmd.transactionSequenceNumber = 0;
  gbzZclCmd.commandId = cmd->commandId;
  gbzZclCmd.payload = &cmd->buffer[cmd->payloadStartIndex];
  gbzZclCmd.payloadLength = cmd->bufLen - cmd->payloadStartIndex;
  gbzZclCmd.direction = cmd->direction;
  gbzZclCmd.clusterSpecific = cmd->clusterSpecific;
  gbzZclCmd.mfgSpecific = cmd->mfgSpecific;
  gbzZclCmd.hasFromDateTime = false;
  gbzZclCmd.encryption = false;

  emberAfPluginGbzMessageControllerCreatorInit(&creator,
                                               EMBER_AF_GBZ_MESSAGE_ALERT,
                                               alertCode,
                                               emberAfGetCurrentTime(),
                                               messageCode,
                                               NULL,
                                               0);
  emberAfPluginGbzMessageControllerAppendCommand(&creator,
                                                 &gbzZclCmd);
  gbzResponse = emberAfPluginGbzMessageControllerCreatorAssemble(&creator);

  emberAfPluginGasProxyFunctionPrintln("GPF: Calling Alert WAN callback with the following ZCL command");
  emberAfPluginGbzMessageControllerPrintCommandInfo(&gbzZclCmd);

  emberAfPluginGasProxyFunctionAlertWANCallback(alertCode, gbzResponse->payload, gbzResponse->payloadLength);
  gbzResponse->freeRequired = false;

  emberAfPluginGbzMessageControllerCreatorCleanup(&creator);
}

/**
 * @brief Send a Non Tap Off Message (Non-TOM) to the GPF
 *
 * This function can be used to send a Non Tap Off Message (Non-TOM) to the Gas
 * Proxy Function (GPF) for processing. Each embedded ZCL command, within the
 * GBZ message, will be sent to the local mirror and the corresponding responses
 * will be written out to the provided response buffer. Once all responses have been collected,
 * emberAfPluginGasProxyFunctionNonTapOffMessageHandlerCompletedCallback() will be invoked.
 *
 * @param gbzCommands       a pointer to the complete GBZ command data.
 * @param gbzCommandsLength the length of the GBZ command.
 * @param messageCode       "Message Code" for the corresponding Non TOM command.
 *
 * @return EMBER_SUCCESS if all messages have been successfully parsed, a non-zero
 *                       status code on error.
 *
 */
EmberStatus emberAfPluginGasProxyFunctionNonTapOffMessageHandler(uint8_t * gbzCommands,
                                                                 uint16_t  gbzCommandsLength,
                                                                 uint16_t  messageCode)
{
  EmberStatus status = EMBER_SUCCESS;

  if (nonTomHandlingActive) {
    emberAfPluginGasProxyFunctionPrintln("GPF: cannot process two non tap off messages at the same time");
    // we are returning directly so no internal bookkeeping state is changed.
    return EMBER_INVALID_CALL;
  }
  if (!emberAfPluginGbzMessageControllerParserInit(&nonTomGbzRequestParser,
                                                   EMBER_AF_GBZ_MESSAGE_COMMAND,
                                                   gbzCommands,
                                                   gbzCommandsLength,
                                                   true,
                                                   messageCode)) {
    status = EMBER_ERR_FATAL;
    goto kickout;
  }

  if (!emberAfPluginGbzMessageControllerCreatorInit(&nonTomGbzResponseCreator,
                                                    EMBER_AF_GBZ_MESSAGE_RESPONSE,
                                                    0,
                                                    0,
                                                    messageCode,
                                                    NULL,
                                                    0)) {
    status = EMBER_ERR_FATAL;
    goto kickout;
  }

  nonTomHandlingActive = true;
  status = sendNextNonTomZclCmd() ? EMBER_SUCCESS : EMBER_ERR_FATAL;

  kickout:
  if (status != EMBER_SUCCESS) {
    emberAfPluginGbzMessageControllerParserCleanup(&nonTomGbzRequestParser);
    emberAfPluginGbzMessageControllerCreatorCleanup(&nonTomGbzResponseCreator);
    nonTomExpectedZclResps = 0;
    nonTomResponsesCount = 0;
    nonTomHandlingActive = false;

    // Log Non-Actioned Non-TOM
    emAfPluginGasProxyFunctionLogEvent(gbzCommands,
                                       gbzCommandsLength,
                                       NULL,
                                       0,
                                       GBCS_EVENT_ID_IMM_HAN_CMD_RXED_NOT_ACTED,
                                       EMBER_AF_GPF_MESSAGE_TYPE_NON_TOM,
                                       messageCode);
  }
  return status;
}

/*
 * @ brief Stores captured Zcl Command Responses internally.
 *
 *   If the response payloads contain user sensitive material,
 *   the payloads are encrypted before being stored.
 *
 */
static void captureNonTomZclCmdResp(EmberAfClusterCommand * cmd)
{
  EmberAfGbzZclCommand gbzZclCmd = { 0 };
  EmberAfGbzMessageCreatorResult *gbzResponse;
  emberAfPluginGasProxyFunctionPrintln("GPF: Intercepting ZCL cmd(0x%X) on endpoint: %d", cmd->commandId, emberAfCurrentEndpoint());

  if (!nonTomHandlingActive) {
    emberAfPluginGasProxyFunctionPrintln("GPF: ERR: Not expecting a Non-TOM response.");
    return;
  }

  gbzZclCmd.clusterId = cmd->apsFrame->clusterId;
  gbzZclCmd.frameControl = cmd->buffer[0]; // first byte of the aps payload is the frame control
  gbzZclCmd.transactionSequenceNumber = cmd->seqNum;
  gbzZclCmd.commandId = cmd->commandId;
  gbzZclCmd.payload = &cmd->buffer[cmd->payloadStartIndex];
  gbzZclCmd.payloadLength = cmd->bufLen - cmd->payloadStartIndex;
  gbzZclCmd.direction = cmd->direction;
  gbzZclCmd.clusterSpecific = cmd->clusterSpecific;
  gbzZclCmd.mfgSpecific = cmd->mfgSpecific;
  gbzZclCmd.hasFromDateTime = false;
  gbzZclCmd.encryption = false;

  emberAfPluginGasProxyFunctionPrintln("GPF: Saving following ZCL response for Non-TOM message response.");
  emberAfPluginGbzMessageControllerPrintCommandInfo(&gbzZclCmd);

  emberAfPluginGbzMessageControllerAppendCommand(&nonTomGbzResponseCreator,
                                                 &gbzZclCmd);

  // allocate final GBZ Non-TOM response buffer
  if (nonTomExpectedZclResps == ++nonTomResponsesCount
      && !sendNextNonTomZclCmd()) {
    gbzResponse = emberAfPluginGbzMessageControllerCreatorAssemble(&nonTomGbzResponseCreator);

    emberAfPluginGasProxyFunctionPrintln("GPF: Total length for GBZ response: %d", gbzResponse->payloadLength);
    emberAfPluginGasProxyFunctionPrint("GPF: GBZ response payload: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzResponse->payload, gbzResponse->payloadLength, true);
    emberAfPluginGasProxyFunctionPrintln("");
    emberAfPluginGasProxyFunctionNonTapOffMessageHandlerCompletedCallback(gbzResponse->payload, gbzResponse->payloadLength);
    gbzResponse->freeRequired = false;

    // Log Actioned Non TOM
    emAfPluginGasProxyFunctionLogEvent(nonTomGbzRequestParser.command,
                                       nonTomGbzRequestParser.length,
                                       gbzResponse->payload,
                                       gbzResponse->payloadLength,
                                       GBCS_EVENT_ID_IMM_HAN_CMD_RXED_ACTED,
                                       EMBER_AF_GPF_MESSAGE_TYPE_NON_TOM,
                                       nonTomGbzRequestParser.messageCode);

    emberAfPluginGbzMessageControllerParserCleanup(&nonTomGbzRequestParser);
    emberAfPluginGbzMessageControllerCreatorCleanup(&nonTomGbzResponseCreator);
    nonTomExpectedZclResps = 0;
    nonTomResponsesCount = 0;
    nonTomHandlingActive = false;
  }
}

/* @brief Capture ZCL response from replayed Non-TOM messages. The captured responses
 * is packed up and delivered to the application via a GBZ message buffer
 */
bool emAfPluginGasProxyFunctionPreCommandReceivedCallback(EmberAfClusterCommand* cmd)
{
  if (!nonTomHandlingActive
      || (cmd->source != emberGetNodeId())) {
    return false;
  }

  if (cmd->apsFrame->destinationEndpoint == EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT) {
    if ((cmd->apsFrame->sourceEndpoint == EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT)
        || (cmd->apsFrame->sourceEndpoint == EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT)) {
      captureNonTomZclCmdResp(cmd);
      return true;
    } else if ((cmd->apsFrame->sourceEndpoint == EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT)
               && (cmd->commandId == ZCL_DEFAULT_RESPONSE_COMMAND_ID)) {
      captureNonTomZclCmdResp(cmd);
      return true;
    }
  }
  return false;
}

bool emberAfPriceClusterPublishCalorificValueCallback(uint32_t issuerEventId,
                                                      uint32_t startTime,
                                                      uint32_t calorificValue,
                                                      uint8_t calorificValueUnit,
                                                      uint8_t calorificValueTrailingDigit)
{
  EmberAfStatus status;

  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishCalorificValue 0x%4X, 0x%4X, 0x%4X, 0x%X, 0x%X",
                                       issuerEventId,
                                       startTime,
                                       calorificValue,
                                       calorificValueUnit,
                                       calorificValueTrailingDigit);

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(startTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  status = emberAfPluginPriceServerCalorificValueAdd(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                                     issuerEventId,
                                                     startTime,
                                                     calorificValue,
                                                     calorificValueUnit,
                                                     calorificValueTrailingDigit);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPluginGasProxyFunctionPrintln("GPF: ERR: Unable to update calorific value (status:0x%X).", status);
  }
  return true;
}

bool emberAfPriceClusterPublishConversionFactorCallback(uint32_t issuerEventId,
                                                        uint32_t startTime,
                                                        uint32_t conversionFactor,
                                                        uint8_t conversionFactorTrailingDigit)
{
  EmberStatus status;

  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishConversionFactor 0x%4X, 0x%4X, 0x%4X, 0x%X",
                                       issuerEventId,
                                       startTime,
                                       conversionFactor,
                                       conversionFactorTrailingDigit);

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(startTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  status = emberAfPluginPriceServerConversionFactorAdd(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                                       issuerEventId,
                                                       startTime,
                                                       conversionFactor,
                                                       conversionFactorTrailingDigit);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPluginGasProxyFunctionPrintln("GPF: ERR: Unable to update conversion factor (status:0x%X).", status);
  }
  return true;
}

bool emberAfCalendarClusterPublishCalendarCallback(uint32_t providerId,
                                                   uint32_t issuerEventId,
                                                   uint32_t issuerCalendarId,
                                                   uint32_t startTime,
                                                   uint8_t  calendarType,
                                                   uint8_t  calendarTimeReference,
                                                   uint8_t  *calendarName,
                                                   uint8_t  numberOfSeasons,
                                                   uint8_t  numberOfWeekProfiles,
                                                   uint8_t  numberOfDayProfiles)
{
  bool status;

  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishCalendar 0x%4x, 0x%4x, 0x%4x, 0x%4x, 0x%x, \"",
                                       providerId,
                                       issuerEventId,
                                       issuerCalendarId,
                                       startTime,
                                       calendarType);
  emberAfPluginGasProxyFunctionPrintString(calendarName);
  emberAfPluginGasProxyFunctionPrintln("\", %d, %d, %d",
                                       numberOfSeasons,
                                       numberOfWeekProfiles,
                                       numberOfDayProfiles);

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(startTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  status = emberAfCalendarCommonAddCalInfo(providerId,
                                           issuerEventId,
                                           issuerCalendarId,
                                           startTime,
                                           calendarType,
                                           calendarName,
                                           numberOfSeasons,
                                           numberOfWeekProfiles,
                                           numberOfDayProfiles);

  if (status) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Updated: Calendar");
    emberAfPluginGasProxyFunctionPrintln("GPF:          providerId: 0x%4X", providerId);
    emberAfPluginGasProxyFunctionPrintln("GPF:          issuerEventId: 0x%4X", issuerEventId);
    emberAfPluginGasProxyFunctionPrintln("GPF:          issuerCalendarId: 0x%4X", issuerCalendarId);
    emberAfPluginGasProxyFunctionPrintln("GPF:          startTimeUtc: 0x%4X", startTime);
    emberAfPluginGasProxyFunctionPrintln("GPF:          calendarType: 0x%X", calendarType);
    emberAfPluginGasProxyFunctionPrint("GPF:          calendarName: ");
    emberAfPluginGasProxyFunctionPrintString(calendarName);
    emberAfPluginGasProxyFunctionPrintln("");
    emberAfPluginGasProxyFunctionPrintln("GPF:          numberOfSeasons: 0x%X", numberOfSeasons);
    emberAfPluginGasProxyFunctionPrintln("GPF:          numberOfWeekProfiles: 0x%X", numberOfWeekProfiles);
    emberAfPluginGasProxyFunctionPrintln("GPF:          numberOfDayProfiles: 0x%X", numberOfDayProfiles);
  }

  return true;
}

bool emberAfCalendarClusterPublishDayProfileCallback(uint32_t providerId,
                                                     uint32_t issuerEventId,
                                                     uint32_t issuerCalendarId,
                                                     uint8_t dayId,
                                                     uint8_t totalNumberOfScheduleEntries,
                                                     uint8_t commandIndex,
                                                     uint8_t totalNumberOfCommands,
                                                     uint8_t calendarType,
                                                     uint8_t *dayScheduleEntries)
{
//[[
//  TODO: sanity checking on totalNumberOfScheduleEntries/commandIndex/totalNumberOfCommands
//  to verify the command count and etc. For now, we are just blindly appending
//  day profiles to the calendar server.
//]]
  bool status;
  uint16_t dayScheduleEntriesLength = fieldLength(dayScheduleEntries);

  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishDayProfile 0x%4x, 0x%4x, 0x%4x, %d, %d, %d, %d, 0x%x",
                                       providerId,
                                       issuerEventId,
                                       issuerCalendarId,
                                       dayId,
                                       totalNumberOfScheduleEntries,
                                       commandIndex,
                                       totalNumberOfCommands,
                                       calendarType);

  status = emberAfCalendarCommonAddDayProfInfo(issuerCalendarId,
                                               dayId,
                                               dayScheduleEntries,
                                               dayScheduleEntriesLength);
  return true;
}

/** @brief Publish Info
 *
 * This function is called by the calendar-server plugin after receiving any of
 * the following commands and just before it starts publishing the response:
 * GetCalendar, GetDayProfiles, GetSeasons, GetSpecialDays, and GetWeekProfiles.
 *
 * @param publishCommandId ZCL command to be published  Ver.: always
 * @param clientNodeId Destination nodeId  Ver.: always
 * @param clientEndpoint Destination endpoint  Ver.: always
 * @param totalCommands Total number of publish commands to be sent  Ver.:
 * always
 */
void emberAfPluginCalendarServerPublishInfoCallback(uint8_t publishCommandId,
                                                    EmberNodeId clientNodeId,
                                                    uint8_t clientEndpoint,
                                                    uint8_t totalCommands)
{
  // If the publish is a result of a non-TOM use case then the clientNodeId
  // will be our nodeId and if so we need to update the number of expected
  // response to include all of the publish commands to be sent. We subtract
  // one from the total commands as we always expect at least one response
  // when we initialize the expected responses count.
  if (nonTomHandlingActive && (clientNodeId == emberGetNodeId())) {
    uint8_t expectedZclResps = nonTomExpectedZclResps + (totalCommands - 1);
    emberAfPluginGasProxyFunctionPrintln("GPF: Updating number of Non-TOM expected responses from %d to %d",
                                         nonTomExpectedZclResps,
                                         expectedZclResps);
    nonTomExpectedZclResps = expectedZclResps;
  }
}

bool emberAfPriceClusterPublishBlockThresholdsCallback(uint32_t providerId,
                                                       uint32_t issuerEventId,
                                                       uint32_t startTime,
                                                       uint32_t issuerTariffId,
                                                       uint8_t commandIndex,
                                                       uint8_t numberOfCommands,
                                                       uint8_t subpayloadControl,
                                                       uint8_t* payload)
{
  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishBlockThresholds 0x%4X, 0x%4X, 0x%4X, 0x%4X, 0x%X, 0x%X, 0x%X",
                                       providerId,
                                       issuerEventId,
                                       startTime,
                                       issuerTariffId,
                                       commandIndex,
                                       numberOfCommands,
                                       subpayloadControl);

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(startTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  emberAfPriceAddBlockThresholdsTableEntry(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                           providerId,
                                           issuerEventId,
                                           startTime,
                                           issuerTariffId,
                                           commandIndex,
                                           numberOfCommands,
                                           subpayloadControl,
                                           payload);

  return true;
}

bool emberAfPriceClusterPublishTariffInformationCallback(uint32_t providerId,
                                                         uint32_t issuerEventId,
                                                         uint32_t issuerTariffId,
                                                         uint32_t startTime,
                                                         uint8_t tariffTypeChargingScheme,
                                                         uint8_t *tariffLabel,
                                                         uint8_t numberOfPriceTiersInUse,
                                                         uint8_t numberOfBlockThresholdsInUse,
                                                         uint8_t unitOfMeasure,
                                                         uint16_t currency,
                                                         uint8_t priceTrailingDigit,
                                                         uint32_t standingCharge,
                                                         uint8_t tierBlockMode,
                                                         uint32_t blockThresholdMultiplier,
                                                         uint32_t blockThresholdDivisor)
{
  EmberAfPriceCommonInfo info;
  EmberAfScheduledTariff tariff;

  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishTariffInformationReceived");
  emberAfPriceClusterPrint("RX: PublishTariffInformation 0x%4x, 0x%4x, 0x%4x, 0x%4x, 0x%x, \"",
                           providerId,
                           issuerEventId,
                           issuerTariffId,
                           startTime,
                           tariffTypeChargingScheme);

  emberAfPriceClusterPrintString(tariffLabel);
  emberAfPriceClusterPrint("\"");
  emberAfPriceClusterPrint(", 0x%x, 0x%x, 0x%x, 0x%2x, 0x%x",
                           numberOfPriceTiersInUse,
                           numberOfBlockThresholdsInUse,
                           unitOfMeasure,
                           currency,
                           priceTrailingDigit);
  emberAfPriceClusterPrintln(", 0x%4x, 0x%x, 0x%4x, 0x%4x",
                             standingCharge,
                             tierBlockMode,
                             blockThresholdMultiplier,
                             blockThresholdDivisor);
  emberAfPriceClusterFlush();

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(startTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  info.startTime = startTime;
  info.issuerEventId = issuerEventId;
  tariff.providerId = providerId;
  tariff.issuerTariffId = issuerTariffId;
  tariff.tariffTypeChargingScheme = tariffTypeChargingScheme;
  emberAfCopyString(tariff.tariffLabel, tariffLabel, ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH);
  tariff.numberOfPriceTiersInUse = numberOfPriceTiersInUse;
  tariff.numberOfBlockThresholdsInUse = numberOfBlockThresholdsInUse;
  tariff.unitOfMeasure = unitOfMeasure;
  tariff.currency = currency;
  tariff.priceTrailingDigit = priceTrailingDigit;
  tariff.standingCharge = standingCharge;
  tariff.tierBlockMode = tierBlockMode;
  tariff.blockThresholdMultiplier = blockThresholdMultiplier;
  tariff.blockThresholdDivisor = blockThresholdDivisor;
  tariff.status |= FUTURE;

  emberAfPriceAddTariffTableEntry(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                  &info,
                                  &tariff);
  return true;
}

bool emberAfPriceClusterPublishPriceMatrixCallback(uint32_t providerId,
                                                   uint32_t issuerEventId,
                                                   uint32_t startTime,
                                                   uint32_t issuerTariffId,
                                                   uint8_t commandIndex,
                                                   uint8_t numberOfCommands,
                                                   uint8_t subPayloadControl,
                                                   uint8_t* payload)
{
  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishPriceMatrix 0x%4X, 0x%4X, 0x%4X, 0x%4X, 0x%X, 0x%X, 0x%X",
                                       providerId,
                                       issuerEventId,
                                       startTime,
                                       issuerTariffId,
                                       commandIndex,
                                       numberOfCommands,
                                       subPayloadControl);

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(startTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  emberAfPriceAddPriceMatrixRaw(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                providerId,
                                issuerEventId,
                                startTime,
                                issuerTariffId,
                                commandIndex,
                                numberOfCommands,
                                subPayloadControl,
                                payload);

  return true;
}

bool emberAfCalendarClusterPublishSeasonsCallback(uint32_t providerId,
                                                  uint32_t issuerEventId,
                                                  uint32_t issuerCalendarId,
                                                  uint8_t commandIndex,
                                                  uint8_t totalNumberOfCommands,
                                                  uint8_t *seasonEntries)
{
//[[
//  TODO: sanity checking on totalNumberOfScheduleEntries/commandIndex/totalNumberOfCommands
//  to verify the command count and etc. For now, we are just blindly appending
//  day profiles to the calendar server.
//]]
  bool success;
  uint16_t seasonEntriesLength = fieldLength(seasonEntries);
  uint8_t unknownWeekIdSeasonsMask = 0;

  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishSeasons 0x%4x, 0x%4x, 0x%4x, %d, %d",
                                       providerId,
                                       issuerEventId,
                                       issuerCalendarId,
                                       commandIndex,
                                       totalNumberOfCommands);

  success = emberAfCalendarServerAddSeasonsInfo(issuerCalendarId,
                                                seasonEntries,
                                                seasonEntriesLength,
                                                &unknownWeekIdSeasonsMask);

  if (!success && unknownWeekIdSeasonsMask != 0) {
    emberAfPluginGasProxyFunctionUnknownSeasonWeekIdCallback(issuerCalendarId,
                                                             seasonEntries,
                                                             seasonEntriesLength,
                                                             unknownWeekIdSeasonsMask);
  }

  return true;
}

bool emberAfCalendarClusterPublishSpecialDaysCallback(uint32_t providerId,
                                                      uint32_t issuerEventId,
                                                      uint32_t issuerCalendarId,
                                                      uint32_t startTime,
                                                      uint8_t calendarType,
                                                      uint8_t totalNumberOfSpecialDays,
                                                      uint8_t commandIndex,
                                                      uint8_t totalNumberOfCommands,
                                                      uint8_t *specialDayEntries)
{
  uint16_t speicalDaysEntriesLength = fieldLength(specialDayEntries);
  uint8_t unknownSpecialDaysMask = 0;
  bool success;

  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishSpecialDays 0x%4x, 0x%4x, 0x%4x, 0x%4x, 0x%x, %d, %d, %d, [",
                                       providerId,
                                       issuerEventId,
                                       issuerCalendarId,
                                       startTime,
                                       calendarType,
                                       totalNumberOfSpecialDays,
                                       commandIndex,
                                       totalNumberOfCommands);
  // TODO: print specialDayEntries
  emberAfPluginGasProxyFunctionPrintln("]");

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(startTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  success = emberAfCalendarCommonAddSpecialDaysInfo(issuerCalendarId,
                                                    totalNumberOfSpecialDays,
                                                    specialDayEntries,
                                                    speicalDaysEntriesLength,
                                                    &unknownSpecialDaysMask);
  if (!success && unknownSpecialDaysMask != 0) {
    emberAfPluginGasProxyFunctionUnknownSpecialDaysDayIdCallback(issuerCalendarId,
                                                                 specialDayEntries,
                                                                 speicalDaysEntriesLength,
                                                                 unknownSpecialDaysMask);
  }

  return true;
}

bool emberAfCalendarClusterPublishWeekProfileCallback(uint32_t providerId,
                                                      uint32_t issuerEventId,
                                                      uint32_t issuerCalendarId,
                                                      uint8_t weekId,
                                                      uint8_t dayIdRefMonday,
                                                      uint8_t dayIdRefTuesday,
                                                      uint8_t dayIdRefWednesday,
                                                      uint8_t dayIdRefThursday,
                                                      uint8_t dayIdRefFriday,
                                                      uint8_t dayIdRefSaturday,
                                                      uint8_t dayIdRefSunday)
{
  bool status;
  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishWeekProfile 0x%4x, 0x%4x, 0x%4x, %d, %d, %d, %d, %d, %d, %d, %d",
                                       providerId,
                                       issuerEventId,
                                       issuerCalendarId,
                                       weekId,
                                       dayIdRefMonday,
                                       dayIdRefTuesday,
                                       dayIdRefWednesday,
                                       dayIdRefThursday,
                                       dayIdRefFriday,
                                       dayIdRefSaturday,
                                       dayIdRefSunday);

  status = emberAfCalendarServerAddWeekProfInfo(issuerCalendarId,
                                                weekId,
                                                dayIdRefMonday,
                                                dayIdRefTuesday,
                                                dayIdRefWednesday,
                                                dayIdRefThursday,
                                                dayIdRefFriday,
                                                dayIdRefSaturday,
                                                dayIdRefSunday);
  return true;
}

bool emberAfMessagingClusterDisplayMessageCallback(uint32_t messageId,
                                                   uint8_t messageControl,
                                                   uint32_t startTime,
                                                   uint16_t durationInMinutes,
                                                   uint8_t* msg,
                                                   uint8_t optionalExtendedMessageControl)
{
  EmberAfPluginMessagingServerMessage message;
  uint8_t msgLength = emberAfStringLength(msg) + 1;

  if (msgLength > EMBER_AF_PLUGIN_MESSAGING_SERVER_MESSAGE_SIZE) {
    emberAfPluginGasProxyFunctionPrint("GPF: ERR: Message too long for messaging server message buffer.");
    return true;
  }

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(startTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  message.messageId = messageId;
  message.messageControl = messageControl;
  message.startTime = startTime;
  message.durationInMinutes = durationInMinutes;
  MEMCOPY(message.message, msg, msgLength);
  message.extendedMessageControl = optionalExtendedMessageControl;

  emberAfPluginGasProxyFunctionPrint("GPF: RX: DisplayMessage"
                                     " 0x%4x, 0x%x, 0x%4x, 0x%2x, \"",
                                     messageId,
                                     messageControl,
                                     startTime,
                                     durationInMinutes);
  emberAfPluginGasProxyFunctionPrintString(msg);
  emberAfPluginGasProxyFunctionPrint(", 0x%X", optionalExtendedMessageControl);
  emberAfPluginGasProxyFunctionPrintln("\"");

  emberAfPluginMessagingServerSetMessage(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                         &message);
  return true;
}

bool emberAfDeviceManagementClusterPublishChangeOfSupplierCallback(uint32_t currentProviderId,
                                                                   uint32_t issuerEventId,
                                                                   uint8_t tariffType,
                                                                   uint32_t proposedProviderId,
                                                                   uint32_t providerChangeImplementationTime,
                                                                   uint32_t providerChangeControl,
                                                                   uint8_t *proposedProviderName,
                                                                   uint8_t *proposedProviderContactDetails)
{
  EmberAfDeviceManagementSupplier supplier;
  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishChangeOfSupplier: 0x%4X, 0x%4X, 0x%X, 0x%4X, 0x%4X, 0x%4X, ",
                                       currentProviderId,
                                       issuerEventId,
                                       tariffType,
                                       proposedProviderId,
                                       providerChangeImplementationTime,
                                       providerChangeControl);
  emberAfPluginGasProxyFunctionPrintString(proposedProviderName);
  emberAfPluginGasProxyFunctionPrintln(", ");
  emberAfPluginGasProxyFunctionPrintString(proposedProviderContactDetails);
  emberAfPluginGasProxyFunctionPrintln("");

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(providerChangeImplementationTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  supplier.proposedProviderId = proposedProviderId;
  supplier.implementationDateTime = providerChangeImplementationTime;
  supplier.providerChangeControl = providerChangeControl;
  emberAfCopyString(supplier.proposedProviderName,
                    proposedProviderName,
                    EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_NAME_LENGTH);
  emberAfCopyString(supplier.proposedProviderContactDetails,
                    proposedProviderContactDetails,
                    EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_CONTACT_DETAILS_LENGTH);

  emberAfPluginDeviceManagementSetSupplier(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT, &supplier);
  return true;
}

bool emberAfDeviceManagementClusterPublishChangeOfTenancyCallback(uint32_t providerId,
                                                                  uint32_t issuerEventId,
                                                                  uint8_t tariffType,
                                                                  uint32_t implementationDateTime,
                                                                  uint32_t proposedTenancyChangeControl)
{
  EmberAfDeviceManagementTenancy tenancy;
  bool result;
  EmberAfStatus status;

  emberAfPluginGasProxyFunctionPrintln("RX: PublishChangeOfTenancy: 0x%4X, 0x%4X, 0x%X, 0x%4X, 0x%4X",
                                       providerId,
                                       issuerEventId,
                                       tariffType,
                                       implementationDateTime,
                                       proposedTenancyChangeControl);

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(implementationDateTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  tenancy.implementationDateTime = implementationDateTime;
  tenancy.tenancy = proposedTenancyChangeControl;
  tenancy.providerId = providerId;
  tenancy.issuerEventId = issuerEventId;
  tenancy.tariffType = tariffType;

  result = emberAfPluginDeviceManagementSetTenancy(&tenancy, true);

  if (result) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Updated: Tenancy");
    emberAfPluginGasProxyFunctionPrintln("              implementationTime: 0x%4X", tenancy.implementationDateTime);
    emberAfPluginGasProxyFunctionPrintln("              tenancy: 0x%4X", tenancy.tenancy);
    status = EMBER_ZCL_STATUS_SUCCESS;
  } else {
    emberAfPluginGasProxyFunctionPrintln("GPF: Unable to update tenancy due to mismatching information.");
    status = EMBER_ZCL_STATUS_FAILURE;
  }

  emberAfSendImmediateDefaultResponse(status);
  return true;
}
/** @brief Simple Metering Cluster Request Mirror
 *
 */
bool emberAfSimpleMeteringClusterRequestMirrorCallback(void)
{
  EmberEUI64 otaEui;
  uint16_t endpointId;

  emberAfPluginGasProxyFunctionPrintln("GPF: RX: RequestMirror");

  if (emberLookupEui64ByNodeId(emberAfResponseDestination, otaEui) == EMBER_SUCCESS) {
    endpointId = emberAfPluginMeterMirrorRequestMirror(otaEui);
    if (endpointId != 0xFFFF) {
      emberAfFillCommandSimpleMeteringClusterRequestMirrorResponse(endpointId);
      emberAfSendResponse();
    } else {
      emberAfPluginGasProxyFunctionPrintln("GPF: Invalid endpoint. Sending Default Response");
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_AUTHORIZED);
    }
  } else {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
  }
  return true;
}

/** @brief Simple Metering Cluster Remove Mirror
 *
 */
bool emberAfSimpleMeteringClusterRemoveMirrorCallback(void)
{
  EmberEUI64 otaEui;
  uint16_t endpointId;

  emberAfPluginGasProxyFunctionPrintln("GPF: RX: RemoveMirror");

  if (emberLookupEui64ByNodeId(emberAfResponseDestination, otaEui) == EMBER_SUCCESS) {
    endpointId = emberAfPluginMeterMirrorRemoveMirror(otaEui);
    emberAfFillCommandSimpleMeteringClusterMirrorRemoved(endpointId);
    emberAfSendResponse();
  } else {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
  }
  return true;
}

bool emberAfPriceClusterPublishBlockPeriodCallback(uint32_t providerId,
                                                   uint32_t issuerEventId,
                                                   uint32_t blockPeriodStartTime,
                                                   uint32_t blockPeriodDuration,
                                                   uint8_t blockPeriodControl,
                                                   uint8_t blockPeriodDurationType,
                                                   uint8_t tariffType,
                                                   uint8_t tariffResolutionPeriod)
{
  uint32_t thresholdMultiplier;
  uint32_t thresholdDivisor;
  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishBlockPeriod 0x%4X, 0x%4X, 0x%4X, 0x%4X, 0x%X, 0x%X, 0x%X, 0x%X",
                                       providerId,
                                       issuerEventId,
                                       blockPeriodStartTime,
                                       blockPeriodDuration,
                                       blockPeriodControl,
                                       blockPeriodDurationType,
                                       tariffType,
                                       tariffResolutionPeriod);

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(blockPeriodStartTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  emberAfReadAttribute(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                       ZCL_PRICE_CLUSTER_ID,
                       ZCL_THRESHOLD_MULTIPLIER_ATTRIBUTE_ID,
                       CLUSTER_MASK_SERVER,
                       (uint8_t *)&(thresholdMultiplier),
                       emberAfGetDataSize(ZCL_INT24U_ATTRIBUTE_TYPE),
                       NULL);

  emberAfReadAttribute(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                       ZCL_PRICE_CLUSTER_ID,
                       ZCL_THRESHOLD_DIVISOR_ATTRIBUTE_ID,
                       CLUSTER_MASK_SERVER,
                       (uint8_t *)&(thresholdDivisor),
                       emberAfGetDataSize(ZCL_INT24U_ATTRIBUTE_TYPE),
                       NULL);

  emberAfPluginPriceServerBlockPeriodAdd(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                         providerId,
                                         issuerEventId,
                                         blockPeriodStartTime,
                                         blockPeriodDuration,
                                         blockPeriodControl,
                                         blockPeriodDurationType,
                                         thresholdMultiplier,
                                         thresholdDivisor,
                                         tariffType,
                                         tariffResolutionPeriod);
  return true;
}

bool emberAfPriceClusterPublishBillingPeriodCallback(uint32_t providerId,
                                                     uint32_t issuerEventId,
                                                     uint32_t billingPeriodStartTime,
                                                     uint32_t billingPeriodDuration,
                                                     uint8_t billingPeriodDurationType,
                                                     uint8_t tariffType)
{
  emberAfPluginGasProxyFunctionPrintln("GPF: RX: PublishBillingPeriod 0x%4X, 0x%4X, 0x%4X, 0x%4X, 0x%X, 0x%X",
                                       providerId,
                                       issuerEventId,
                                       billingPeriodStartTime,
                                       billingPeriodDuration,
                                       billingPeriodDurationType,
                                       tariffType);

  if (emberAfPluginGasProxyFunctionIgnoreFutureCommand(billingPeriodStartTime)) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Ignoring future dated command.");
    return true;
  }

  emberAfPluginPriceServerBillingPeriodAdd(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                           billingPeriodStartTime,
                                           issuerEventId,
                                           providerId,
                                           billingPeriodDuration,
                                           billingPeriodDurationType,
                                           tariffType);
  return true;
}

EmberAfGpfMessage * emberAfPluginGasProxyFunctionGetCurrentMessage()
{
  if ((currentGpfMessage.gbzCommands == NULL)
      && (currentGpfMessage.gbzCommandsLength == 0)) {
    return NULL;
  }
  return &currentGpfMessage;
}

bool emberAfPluginGasProxyFunctionIgnoreFutureCommand(uint32_t startTime)
{
  uint32_t now = emberAfGetCurrentTime();
  if ((startTime > now)
      && (emberAfPluginGasProxyFunctionGetCurrentMessage() != NULL)) {
    emberAfPluginGasProxyFunctionTapOffMessageFutureCommandIgnoredCallback(emberAfPluginGasProxyFunctionGetCurrentMessage(),
                                                                           emberAfCurrentCommand());
    return true;
  }

  return false;
}
