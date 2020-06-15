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
 * @brief Definitions for the Gateway Relay MQTT plugin.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/util.h"
#include EMBER_AF_API_DEVICE_TABLE
#include EMBER_AF_API_COMMAND_RELAY
#include EMBER_AF_API_LINKED_LIST
#include EMBER_AF_API_TRANSPORT_MQTT
#include EMBER_AF_API_CJSON
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "stack/include/trust-center.h"
#include "gateway-relay-mqtt.h"
#include "gateway-relay-mqtt-commands.h"

#include <sys/time.h>
#include <stdlib.h>

// ****************************************************************************
// custom commands forward declarations
static const EmberEUI64 NULL_EUI = { 0, 0, 0, 0, 0, 0, 0, 0 };
#define isNullEui(eui) (MEMCOMPARE(eui, NULL_EUI, EUI64_SIZE) == 0)

// Constants
#define HOSTVER_STRING_LENGTH 14 // 13 characters + NULL (99.99.99-9999)
#define EUI64_STRING_LENGTH 19 // "0x" + 16 characters + NULL
#define NODEID_STRING_LENGTH 7 // "0x" + 4 characters + NULL
#define CLUSTERID_STRING_LENGTH 7 // "0x" + 4 chars + NULL
#define GATEWAY_TOPIC_PREFIX_LENGTH 22 // 21 chars `gw/xxxxxxxxxxxxxxxx/` + NULL
#define DEFAULT_HEARTBEAT_RATE_MS 5000 // milliseconds
#define PROCESS_COMMAND_RATE_MS 20 // milliseconds
#define BLOCK_SENT_THROTTLE_VALUE 25 // blocks to receive before sending updates
#define HEX_TOKEN_SIZE 2 // "0x"

// Attribute reading buffer location definitions
#define ATTRIBUTE_BUFFER_ATTRIBUTEID_LOW_BITS  0
#define ATTRIBUTE_BUFFER_ATTRIBUTEID_HIGH_BITS 1
#define ATTRIBUTE_BUFFER_SUCCESS_CODE        2
#define ATTRIBUTE_BUFFER_DATA_TYPE           3
#define ATTRIBUTE_BUFFER_DATA_START          4

// Attribute reporting / IAS ZONE buffer location definitions
#define ATTRIBUTE_BUFFER_REPORT_DATA_START          3
#define ATTRIBUTE_BUFFER_REPORT_CLUSTERID_HIGH_BITS 1
#define ATTRIBUTE_BUFFER_REPORT_CLUSTERID_LOW_BITS  0
#define ATTRIBUTE_BUFFER_REPORT_DATA_TYPE           2

#define COMMAND_OFFSET 2
#define ZCL_RESPONSE_TOPIC "zclresponse"
#define ZDO_RESPONSE_TOPIC "zdoresponse"
#define APS_RESPONSE_TOPIC "apsresponse"

#define ONE_BYTE_HEX_STRING_SIZE  5
#define TWO_BYTE_HEX_STRING_SIZE  7
#define FOUR_BYTE_HEX_STRING_SIZE 11

#define READ_REPORT_CONFIG_STATUS       0
#define READ_REPORT_CONFIG_DIRECTION    1
#define READ_REPORT_CONFIG_ATTRIBUTE_ID 2
#define READ_REPORT_CONFIG_DATA_TYPE    4
#define READ_REPORT_CONFIG_MIN_INTERVAL 5
#define READ_REPORT_CONFIG_MAX_INTERVAL 7
#define READ_REPORT_CONFIG_DATA         9

#define DEVICE_TABLE_BIND_RESPONSE_STATUS 1

#define BINDING_TABLE_RESPONSE_NUM_ENTRIES 4
#define BINDING_TABLE_RESPONSE_STATUS      1
#define BINDING_TABLE_RESPONSE_ENTRIES     5
#define BINDING_TABLE_RESPONSE_ENTRY_SIZE  21
#define BINDING_ENTRY_EUI                0
#define BINDING_ENTRY_SOURCE_ENDPOINT    8
#define BINDING_ENTRY_CLUSTER_ID         9
#define BINDING_ENTRY_ADDRESS_MODE       11
#define BINDING_ENTRY_DEST_EUI           12
#define BINDING_ENTRY_DEST_ENDPOINT      20

// Gateway global variables
static EmberEUI64 gatewayEui64;
static char gatewayEui64String[EUI64_STRING_LENGTH] = { 0 };
static char gatewayTopicUriPrefix[GATEWAY_TOPIC_PREFIX_LENGTH] = { 0 };
static bool trafficReporting = false;
static uint32_t otaBlockSent = 0; // This only supports one device OTA at a time
static uint16_t heatBeatRateMs = DEFAULT_HEARTBEAT_RATE_MS;

// Command list global variables
// We need to keep a list of commands to process as the come in from an external
// source to the gateway, these are the structs, defines and list needed for
// that list
#define COMMAND_TYPE_CLI        0x01
#define COMMAND_TYPE_GATEWAY    0x02
#define COMMAND_TYPE_POST_DELAY 0x03

typedef struct _GatewayCommand {
  uint8_t commandType;
  char* cliCommand; // used for COMMAND_TYPE_CLI
  uint32_t postDelayMs; // used for COMMAND_TYPE_POST_DELAY
  uint64_t resumeTime; // used for COMMAND_TYPE_POST_DELAY
} GatewayCommand;

static EmberAfPluginLinkedList* commandList;

static GatewayCommand* allocateGatewayCommand();
static void freeGatewayCommand(GatewayCommand* gatewayCommand);
static void addCommandToList(EmberAfPluginLinkedList* commandList,
                             const char* cliCommand,
                             uint8_t commandType);
static void addPostDelayMsToList(EmberAfPluginLinkedList* commandList,
                                 uint32_t postDelayMs);

// Events
EmberEventControl emberAfPluginGatewayRelayMqttHeartbeatEventControl;
void emberAfPluginGatewayRelayMqttHeartbeatEventHandler(void);

EmberEventControl emberAfPluginGatewayRelayMqttProcessCommandEventControl;
void emberAfPluginGatewayRelayMqttProcessCommandEventHandler(void);

EmberEventControl emberAfPluginGatewayRelayMqttStateUpdateEventControl;
void emberAfPluginGatewayRelayMqttStateUpdateEventHandler(void);

// String/other helpers
static void nodeIdToString(EmberNodeId nodeId, char* nodeIdString);
static void eui64ToString(EmberEUI64 eui, char* euiString);
static void printAttributeBuffer(uint16_t clusterId,
                                 uint8_t* buffer,
                                 uint16_t bufLen);

// MQTT API publishing helpers
static char* allocateAndFormMqttGatewayTopic(char* channel);
static void publishMqttTopic(char * topicNameString, cJSON * nodeJson);
static void publishMqttHeartbeat(void);
static void publishMqttRelays(void);
static void publishMqttGatewayState(void);
static void publishMqttSettings(void);
static void publishMqttDevices(void);
static void publishMqttDeviceStateChange(EmberEUI64 eui64,
                                         uint8_t state);
static void publishMqttDeviceJoined(EmberEUI64 eui64);
static void publishMqttDeviceLeft(EmberEUI64 eui64);
static void publishMqttAttribute(EmberEUI64 eui64,
                                 EmberAfClusterId clusterId,
                                 uint8_t* buffer,
                                 uint16_t bufLen);
static void publishMqttTrafficReportEvent(char* messageType,
                                          EmberStatus* status,
                                          int8_t* lastHopRssi,
                                          uint8_t* lastHopLqi,
                                          uint8_t* zclSequenceNumber,
                                          uint64_t timeMS);
static void publishMqttOtaEvent(char* messageType,
                                EmberEUI64 eui64,
                                uint8_t* status,
                                uint32_t* blockSent,
                                uint8_t* actualLength,
                                uint16_t* manufacturerId,
                                uint16_t* imageTypeId,
                                uint32_t* firmwareVersion);
static void publishMqttCommandExecuted(char* cliCommand);
static void publishMqttDelayExecuted(uint32_t postDelayMs);
static cJSON* buildNodeJson(uint16_t deviceTableIndex);
static void publishMqttZclCommand(uint8_t commandId,
                                  boolean clusterSpecific,
                                  uint16_t clusterId,
                                  boolean mfgSpecific,
                                  uint16_t mfgCode,
                                  uint8_t* buffer,
                                  uint8_t bufLen,
                                  uint8_t payloadStartIndex);
static void publishMqttBindResponse(EmberNodeId nodeId,
                                    EmberApsFrame* apsFrame,
                                    uint8_t* message,
                                    uint16_t length);
static void publishMqttBindTableReponse(EmberNodeId nodeId,
                                        EmberApsFrame* apsFrame,
                                        uint8_t* message,
                                        uint16_t length);

// MQTT topic and handler list
typedef void (*MqttTopicHandler)(cJSON* messageJson);

typedef struct _MqttTopicHandlerMap{
  char* topic;
  MqttTopicHandler topicHandler;
} MqttTopicHandlerMap;

EmberAfPluginLinkedList* topicHandlerList;

static MqttTopicHandlerMap* buildTopicHandler(
  char* topicString,
  MqttTopicHandler topicHandlerFunction);

// MQTT API subscription helpers
static void handleCommandsMessage(cJSON* messageJson);
static void handlePublishStateMessage(cJSON* messageJson);
static void handleUpdateSettingsMessage(cJSON* messageJson);

// Handy string creation routines.
static char* createOneByteHexString(uint8_t value)
{
  char* outputString = (char *) malloc(ONE_BYTE_HEX_STRING_SIZE);

  sprintf(outputString, "0x%02X", value);
  return outputString;
}

static char* createTwoByteHexString(uint16_t value)
{
  char* outputString = (char *) malloc(TWO_BYTE_HEX_STRING_SIZE);

  sprintf(outputString, "0x%04X", value);
  return outputString;
}

// MQTT Helper Functions
static MqttTopicHandlerMap* buildTopicHandler(
  char* topicString,
  MqttTopicHandler topicHandlerFunction)
{
  MqttTopicHandlerMap* topicHandlerMap =
    (MqttTopicHandlerMap*)malloc(sizeof(MqttTopicHandlerMap));
  topicHandlerMap->topic = allocateAndFormMqttGatewayTopic(topicString);
  topicHandlerMap->topicHandler = topicHandlerFunction;
  return topicHandlerMap;
}

static char* allocateAndFormMqttGatewayTopic(char* topic)
{
  // Add our string sizes + one NULL char
  uint16_t stringSize = strlen(gatewayTopicUriPrefix) + strlen(topic) + 1;
  char* fullTopicUri = (char*)malloc(stringSize);

  if (fullTopicUri == NULL) {
    emberAfAppPrintln("FATAL ERR: Unable to allocate more memory!");
    assert(false);
  }
  strcpy(fullTopicUri, gatewayTopicUriPrefix);
  strcat(fullTopicUri, topic);
  return fullTopicUri;
}

static cJSON* buildDeviceEndpoint(EmberEUI64 eui64, uint8_t endpoint)
{
  cJSON* deviceEndpointObj;
  char euiString[EUI64_STRING_LENGTH] = { 0 };
  deviceEndpointObj = cJSON_CreateObject();

  eui64ToString(eui64, euiString);
  cJSON_AddStringToObject(deviceEndpointObj, "eui64", euiString);
  cJSON_AddIntegerToObject(deviceEndpointObj, "endpoint", endpoint);
  return deviceEndpointObj;
}

static cJSON* buildDeviceEndpointWithClusterId(EmberEUI64 eui64,
                                               uint8_t endpoint,
                                               uint16_t clusterId)
{
  cJSON* deviceEndpointObj;
  char clusterIdString[CLUSTERID_STRING_LENGTH] = { 0 };

  sprintf(clusterIdString, "0x%04X", clusterId);
  deviceEndpointObj = buildDeviceEndpoint(eui64, endpoint);
  cJSON_AddStringToObject(deviceEndpointObj, "clusterId", clusterIdString);
  return deviceEndpointObj;
}

static cJSON* buildDeviceEndpointWithClusterInfo(
  EmberEUI64 eui64,
  uint8_t endpoint,
  uint16_t *clusterIds,
  uint8_t clusterOutStartPosition)
{
  cJSON* deviceEndpointObj;
  cJSON* clusterInfoArray;
  cJSON* clusterInfoItem;
  uint16_t clusterIdIndex;
  char clusterIdString[CLUSTERID_STRING_LENGTH] = { 0 };

  clusterInfoArray = cJSON_CreateArray();
  deviceEndpointObj = buildDeviceEndpoint(eui64, endpoint);

  for (clusterIdIndex = 0;
       clusterIdIndex < EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE;
       clusterIdIndex++) {
    clusterInfoItem = cJSON_CreateObject();
    if (clusterIds[clusterIdIndex] != ZCL_NULL_CLUSTER_ID) {
      sprintf(clusterIdString, "0x%04X", clusterIds[clusterIdIndex]);
      cJSON_AddStringToObject(clusterInfoItem, "clusterId", clusterIdString);
      if (clusterIdIndex < clusterOutStartPosition) {
        cJSON_AddStringToObject(clusterInfoItem, "clusterType", "In");
      } else {
        cJSON_AddStringToObject(clusterInfoItem, "clusterType", "Out");
      }
      cJSON_AddItemToArray(clusterInfoArray, clusterInfoItem);
      clusterInfoItem = NULL;
    } else {
      cJSON_Delete(clusterInfoItem);
      clusterInfoItem = NULL;
      break;
    }
  }
  cJSON_AddItemToObject(deviceEndpointObj, "clusterInfo", clusterInfoArray);
  return deviceEndpointObj;
}

static cJSON* buildNodeJson(uint16_t nodeIndex)
{
  cJSON* nodeJson;
  cJSON* deviceEndpoint;
  char nodeIdString[NODEID_STRING_LENGTH] = { 0 };
  char* deviceTypeString;
  uint32_t timeSinceLastMessage = halCommonGetInt32uMillisecondTick();
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();

  timeSinceLastMessage -= deviceTable[nodeIndex].lastMsgTimestamp;
  timeSinceLastMessage /= MILLISECOND_TICKS_PER_SECOND;

  nodeJson = cJSON_CreateObject();
  nodeIdToString(deviceTable[nodeIndex].nodeId, nodeIdString);
  cJSON_AddStringToObject(nodeJson, "nodeId", nodeIdString);
  cJSON_AddIntegerToObject(nodeJson,
                           "deviceState",
                           deviceTable[nodeIndex].state);
  deviceTypeString = createTwoByteHexString(deviceTable[nodeIndex].deviceId);
  cJSON_AddStringToObject(nodeJson, "deviceType", deviceTypeString);
  free(deviceTypeString);

  cJSON_AddIntegerToObject(nodeJson,
                           "timeSinceLastMessage",
                           timeSinceLastMessage);

  deviceEndpoint = buildDeviceEndpointWithClusterInfo(
    deviceTable[nodeIndex].eui64,
    deviceTable[nodeIndex].endpoint,
    deviceTable[nodeIndex].clusterIds,
    deviceTable[nodeIndex].clusterOutStartPosition);
  cJSON_AddItemToObject(nodeJson, "deviceEndpoint", deviceEndpoint);
  return nodeJson;
}

static void publishMqttTopic(char * topicNameString, cJSON * nodeJson)
{
  char* topic = allocateAndFormMqttGatewayTopic(topicNameString);
  char* nodeJsonString;

  nodeJsonString = cJSON_PrintUnformatted(nodeJson);
  emberAfPluginTransportMqttPublish(topic, nodeJsonString);
  free(nodeJsonString);
  cJSON_Delete(nodeJson);
  free(topic);
}

static void publishMqttHeartbeat(void)
{
  EmberNetworkParameters parameters;
  EmberNodeType nodeType;
  cJSON* heartbeatJson;
  char* panIdString;

  EmberStatus status = emberAfGetNetworkParameters(&nodeType, &parameters);
  heartbeatJson = cJSON_CreateObject();

  if (!emberAfNcpNeedsReset() && status == EMBER_SUCCESS) {
    cJSON_AddTrueToObject(heartbeatJson, "networkUp");
    panIdString = createTwoByteHexString(parameters.panId);
    cJSON_AddStringToObject(heartbeatJson, "networkPanId", panIdString);
    free(panIdString);
    cJSON_AddIntegerToObject(heartbeatJson,
                             "radioTxPower",
                             parameters.radioTxPower + 22);
    cJSON_AddIntegerToObject(heartbeatJson,
                             "radioChannel",
                             parameters.radioChannel);
  } else {
    cJSON_AddFalseToObject(heartbeatJson, "networkUp");
  }
  publishMqttTopic("heartbeat", heartbeatJson);
}

static void publishMqttDevices(void)
{
  uint16_t nodeIndex;
  cJSON* nodeJson;
  cJSON* devicesJson;
  cJSON* devicesJsonNodeArray;

  devicesJson = cJSON_CreateObject();
  devicesJsonNodeArray = cJSON_CreateArray();
  cJSON_AddItemToObject(devicesJson, "devices", devicesJsonNodeArray);

  for (nodeIndex = 0;
       nodeIndex < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE;
       nodeIndex++) {
    if (emberAfDeviceTableGetNodeIdFromIndex(nodeIndex)
        != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      nodeJson = buildNodeJson(nodeIndex);
      cJSON_AddItemToArray(devicesJsonNodeArray, nodeJson);
    }
  }
  publishMqttTopic("devices", devicesJson);
}

static void publishMqttRelays(void)
{
  cJSON* json;
  cJSON* itemJson;
  cJSON* itemsJsonArray;
  cJSON* deviceEndpointJson;
  EmberAfPluginCommandRelayEntry *relayTable;
  uint16_t i;

  relayTable = emberAfPluginCommandRelayTablePointer();
  json = cJSON_CreateObject();
  itemsJsonArray = cJSON_CreateArray();
  cJSON_AddItemToObject(json, "relays", itemsJsonArray);

  for (i = 0; i < EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE; i++) {
    if (isNullEui(relayTable[i].inDeviceEndpoint.eui64)) {
      continue;
    }

    itemJson = cJSON_CreateObject();
    deviceEndpointJson = buildDeviceEndpointWithClusterId(
      relayTable[i].inDeviceEndpoint.eui64,
      relayTable[i].inDeviceEndpoint.endpoint,
      relayTable[i].inDeviceEndpoint.clusterId);
    cJSON_AddItemToObject(itemJson, "inDeviceEndpoint", deviceEndpointJson);

    deviceEndpointJson = buildDeviceEndpointWithClusterId(
      relayTable[i].outDeviceEndpoint.eui64,
      relayTable[i].outDeviceEndpoint.endpoint,
      relayTable[i].outDeviceEndpoint.clusterId);
    cJSON_AddItemToObject(itemJson, "outDeviceEndpoint", deviceEndpointJson);

    cJSON_AddItemToArray(itemsJsonArray, itemJson);
  }
  publishMqttTopic("relays", json);
}

static void publishMqttSettings(void)
{
  EmberNetworkParameters parameters;
  EmberNodeType nodeType;
  cJSON* settingsJson;
  char ncpStackVerString[HOSTVER_STRING_LENGTH] = { 0 };
  EmberVersion versionStruct;
  uint8_t ncpEzspProtocolVer;
  uint8_t ncpStackType;
  uint16_t ncpStackVer;
  char* panIdString;
  uint8_t hostEzspProtocolVer = EZSP_PROTOCOL_VERSION;

  EmberStatus status = emberAfGetNetworkParameters(&nodeType, &parameters);

  ncpEzspProtocolVer = ezspVersion(hostEzspProtocolVer,
                                   &ncpStackType,
                                   &ncpStackVer);

  if (EZSP_SUCCESS == ezspGetVersionStruct(&versionStruct)) {
    sprintf(ncpStackVerString,
            "%d.%d.%d-%d",
            versionStruct.major,
            versionStruct.minor,
            versionStruct.patch,
            versionStruct.build);
  } else {
    sprintf(ncpStackVerString, "%2x", ncpStackVer);
  }

  settingsJson = cJSON_CreateObject();
  cJSON_AddStringToObject(settingsJson, "ncpStackVersion", ncpStackVerString);
  if (!emberAfNcpNeedsReset() && status == EMBER_SUCCESS) {
    cJSON_AddTrueToObject(settingsJson, "networkUp");
    panIdString = createTwoByteHexString(parameters.panId);
    cJSON_AddStringToObject(settingsJson, "networkPanId", panIdString);
    free(panIdString);
    cJSON_AddIntegerToObject(settingsJson,
                             "radioTxPower",
                             parameters.radioTxPower + 22);
    cJSON_AddIntegerToObject(settingsJson,
                             "radioChannel",
                             parameters.radioChannel);
  } else {
    cJSON_AddFalseToObject(settingsJson, "networkUp");
  }
  publishMqttTopic("settings", settingsJson);
}

static void publishMqttGatewayState(void)
{
  // Set an event to publish all the state updates so that they will be in scope
  // of the stack
  emberEventControlSetActive(emberAfPluginGatewayRelayMqttStateUpdateEventControl);
}

static void publishMqttDeviceStateChange(EmberEUI64 eui64,
                                         uint8_t state)
{
  char euiString[EUI64_STRING_LENGTH] = { 0 };
  cJSON* stateChangeJson;

  eui64ToString(eui64, euiString);

  stateChangeJson = cJSON_CreateObject();
  cJSON_AddStringToObject(stateChangeJson, "eui64", euiString);
  cJSON_AddIntegerToObject(stateChangeJson, "deviceState", state);
  publishMqttTopic("devicestatechange", stateChangeJson);
}

static void publishMqttDeviceJoined(EmberEUI64 eui64)
{
  //This function call breaks simulation test, mask it out
  //at this point as Mqtt messages are actually not included
  //in simluation test. It works normally with the real devices.
#ifndef EMBER_TEST
  uint16_t deviceTableIndex;
  cJSON* nodeJson;

  deviceTableIndex = emberAfDeviceTableGetFirstIndexFromEui64(eui64);
  nodeJson = buildNodeJson(deviceTableIndex);
  publishMqttTopic("devicejoined", nodeJson);
#endif //EMBER_TEST
}

static void publishMqttDeviceLeft(EmberEUI64 eui64)
{
  char euiString[EUI64_STRING_LENGTH] = { 0 };
  cJSON* nodeLeftJson;

  eui64ToString(eui64, euiString);
  nodeLeftJson = cJSON_CreateObject();
  cJSON_AddStringToObject(nodeLeftJson, "eui64", euiString);
  publishMqttTopic("deviceleft", nodeLeftJson);
}

static void publishMqttAttribute(EmberEUI64 eui64,
                                 EmberAfClusterId clusterId,
                                 uint8_t* buffer,
                                 uint16_t bufLen)
{
  uint16_t bufferIndex;
  char clusterIdString[CLUSTERID_STRING_LENGTH] = { 0 };
  char attribString[TWO_BYTE_HEX_STRING_SIZE] = { 0 };
  char dataTypeString[ONE_BYTE_HEX_STRING_SIZE] = { 0 };
  char* statusString;
  cJSON* globalReadJson;
  cJSON* deviceEndpointJson;
  // "0x" + 2 chars per byte + null char
  uint16_t bufferStringLength = ((2 * bufLen) + HEX_TOKEN_SIZE + 1);
  char* bufferString = (char*)malloc(bufferStringLength);
  uint8_t sourceEndpoint;

  memset(bufferString, 0, bufferStringLength);

  if (bufLen > 0) {
    sprintf(&bufferString[0], "0x");
  }

  // Print buffer data as a hex string, starting at the data start byte
  for (bufferIndex = ATTRIBUTE_BUFFER_DATA_START;
       bufferIndex < bufLen;
       bufferIndex++) {
    sprintf(&bufferString[(2 * (bufferIndex - ATTRIBUTE_BUFFER_DATA_START))
                          + HEX_TOKEN_SIZE],
            "%02X",
            buffer[bufferIndex]);
  }

  sprintf(clusterIdString, "0x%04X", clusterId);
  sprintf(attribString,
          "0x%02X%02X",
          buffer[ATTRIBUTE_BUFFER_ATTRIBUTEID_HIGH_BITS],
          buffer[ATTRIBUTE_BUFFER_ATTRIBUTEID_LOW_BITS]);
  sprintf(dataTypeString, "0x%02X", buffer[ATTRIBUTE_BUFFER_DATA_TYPE]);

  globalReadJson = cJSON_CreateObject();
  cJSON_AddStringToObject(globalReadJson, "clusterId", clusterIdString);
  cJSON_AddStringToObject(globalReadJson, "attributeId", attribString);
  cJSON_AddStringToObject(globalReadJson, "attributeBuffer", bufferString);
  cJSON_AddStringToObject(globalReadJson, "attributeDataType", dataTypeString);

  sourceEndpoint = emberAfCurrentCommand()->apsFrame->sourceEndpoint;
  deviceEndpointJson = buildDeviceEndpoint(eui64, sourceEndpoint);
  cJSON_AddItemToObject(globalReadJson, "deviceEndpoint", deviceEndpointJson);

  statusString = createOneByteHexString(buffer[ATTRIBUTE_BUFFER_SUCCESS_CODE]);
  cJSON_AddStringToObject(globalReadJson, "status", statusString);
  free(statusString);

  free(bufferString);
  publishMqttTopic(ZCL_RESPONSE_TOPIC, globalReadJson);
}

static void publishMqttAttributeReport(EmberEUI64 eui64,
                                       EmberAfClusterId clusterId,
                                       uint8_t* buffer,
                                       uint16_t bufLen)
{
  uint16_t bufferIndex;
  char euiString[EUI64_STRING_LENGTH] = { 0 };
  char clusterIdString[CLUSTERID_STRING_LENGTH] = { 0 };
  // "0x" + 4 chars + null char
  char attribString[TWO_BYTE_HEX_STRING_SIZE] = { 0 };
  // "0x" + 2 chars + null char
  char dataTypeString[ONE_BYTE_HEX_STRING_SIZE] = { 0 };
  cJSON* globalReadJson;
  cJSON* deviceEndpointJson;
  // "0x" + 2 chars per byte + null char
  uint16_t bufferStringLength = (2 * bufLen) + HEX_TOKEN_SIZE + 1;
  char* bufferString = (char*)malloc(bufferStringLength);
  memset(bufferString, 0, bufferStringLength);
  uint8_t sourceEndpoint;

  if (bufLen > 0) {
    sprintf(&bufferString[0], "0x");
  }

  // Print buffer data as a hex string, starting at the data start byte
  for (bufferIndex = ATTRIBUTE_BUFFER_REPORT_DATA_START;
       bufferIndex < bufLen;
       bufferIndex++) {
    sprintf(&bufferString[2 * (bufferIndex - ATTRIBUTE_BUFFER_REPORT_DATA_START)
                          + HEX_TOKEN_SIZE],
            "%02X",
            buffer[bufferIndex]);
  }

  eui64ToString(eui64, euiString);
  sprintf(clusterIdString, "0x%04X", clusterId);
  sprintf(attribString,
          "0x%02X%02X",
          buffer[ATTRIBUTE_BUFFER_ATTRIBUTEID_HIGH_BITS],
          buffer[ATTRIBUTE_BUFFER_ATTRIBUTEID_LOW_BITS]);
  sprintf(dataTypeString, "0x%02X", buffer[ATTRIBUTE_BUFFER_REPORT_DATA_TYPE]);

  globalReadJson = cJSON_CreateObject();
  cJSON_AddStringToObject(globalReadJson, "clusterId", clusterIdString);
  cJSON_AddStringToObject(globalReadJson, "attributeId", attribString);
  cJSON_AddStringToObject(globalReadJson, "attributeBuffer", bufferString);
  cJSON_AddStringToObject(globalReadJson, "attributeDataType", dataTypeString);

  sourceEndpoint = emberAfCurrentCommand()->apsFrame->sourceEndpoint;
  deviceEndpointJson = buildDeviceEndpoint(eui64, sourceEndpoint);
  cJSON_AddItemToObject(globalReadJson, "deviceEndpoint", deviceEndpointJson);

  free(bufferString);
  publishMqttTopic(ZCL_RESPONSE_TOPIC, globalReadJson);
}

static void publishMqttApsStatus(EmberStatus status,
                                 EmberAfClusterId clusterId,
                                 uint8_t commandId,
                                 uint16_t indexOrDestination)
{
  char* topic = allocateAndFormMqttGatewayTopic(APS_RESPONSE_TOPIC);
  cJSON* defaultResponseJson;
  char* defaultResponseString;
  char* stringData;
  char* statusString;
  bool squelchMessage = false;
  EmberEUI64 eui64;
  char euiString[EUI64_STRING_LENGTH] = { 0 };

  defaultResponseJson = cJSON_CreateObject();

  cJSON_AddStringToObject(defaultResponseJson, "statusType", "apsAck");

  emberAfDeviceTableGetEui64FromNodeId((EmberNodeId)indexOrDestination, eui64);
  eui64ToString(eui64, euiString);
  cJSON_AddStringToObject(defaultResponseJson, "eui64", euiString);

  statusString = createOneByteHexString(status);
  cJSON_AddStringToObject(defaultResponseJson, "status", statusString);
  free(statusString);

  stringData = createTwoByteHexString(clusterId);
  cJSON_AddStringToObject(defaultResponseJson, "clusterId", stringData);
  free(stringData);

  stringData = createOneByteHexString(commandId);
  cJSON_AddStringToObject(defaultResponseJson, "commandId", stringData);
  free(stringData);

  if (clusterId == ZCL_OTA_BOOTLOAD_CLUSTER_ID
      && commandId == ZCL_WRITE_ATTRIBUTES_NO_RESPONSE_COMMAND_ID) {
    squelchMessage = true;
  }

  defaultResponseString = cJSON_PrintUnformatted(defaultResponseJson);
  if (!squelchMessage) {
    emberAfPluginTransportMqttPublish(topic, defaultResponseString);
  }
  free(defaultResponseString);
  cJSON_Delete(defaultResponseJson);
  free(topic);
}

static void publishMqttTrafficReportEvent(char* messageType,
                                          EmberStatus* status,
                                          int8_t* lastHopRssi,
                                          uint8_t* lastHopLqi,
                                          uint8_t* zclSequenceNumber,
                                          uint64_t timeMS)
{
  cJSON* trafficReportJson;
  char* statusString;
  char timeString[20] = { 0 }; // 20 character maximum for in64u, including null

  sprintf(timeString, "%llu", timeMS);

  trafficReportJson = cJSON_CreateObject();
  cJSON_AddStringToObject(trafficReportJson, "messageType", messageType);

  if (status) {
    statusString = createOneByteHexString(*status);
    cJSON_AddStringToObject(trafficReportJson, "status", statusString);
    free(statusString);
  }

  if (lastHopRssi) {
    cJSON_AddIntegerToObject(trafficReportJson, "rssi", *lastHopRssi);
  }
  if (lastHopLqi) {
    cJSON_AddIntegerToObject(trafficReportJson, "linkQuality", *lastHopLqi);
  }
  if (zclSequenceNumber) {
    cJSON_AddIntegerToObject(trafficReportJson,
                             "sequenceNumber",
                             *zclSequenceNumber);
  }
  cJSON_AddStringToObject(trafficReportJson, "currentTimeMs", timeString);
  publishMqttTopic("trafficreportevent", trafficReportJson);
}

static void publishMqttOtaEvent(char* messageType,
                                EmberEUI64 eui64,
                                uint8_t* status,
                                uint32_t* blocksSent,
                                uint8_t* blockSize,
                                uint16_t* manufacturerId,
                                uint16_t* imageTypeId,
                                uint32_t* firmwareVersion)
{
  char euiString[EUI64_STRING_LENGTH] = { 0 };
  char manufacturerIdString[TWO_BYTE_HEX_STRING_SIZE] = { 0 };
  char imageTypeIdString[TWO_BYTE_HEX_STRING_SIZE] = { 0 };
  char firmwareVersionString[FOUR_BYTE_HEX_STRING_SIZE] = { 0 };
  cJSON* otaJson;
  char* statusString;

  eui64ToString(eui64, euiString);

  otaJson = cJSON_CreateObject();
  cJSON_AddStringToObject(otaJson, "messageType", messageType);
  cJSON_AddStringToObject(otaJson, "eui64", euiString);
  if (status) {
    statusString = createOneByteHexString(*status);
    cJSON_AddStringToObject(otaJson, "status", statusString);
    free(statusString);
  }
  if (blocksSent && blockSize) {
    cJSON_AddIntegerToObject(otaJson,
                             "bytesSent",
                             (*blocksSent) * (*blockSize));
  }

  if (manufacturerId) {
    sprintf(manufacturerIdString, "0x%04X", *manufacturerId);
    cJSON_AddStringToObject(otaJson, "manufacturerId", manufacturerIdString);
  }
  if (imageTypeId) {
    sprintf(imageTypeIdString, "0x%04X", *imageTypeId);
    cJSON_AddStringToObject(otaJson, "imageTypeId", imageTypeIdString);
  }
  if (firmwareVersion) {
    sprintf(firmwareVersionString, "0x%08X", *firmwareVersion);
    cJSON_AddStringToObject(otaJson, "firmwareVersion", firmwareVersionString);
  }
  publishMqttTopic("otaevent", otaJson);
}

static void publishMqttCommandExecuted(char* cliCommand)
{
  cJSON* executedJson;

  executedJson = cJSON_CreateObject();
  cJSON_AddStringToObject(executedJson, "command", cliCommand);
  publishMqttTopic("executed", executedJson);
}

static void publishMqttDelayExecuted(uint32_t postDelayMs)
{
  cJSON* executedJson;

  executedJson = cJSON_CreateObject();
  cJSON_AddIntegerToObject(executedJson, "delayMs", postDelayMs);
  publishMqttTopic("executed", executedJson);
}

static void publishMqttBindResponse(EmberNodeId nodeId,
                                    EmberApsFrame* apsFrame,
                                    uint8_t* message,
                                    uint16_t length)
{
  cJSON* objectJson;
  char* dataString;
  EmberEUI64 eui64;
  char euiString[EUI64_STRING_LENGTH] = { 0 };

  objectJson = cJSON_CreateObject();

  cJSON_AddStringToObject(objectJson, "zdoType", "bindResponse");

  emberAfDeviceTableGetEui64FromNodeId(nodeId, eui64);
  eui64ToString(eui64, euiString);
  cJSON_AddStringToObject(objectJson, "eui64", euiString);

  dataString = createOneByteHexString(
    message[DEVICE_TABLE_BIND_RESPONSE_STATUS]);
  cJSON_AddStringToObject(objectJson, "status", dataString);
  free(dataString);
  publishMqttTopic(ZDO_RESPONSE_TOPIC, objectJson);
}

static void publishMqttBindTableReponse(EmberNodeId nodeId,
                                        EmberApsFrame* apsFrame,
                                        uint8_t* message,
                                        uint16_t length)
{
  cJSON* objectJson;
  cJSON* entryArrayJson;
  cJSON* tableEntryJson;
  cJSON* deviceEndpointJson;
  uint8_t* messagePointer;
  uint8_t numEntries, entryCounter;
  char* dataString;
  char euiString[EUI64_STRING_LENGTH] = { 0 };
  EmberEUI64 eui64;

  numEntries = message[BINDING_TABLE_RESPONSE_NUM_ENTRIES]; // list count

  objectJson = cJSON_CreateObject();
  entryArrayJson = cJSON_CreateArray();

  cJSON_AddStringToObject(objectJson, "zdoType", "bindTableResponse");

  dataString = createOneByteHexString(message[BINDING_TABLE_RESPONSE_STATUS]);
  cJSON_AddStringToObject(objectJson, "status", dataString);
  free(dataString);

  emberAfDeviceTableGetEui64FromNodeId(nodeId, eui64);
  eui64ToString(eui64, euiString);
  cJSON_AddStringToObject(objectJson, "eui64", euiString);

  messagePointer = message + BINDING_TABLE_RESPONSE_ENTRIES;

  for (entryCounter = 0; entryCounter < numEntries; entryCounter++) {
    tableEntryJson = cJSON_CreateObject();

    deviceEndpointJson = buildDeviceEndpoint(
      &(messagePointer[BINDING_ENTRY_EUI]),
      messagePointer[BINDING_ENTRY_SOURCE_ENDPOINT]);
    cJSON_AddItemToObject(tableEntryJson,
                          "sourceDeviceEndpoint",
                          deviceEndpointJson);

    cJSON_AddIntegerToObject(tableEntryJson,
                             "addressMode",
                             messagePointer[BINDING_ENTRY_ADDRESS_MODE]);

    dataString =
      createTwoByteHexString(
        HIGH_LOW_TO_INT(messagePointer[BINDING_ENTRY_CLUSTER_ID + 1],
                        messagePointer[BINDING_ENTRY_CLUSTER_ID]));
    cJSON_AddStringToObject(tableEntryJson, "clusterId", dataString);
    free(dataString);

    deviceEndpointJson = buildDeviceEndpoint(
      &(messagePointer[BINDING_ENTRY_DEST_EUI]),
      messagePointer[BINDING_ENTRY_DEST_ENDPOINT]);
    cJSON_AddItemToObject(tableEntryJson,
                          "destDeviceEndpoint",
                          deviceEndpointJson);

    cJSON_AddItemToArray(entryArrayJson, tableEntryJson);

    messagePointer += BINDING_TABLE_RESPONSE_ENTRY_SIZE;
  }

  cJSON_AddItemToObject(objectJson, "bindTable", entryArrayJson);
  publishMqttTopic(ZDO_RESPONSE_TOPIC, objectJson);
}

static void publishMqttZclCommand(uint8_t commandId,
                                  boolean clusterSpecific,
                                  uint16_t clusterId,
                                  boolean mfgSpecific,
                                  uint16_t mfgCode,
                                  uint8_t* buffer,
                                  uint8_t bufLen,
                                  uint8_t payloadStartIndex)
{
  char* topic = allocateAndFormMqttGatewayTopic(ZCL_RESPONSE_TOPIC);
  cJSON* cmdResponseJson;
  cJSON* deviceEndpointJson;
  char* cmdResponseString;
  char* tempString;
  char* dataString = (char*)malloc(2 * (bufLen) + HEX_TOKEN_SIZE);
  uint8_t* bufPtr;
  uint16_t i;
  bool squelchMessage = false;
  EmberEUI64 eui64;
  EmberNodeId nodeId;
  uint8_t sourceEndpoint;

  if (dataString == NULL) {
    emberAfAppPrintln("FATAL ERR: Unable to allocate more memory!");
    assert(false);
  }

  for (i = 0; i < (2 * bufLen); i++) {
    dataString[i] = 0;
  }

  cmdResponseJson = cJSON_CreateObject();

  tempString = createTwoByteHexString(clusterId);
  cJSON_AddStringToObject(cmdResponseJson, "clusterId", tempString);
  free(tempString);

  tempString = createOneByteHexString(commandId);
  cJSON_AddStringToObject(cmdResponseJson, "commandId", tempString);
  free(tempString);

  if (clusterId == ZCL_OTA_BOOTLOAD_CLUSTER_ID
      && commandId == ZCL_WRITE_ATTRIBUTES_UNDIVIDED_COMMAND_ID) {
    squelchMessage = true;
  }

  if ((bufLen - payloadStartIndex) > 0) {
    sprintf(&dataString[0], "0x");
  }

  bufPtr = buffer + payloadStartIndex;
  for (i = 0; i < (bufLen - payloadStartIndex); i++) {
    sprintf(&(dataString[2 * i + HEX_TOKEN_SIZE]), "%02X", bufPtr[i]);
  }

  cJSON_AddStringToObject(cmdResponseJson, "commandData", dataString);
  free(dataString);

  if (clusterSpecific) {
    cJSON_AddTrueToObject(cmdResponseJson, "clusterSpecific");
  } else {
    cJSON_AddFalseToObject(cmdResponseJson, "clusterSpecific");
  }

  if (mfgSpecific) {
    tempString = createTwoByteHexString(mfgCode);
    cJSON_AddStringToObject(cmdResponseJson, "mfgCode", tempString);
    free(tempString);
  }

  deviceEndpointJson = cJSON_CreateObject();
  sourceEndpoint = emberAfCurrentCommand()->apsFrame->sourceEndpoint;
  nodeId = emberAfCurrentCommand()->source;
  emberAfDeviceTableGetEui64FromNodeId(nodeId, eui64);
  deviceEndpointJson = buildDeviceEndpoint(eui64, sourceEndpoint);
  cJSON_AddItemToObject(cmdResponseJson, "deviceEndpoint", deviceEndpointJson);

  cmdResponseString = cJSON_PrintUnformatted(cmdResponseJson);
  if (!squelchMessage) {
    emberAfPluginTransportMqttPublish(topic, cmdResponseString);
  }
  free(cmdResponseString);
  cJSON_Delete(cmdResponseJson);
  free(topic);
}

//command handler helper functions
static void handleCommandsMessage(cJSON* messageJson)
{
  uint8_t commandIndex;
  cJSON* commandsJson;
  cJSON* commandJson;
  cJSON* commandCliStringJson;
  cJSON* postDelayMsJson;
  cJSON* commandStringJson;

  if (messageJson != NULL) {
    char *messageString = cJSON_PrintUnformatted(messageJson);
    emberAfAppPrintln("Handling Commands Message: %s",
                      messageString);
    free(messageString);
    commandsJson = cJSON_GetObjectItem(messageJson, "commands");
    if (commandsJson != NULL) {
      for (commandIndex = 0;
           commandIndex < cJSON_GetArraySize(commandsJson);
           commandIndex++) {
        commandJson = cJSON_GetArrayItem(commandsJson, commandIndex);
        if (commandJson != NULL) {
          commandCliStringJson = cJSON_GetObjectItem(commandJson, "commandcli");
          if (commandCliStringJson != NULL) {
            addCommandToList(commandList,
                             commandCliStringJson->valuestring,
                             COMMAND_TYPE_CLI);
          } else {
            commandStringJson = cJSON_GetObjectItem(commandJson, "command");
            if (commandStringJson != NULL) {
              addCommandToList(commandList,
                               commandStringJson->valuestring,
                               COMMAND_TYPE_GATEWAY);
            }
          }
          postDelayMsJson = cJSON_GetObjectItem(commandJson, "postDelayMs");
          if (postDelayMsJson != NULL) {
            addPostDelayMsToList(commandList,
                                 (uint32_t)postDelayMsJson->valueint);
          }
        }
      }
    }
  }
}

static void addCommandToList(EmberAfPluginLinkedList* list,
                             const char* cliCommandString,
                             uint8_t commandType)
{
  GatewayCommand* gatewayCommand = allocateGatewayCommand();
  char* cliCommandStringForList =
    (char*)malloc(strlen(cliCommandString) + 1); // Add NULL char
  // Copies string including NULL char
  strcpy(cliCommandStringForList, cliCommandString);

  gatewayCommand->commandType = commandType;
  gatewayCommand->cliCommand = cliCommandStringForList;

  emberAfPluginLinkedListPushBack(list, (void*)gatewayCommand);
}

static void addPostDelayMsToList(EmberAfPluginLinkedList* commandList,
                                 uint32_t postDelayMs)
{
  GatewayCommand* gatewayCommand = allocateGatewayCommand();

  gatewayCommand->commandType = COMMAND_TYPE_POST_DELAY;
  gatewayCommand->postDelayMs = postDelayMs;

  emberAfPluginLinkedListPushBack(commandList, (void*)gatewayCommand);
}

static GatewayCommand* allocateGatewayCommand()
{
  GatewayCommand* gatewayCommand =
    (GatewayCommand*)malloc(sizeof(GatewayCommand));
  gatewayCommand->commandType = 0;
  gatewayCommand->cliCommand = NULL;
  gatewayCommand->resumeTime = 0;
  gatewayCommand->postDelayMs = 0;
  return gatewayCommand;
}

static void freeGatewayCommand(GatewayCommand* gatewayCommand)
{
  if (gatewayCommand != NULL) {
    if (gatewayCommand->cliCommand != NULL) {
      free(gatewayCommand->cliCommand);
    }
    free(gatewayCommand);
  }
}

static void handlePublishStateMessage(cJSON* messageJson)
{
  emberAfAppPrintln("Handling Publish State Message");
  publishMqttGatewayState();
}

static void handleUpdateSettingsMessage(cJSON* messageJson)
{
  if (messageJson != NULL) {
    char *messageString = cJSON_PrintUnformatted(messageJson);
    emberAfAppPrintln("Handling Update Settings Message: %s",
                      messageString);
    free(messageString);
  }
}

// String/other helpers
static void eui64ToString(EmberEUI64 eui, char* euiString)
{
  sprintf(euiString, "0x%02X%02X%02X%02X%02X%02X%02X%02X",
          eui[7],
          eui[6],
          eui[5],
          eui[4],
          eui[3],
          eui[2],
          eui[1],
          eui[0]);
}

static void nodeIdToString(EmberNodeId nodeId, char* nodeIdString)
{
  sprintf(nodeIdString, "0x%04X", nodeId);
}

static void printAttributeBuffer(uint16_t clusterId,
                                 uint8_t* buffer,
                                 uint16_t bufLen)
{
  uint16_t bufferIndex;

  emberAfAppPrintln(" Cluster, Attribute: %04X, %02X%02X"
                    " Success Code: %02X"
                    " Data Type: %02X\n"
                    " Hex Buffer: ",
                    clusterId,
                    buffer[ATTRIBUTE_BUFFER_ATTRIBUTEID_HIGH_BITS],
                    buffer[ATTRIBUTE_BUFFER_ATTRIBUTEID_LOW_BITS],
                    buffer[ATTRIBUTE_BUFFER_SUCCESS_CODE],
                    buffer[ATTRIBUTE_BUFFER_DATA_TYPE]);

  // Print buffer data as a hex string, starting at the data start byte
  for (bufferIndex = ATTRIBUTE_BUFFER_DATA_START;
       bufferIndex < bufLen;
       bufferIndex++) {
    emberAfAppPrint("%02X", buffer[bufferIndex]);
  }
  emberAfAppPrintln("");
}
//callbacks starts from here
void emberAfPluginGatewayRelayMqttInitCallback(void)
{
  // Save our EUI information
  emberAfGetEui64(gatewayEui64);
  sprintf(gatewayEui64String, "%02X%02X%02X%02X%02X%02X%02X%02X",
          gatewayEui64[7],
          gatewayEui64[6],
          gatewayEui64[5],
          gatewayEui64[4],
          gatewayEui64[3],
          gatewayEui64[2],
          gatewayEui64[1],
          gatewayEui64[0]);

  strcat(gatewayTopicUriPrefix, "gw/");
  strcat(gatewayTopicUriPrefix, gatewayEui64String);
  strcat(gatewayTopicUriPrefix, "/");
  emberAfAppPrintln("HA Gateweay EUI64 = %s",
                    gatewayEui64String);

  // Init our command list
  commandList = emberAfPluginLinkedListInit();

  // Init our topic handler list and the maps in the list, note that this is
  // done after the topicUriPrefix is assigned above, since it is used here
  topicHandlerList = emberAfPluginLinkedListInit();
  emberAfPluginLinkedListPushBack(topicHandlerList,
                                  (void*)buildTopicHandler(
                                    "commands",
                                    handleCommandsMessage));
  emberAfPluginLinkedListPushBack(topicHandlerList,
                                  (void*)buildTopicHandler(
                                    "publishstate",
                                    handlePublishStateMessage));
  emberAfPluginLinkedListPushBack(topicHandlerList,
                                  (void*)buildTopicHandler(
                                    "updatesettings",
                                    handleUpdateSettingsMessage));
  emberEventControlSetActive(emberAfPluginGatewayRelayMqttStateUpdateEventControl);
}

//plugin specific callbacks
void emberAfPluginOtaServerUpdateCompleteCallback(uint16_t manufacturerId,
                                                  uint16_t imageTypeId,
                                                  uint32_t firmwareVersion,
                                                  EmberNodeId nodeId,
                                                  uint8_t status)
{
  char* messageType = (status == EMBER_ZCL_STATUS_SUCCESS) ? "otaFinished"
                      : "otaFailed";

  EmberEUI64 nodeEui64;
  emberAfDeviceTableGetEui64FromNodeId(nodeId, nodeEui64);

  publishMqttOtaEvent(messageType,
                      nodeEui64,
                      &status,
                      NULL, // blockSent is unused
                      NULL, // actualLength is unused
                      &manufacturerId,
                      &imageTypeId,
                      &firmwareVersion);

  // Note that this global block sent count only supports 1 OTA
  otaBlockSent = 0;
}

void emberAfPluginOtaServerBlockSentCallback(uint8_t actualLength,
                                             uint16_t manufacturerId,
                                             uint16_t imageTypeId,
                                             uint32_t firmwareVersion)
{
  // Use a throttle value here to control the amount of updates being published
  if (otaBlockSent % BLOCK_SENT_THROTTLE_VALUE == 0) {
    EmberNodeId nodeId = emberAfCurrentCommand()->source;
    EmberEUI64 nodeEui64;
    emberAfDeviceTableGetEui64FromNodeId(nodeId, nodeEui64);
    publishMqttOtaEvent("otaBlockSent",
                        nodeEui64,
                        NULL, // status is unused
                        &otaBlockSent,
                        &actualLength,
                        &manufacturerId,
                        &imageTypeId,
                        &firmwareVersion);
  }

  otaBlockSent++;
}

void emberAfPluginOtaServerUpdateStartedCallback(uint16_t manufacturerId,
                                                 uint16_t imageTypeId,
                                                 uint32_t firmwareVersion,
                                                 uint8_t maxDataSize,
                                                 uint32_t offset)
{
  // Note that this global block sent count only supports 1 OTA
  otaBlockSent = 0;
  EmberNodeId nodeId = emberAfCurrentCommand()->source;
  EmberEUI64 nodeEui64;
  emberAfDeviceTableGetEui64FromNodeId(nodeId, nodeEui64);
  publishMqttOtaEvent("otaStarted",
                      nodeEui64,
                      NULL, // status is unused
                      NULL, // blockSent is unused
                      NULL, // actualLength is unused
                      &manufacturerId,
                      &imageTypeId,
                      &firmwareVersion);
}

void emberAfPluginDeviceTableNewDeviceCallback(EmberEUI64 nodeEui64)
{
  publishMqttDeviceJoined(nodeEui64);
}

void emberAfPluginDeviceTableDeviceLeftCallback(EmberEUI64 nodeEui64)
{
  publishMqttDeviceLeft(nodeEui64);
}

void emberAfPluginDeviceTableRejoinDeviceCallback(EmberEUI64 nodeEui64)
{
  publishMqttDeviceJoined(nodeEui64);
}

void emberAfPluginDeviceTableStateChangeCallback(EmberNodeId nodeId,
                                                 uint8_t state)
{
  EmberEUI64 nodeEui64;
  emberAfDeviceTableGetEui64FromNodeId(nodeId, nodeEui64);
  publishMqttDeviceStateChange(nodeEui64, state);
}

void emberAfPluginDeviceTableClearedCallback(void)
{
  publishMqttGatewayState();
}

void emberAfPluginCommandRelayChangedCallback(void)
{
  publishMqttRelays();
}

void emberAfPluginTransportMqttStateChangedCallback(
  EmberAfPluginTransportMqttState state)
{
  switch (state) {
    case EMBER_AF_PLUGIN_TRANSPORT_MQTT_STATE_CONNECTED:
    {
      EmberAfPluginLinkedListElement* currentElement = NULL;
      MqttTopicHandlerMap* topicHandlerMap = NULL;

      emberAfAppPrintln(
        "MQTT connected, starting gateway heartbeat and command processing");
      emberEventControlSetActive(
        emberAfPluginGatewayRelayMqttHeartbeatEventControl);
      emberEventControlSetActive(
        emberAfPluginGatewayRelayMqttProcessCommandEventControl);

      // Loop through the Topic Handler Map to subscribe to all the topics
      do {
        currentElement = emberAfPluginLinkedListNextElement(topicHandlerList,
                                                            currentElement);
        if (currentElement != NULL) {
          topicHandlerMap = (MqttTopicHandlerMap*)currentElement->content;
          emberAfPluginTransportMqttSubscribe(topicHandlerMap->topic);
        }
      } while (currentElement != NULL);

      // Since we are newly connecting, dump our complete device state and relay
      // list
      publishMqttGatewayState();
      break;
    }
    case EMBER_AF_PLUGIN_TRANSPORT_MQTT_STATE_DISCONNECTED:
      emberAfAppPrintln("MQTT disconnected, stopping gateway heartbeat");
      emberEventControlSetInactive(
        emberAfPluginGatewayRelayMqttHeartbeatEventControl);
      emberEventControlSetInactive(
        emberAfPluginGatewayRelayMqttProcessCommandEventControl);
      break;
    default:
      // Unknown state
      emberAfAppPrintln("Unknown MQTT state");
      assert(false);
      break;
  }
}

bool emberAfPluginTransportMqttMessageArrivedCallback(const char* topic,
                                                      const char* payload)
{
  cJSON* incomingMessageJson;
  EmberAfPluginLinkedListElement* currentElement = NULL;
  MqttTopicHandlerMap* topicHandlerMap = NULL;

  incomingMessageJson = cJSON_Parse(payload);

  // Loop through the Topic Handler Map to determine which handler to call
  do {
    currentElement = emberAfPluginLinkedListNextElement(topicHandlerList,
                                                        currentElement);
    if (currentElement != NULL) {
      topicHandlerMap = (MqttTopicHandlerMap*)currentElement->content;

      // If the incoming topic matches a topic in the map, call it's handler
      if (strcmp(topic, topicHandlerMap->topic) == 0) {
        topicHandlerMap->topicHandler(incomingMessageJson);
        break;
      }
    }
  } while (currentElement != NULL);

  cJSON_Delete(incomingMessageJson);

  // Return true, this tells the MQTT client we have handled the incoming
  // message
  return true;
}

//Non-cluster related callbacks
bool emberAfMessageSentCallback(EmberOutgoingMessageType type,
                                uint16_t indexOrDestination,
                                EmberApsFrame* apsFrame,
                                uint16_t msgLen,
                                uint8_t* message,
                                EmberStatus status)
{
  if (trafficReporting) {
    // This specifically uses the emberAfIncomingZclSequenceNumber instead
    // of the adjusted sequence number used in emberAfPreMessageReceivedCallback
    // and emberAfPreMessageSendCallback
    publishMqttTrafficReportEvent("messageSent",
                                  &status,
                                  NULL, // rssi unused
                                  NULL, // lqi unused
                                  NULL,
                                  halCommonGetInt32uMillisecondTick());
  }

  publishMqttApsStatus(status,
                       apsFrame->clusterId,
                       message[COMMAND_OFFSET],
                       indexOrDestination);

  // track the state of the device, except for broadcasts
  if (emberIsZigbeeBroadcastAddress(indexOrDestination)) {
    return false;
  }

  emberAfPluginDeviceTableMessageSentStatus(indexOrDestination,
                                            status,
                                            apsFrame->profileId,
                                            apsFrame->clusterId);

  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("%2x failed with code %x",
                      indexOrDestination,
                      status);

    if (indexOrDestination >= EMBER_DISCOVERY_ACTIVE_NODE_ID) {
      return false;
    }
    emberAfPluginDeviceTableInitiateRouteRepair(indexOrDestination);
  }

  return false;
}

bool emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* incomingMessage)
{
  if (trafficReporting) {
    publishMqttTrafficReportEvent("preMessageReceived",
                                  NULL, // status unsused
                                  &(incomingMessage->lastHopRssi),
                                  &(incomingMessage->lastHopLqi),
                                  NULL,
                                  halCommonGetInt32uMillisecondTick());
  }
  return false;
}

bool emberAfPreMessageSendCallback(EmberAfMessageStruct* messageStruct,
                                   EmberStatus* status)
{
  if (trafficReporting) {
    publishMqttTrafficReportEvent("preMessageSend",
                                  NULL, // status unsused
                                  NULL, // rssi unused
                                  NULL, // lqi unused
                                  NULL,
                                  halCommonGetInt32uMillisecondTick());
  }
  return false;
}

boolean emberAfConfigureReportingResponseCallback(EmberAfClusterId clusterId,
                                                  uint8_t *buffer,
                                                  uint16_t bufLen)
{
  cJSON* configureReportResponseJson;
  cJSON* deviceEndpointJson;
  char* dataString;
  EmberEUI64 eui64;
  EmberNodeId nodeId;
  uint8_t sourceEndpoint;

  configureReportResponseJson = cJSON_CreateObject();

  cJSON_AddStringToObject(configureReportResponseJson,
                          "zdoType",
                          "configureReportResponse");

  nodeId = emberAfCurrentCommand()->source;
  emberAfDeviceTableGetEui64FromNodeId(nodeId, eui64);
  sourceEndpoint = emberAfCurrentCommand()->apsFrame->sourceEndpoint;
  deviceEndpointJson = buildDeviceEndpoint(eui64, sourceEndpoint);
  cJSON_AddItemToObject(configureReportResponseJson,
                        "deviceEndpoint",
                        deviceEndpointJson);

  dataString = createOneByteHexString(buffer[0]);
  cJSON_AddStringToObject(configureReportResponseJson,
                          "status",
                          dataString);
  free(dataString);
  publishMqttTopic(ZDO_RESPONSE_TOPIC, configureReportResponseJson);
  return false;
}

bool emberAfReadReportingConfigurationResponseCallback(
  EmberAfClusterId clusterId,
  uint8_t *buffer,
  uint16_t bufLen)
{
  cJSON* reportTableJson;
  cJSON* deviceEndpointJson;
  //"0x" + buffer
  char* dataString = (char*)malloc(2 * (bufLen) + HEX_TOKEN_SIZE);
  uint8_t i;
  uint16_t maxInterval;
  uint16_t minInterval;
  char* tempString;
  EmberEUI64 eui64;
  EmberNodeId nodeId;
  uint8_t sourceEndpoint;

  if (dataString == NULL) {
    emberAfAppPrintln("FATAL ERR: Unable to allocate more memory!");
    assert(false);
  }

  for (i = 0; i < (2 * bufLen); i++) {
    dataString[i] = 0;
  }

  reportTableJson = cJSON_CreateObject();

  cJSON_AddStringToObject(reportTableJson,
                          "zclType",
                          "reportTableEntry");

  nodeId = emberAfCurrentCommand()->source;
  emberAfDeviceTableGetEui64FromNodeId(nodeId, eui64);
  sourceEndpoint = emberAfCurrentCommand()->apsFrame->sourceEndpoint;
  deviceEndpointJson = buildDeviceEndpoint(eui64, sourceEndpoint);
  cJSON_AddItemToObject(reportTableJson, "deviceEndpoint", deviceEndpointJson);

  tempString = createOneByteHexString(buffer[READ_REPORT_CONFIG_STATUS]);
  cJSON_AddStringToObject(reportTableJson, "status", tempString);
  free(tempString);

  cJSON_AddIntegerToObject(reportTableJson,
                           "direction",
                           buffer[READ_REPORT_CONFIG_DIRECTION]);

  tempString = createTwoByteHexString(clusterId);
  cJSON_AddStringToObject(reportTableJson, "clusterId", tempString);
  free(tempString);

  tempString = createTwoByteHexString(
    HIGH_LOW_TO_INT(buffer[READ_REPORT_CONFIG_ATTRIBUTE_ID + 1],
                    buffer[READ_REPORT_CONFIG_ATTRIBUTE_ID]));
  cJSON_AddStringToObject(reportTableJson, "attributeId", tempString);
  free(tempString);

  tempString = createOneByteHexString(buffer[READ_REPORT_CONFIG_DATA_TYPE]);
  cJSON_AddStringToObject(reportTableJson, "dataType", tempString);
  free(tempString);

  minInterval = HIGH_LOW_TO_INT(buffer[READ_REPORT_CONFIG_MIN_INTERVAL + 1],
                                buffer[READ_REPORT_CONFIG_MIN_INTERVAL]);

  cJSON_AddIntegerToObject(reportTableJson, "minInterval", minInterval);

  maxInterval = HIGH_LOW_TO_INT(buffer[READ_REPORT_CONFIG_MAX_INTERVAL + 1],
                                buffer[READ_REPORT_CONFIG_MAX_INTERVAL]);

  cJSON_AddIntegerToObject(reportTableJson, "maxInterval", maxInterval);

  if (bufLen > 0) {
    sprintf(&dataString[0], "0x");
  }

  for (i = READ_REPORT_CONFIG_DATA; i < bufLen; i++) {
    sprintf(&(dataString[2 * (i - READ_REPORT_CONFIG_DATA) + HEX_TOKEN_SIZE]),
            "%02X", buffer[i]);
  }
  cJSON_AddStringToObject(reportTableJson, "data", dataString);
  publishMqttTopic(ZCL_RESPONSE_TOPIC, reportTableJson);
  free(dataString);
  return false;
}

bool emberAfReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                           uint8_t* buffer,
                                           uint16_t bufLen)
{
  EmberEUI64 nodeEui64;
  EmberNodeId nodeId = emberAfCurrentCommand()->source;
  emberAfDeviceTableGetEui64FromNodeId(nodeId, nodeEui64);

  // If a zero-length attribute is reported, just leave
  if (bufLen == 0) {
    emberAfAppPrintln("Read attributes callback: zero length buffer");
    return false;
  }

  emberAfAppPrintln("Read attributes: 0x%2x", clusterId);
  publishMqttAttribute(nodeEui64,
                       clusterId,
                       buffer,
                       bufLen);
  printAttributeBuffer(clusterId, buffer, bufLen);

  return false;
}

bool emberAfReportAttributesCallback(EmberAfClusterId clusterId,
                                     uint8_t * buffer,
                                     uint16_t bufLen)
{
  EmberEUI64 nodeEui64;
  EmberNodeId nodeId = emberAfCurrentCommand()->source;
  emberAfDeviceTableGetEui64FromNodeId(nodeId, nodeEui64);
  uint8_t * bufferTemp;
  uint8_t * bufferPtr = buffer;
  uint8_t i, bufferSize;

  if (bufLen == 0) {
    emberAfAppPrintln("Report attributes callback: zero length buffer");
    return false;
  }

  // Buffer [0] is low bits, Buffer [1] is high bits, Buffer [2] is dataType,
  // Buffer [3+] is data
  emberAfAppPrintln("Reporting attributes for cluster: 0x%2x", clusterId);
  for (i = 0; i < bufLen; ) {
    // Get Length of Attribute Buffer
    bufferSize = emberAfGetDataSize(
      bufferPtr[ATTRIBUTE_BUFFER_REPORT_DATA_TYPE]);
    // Add 3 bytes for header size
    bufferSize += 3;

    //Copy buffer to attributeBufferI
    bufferTemp = (uint8_t*)malloc(bufferSize);
    memcpy(bufferTemp, bufferPtr, bufferSize);

    // Set i to point to:
    // [attrLSB,attrMSB,dataT,buffer,nAttrLSB,nAttrMSB,nextDataT,nextBuffer]
    // [       ,       ,     , ..n  ,   X    ,        ,         ,  ..n     ]
    bufferPtr = bufferPtr + bufferSize;
    i = i + bufferSize;

    emberAfAppPrintln("Reported attribute: 0x%02X%02X, Type: %02X",
                      bufferTemp[ATTRIBUTE_BUFFER_ATTRIBUTEID_HIGH_BITS],
                      bufferTemp[ATTRIBUTE_BUFFER_ATTRIBUTEID_LOW_BITS],
                      bufferTemp[ATTRIBUTE_BUFFER_REPORT_DATA_TYPE]);

    publishMqttAttributeReport(nodeEui64,
                               clusterId,
                               bufferTemp,
                               bufferSize);
    free(bufferTemp);
  }

  return false;
}

//bookkeeping callbacks
bool emberAfPluginGatewayRelayMqttPreCommandReceivedCallback(
  EmberAfClusterCommand* cmd)
{
  publishMqttZclCommand(cmd->commandId,
                        cmd->clusterSpecific,
                        cmd->apsFrame->clusterId,
                        cmd->mfgSpecific,
                        cmd->mfgCode,
                        cmd->buffer,
                        cmd->bufLen,
                        cmd->payloadStartIndex);
  return false;
}

bool emberAfPluginGatewayRelayMqttPreZDOMessageReceivedCallback(
  EmberNodeId emberNodeId,
  EmberApsFrame* apsFrame,
  uint8_t* message,
  uint16_t length)
{
  switch (apsFrame->clusterId) {
    case ACTIVE_ENDPOINTS_RESPONSE:
      break;
    case SIMPLE_DESCRIPTOR_RESPONSE:
      break;
    case END_DEVICE_ANNOUNCE:
      break;
    case PERMIT_JOINING_RESPONSE:
      break;
    case LEAVE_RESPONSE:
      break;
    case BIND_RESPONSE:
      publishMqttBindResponse(emberNodeId, apsFrame, message, length);
      break;
    case BINDING_TABLE_RESPONSE:
      publishMqttBindTableReponse(emberNodeId, apsFrame, message, length);
      break;
    case NETWORK_ADDRESS_RESPONSE:
      break;
    case IEEE_ADDRESS_RESPONSE:
      break;
    default:
      break;
  }
  return false;
}

// event handlers
void emberAfPluginGatewayRelayMqttHeartbeatEventHandler(void)
{
  publishMqttHeartbeat();
  emberEventControlSetDelayMS(emberAfPluginGatewayRelayMqttHeartbeatEventControl,
                              heatBeatRateMs);
}

void emberAfPluginGatewayRelayMqttStateUpdateEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginGatewayRelayMqttStateUpdateEventControl);
  publishMqttSettings();
  publishMqttRelays();
  publishMqttDevices();
}

void emberAfPluginGatewayRelayMqttProcessCommandEventHandler(void)
{
  emberEventControlSetDelayMS(
    emberAfPluginGatewayRelayMqttProcessCommandEventControl,
    PROCESS_COMMAND_RATE_MS);
  EmberAfPluginLinkedListElement* commandListItem = NULL;
  GatewayCommand* gatewayCommand;

  assert(commandList != NULL);

  // Get the head of the command list
  commandListItem = emberAfPluginLinkedListNextElement(commandList,
                                                       NULL);

  // If there is nothing there, continue on
  if (commandListItem == NULL) {
    return;
  }

  gatewayCommand = commandListItem->content;
  assert(gatewayCommand != NULL);

  // CLI command processing
  if (gatewayCommand->commandType == COMMAND_TYPE_CLI) {
    // Process our command string, then pop the command from the list
    // First send the CLI, then send a /n to simulate the "return" key
    emberProcessCommandString((uint8_t*)gatewayCommand->cliCommand,
                              strlen(gatewayCommand->cliCommand));
    emberProcessCommandString((uint8_t*)"\n",
                              strlen("\n"));
    publishMqttCommandExecuted(gatewayCommand->cliCommand);
    emberAfAppPrintln("CLI command executed: %s",
                      gatewayCommand->cliCommand);
    freeGatewayCommand(gatewayCommand);
    emberAfPluginLinkedListPopFront(commandList);
    //gateway command processing
  } else if (gatewayCommand->commandType == COMMAND_TYPE_GATEWAY) {
    emAfPluginGatewayRelayMqttCommandsProccessing(
      (uint8_t*)gatewayCommand->cliCommand);
    publishMqttCommandExecuted(gatewayCommand->cliCommand);
    emberAfAppPrintln("gateway command executed: %s",
                      gatewayCommand->cliCommand);
    freeGatewayCommand(gatewayCommand);
    emberAfPluginLinkedListPopFront(commandList);
    // Delay processing
  } else if (gatewayCommand->commandType == COMMAND_TYPE_POST_DELAY) {
    // If our resume time hasn't been initialized we are starting the delay
    if (gatewayCommand->resumeTime == 0) {
      // Make sure delay isn't 0, if so pop the list and move on
      if (gatewayCommand->postDelayMs == 0) {
        freeGatewayCommand(gatewayCommand);
        emberAfPluginLinkedListPopFront(commandList);
      }
      // Calculate the time to resume
      gatewayCommand->resumeTime = halCommonGetInt32uMillisecondTick()
                                   + gatewayCommand->postDelayMs;
    } else {
      // If we are already delaying, see if it's time to resume
      if (halCommonGetInt32uMillisecondTick() > gatewayCommand->resumeTime) {
        // Resume by popping this delay from the list
        publishMqttDelayExecuted(gatewayCommand->postDelayMs);
        emberAfAppPrintln("Delay executed for: %d ms",
                          gatewayCommand->postDelayMs);
        freeGatewayCommand(gatewayCommand);
        emberAfPluginLinkedListPopFront(commandList);
      }
    }
  }
}

//APIs
/** @brief Function to set the heart beat interval
 *
 * Function to set the mqtt heart beat interval in milliseconds
 *
 * @param intervalMs heart beat interval in milliseconds
 */
void emberPluginGatewayRelayMqttSetHeartBeat(uint16_t intervalMs)
{
  heatBeatRateMs = intervalMs;
}
