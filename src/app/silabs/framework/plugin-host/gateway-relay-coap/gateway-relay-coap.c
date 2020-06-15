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
 * @brief Definitions for the Gateway Relay COAP plugin.
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
#include "app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap-cbor.h"
#include "app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap-handle-read-attr-resp.h"
#include "util/third_party/libcoap/include/coap/pdu.h"

static uint16_t returnCode = COAP_RESPONSE_OK;
#define RETURN_STRING_LENGTH 100

#define CLUSTER_STRING_LENGTH 6
static char clusterString[CLUSTER_STRING_LENGTH];

#define CLUSTER_LIST_START 11

// read attribute response code.
static void setUpErrorCode(uint16_t error);

// --------------------------------
void emberAfPluginCoapServerSendDelayedResponse(uint8_t *data, uint16_t length);

void emAfGatewayRelayCoapProcessIncomingCommand(uint16_t cluster,
                                                uint8_t commandId,
                                                int16_t portCounter,
                                                uint16_t endpoint);

static void sendAttributeReportMessage(uint16_t device,
                                       uint8_t endpoint,
                                       uint16_t cluster,
                                       uint16_t attribute,
                                       uint32_t data);

static uint8_t getNextOptByte(void);

void resetOpt(void);
static void printData(uint8_t * data, uint16_t length, char *formatString);

bool isInt(uint8_t *buffer, uint8_t maxLength)
{
  uint8_t i;

  for (i = 0; i < maxLength; i++) {
    if (buffer[i] == 0) {
      return i != 0;
    }
    if (buffer[i] < '0'
        || buffer[i] > '9') {
      return false;
    }
  }

  return false; // too long
}

uint16_t stringToInt16(uint8_t *buffer)
{
  uint32_t data;
  uint16_t val16;

  sscanf((const char *)buffer, "%x", &data);

  val16 = (uint16_t) data;

  return val16;
}

void emberAfPluginGatewayRelayCoapInitCallback(void)
{
  setUpErrorCode(COAP_RESPONSE_NOT_FOUND);
}

// device-table callbacks and interface
void emberAfPluginDeviceTableInitialized(void)
{
  uint16_t i;
  // call to coap server to set up the channels.
  for (i = 0; i < PLUGIN_COAP_SERVER_MAX_PORTS; i++) {
    if (emberAfDeviceTableGetNodeIdFromIndex(i)
        != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      emberAfPluginCoapServerCreateDevice(i);
    }
  }
}

void emberAfPluginDeviceTableIndexRemovedCallback(uint16_t index)
{
  emberAfPluginCoapServerRemoveDevice(index);
}

void emberAfPluginDeviceTableIndexAddedCallback(uint16_t index)
{
  emberAfPluginCoapServerCreateDevice(index);
}

int emberAfPluginCoapServerCurrentPort(void);

static void handleAttributeRequest(uint16_t clusterId,
                                   int portCounter,
                                   uint16_t endpoint,
                                   uint8_t *ptr);

// response data
static uint8_t returnString[RETURN_STRING_LENGTH] = { 0, };

static void constructEndpointList(void)
{
  emAfPluginGatewayRelayCoapCborWriteInit(returnString, RETURN_STRING_LENGTH - 1);

  if (emAfPluginGatewayRelayCoapCborWriteArrayTag()) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }
  if (emAfPluginGatewayRelayCoapCborWriteUint8(1)) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }
  if (emAfPluginGatewayRelayCoapCborWriteBreak()) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }

  returnCode = COAP_RESPONSE_DATA;
}

static void constructClusterList(uint16_t deviceIndex)
{
  EmberAfPluginDeviceTableEntry *currentDevice;
  char direction;
  uint8_t i;

  currentDevice = emberAfDeviceTableFindDeviceTableEntry(deviceIndex);

  if (currentDevice->nodeId == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
    setUpErrorCode(COAP_RESPONSE_NOT_FOUND);
    return;
  }

  emAfPluginGatewayRelayCoapCborWriteInit(returnString, RETURN_STRING_LENGTH - 1);

  emAfPluginGatewayRelayCoapCborWriteArrayTag();

  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE; i++) {
    if (currentDevice->clusterIds[i] == ZCL_NULL_CLUSTER_ID) {
      emAfPluginGatewayRelayCoapCborWriteBreak();
      returnCode = COAP_RESPONSE_DATA;
      return;
    }

    if (i >= currentDevice->clusterOutStartPosition) {
      direction = 'c';
    } else {
      direction = 's';
    }
    snprintf(clusterString,
             CLUSTER_STRING_LENGTH,
             "%c%x",
             direction,
             currentDevice->clusterIds[i]);
    if (emAfPluginGatewayRelayCoapCborWriteString(clusterString)) {
      setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
      return;
    }
  }
  emAfPluginGatewayRelayCoapCborWriteBreak();
  returnCode = COAP_RESPONSE_DATA;

  return;
}

// take a string of characters and make it into a list.
static void constructResourceList(char *list)
{
  uint8_t i;
  char resourceString[3];

  emAfPluginGatewayRelayCoapCborWriteInit(returnString, RETURN_STRING_LENGTH - 1);

  emAfPluginGatewayRelayCoapCborWriteArrayTag();
  resourceString[1] = 0;
  for (i = 0; i < strlen(list); i++) {
    resourceString[0] = list[i];
    emAfPluginGatewayRelayCoapCborWriteString(resourceString);
  }

  emAfPluginGatewayRelayCoapCborWriteBreak();

  returnCode = COAP_RESPONSE_DATA;
}

EmberStatus checkEndpoint(uint16_t endpoint, uint16_t portCounter)
{
  EmberAfPluginDeviceTableEntry *currentDevice
    = emberAfDeviceTableFindDeviceTableEntry(portCounter);

  if (currentDevice->endpoint != endpoint) {
    return EMBER_BAD_ARGUMENT;
  }

  return EMBER_SUCCESS;
}

static void setUpErrorCode(uint16_t error)
{
  returnCode = error;
  emAfPluginGatewayRelayCoapCborWriteInit(returnString, RETURN_STRING_LENGTH - 1);
}

static bool checkMethod(uint8_t method, uint8_t desiredMethod)
{
  if (method == 0
      || method == desiredMethod) {
    return false;
  } else {
    setUpErrorCode(COAP_RESPONSE_METHOD_NOT_ALLOWED);
    return true;
  }
}

uint16_t emberAfPluginGatewayRelayCoapReturnStringLength(void)
{
  //return strlen(returnString);
  return emAfPluginGatewayRelayCoapCborCurrentWriteDataLength();
}

uint16_t emberAfPluginGatewayRelayCoapReturnCode(void)
{
  return returnCode;
}

void emberAfPluginGatewayRelayCoapCopyReturnString(char *ptr)
{
  MEMCOPY((uint8_t *) ptr,
          (uint8_t *) returnString,
          emAfPluginGatewayRelayCoapCborCurrentWriteDataLength());

  returnString[0] = 0;
}

static bool getUint16Opttok(uint16_t *val16);

static volatile bool waitForResponse = true;

bool emberAfPluginCoapServerPrepareDelayedResponse(void);
static void fillAttributeRequestWildcard(uint16_t clusterId);

static void handleAttributeRequest(uint16_t clusterId,
                                   int portCounter,
                                   uint16_t endpoint,
                                   uint8_t *ptr)
{
  uint16_t attributeId;
  uint8_t attributeIdBuffer[2];

  printf("handleAttributeRequest %c\r\n", getNextOptByte());

  if (getNextOptByte() == 'C') {
    printf("handle wildcard\r\n");

    // this is a complex query.  Assume it is a wildcard for now.
    fillAttributeRequestWildcard(clusterId);
  } else {
    printf("handle single attribute\r\n");

    // asking for a single attribute.
    getUint16Opttok(&attributeId);
    attributeIdBuffer[0] = LOW_BYTE(attributeId);
    attributeIdBuffer[1] = HIGH_BYTE(attributeId);

    emberAfFillCommandGlobalClientToServerReadAttributes(clusterId,
                                                         attributeIdBuffer,
                                                         2);
  }

  if (portCounter < 0) {
    // message sent to 5683
    emberAfSetCommandEndpoints(1, 1);
    emberAfDeviceTableCommandIndexSend(endpoint - 1);
  } else {
    // port counter happens to be the index into the device table.
    emberAfSetCommandEndpoints(1, endpoint);
    emberAfDeviceTableCommandIndexSend((uint16_t) portCounter);
  }

  printf("send read request: %u\r\n", halCommonGetInt32uMillisecondTick());

  // Inform coap server that we plan to send a response.
  emberAfPluginCoapServerPrepareDelayedResponse();
}

bool emAfGatewayRelayCoapReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                                        uint8_t* buffer,
                                                        uint16_t bufLen)
{
  // Note:  for now, we assume the device has a single endpoint.
  uint16_t nodeId = emberGetSender();
  uint16_t device = emberAfDeviceTableGetIndexFromNodeId(nodeId);
  EmberAfPluginDeviceTableEntry *dtEntry;
  uint8_t endpoint;

  printf("receive read response: %u\r\n", halCommonGetInt32uMillisecondTick());

  if (device == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    // don't have a valid index.  Need to return.
    return false;
  }

  dtEntry = emberAfDeviceTableFindDeviceTableEntry(device);
  endpoint = dtEntry->endpoint;

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    buffer,
    bufLen,
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode);

  emberAfPluginCoapServerSendDelayedResponse(
    returnString,
    emAfPluginGatewayRelayCoapCborCurrentWriteDataLength());

  emAfPluginGatewayRelayCoapCborWriteInit(returnString, 0);

  waitForResponse = false;

  return false;
}

bool emAfGatewayRelayCoapReportAttributesCallback(EmberAfClusterId clusterId,
                                                  uint8_t * buffer,
                                                  uint16_t bufLen)
{
  emAfGatewayRelayCoapReadAttributesResponseCallback(clusterId,
                                                     buffer,
                                                     bufLen);
  return false;
}

// ----------------------------------------------------------------------------
// Construct outgoing attributes request for a wildcard.
#define ATTRIBUTE_TERMINATOR 0xffff
typedef struct {
  uint16_t clusterId;
  uint16_t attributeArray[10];
} AttributeList;

AttributeList attributeList[] = {
  { 0x0006, { 0x0000, 0xfffd, ATTRIBUTE_TERMINATOR } },
  { 0x0008, { 0x0000, 0xfffd, ATTRIBUTE_TERMINATOR } },
  { 0x0300, { 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0007, 0x0008, 0x000F,
              0xfffd, ATTRIBUTE_TERMINATOR } },
  { ATTRIBUTE_TERMINATOR, { 0x0000, 0xfffd, ATTRIBUTE_TERMINATOR } },
};

static uint16_t findEntry(uint16_t clusterId)
{
  uint16_t i = 0;

  while (attributeList[i].clusterId != clusterId
         && attributeList[i].clusterId != ATTRIBUTE_TERMINATOR) {
    i++;
  }
  return i;
}

uint16_t findNumAttributes(uint16_t entry)
{
  uint16_t i = 0;

  while (attributeList[entry].attributeArray[i] != ATTRIBUTE_TERMINATOR) {
    i++;
  }
  return i;
}

static void fillAttributeRequestWildcard(uint16_t clusterId)
{
  uint16_t entry = findEntry(clusterId);
  uint16_t numAttributes = findNumAttributes(entry);
  uint16_t i;
  uint16_t *attributes = attributeList[entry].attributeArray;
  uint8_t *buffer;

  buffer = malloc(numAttributes * 2);
  for (i = 0; i < numAttributes; i++) {
    buffer[2 * i] = LOW_BYTE(attributes[i]);
    buffer[2 * i + 1] = HIGH_BYTE(attributes[i]);
  }

  printf("ReadAttributeData: ");
  printData(buffer, numAttributes * 2, "%02x ");
  emberAfFillCommandGlobalClientToServerReadAttributes(clusterId,
                                                       buffer,
                                                       numAttributes * 2);

  free(buffer);
}

// ----------------------------------------------------------------------------
// gateway rely coap heartbeat code
EmberEventControl emberAfPluginGatewayRelayCoapHeartbeatEventControl;
static uint16_t heartbeatPeriodSeconds = 0;

static void sendAttributeReportMessage(uint16_t device,
                                       uint8_t endpoint,
                                       uint16_t cluster,
                                       uint16_t attribute,
                                       uint32_t data);

void emberAfPluginGatewayRelayCoapHeartbeatEventHandler(void)
{
  uint16_t i;
  EmberAfPluginDeviceTableEntry *pEntry;

  emberEventControlSetDelayMS(emberAfPluginGatewayRelayCoapHeartbeatEventControl,
                              1000 * (uint32_t) heartbeatPeriodSeconds);

  for (i = 0; i < PLUGIN_COAP_SERVER_MAX_PORTS; i++) {
    pEntry = emberAfDeviceTableFindDeviceTableEntry(i);

    if (pEntry->nodeId != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      sendAttributeReportMessage(i, pEntry->endpoint, 8, 0, 200);
    }
  }
}

void emberAfPluginGatewayRelayCoapStartHeartbeat(uint16_t seconds)
{
  heartbeatPeriodSeconds = seconds;

  emberEventControlSetActive(emberAfPluginGatewayRelayCoapHeartbeatEventControl);
}

void emberAfPluginGatewayRelayCoapStopHeartbeat(void)
{
  emberEventControlSetInactive(emberAfPluginGatewayRelayCoapHeartbeatEventControl);
}

// ----------------------------------------------------------------------------
// gateway relay coap construct messages code
#define BUFFER_LENGTH 50
static uint8_t outgoingMessageUri[BUFFER_LENGTH];

static void sendAttributeReportMessage(uint16_t device,
                                       uint8_t endpoint,
                                       uint16_t cluster,
                                       uint16_t attribute,
                                       uint32_t data)
{
  uint16_t bytesUsed = 0;
  uint8_t *buffer = outgoingMessageUri;

  sprintf((char *) buffer, "zcl/e/%d/s%x/n", endpoint, cluster);

  emAfPluginGatewayRelayCoapCborWriteInit(returnString, RETURN_STRING_LENGTH);

  if (emAfPluginGatewayRelayCoapCborWriteMapTag()) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }
  if (emAfPluginGatewayRelayCoapCborWriteString("r")) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }
  if (emAfPluginGatewayRelayCoapCborWriteUint32(0)) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }
  if (emAfPluginGatewayRelayCoapCborWriteString("a")) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }
  if (emAfPluginGatewayRelayCoapCborWriteMapTag()) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }
  if (emAfPluginGatewayRelayCoapCborWriteUint16(attribute)) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }
  if (emAfPluginGatewayRelayCoapCborWriteUint32(data)) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }
  if (emAfPluginGatewayRelayCoapCborWriteBreak()) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }
  if (emAfPluginGatewayRelayCoapCborWriteBreak()) {
    setUpErrorCode(COAP_RESPONSE_INTERNAL_ERROR);
  }

  bytesUsed += emAfPluginGatewayRelayCoapCborCurrentWriteDataLength();
}

// ----------------------------------------------------------------------------
// gateway relay coap parse Options
static uint16_t bufferLength;
static uint8_t returnToken[BUFFER_LENGTH];
static uint8_t *currentBuffer;
static uint16_t finger;
static uint8_t currentOption;

static uint8_t getNextOptByte(void)
{
  return currentBuffer[finger];
}

uint8_t *optTok(uint8_t *buffer, uint16_t length, uint8_t *option)
{
  uint8_t optionByte, optionLength, i;

  if (buffer != NULL) {
    currentBuffer = buffer;
    bufferLength = length;
    currentOption = COAP_OPTION_URI_PATH;
    finger = 0;
  }

  if (finger >= bufferLength) {
    return NULL;
  }

  optionByte = currentBuffer[finger++];

  if (optionByte == 0xff) {
    return NULL;
  }

  optionLength = optionByte & 0x0f;
  currentOption = currentOption + (optionByte >> 4);

  for (i = 0; i < optionLength; i++) {
    returnToken[i] = currentBuffer[finger++];
  }

  returnToken[i] = 0;
  *option = currentOption;
  return returnToken;
}

void resetOpt(void)
{
  finger = 0;
}

static bool getUint16Opttok(uint16_t *val16)
{
  uint32_t data = 0;
  uint8_t opt;
  uint8_t *ptr = optTok(NULL, 0, &opt);

  if (ptr == NULL) {
    return false;
  }

  sscanf((char *) ptr, "%x", &data);

  *val16 = (uint16_t) data;

  return true;
}

static bool getClusterOpttok(uint16_t *cluster, char *clientServer)
{
  uint32_t data;
  uint8_t opt;
  uint8_t *ptr = optTok(NULL, 0, &opt);

  if (ptr == NULL) {
    return false;
  }

  *clientServer = *ptr;

  ptr++;

  sscanf((char *) ptr, "%x", &data);

  *cluster = (uint16_t) data;

  return true;
}

static void printData(uint8_t * data, uint16_t length, char *formatString)
{
  uint16_t i;
  printf("Data: ");

  for (i = 0; i < length; i++) {
    printf(formatString, data[i]);
  }

  printf("\r\n");
}

bool emberAfGatewayRelayCoapParseDotdotMessage(uint8_t length,
                                               uint8_t *string,
                                               int portCounter,
                                               uint16_t method)
{
  uint8_t *ptr, option;
  uint16_t endpoint = 0xffff;
  uint16_t cluster;
  char clientServer;
  uint16_t commandId;

  uint16_t i, cborPayload = length, cborLength;

  printData(string, length, "%02x ");

  returnCode = COAP_RESPONSE_BAD_REQUEST; // bad request by default.

  for (i = 0; i < length; i++) {
    if (string[i] == 0xff) {
      cborPayload = i + 1;
      string[i] = ':';
      break;
    }
  }

  cborLength = length - cborPayload;

  if (cborLength > 0) {
    emAfPluginGatewayRelayCoapCborReadInit(string + cborPayload, cborLength);
  } else {
    emAfPluginGatewayRelayCoapCborReadInit(NULL, 0);
  }

  ptr = optTok(string, length, &option);

  if (ptr == NULL) {
    // respond with {0,"e"} here

    if (!checkMethod(method, COAP_REQUEST_GET)) {
      constructResourceList("e");
    }

    return false;
  }

  if (strncmp((char *) ptr, "e", 1) != 0) {
    if (isInt(ptr, 3)) {
      resetOpt();
    } else {
      // error:  only accept endpoint here.
      setUpErrorCode(404);
      return false;
    }
  }

  if (!getUint16Opttok(&endpoint)) {
    // respond with endpoints list here
    if (!checkMethod(method, COAP_REQUEST_GET)) {
      constructEndpointList();
    }
    return false;
  }

  // TBD: need a way to handle multiple endpoints here.
  if (portCounter == -1) {
    printf("Default port, switching to endpoint mode\r\n");
    portCounter = endpoint - 1;
    endpoint = 1;
  }

  if (checkEndpoint(endpoint, portCounter)) {
    setUpErrorCode(404);
    return false;
  }

  if (!getClusterOpttok(&cluster, &clientServer)) {
    // respond with cluster list here
    if (!checkMethod(method, COAP_REQUEST_GET)) {
      constructClusterList(portCounter);
    }
    return false;
  }

  ptr = optTok(NULL, 0, &option);
  if (ptr == NULL) {
    if (!checkMethod(method, COAP_REQUEST_GET)) {
      constructResourceList("ac");
    }
    return false;
  }

  switch (ptr[0]) {
    case 'c':
      if (!getUint16Opttok(&commandId)) {
        if (!checkMethod(method, COAP_REQUEST_GET)) {
          setUpErrorCode(COAP_RESPONSE_NOT_IMPLEMENTED);
          return false;
        }
      } else {
        if (!checkMethod(method, COAP_REQUEST_POST)) {
          emAfGatewayRelayCoapProcessIncomingCommand(cluster,
                                                     commandId,
                                                     (uint16_t) portCounter,
                                                     endpoint);
          returnCode = COAP_RESPONSE_CREATED;
        }
      }
      break;
    case 'a':
      if (!checkMethod(method, COAP_REQUEST_GET)) {
        handleAttributeRequest(cluster, portCounter, endpoint, ptr);
        returnCode = COAP_RESPONSE_CREATED;
      }
      break;
    default:
      emberAfCorePrintln("Unknown payload: %s", ptr);
      break;
  }

  return true;
}
