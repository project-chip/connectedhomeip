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
 * @brief A sample of custom EZSP protocol.
 *******************************************************************************
   ******************************************************************************/

#include "custom-ezsp.h"

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/xncp/xncp-sample-custom-ezsp-protocol.h"

void emberAfPluginCustomEzspSetPowerModeCommand(void);
void emberAfPluginCustomEzspGetPowerModeCommand(void);
void emberAfPluginCustomEzspAddClusterToFilteringListCommand(void);
void emberAfPluginCustomEzspRemoveClusterToFilteringListCommand(void);
void emberAfPluginCustomEzspPrintClusterFilteringListCommand(void);
void emberAfPluginCustomEzspEnableReportsCommand(void);
void emberAfPluginCustomEzspDisableReportsCommand(void);
void emberAfPluginCustomEzspSetCustomTokenCommand(void);
void emberAfPluginCustomEzspGetCustomTokenCommand(void);
void emberAfPluginCustomEzspGetXNcpInfoCommand(void);

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginCustomEzspCommands[] = {
  emberCommandEntryAction("set_mode", emberAfPluginCustomEzspSetPowerModeCommand, "u", "Set the XNCP's mode to normal or low power."),
  emberCommandEntryAction("get_mode", emberAfPluginCustomEzspGetPowerModeCommand, "", "Print the XNCP's current power mode."),
  emberCommandEntryAction("add_cluster", emberAfPluginCustomEzspAddClusterToFilteringListCommand, "v", "Add a cluster to the list that will be filtered by the XNCP"),
  emberCommandEntryAction("rem_cluster", emberAfPluginCustomEzspRemoveClusterToFilteringListCommand, "v", "Remove a cluster from the list that is filtered by the XNCP"),
  emberCommandEntryAction("print_list", emberAfPluginCustomEzspPrintClusterFilteringListCommand, "", "Print the list of clusters filtered by the XNCP."),
  emberCommandEntryAction("reports_on", emberAfPluginCustomEzspEnableReportsCommand, "v", ""),
  emberCommandEntryAction("reports_off", emberAfPluginCustomEzspDisableReportsCommand, "", ""),
  emberCommandEntryAction("set_token", emberAfPluginCustomEzspSetCustomTokenCommand, "uvv", ""),
  emberCommandEntryAction("get_token", emberAfPluginCustomEzspGetCustomTokenCommand, "", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin custom-ezsp set_mode <mode:1>
void emberAfPluginCustomEzspSetPowerModeCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte) + power mode (1 byte)
  uint8_t message[2];
  uint8_t mode = emberUnsignedCommandArgument(0);
  uint8_t replyLength = 0;

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_SET_POWER_MODE;
  message[1] = (mode == 0)
               ? EMBER_XNCP_NORMAL_MODE
               : EMBER_XNCP_LOW_POWER_MODE;

  if (mode > 1) {
    emberAfCorePrintln("Invalid mode, allowed values: {0,1}");
    return;
  }

  status = ezspCustomFrame(2, message, &replyLength, NULL);

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Power mode set");
  } else {
    emberAfCorePrintln("Power mode set failed, status:0x%x", status);
  }

  assert(replyLength == 0);
}

// plugin custom-ezsp get_mode
void emberAfPluginCustomEzspGetPowerModeCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte)
  uint8_t message[1];
  uint8_t replyLength = 1;
  uint8_t reply[1];

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_GET_POWER_MODE;

  status = ezspCustomFrame(1, message, &replyLength, reply);

  if (status == EMBER_SUCCESS) {
    assert(replyLength == 1);

    if (reply[0] == EMBER_XNCP_NORMAL_MODE) {
      emberAfCorePrintln("Power mode: NORMAL");
    } else if (reply[0] == EMBER_XNCP_LOW_POWER_MODE) {
      emberAfCorePrintln("Power mode: LOW_POWER");
    } else {
      emberAfCorePrintln("Power mode: UNDEFINED");
    }
  } else {
    emberAfCorePrintln("Command failed, status:0x%x", status);
  }
}

// plugin custom-ezsp add_cluster <clusterId:2>
void emberAfPluginCustomEzspAddClusterToFilteringListCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte) + cluster ID (2 bytes)
  uint8_t message[3];
  uint16_t clusterId = emberUnsignedCommandArgument(0);
  uint8_t replyLength = 0;

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_ADD_CLUSTER_TO_FILTERING_LIST;
  message[1] = LOW_BYTE(clusterId);
  message[2] = HIGH_BYTE(clusterId);

  status = ezspCustomFrame(3, message, &replyLength, NULL);

  if (status == EMBER_SUCCESS) {
    assert(replyLength == 0);
    emberAfCorePrintln("Cluster ADD success");
  } else {
    emberAfCorePrintln("Cluster ADD failed, status:0x%x", status);
  }
}

// plugin custom-ezsp rem_cluster <clusterId:2>
void emberAfPluginCustomEzspRemoveClusterToFilteringListCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte) + cluster ID (2 bytes)
  uint8_t message[3];
  uint16_t clusterId = emberUnsignedCommandArgument(0);
  uint8_t replyLength = 0;

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_REMOVE_CLUSTER_TO_FILTERING_LIST;
  message[1] = LOW_BYTE(clusterId);
  message[2] = HIGH_BYTE(clusterId);

  status = ezspCustomFrame(3, message, &replyLength, NULL);

  if (status == EMBER_SUCCESS) {
    assert(replyLength == 0);
    emberAfCorePrintln("Cluster REMOVE success");
  } else {
    emberAfCorePrintln("Cluster REMOVE failed, status:0x%x", status);
  }
}

// plugin custom-ezsp print_list
void emberAfPluginCustomEzspPrintClusterFilteringListCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte)
  uint8_t message[1];
  uint8_t replyLength = EMBER_MAX_CUSTOM_EZSP_MESSAGE_PAYLOAD;
  uint8_t reply[EMBER_MAX_CUSTOM_EZSP_MESSAGE_PAYLOAD];
  uint8_t i;

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_GET_CLUSTER_FILTERING_LIST;

  status = ezspCustomFrame(1, message, &replyLength, reply);

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Cluster filtering list, size=0x%x", reply[0]);

    // First byte of the reply payload is the number of entries
    for (i = 1; i < replyLength; i += 2) {
      emberAfCorePrintln("Cluster filtering entry -> Cluster ID 0x%2X",
                         HIGH_LOW_TO_INT(reply[i + 1], reply[i]));
    }
  } else {
    emberAfCorePrintln("Command failed, status:0x%x", status);
  }
}

// plugin custom-ezsp reports_on <time:2>
void emberAfPluginCustomEzspEnableReportsCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte) + report time (seconds) (2 bytes)
  uint8_t message[3];
  uint16_t time = emberUnsignedCommandArgument(0);
  uint8_t replyLength = 0;

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_ENABLE_PERIODIC_REPORTS;
  message[1] = LOW_BYTE(time);
  message[2] = HIGH_BYTE(time);

  status = ezspCustomFrame(3, message, &replyLength, NULL);

  if (status == EMBER_SUCCESS) {
    assert(replyLength == 0);
    emberAfCorePrintln("Reports enabled");
  } else {
    emberAfCorePrintln("Reports enabling failed, status:0x%x", status);
  }
}

// plugin custom-ezsp reports_off
void emberAfPluginCustomEzspDisableReportsCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte)
  uint8_t message[1];
  uint8_t replyLength = 0;

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_DISABLE_PERIODIC_REPORTS;

  status = ezspCustomFrame(1, message, &replyLength, NULL);

  if (status == EMBER_SUCCESS) {
    assert(replyLength == 0);
    emberAfCorePrintln("Reports disabled");
  } else {
    emberAfCorePrintln("Reports disabling failed, status:0x%x", status);
  }
}

// plugin custom-ezsp set_token <nodeType:1> <nodeId:2> <panId:2>
void emberAfPluginCustomEzspSetCustomTokenCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte) + nodeType (1 byte) + nodeID (2 bytes) +
  // + panID (2 bytes)
  uint8_t message[6];
  uint8_t nodeType = emberUnsignedCommandArgument(0);
  uint16_t nodeId = emberUnsignedCommandArgument(1);
  uint16_t panId = emberUnsignedCommandArgument(2);
  uint8_t replyLength = 0;

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_SET_CUSTOM_TOKEN;
  message[1] = nodeType;
  message[2] = LOW_BYTE(nodeId);
  message[3] = HIGH_BYTE(nodeId);
  message[4] = LOW_BYTE(panId);
  message[5] = HIGH_BYTE(panId);

  status = ezspCustomFrame(6, message, &replyLength, NULL);

  if (status == EMBER_SUCCESS) {
    assert(replyLength == 0);
    emberAfCorePrintln("Custom token set");
  } else {
    emberAfCorePrintln("Custom token set failed, status:0x%x", status);
  }
}

// plugin custom-ezsp get_token
void emberAfPluginCustomEzspGetCustomTokenCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte)
  uint8_t message[1];
  uint8_t replyLength = 5;
  // nodeType (1 byte) + nodeID (2 bytes) + panID (2 bytes)
  uint8_t reply[5];

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_GET_CUSTOM_TOKEN;

  status = ezspCustomFrame(1, message, &replyLength, reply);

  if (status == EMBER_SUCCESS) {
    assert(replyLength == 5);
    emberAfCorePrintln("Custom token - nodeType:0x%x nodeId:0x%2x panId:0x%2x",
                       reply[0],
                       HIGH_LOW_TO_INT(reply[1], reply[2]),
                       HIGH_LOW_TO_INT(reply[3], reply[4]));
  } else {
    emberAfCorePrintln("Custom token read failed, status:0x%x", status);
  }
}

// plugin custom-ezsp get_info
void emberAfPluginCustomEzspGetXNcpInfoCommand(void)
{
  EmberStatus status;
  uint16_t manufacturerId;
  uint16_t versionNumber;

  status = ezspGetXncpInfo(&manufacturerId, &versionNumber);

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("XNCP library present, manufacturer ID:0x%2x version:0x%2x",
                       manufacturerId,
                       versionNumber);
  } else {
    emberAfCorePrintln("XNCP library not present");
  }
}
