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
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "stack/include/trust-center.h"
#include EMBER_AF_API_DEVICE_TABLE
#include EMBER_AF_API_COMMAND_RELAY
#include EMBER_AF_API_NETWORK_CREATOR
#include EMBER_AF_API_NETWORK_CREATOR_SECURITY
#include "app/framework/plugin/ota-common/ota-cli.h"
#include "app/framework/plugin/ota-server-policy/ota-server-policy.h"
#include "app/framework/plugin/ota-server/ota-server.h"
#include "app/framework/plugin/ota-storage-simple/ota-storage-simple-driver.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "app/framework/plugin/ias-zone-client/ias-zone-client.h"
#include "gateway-relay-mqtt.h"
#include "gateway-relay-mqtt-commands.h"

#include <stdlib.h>
#include <string.h>

#define ZCL_BUFFER_SIZE 255
#define MAX_COMMAND_LEN 255

#define NUMBER_OF_COMMANDS (sizeof(commandArray) \
                            / sizeof(EmAfPluginGatewayRelayMqttCommand))

#define setTcRejoinsUsingWellKnownKeyAllowed(allow) \
  (void)ezspSetPolicy(EZSP_TC_REJOINS_USING_WELL_KNOWN_KEY_POLICY, (allow))
#define setTcRejoinsUsingWellKnownKeyTimeout(timeout) \
  (void)ezspSetConfigurationValue(EZSP_CONFIG_TC_REJOINS_USING_WELL_KNOWN_KEY_TIMEOUT_S, (timeout))

//command struct array definition
static const EmAfPluginGatewayRelayMqttCommand commandArray[] = {
  { .commandString = COMMAND_STRING_START_NETWORK,
    .functionPtr = &emAfPluginGatewayRelayMqttStartNetwork },
  { .commandString = COMMAND_STRING_FORM_NETWORK,
    .functionPtr = &emAfPluginGatewayRelayMqttStartNetworkWithParameters },
  { .commandString = COMMAND_STRING_OPEN_NETWORK,
    .functionPtr = &emAfPluginGatewayRelayMqttOpenNetwork },
  { .commandString = COMMAND_STRING_CLOSE_NETWORK,
    .functionPtr = &emAfPluginGatewayRelayMqttCloseNetwork },
  { .commandString = COMMAND_STRING_SET_KEY,
    .functionPtr = &emAfPluginGatewayRelayMqttSetJoiningKey },
  { .commandString = COMMAND_STRING_CLEAR_KEY,
    .functionPtr = &emAfPluginGatewayRelayMqttClearJoiningKeys },
  { .commandString = COMMAND_STRING_OPEN_WITH_KEY,
    .functionPtr = &emAfPluginGatewayRelayMqttOpenWithKey },
  { .commandString = COMMAND_STRING_ZCL_SEND,
    .functionPtr = &emAfPluginGatewayRelayMqttZclSend },
  { .commandString = COMMAND_STRING_DEVICE_TABLE_CLEAR,
    .functionPtr = &emAfPluginGatewayRelayMqttDeviceTableClear },
  { .commandString = COMMAND_STRING_COMMAND_RElAY_ADD,
    .functionPtr = &emAfPluginGatewayRelayMqttCommandRelayAdd },
  { .commandString = COMMAND_STRING_COMMAND_RELAY_REMOVE,
    .functionPtr = &emAfPluginGatewayRelayMqttCommandRelayRemove },
  { .commandString = COMMAND_STRING_COMMAND_RELAY_CLEAR,
    .functionPtr = &emAfPluginGatewayRelayMqttCommandRelayClear },
  { .commandString = COMMAND_STRING_OTA_RELOAD,
    .functionPtr = &emAfPluginGatewayRelayMqttOtaStorageCommonReload },
  { .commandString = COMMAND_STRING_OTA_POLICY_QUERY,
    .functionPtr = &emAfPluginGatewayRelayMqttOtaServerPolicyQuery },
  { .commandString = COMMAND_STRING_OTA_NOTIFY,
    .functionPtr = &emAfPluginGatewayRelayMqttOtaServerNotify },
  { .commandString = COMMAND_STRING_IAS_ZONE_CLEAR,
    .functionPtr = &emAfPluginGatewayRelayMqttIasZoneClientClearAll },
  { .commandString = COMMAND_STRING_ZDO_LEAVE,
    .functionPtr = &emAfPluginGatewayRelayMqttZdoLeave },
  { .commandString = COMMAND_STRING_ZDO_BIND,
    .functionPtr = &emAfPluginGatewayRelayMqttZdoBind },
  { .commandString = COMMAND_STRING_BROAD_PJOIN,
    .functionPtr = &emAfPluginGatewayRelayMqttNetworkBoardPjoin },
  { .commandString = COMMAND_STRING_NETWORK_LEAVE,
    .functionPtr = &emAfPluginGatewayRelayMqttNetworkLeave },
  { .commandString = COMMAND_STRING_NETWORK_FORM,
    .functionPtr = &emAfPluginGatewayRelayMqttNetworkForm },
  { .commandString = COMMAND_STRING_ZCL_GLOBAL_DIRECTION,
    .functionPtr = &emAfPluginGatewayRelayMqttZclGlobalDirection },
  { .commandString = COMMAND_STRING_ZCL_GLOBAL_READ,
    .functionPtr = &emAfPluginGatewayRelayMqttZclGlobalRead },
  { .commandString = COMMAND_STRING_ZCL_GLOBAL_WRITE,
    .functionPtr = &emAfPluginGatewayRelayMqttZclGlobalWrite },
  { .commandString = COMMAND_STRING_ZCL_ON_OFF,
    .functionPtr = &emAfPluginGatewayRelayMqttZclOnOff },
  { .commandString = COMMAND_STRING_ZCL_ON_MOVE_TO_LEVEL,
    .functionPtr = &emAfPluginGatewayRelayMqttZclOnAndMoveToLevel },
  { .commandString = COMMAND_STRING_ZCL_MOVE_TO_COLORTEMP,
    .functionPtr = &emAfPluginGatewayRelayMqttZclMoveToColorTemp },
  { .commandString = COMMAND_STRING_ZCL_MOVE_TO_HUESAT,
    .functionPtr = &emAfPluginGatewayRelayMqttZclMoveToHueAndSat },
  { .commandString = COMMAND_STRING_ZCL_SEND_ME_A_REPORT,
    .functionPtr = &emAfPluginGatewayRelayMqttZclGlobalSendMeAReport },
  { .commandString = COMMAND_STRING_ZCL_GROUP,
    .functionPtr = &emAfPluginGatewayRelayMqttZclGroup },
  { .commandString = COMMAND_STRING_OPTION_SECURITY_ALLOW_UNSECURE_REJOIN,
    .functionPtr = &emAfPluginGatewayRelayMqttAllowRejoin },
  { .commandString = COMMAND_STRING_OPTION_SECURITY_SET_UNSECURE_REJOIN_TIMEOUT,
    .functionPtr = &emAfPluginGatewayRelayMqttSetRejoinTimeout }
};

static uint8_t zclFrameControlDirection = ZCL_FRAME_CONTROL_CLIENT_TO_SERVER;
EmberApsFrame localApsFrame;
uint8_t appZclBuffer[ZCL_BUFFER_SIZE];
uint16_t appZclBufferLen;
static bool zclCmdIsBuilt = false;

//forward declraration of command helper functions
static const uint8_t * scanBracketsStringIntoArray(const uint8_t *,
                                                   uint8_t *,
                                                   uint8_t,
                                                   bool);
static void scanEndpoints(
  const uint8_t *,
  EmberAfPluginCommandRelayDeviceEndpoint *,
  EmberAfPluginCommandRelayDeviceEndpoint *);
static void setApsFrame(uint16_t clusterId);

//Command Processing Helper functions
//it scans all elements of inDeviceEndpoint and outDeviceEndpoint
//from argString.
static void scanEndpoints(
  const uint8_t * argString,
  EmberAfPluginCommandRelayDeviceEndpoint *inDeviceEndpoint,
  EmberAfPluginCommandRelayDeviceEndpoint *outDeviceEndpoint)
{
  const uint8_t * stringPtr;

  stringPtr = argString;
  stringPtr = scanBracketsStringIntoArray(stringPtr,
                                          inDeviceEndpoint->eui64,
                                          EUI64_SIZE,
                                          true);
  stringPtr++;
  sscanf(stringPtr,
         "%hhx %hx",
         &inDeviceEndpoint->endpoint,
         &inDeviceEndpoint->clusterId);
  stringPtr = scanBracketsStringIntoArray(stringPtr,
                                          outDeviceEndpoint->eui64,
                                          EUI64_SIZE,
                                          true);
  stringPtr++;
  sscanf(stringPtr,
         "%hhx %hx",
         &outDeviceEndpoint->endpoint,
         &outDeviceEndpoint->clusterId);
}

//it scans the string in brackets "{}" and store it as a byte array
//and return the pointer pointing the character follows the close bracket '}'
static const uint8_t * scanBracketsStringIntoArray(const uint8_t * inputString,
                                                   uint8_t * contentArray,
                                                   uint8_t len,
                                                   bool bigEndian)
{
  const uint8_t * stringPtr;
  uint8_t i;

  stringPtr = inputString;
  while (*stringPtr != '\0') {
    if (*stringPtr == '{') {
      stringPtr++;
      for (i = 0; i < len; ) {
        if (*stringPtr != ' ') {
          sscanf(stringPtr,
                 "%2hhx",
                 &contentArray[(bigEndian ? (len - 1 - i) : i)]);
          stringPtr += 2;
          i++;
        } else {
          stringPtr++;
        }
      }
    } else if (*stringPtr == '}') {
      return (++stringPtr);
    } else {
      stringPtr++;
    }
  }
}

static void setApsFrame(uint16_t clusterId)
{
  localApsFrame.options = EMBER_AF_DEFAULT_APS_OPTIONS;
  localApsFrame.clusterId = clusterId;
}

//command functions definitions
void emAfPluginGatewayRelayMqttCommandRelayAdd(const uint8_t * argString)
{
  EmberAfPluginCommandRelayDeviceEndpoint inDeviceEndpoint;
  EmberAfPluginCommandRelayDeviceEndpoint outDeviceEndpoint;

  scanEndpoints(argString, &inDeviceEndpoint, &outDeviceEndpoint);

  emberAfPluginCommandRelayAdd(&inDeviceEndpoint, &outDeviceEndpoint);
}

void emAfPluginGatewayRelayMqttCommandRelayRemove(const uint8_t * argString)
{
  EmberAfPluginCommandRelayDeviceEndpoint inDeviceEndpoint;
  EmberAfPluginCommandRelayDeviceEndpoint outDeviceEndpoint;

  scanEndpoints(argString, &inDeviceEndpoint, &outDeviceEndpoint);
  emberAfPluginCommandRelayRemove(&inDeviceEndpoint, &outDeviceEndpoint);
}

void emAfPluginGatewayRelayMqttCommandRelayClear(const uint8_t * argString)
{
  emberAfPluginCommandRelayClear();
}

void emAfPluginGatewayRelayMqttOtaServerNotify(const uint8_t * argString)
{
  EmberAfOtaImageId id;
  EmberNodeId dest;
  uint8_t endpoint;
  uint8_t payloadType;
  uint8_t jitter;

  sscanf(argString,
         "%hx %hhx %hhx %hhx %hx %hx %x",
         &dest,
         &endpoint,
         &payloadType,
         &jitter,
         &id.manufacturerId,
         &id.imageTypeId,
         &id.firmwareVersion);
  emberAfOtaServerSendImageNotifyCallback(dest,
                                          endpoint,
                                          payloadType,
                                          jitter,
                                          &id);
}

void emAfPluginGatewayRelayMqttOtaServerPolicyQuery(const uint8_t * argString)
{
  uint8_t value;

  sscanf(argString, "%hhx", &value);
  emAfOtaServerSetQueryPolicy(value);
}

void emAfPluginGatewayRelayMqttOtaStorageCommonReload(const uint8_t * argString)
{
#if defined (EMBER_AF_PLUGIN_OTA_STORAGE_POSIX_FILESYSTEM)
  emAfOtaStorageClose();
#endif

  emberAfOtaStorageInitCallback();
}

void emAfPluginGatewayRelayMqttZdoLeave(const uint8_t * argString)
{
  EmberNodeId target;
  uint8_t removeChildren;
  uint8_t rejoin;
  const uint8_t * stringPtr = argString;
  EmberEUI64 nullEui64 = { 0, 0, 0, 0, 0, 0, 0, 0 };
  EmberStatus status;
  uint8_t options = 0;

  // CCB 2047
  // - CCB makes the first step to depracate the 'leave and remove children' functionality.
  // - Parameter is ignored and signed to be deprecated.
  sscanf(stringPtr, "%hx %hhx %hhx", &target, &removeChildren, &rejoin);
  if (rejoin) {
    options |= EMBER_ZIGBEE_LEAVE_AND_REJOIN;
  }

  status = emberLeaveRequest(target,
                             nullEui64,
                             options,
                             EMBER_APS_OPTION_RETRY);
}

void emAfPluginGatewayRelayMqttZdoBind(const uint8_t * argString)
{
  EmberStatus status;
  EmberEUI64 sourceEui, destEui;// names relative to binding sent over-the-air
  EmberNodeId target;
  uint8_t sourceEndpoint;
  uint8_t destinationEndpoint;
  uint16_t clusterId;
  const uint8_t * stringPtr = argString;

  sscanf(stringPtr,
         "%hx %hhx %hhx %hx",
         &target,
         &sourceEndpoint,
         &destinationEndpoint,
         &clusterId);

  stringPtr = scanBracketsStringIntoArray(stringPtr,
                                          sourceEui,
                                          EUI64_SIZE,
                                          true);
  stringPtr = scanBracketsStringIntoArray(stringPtr,
                                          destEui,
                                          EUI64_SIZE,
                                          true);

  status = emberBindRequest(target,           // who gets the bind req
                            sourceEui,       // source eui IN the binding
                            sourceEndpoint,
                            clusterId,
                            UNICAST_BINDING, // binding type
                            destEui,         // destination eui IN the binding
                            0,               // groupId for new binding
                            destinationEndpoint,
                            EMBER_AF_DEFAULT_APS_OPTIONS);
  UNUSED_VAR(status);
}

void emAfPluginGatewayRelayMqttNetworkBoardPjoin(const uint8_t * argString)
{
  uint8_t time;
  sscanf(argString, "%hhx", &time);
  emberAfPermitJoin(time, true);
}

void emAfPluginGatewayRelayMqttNetworkLeave(const uint8_t * argString)
{
  EmberStatus status;
  status = emberLeaveNetwork();
  UNUSED_VAR(status);
}

void emAfPluginGatewayRelayMqttNetworkForm(const uint8_t * argString)
{
  EmberStatus status;
  EmberNetworkParameters networkParams;

  MEMSET(&networkParams, 0, sizeof(EmberNetworkParameters));
  sscanf(argString,
         "%hhx %hhx %hx",
         &networkParams.radioChannel,
         &networkParams.radioTxPower,
         &networkParams.panId);

  emberAfGetFormAndJoinExtendedPanIdCallback(networkParams.extendedPanId);

  status = emberAfFormNetwork(&networkParams);
  emberAfAppFlush();
}

void emAfPluginGatewayRelayMqttZclGlobalDirection(const uint8_t * argString)
{
  uint8_t zclDirection;

  sscanf(argString, "%hhx", &zclDirection);
  zclFrameControlDirection = ((zclDirection == ZCL_DIRECTION_CLIENT_TO_SERVER)
                              ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                              : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT);
}

void emAfPluginGatewayRelayMqttZclGlobalRead(const uint8_t * argString)
{
  uint16_t clusterId;
  uint16_t attributeId;
  uint8_t index = 0;

  sscanf(argString, "%hx %hx", &clusterId, &attributeId);

  setApsFrame(clusterId);
  appZclBuffer[index++] = (ZCL_GLOBAL_COMMAND
                           | zclFrameControlDirection
                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER);
  appZclBuffer[index++] = emberAfNextSequence();
  appZclBuffer[index++] = ZCL_READ_ATTRIBUTES_COMMAND_ID;
  appZclBuffer[index++] = LOW_BYTE(attributeId);
  appZclBuffer[index++] = HIGH_BYTE(attributeId);

  appZclBufferLen = index;
  zclCmdIsBuilt = true;
}

void emAfPluginGatewayRelayMqttZclGlobalWrite(const uint8_t * argString)
{
  uint16_t clusterId;
  uint16_t attributeId;
  const uint8_t * stringPtr;
  uint8_t dataSize;
  uint8_t type;
  uint8_t index = 0;

  sscanf(argString, "%hx %hx %hhx %*s[\0]", &clusterId, &attributeId, &type);

  stringPtr = argString;
  setApsFrame(clusterId);
  appZclBuffer[index++] = (ZCL_GLOBAL_COMMAND
                           | zclFrameControlDirection
                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER);
  appZclBuffer[index++] = emberAfNextSequence();
  appZclBuffer[index++] = ZCL_WRITE_ATTRIBUTES_COMMAND_ID;
  appZclBuffer[index++] = LOW_BYTE(attributeId);
  appZclBuffer[index++] = HIGH_BYTE(attributeId);
  appZclBuffer[index++] = type;

  if (emberAfIsThisDataTypeAStringType(type)) {
    emberAfAppPrintln("ZCL write of string data is not supported!");
  } else {
    dataSize = emberAfGetDataSize(type);
    stringPtr = scanBracketsStringIntoArray(stringPtr,
                                            &appZclBuffer[index],
                                            dataSize,
                                            false);
    index += dataSize;
  }
  appZclBufferLen = index;
  zclCmdIsBuilt = true;
}

void emAfPluginGatewayRelayMqttZclOnOff(const uint8_t * argString)
{
  uint8_t command;
  uint8_t secondChar;
  uint8_t index = 0;

  secondChar = *(argString + 1);
  if (secondChar == 'n') {
    command = ZCL_ON_COMMAND_ID;
  } else if (secondChar == 'f') {
    command = ZCL_OFF_COMMAND_ID;
  } else {
    command = ZCL_TOGGLE_COMMAND_ID;
  }
  setApsFrame(ZCL_ON_OFF_CLUSTER_ID);
  appZclBuffer[index++] = (ZCL_CLUSTER_SPECIFIC_COMMAND
                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER);
  appZclBuffer[index++] = emberAfNextSequence();
  appZclBuffer[index++] = command;

  appZclBufferLen = index;
  zclCmdIsBuilt = true;
}

void emAfPluginGatewayRelayMqttZclOnAndMoveToLevel(const uint8_t * argString)
{
  uint8_t level;
  uint16_t transTime;
  uint8_t index = 0;

  sscanf(argString, "%hhx %hx", &level, &transTime);

  setApsFrame(ZCL_LEVEL_CONTROL_CLUSTER_ID);
  appZclBuffer[index++] = (ZCL_CLUSTER_SPECIFIC_COMMAND
                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER);
  appZclBuffer[index++] = emberAfNextSequence();
  appZclBuffer[index++] = ZCL_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID;
  appZclBuffer[index++] = level;
  appZclBuffer[index++] = LOW_BYTE(transTime);
  appZclBuffer[index++] = HIGH_BYTE(transTime);

  appZclBufferLen = index;
  zclCmdIsBuilt = true;
}

void emAfPluginGatewayRelayMqttZclMoveToColorTemp(const uint8_t * argString)
{
  uint16_t colorTemp;
  uint16_t transTime;
  uint8_t index = 0;

  sscanf(argString, "%hx %hx", &colorTemp, &transTime);

  setApsFrame(ZCL_COLOR_CONTROL_CLUSTER_ID);
  appZclBuffer[index++] = (ZCL_CLUSTER_SPECIFIC_COMMAND
                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER);
  appZclBuffer[index++] = emberAfNextSequence();
  appZclBuffer[index++] = ZCL_MOVE_TO_COLOR_TEMPERATURE_COMMAND_ID;
  appZclBuffer[index++] = LOW_BYTE(colorTemp);
  appZclBuffer[index++] = HIGH_BYTE(colorTemp);
  appZclBuffer[index++] = LOW_BYTE(transTime);
  appZclBuffer[index++] = HIGH_BYTE(transTime);

  appZclBufferLen = index;
  zclCmdIsBuilt = true;
}

void emAfPluginGatewayRelayMqttZclMoveToHueAndSat(const uint8_t * argString)
{
  uint8_t hue;
  uint8_t sat;
  uint16_t transTime;
  uint8_t index = 0;

  sscanf(argString, "%hhx %hhx %hx", &hue, &sat, &transTime);

  setApsFrame(ZCL_COLOR_CONTROL_CLUSTER_ID);
  appZclBuffer[index++] = (ZCL_CLUSTER_SPECIFIC_COMMAND
                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER);
  appZclBuffer[index++] = emberAfNextSequence();
  appZclBuffer[index++] = ZCL_MOVE_TO_HUE_AND_SATURATION_COMMAND_ID;
  appZclBuffer[index++] = hue;
  appZclBuffer[index++] = sat;
  appZclBuffer[index++] = LOW_BYTE(transTime);
  appZclBuffer[index++] = HIGH_BYTE(transTime);

  appZclBufferLen = index;
  zclCmdIsBuilt = true;
}

void emAfPluginGatewayRelayMqttOpenNetwork(const uint8_t * argString)
{
  emberAfPluginNetworkCreatorSecurityOpenNetwork();
}

void emAfPluginGatewayRelayMqttCloseNetwork(const uint8_t * argString)
{
  emberAfPluginNetworkCreatorSecurityCloseNetwork();
}

void emAfPluginGatewayRelayMqttStartNetwork(const uint8_t * argString)
{
  uint8_t centralizedSecurity;

  sscanf(argString, "%hhx", &centralizedSecurity);
  if (centralizedSecurity != 1) {
    emberAfAppPrintln("Distributed network is not supported!");
  } else {
    emberAfPluginNetworkCreatorStart(true);
  }
}

void emAfPluginGatewayRelayMqttStartNetworkWithParameters(const uint8_t * argString)
{
  uint8_t centralizedSecurity;
  EmberPanId panId;
  int8_t radioTxPower;
  uint8_t channel;
  EmberStatus status;

  sscanf(argString,
         "%hhx %hx %hhx %hhx",
         &centralizedSecurity,
         &panId,
         &radioTxPower,
         &channel);

  if (centralizedSecurity != 1) {
    emberAfAppPrintln("Distributed network is not supported!");
  } else {
    status = emberAfPluginNetworkCreatorNetworkForm(true,
                                                    panId,
                                                    radioTxPower,
                                                    channel);
  }
}

void emAfPluginGatewayRelayMqttZclSend(const uint8_t * argString)
{
  uint8_t i, endpoint;
  const uint8_t * stringPtr;
  EmberEUI64 eui64;
  uint16_t index;
  EmberNodeId nodeId;
  EmberStatus status;

  if (!zclCmdIsBuilt) {
    emberAfAppPrintln("no ZCL cmd is built!");
    return;
  }
  stringPtr = argString;

  stringPtr = scanBracketsStringIntoArray(stringPtr, eui64, EUI64_SIZE, true);
  stringPtr++;
  sscanf(stringPtr, "%hhx", &endpoint);

  index = emberAfDeviceTableGetIndexFromEui64AndEndpoint(eui64, endpoint);

  if (index != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    nodeId = emberAfDeviceTableGetNodeIdFromIndex(index);
    localApsFrame.sourceEndpoint = 1;
    localApsFrame.destinationEndpoint = endpoint;
    status = emberAfSendUnicast(EMBER_OUTGOING_DIRECT,
                                nodeId,
                                &localApsFrame,
                                appZclBufferLen,
                                appZclBuffer);
  }

  zclCmdIsBuilt = false;
}

void emAfPluginGatewayRelayMqttZclGroup(const uint8_t * argString)
{
  const uint8_t * stringPtr;
  uint16_t groupId;
  uint8_t groupName[MAX_COMMAND_LEN];
  uint8_t nameLength, dataSize, i, count;
  uint8_t index = 0;

  MEMSET(groupName, 0, MAX_COMMAND_LEN);
  setApsFrame(ZCL_GROUPS_CLUSTER_ID);
  appZclBuffer[index++] = (ZCL_CLUSTER_SPECIFIC_COMMAND
                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER); //01
  appZclBuffer[index++] = emberAfNextSequence(); //04

  switch ( argString[2] ) {
    case 'd': //ad'd'
      stringPtr = argString + 4;
      sscanf(stringPtr, "%hx %s", &groupId, groupName);
      nameLength = strlen(groupName);
      appZclBuffer[index++] = ZCL_ADD_GROUP_COMMAND_ID; //00
      appZclBuffer[index++] = LOW_BYTE(groupId);
      appZclBuffer[index++] = HIGH_BYTE(groupId);
      appZclBuffer[index++] = nameLength;

      for (i = 0; i < nameLength; i++) {
        appZclBuffer[index++] = groupName[i];
      }

      break;
    case 'e': //vi'e'w
      stringPtr = argString + 5;
      sscanf(stringPtr, "%hx", &groupId);
      appZclBuffer[index++] = ZCL_VIEW_GROUP_COMMAND_ID; //01
      appZclBuffer[index++] = LOW_BYTE(groupId);
      appZclBuffer[index++] = HIGH_BYTE(groupId);
      break;
    case 't': //ge't'
      stringPtr = argString + 4;
      sscanf(stringPtr, "%hhx %*s[\0]", &count);
      stringPtr = strtok(stringPtr, " ");
      appZclBuffer[index++] = ZCL_GET_GROUP_MEMBERSHIP_COMMAND_ID; //02
      appZclBuffer[index++] = count;
      for (i = 0; i < count; i++) {
        stringPtr = strtok(NULL, " ");
        sscanf(stringPtr, "%hx %*s[\0]", &groupId);
        appZclBuffer[index++] = LOW_BYTE(groupId);
        appZclBuffer[index++] = HIGH_BYTE(groupId);
      }
      break;
    case 'm': //re'm'ove
      stringPtr = argString + 7;
      sscanf(stringPtr, "%hx", &groupId);
      appZclBuffer[index++] = ZCL_REMOVE_GROUP_COMMAND_ID; //03
      appZclBuffer[index++] = LOW_BYTE(groupId);
      appZclBuffer[index++] = HIGH_BYTE(groupId);
      break;
    case 'a': //rm'a'll
      stringPtr = argString + 6;
      appZclBuffer[index++] = ZCL_REMOVE_ALL_GROUPS_COMMAND_ID; //04
      break;
    case '-': //ad'-'if-id
      stringPtr = argString + 4;
      sscanf(stringPtr, "%hx %s", &groupId, groupName);
      nameLength = strlen(groupName);
      appZclBuffer[index++] = ZCL_ADD_GROUP_IF_IDENTIFYING_COMMAND_ID; //05
      appZclBuffer[index++] = LOW_BYTE(groupId);
      appZclBuffer[index++] = HIGH_BYTE(groupId);
      appZclBuffer[index++] = nameLength;
      for (i = 0; i < nameLength; i++) {
        appZclBuffer[index++] = groupName[i];
      }
      break;
    default:
      break;
  }
  appZclBufferLen = index;
  zclCmdIsBuilt = true;
}

void emAfPluginGatewayRelayMqttDeviceTableClear(const uint8_t * argString)
{
  emberAfDeviceTableClear();
}

void emAfPluginGatewayRelayMqttZclGlobalSendMeAReport(const uint8_t * argString)
{
  uint16_t clusterId;
  uint16_t attributeId;
  uint16_t minInterval;
  uint16_t maxInterval;
  uint8_t type, i;
  const uint8_t * dataString;
  const uint8_t * reportableChangeString = NULL;
  uint8_t index = 0, dataSize = 0;

  dataString = argString;
  sscanf(argString,
         "%hx %hx %hhx %hx %hx %*s[\0]",
         &clusterId,
         &attributeId,
         &type,
         &minInterval,
         &maxInterval);

  setApsFrame(clusterId);
  appZclBuffer[index++] = (ZCL_GLOBAL_COMMAND
                           | zclFrameControlDirection
                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER);
  appZclBuffer[index++] = emberAfNextSequence();
  appZclBuffer[index++] = ZCL_CONFIGURE_REPORTING_COMMAND_ID;
  appZclBuffer[index++] = EMBER_ZCL_REPORTING_DIRECTION_REPORTED;
  appZclBuffer[index++] = LOW_BYTE(attributeId);
  appZclBuffer[index++] = HIGH_BYTE(attributeId);
  appZclBuffer[index++] = type;
  appZclBuffer[index++] = LOW_BYTE(minInterval);
  appZclBuffer[index++] = HIGH_BYTE(minInterval);
  appZclBuffer[index++] = LOW_BYTE(maxInterval);
  appZclBuffer[index++] = HIGH_BYTE(maxInterval);

  dataSize = emberAfGetDataSize(type);

  while ((*dataString != 0) && (*dataString != '}')) {
    if (*dataString == '{') {
      scanBracketsStringIntoArray(dataString,
                                  &appZclBuffer[index],
                                  dataSize,
                                  true);
      break;
    }
    dataString++;
  }
  appZclBufferLen = index + dataSize;

  zclCmdIsBuilt = true;
}

void emAfPluginGatewayRelayMqttSetJoiningKey(const uint8_t * argString)
{
  EmberEUI64 eui64;
  EmberKeyData keyData;
  EmberStatus status;
  const uint8_t * stringPtr = argString;

  stringPtr = scanBracketsStringIntoArray(stringPtr, eui64, EUI64_SIZE, true);
  stringPtr = scanBracketsStringIntoArray(stringPtr,
                                          keyData.contents,
                                          EMBER_ENCRYPTION_KEY_SIZE,
                                          false);
  status = ezspAddTransientLinkKey(eui64, &keyData);

  emberAfCorePrintln("%p: 0x%X", "Set joining link key", status);
}

void emAfPluginGatewayRelayMqttClearJoiningKeys(const uint8_t * argString)
{
  ezspClearTransientLinkKeys();
}

void emAfPluginGatewayRelayMqttOpenWithKey(const uint8_t * argString)
{
  EmberEUI64 eui64;
  EmberKeyData keyData;
  EmberStatus status;
  const uint8_t * stringPtr = argString;

  stringPtr = scanBracketsStringIntoArray(stringPtr, eui64, EUI64_SIZE, true);
  stringPtr = scanBracketsStringIntoArray(stringPtr,
                                          keyData.contents,
                                          EMBER_ENCRYPTION_KEY_SIZE,
                                          false);
  status = emberAfPluginNetworkCreatorSecurityOpenNetworkWithKeyPair(eui64,
                                                                     keyData);

  emberAfCorePrintln("%p: Open network: 0x%X",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                     status);
}

void emAfPluginGatewayRelayMqttIasZoneClientClearAll(const uint8_t * argString)
{
  emAfClearServers();
}

void emAfPluginGatewayRelayMqttAllowRejoin(const uint8_t * argString)
{
  uint8_t allow;

  sscanf(argString, "%hhx", &allow);
  setTcRejoinsUsingWellKnownKeyAllowed((bool)allow);
}

void emAfPluginGatewayRelayMqttSetRejoinTimeout(const uint8_t * argString)
{
  uint16_t timeout;

  sscanf(argString, "%hx", &timeout);
  setTcRejoinsUsingWellKnownKeyTimeout(timeout);
}

void emAfPluginGatewayRelayMqttCommandsProccessing(uint8_t * inputString)
{
  const uint8_t * inputArgPtr;
  uint8_t index;
  uint8_t argStartPosition;
  uint8_t strLength;

  inputArgPtr = NULL;
  for (index = 0; index < NUMBER_OF_COMMANDS; index++) {
    strLength = strnlen(commandArray[index].commandString, MAX_COMMAND_LEN);
    if (strncmp(commandArray[index].commandString,
                inputString,
                strLength) == 0) {
      argStartPosition =
        strlen(commandArray[index].commandString) + 1;
      inputArgPtr = &inputString[argStartPosition];
      commandArray[index].functionPtr(inputArgPtr);
    }
  }

  if (inputArgPtr == NULL) {
    emberAfAppPrintln("command not found!\r\n");
  }
}
