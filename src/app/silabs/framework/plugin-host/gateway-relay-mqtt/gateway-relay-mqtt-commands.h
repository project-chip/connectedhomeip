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

typedef struct {
  uint8_t * commandString;
  void (*functionPtr)(const uint8_t *);
} EmAfPluginGatewayRelayMqttCommand;

#define COMMAND_STRING_START_NETWORK \
  "plugin network-creator start\0"
#define COMMAND_STRING_FORM_NETWORK \
  "plugin network-creator form\0"
#define COMMAND_STRING_OPEN_NETWORK \
  "plugin network-creator-security open-network\0"
#define COMMAND_STRING_CLOSE_NETWORK \
  "plugin network-creator-security close-network\0"
#define COMMAND_STRING_SET_KEY \
  "plugin network-creator-security set-joining-link-key\0"
#define COMMAND_STRING_CLEAR_KEY \
  "plugin network-creator-security clear-joining-link-keys\0"
#define COMMAND_STRING_OPEN_WITH_KEY \
  "plugin network-creator-security open-with-key\0"
#define COMMAND_STRING_ZCL_SEND \
  "plugin device-table send\0"
#define COMMAND_STRING_DEVICE_TABLE_CLEAR \
  "plugin device-table clear\0"
#define COMMAND_STRING_COMMAND_RElAY_ADD \
  "plugin command-relay add\0"
#define COMMAND_STRING_COMMAND_RELAY_REMOVE \
  "plugin command-relay remove\0"
#define COMMAND_STRING_COMMAND_RELAY_CLEAR \
  "plugin command-relay clear\0"
#define COMMAND_STRING_OTA_RELOAD \
  "plugin ota-storage-common reload\0"
#define COMMAND_STRING_OTA_POLICY_QUERY \
  "plugin ota-server policy query\0"
#define COMMAND_STRING_OTA_NOTIFY \
  "plugin ota-server notify\0"
#define COMMAND_STRING_IAS_ZONE_CLEAR \
  "plugin ias-zone-client clear-all\0"
#define COMMAND_STRING_ZDO_LEAVE \
  "zdo leave\0"
#define COMMAND_STRING_ZDO_BIND \
  "zdo bind\0"
#define COMMAND_STRING_BROAD_PJOIN \
  "network broad-pjoin\0"
#define COMMAND_STRING_NETWORK_LEAVE \
  "network leave\0"
#define COMMAND_STRING_NETWORK_FORM \
  "network form\0"
#define COMMAND_STRING_ZCL_GLOBAL_DIRECTION \
  "zcl global direction\0"
#define COMMAND_STRING_ZCL_GLOBAL_READ \
  "zcl global read\0"
#define COMMAND_STRING_ZCL_GLOBAL_WRITE \
  "zcl global write\0"
#define COMMAND_STRING_ZCL_ON_OFF \
  "zcl on-off\0"
#define COMMAND_STRING_ZCL_ON_MOVE_TO_LEVEL \
  "zcl level-control o-mv-to-level\0"
#define COMMAND_STRING_ZCL_MOVE_TO_COLORTEMP \
  "zcl color-control movetocolortemp\0"
#define COMMAND_STRING_ZCL_MOVE_TO_HUESAT \
  "zcl color-control movetohueandsat\0"
#define COMMAND_STRING_ZCL_SEND_ME_A_REPORT \
  "zcl global send-me-a-report\0"
#define COMMAND_STRING_ZCL_GROUP \
  "zcl groups\0"
#define COMMAND_STRING_OPTION_SECURITY_ALLOW_UNSECURE_REJOIN \
  "option security set-allow-trust-center-rejoin-using-well-known-key\0"
#define COMMAND_STRING_OPTION_SECURITY_SET_UNSECURE_REJOIN_TIMEOUT \
  "option security set-allow-trust-center-rejoin-using-well-known-key-timeout\0"

//forward declaration of command functions
void emAfPluginGatewayRelayMqttStartNetwork(const uint8_t *);
void emAfPluginGatewayRelayMqttStartNetworkWithParameters(const uint8_t *);
void emAfPluginGatewayRelayMqttCommandRelayAdd(const uint8_t *);
void emAfPluginGatewayRelayMqttCommandRelayRemove(const uint8_t *);
void emAfPluginGatewayRelayMqttCommandRelayClear(const uint8_t *);
void emAfPluginGatewayRelayMqttZdoLeave(const uint8_t *);
void emAfPluginGatewayRelayMqttZdoBind(const uint8_t *);
void emAfPluginGatewayRelayMqttNetworkBoardPjoin(const uint8_t *);
void emAfPluginGatewayRelayMqttNetworkLeave(const uint8_t *);
void emAfPluginGatewayRelayMqttNetworkForm(const uint8_t *);
void emAfPluginGatewayRelayMqttZclGlobalDirection(const uint8_t *);
void emAfPluginGatewayRelayMqttZclGlobalRead(const uint8_t *);
void emAfPluginGatewayRelayMqttZclGlobalWrite(const uint8_t *);
void emAfPluginGatewayRelayMqttZclOnOff(const uint8_t *);
void emAfPluginGatewayRelayMqttZclOnAndMoveToLevel(const uint8_t *);
void emAfPluginGatewayRelayMqttZclMoveToColorTemp(const uint8_t *);
void emAfPluginGatewayRelayMqttZclMoveToHueAndSat(const uint8_t *);
void emAfPluginGatewayRelayMqttZclGlobalSendMeAReport(const uint8_t *);
void emAfPluginGatewayRelayMqttZclGroup(const uint8_t *);
void emAfPluginGatewayRelayMqttOpenNetwork(const uint8_t *);
void emAfPluginGatewayRelayMqttCloseNetwork(const uint8_t *);
void emAfPluginGatewayRelayMqttZclSend(const uint8_t *);
void emAfPluginGatewayRelayMqttDeviceTableClear(const uint8_t *);
void emAfPluginGatewayRelayMqttSetJoiningKey(const uint8_t *);
void emAfPluginGatewayRelayMqttClearJoiningKeys(const uint8_t *);
void emAfPluginGatewayRelayMqttOpenWithKey(const uint8_t *);
void emAfPluginGatewayRelayMqttOtaStorageCommonReload(const uint8_t *);
void emAfPluginGatewayRelayMqttOtaServerPolicyQuery(const uint8_t *);
void emAfPluginGatewayRelayMqttOtaServerNotify(const uint8_t *);
void emAfPluginGatewayRelayMqttIasZoneClientClearAll(const uint8_t *);
void emAfPluginGatewayRelayMqttAllowRejoin(const uint8_t *);
void emAfPluginGatewayRelayMqttSetRejoinTimeout(const uint8_t *);

void emAfPluginGatewayRelayMqttCommandsProccessing(uint8_t * inputString);
