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
 * @brief Command definitions for the Gateway Relay COAP plugin.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER //compiler/micro specifics, types

#include "app/framework/include/af.h"
#include "stack/include/ember-types.h"
#include "stack/include/error.h"

#include "hal/hal.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/device-table/device-table.h"
#include <string.h>
#include "app/framework/plugin-host/coap-server/coap-server.h"
#include "app/framework/plugin/device-table/device-table.h"
#include "app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap-commands.h"
#include "app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap-cbor.h"

// -------- dotdot to Zigbee command translation ------------
static char *findFormatString(uint16_t clusterId, uint8_t commandId)
{
  uint16_t i;

  for (i = 0; dotdotCommandList[i].clusterId != 0xffff; i++) {
    if (dotdotCommandList[i].clusterId == clusterId
        && dotdotCommandList[i].commandId == commandId) {
      return dotdotCommandList[i].format;
    }
  }
  return NULL;
}

static char *findSignString(uint16_t clusterId, uint8_t commandId)
{
  uint16_t i;

  for (i = 0; dotdotCommandList[i].clusterId != 0xffff; i++) {
    if (dotdotCommandList[i].clusterId == clusterId
        && dotdotCommandList[i].commandId == commandId) {
      return dotdotCommandList[i].signs;
    }
  }
  return NULL;
}

// code borrowed heavily from client-api.c:vFillBuffer (line 25)
static uint16_t addCborArgumentsToCommand(uint8_t *buffer,
                                          uint16_t bufferLength,
                                          char *format)
{
  uint8_t i;
  uint8_t valueLen;
  uint8_t dataLen;
  uint32_t value;
  uint16_t bytes = 0;
  char cmd;

  for (i = 0; format[i] != 0; i++) {
    cmd = format[i];
    value = 0;
    valueLen = 0;

    if (cmd <= 's') {
      printf("Error:  don't handle argument %c\r\n", cmd);
      return 0;
    } else {
      // u, v, x, and w are one-, two-, three-, or four-byte integers.  u and v
      // must be extracted as an int while x and w come through as an uint32_t.
      // In all cases, the value is copied to the destination buffer in little-
      // endian format.
      dataLen = 0;
      if (cmd == 'u') {
        valueLen = 1;
      } else if (cmd == 'v') {
        valueLen = 2;
      } else if (cmd == 'x') {
        valueLen = 3;
      } else if (cmd == 'w') {
        valueLen = 4;
      } else {
        emberAfDebugPrintln("ERR: Unknown format '%c'", cmd);
        return 0;
      }

      // burn the CBOR map keys.
      emAfPluginGatewayRelayCoapCborGetUnsignedInteger(&value, &dataLen);

      emAfPluginGatewayRelayCoapCborGetUnsignedInteger(&value, &dataLen);
    }

    // If there is an integer value, add it to destination buffer in little-
    // endian format.
    for (; 0 < valueLen; valueLen--) {
      buffer[bytes++] = LOW_BYTE(value);
      value = value >> 8;
    }
  }

  return bytes;
}

// test to send command to light.
static void fillOutgoingCommand(uint16_t clusterId, uint8_t commandId)
{
  static uint8_t frameCounter = 0;
  char *formatString;

  emAfCommandApsFrame->clusterId = clusterId;
  emAfCommandApsFrame->options = EMBER_AF_DEFAULT_APS_OPTIONS;
  *emAfResponseLengthPtr = 3;

  emAfZclBuffer[0] = 0x01;           // frame control. TBD
  emAfZclBuffer[1] = frameCounter++;
  emAfZclBuffer[2] = commandId;

  formatString = findFormatString(clusterId, commandId);

  if (formatString != NULL) {
    *emAfResponseLengthPtr += addCborArgumentsToCommand(emAfZclBuffer + 3, 100, formatString);
  }
}

void emAfGatewayRelayCoapProcessIncomingCommand(uint16_t cluster,
                                                uint8_t commandId,
                                                int16_t portCounter,
                                                uint16_t endpoint)
{
  fillOutgoingCommand(cluster, commandId);

  if (portCounter < 0) {
    // message sent to 5683
    emberAfSetCommandEndpoints(1, 1);
    emberAfDeviceTableCommandIndexSend(endpoint - 1);
  } else {
    // port counter happens to be the index into the device table.
    emberAfSetCommandEndpoints(1, endpoint);
    emberAfDeviceTableCommandIndexSend((uint16_t) portCounter);
  }
}

// -------- Zigbee to dotdot command translation ------------

#define BUFFER_LENGTH 50
static uint8_t outgoingMessageUri[BUFFER_LENGTH];
static uint8_t outgoingMessagePayload[BUFFER_LENGTH];

bool emAfGatewayRelayCoapPreCommandReceivedCallback(EmberAfClusterCommand* cmd)
{
  uint16_t i;
  char *formatString, *signString;
  uint8_t *buffer = (cmd->buffer) + 3;
  uint8_t bufferFinger = 0;
  uint8_t argCount = 0;
  uint16_t data16;

  // Only want to forward this if it is not a global command (i.e byte 0 is
  // odd).
  if ((cmd->buffer[0] & 0x01) != 0x01) {
    return false;
  }

  // Note:  for now, we assume the device has a single endpoint.
  uint16_t nodeId = emberGetSender();
  uint16_t device = emberAfDeviceTableGetIndexFromNodeId(nodeId);
  EmberAfPluginDeviceTableEntry *dtEntry;
  uint8_t endpoint;

  if (device == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    // don't have a valid index.  Need to return.
    return false;
  }

  dtEntry = emberAfDeviceTableFindDeviceTableEntry(device);
  endpoint = dtEntry->endpoint;

  sprintf((char *) outgoingMessageUri, "zcl/e/%d/c%x/c/%x",
          endpoint,
          cmd->apsFrame->clusterId,
          cmd->commandId);

  // Process command payloads
  emAfPluginGatewayRelayCoapCborWriteInit(outgoingMessagePayload, BUFFER_LENGTH);

  signString = findSignString(cmd->apsFrame->clusterId, cmd->commandId);
  formatString = findFormatString(cmd->apsFrame->clusterId, cmd->commandId);

  if (formatString != NULL) {
    emAfPluginGatewayRelayCoapCborWriteMapTag();

    for (i = 0; formatString[i] != 0; i++) {
      emAfPluginGatewayRelayCoapCborWriteUint8(argCount++);
      if (formatString[i] == 'u') {
        // single byte
        if (signString == NULL
            || signString[i] == 'u') {
          emAfPluginGatewayRelayCoapCborWriteUint8(buffer[bufferFinger++]);
        } else {
          emAfPluginGatewayRelayCoapCborWriteInt8(buffer[bufferFinger++]);
        }
      } else if (formatString[i] == 'v') {
        data16 = HIGH_LOW_TO_INT(buffer[bufferFinger + 1], buffer[bufferFinger]);
        bufferFinger += 2;
        if (signString == NULL
            || signString[i] == 'u') {
          emAfPluginGatewayRelayCoapCborWriteUint16(data16);
        } else {
          emAfPluginGatewayRelayCoapCborWriteInt16(data16);
        }
      } else {
        printf("Error:  unknown format %c\r\n", formatString[i]);
      }
    }
    emAfPluginGatewayRelayCoapCborWriteBreak();
  }

  emberAfPluginCoapServerSendMessage(outgoingMessageUri,
                                     outgoingMessagePayload,
                                     emAfPluginGatewayRelayCoapCborCurrentWriteDataLength(),
                                     device,
                                     COAP_REQUEST_POST);

  return false;
}
