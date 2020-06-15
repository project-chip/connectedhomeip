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
 * @brief This file provides an implementation of a Ble cli over custom EZSP
 * protocol to communicate with dynamic multi-protocol (DMP) Zigbee-BLE NCP
 * from Zigbee host application.
 * This plugin require the NCP to support custom EZSP commands. This is typically
 * achieved by building the NCP image including the XNCP library which is been
 * included by default DMP NCP application. This plugin is NOT compatible with
 * an system-on-a-chip (SOC) platform.
 * This sample code demonstrate how to invoke Bluetooth operations from Zigbee
 * host with DMP NCP using custom ezsp protocol if application would like to.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/xncp/xncp-sample-custom-ezsp-protocol.h"

void emberAfPluginBleCustomEzspSetConnectionParamsCommand(void);
void emberAfPluginBleCustomEzspSetAdvertisementParamsCommand(void);
void emberAfPluginBleCustomEzspSetGapModeCommand(void);
void emberAfPluginBleCustomEzspGapConnectionOpenCommand(void);
void emberAfPluginBleCustomEzspPrintConnectionTableCommand(void);

#define EMBER_MAX_CUSTOM_EZSP_MESSAGE_PAYLOAD 119

#define LE_GAP_MAX_DISCOVERABLE_MODE   0x04
#define LE_GAP_MAX_CONNECTABLE_MODE    0x03
#define LE_GAP_MAX_DISCOVERY_MODE      0x02

//plugin ble-custon-ezsp set_conn_params
void emberAfPluginBleCustomEzspSetConnectionParamsCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte) + minInterval (2 bytes) + maxInterval (2 bytes)
  // + slaveLatency (2 bytes) + supervisionTimeout (2 bytes)
  uint8_t message[9];
  uint8_t replyLength = 2;  // for Ble status
  uint8_t reply[2];
  uint16_t minInterval = emberUnsignedCommandArgument(0);
  uint16_t maxInterval = emberUnsignedCommandArgument(1);
  uint16_t slaveLatency = emberUnsignedCommandArgument(2);
  uint16_t supervisionTimeout = emberUnsignedCommandArgument(3);

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_SET_BLE_CONNECTION_PARAMS;
  message[1] = LOW_BYTE(minInterval);
  message[2] = HIGH_BYTE(minInterval);
  message[3] = LOW_BYTE(maxInterval);
  message[4] = HIGH_BYTE(maxInterval);
  message[5] = LOW_BYTE(slaveLatency);
  message[6] = HIGH_BYTE(slaveLatency);
  message[7] = LOW_BYTE(supervisionTimeout);
  message[8] = HIGH_BYTE(supervisionTimeout);

  status = ezspCustomFrame(sizeof(message), message, &replyLength, reply);
  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("set connection params: ble status 0x%2x ", HIGH_LOW_TO_INT(reply[1], reply[0]));
  } else {
    emberAfCorePrintln("set connection params: failed 0x%x", status);
  }
}

// plugin ble-custom-ezsp set_adv_params
void emberAfPluginBleCustomEzspSetAdvertisementParamsCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte) + minInterval (2 bytes) + maxInterval (2 byte)
  // + channel map (1 byte)
  uint8_t message[6];
  uint8_t replyLength = 2;  // for Ble status
  uint8_t reply[2];

  uint16_t minInterval = emberUnsignedCommandArgument(0);
  uint16_t maxInterval = emberUnsignedCommandArgument(1);
  uint8_t channelMap = emberUnsignedCommandArgument(2);

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_SET_BLE_ADVERTISEMENT_PARAMS;
  message[1] = LOW_BYTE(minInterval);
  message[2] = HIGH_BYTE(minInterval);
  message[3] = LOW_BYTE(maxInterval);
  message[4] = HIGH_BYTE(maxInterval);
  message[5] = channelMap;

  status = ezspCustomFrame(sizeof(message), message, &replyLength, reply);
  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("set advertisement params: ble status 0x%2x ", HIGH_LOW_TO_INT(reply[1], reply[0]));
  } else {
    emberAfCorePrintln("set advertisement params: failed 0x%x", status);
  }
}

// plugin ble-custom-ezsp set_mode
void emberAfPluginBleCustomEzspSetGapModeCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte) + discoverableMode (1 byte) + addrconnectableMode (1 byte)
  uint8_t message[3];
  uint8_t replyLength = 2;  // for Ble status
  uint8_t reply[2];
  uint8_t discoverableMode = emberUnsignedCommandArgument(0);
  uint8_t connectableMode = emberUnsignedCommandArgument(1);

  if (discoverableMode > LE_GAP_MAX_DISCOVERABLE_MODE
      || connectableMode > LE_GAP_MAX_CONNECTABLE_MODE) {
    emberAfCorePrintln("Invalid params");
    return;
  }

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_SET_BLE_ADVERTISEMENT_MODE;
  message[1] = discoverableMode;
  message[2] = connectableMode;

  status = ezspCustomFrame(sizeof(message), message, &replyLength, reply);

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("set advertisement mode: ble status 0x%2x ", HIGH_LOW_TO_INT(reply[1], reply[0]));
  } else {
    emberAfCorePrintln("set advertisement mode: failed 0x%x", status);
  }
}

// plugin ble-custom-ezsp conn_open
void emberAfPluginBleCustomEzspGapConnectionOpenCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte) + address (8 bytes) + addressType (1 byte)
  uint8_t message[10];
  uint8_t replyLength = 2;  // for Ble status
  uint8_t reply[2];
  uint8_t addressType = emberUnsignedCommandArgument(1);
  EmberEUI64 address;
  emberCopyBigEndianEui64Argument(0, address);

  // TODO: use enums
  if (addressType > 3 && addressType != 16) {
    emberAfCorePrintln("Invalid params");
    return;
  }

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_OPEN_BLE_CONNECTION;
  MEMMOVE(message + 1, address, sizeof(EmberEUI64));
  message[9] = addressType;

  status = ezspCustomFrame(sizeof(message), message, &replyLength, reply);

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("connection open: ble status 0x%2x ", HIGH_LOW_TO_INT(reply[1], reply[0]));
  } else {
    emberAfCorePrintln("connection open: failed 0x%x", status);
  }
}

// plugin ble-custom-ezsp print_connections
void emberAfPluginBleCustomEzspPrintConnectionTableCommand(void)
{
  EmberStatus status;
  // Custom command ID (1 byte)
  uint8_t message[1];
  uint8_t replyLength = EMBER_MAX_CUSTOM_EZSP_MESSAGE_PAYLOAD;
  uint8_t reply[EMBER_MAX_CUSTOM_EZSP_MESSAGE_PAYLOAD];

  message[0] = EMBER_CUSTOM_EZSP_COMMAND_PRINT_BLE_CONNECTION_TABLE;
  status = ezspCustomFrame(sizeof(message), message, &replyLength, reply);

  if (status == EMBER_SUCCESS) {
    if ( replyLength > 0) {
      uint8_t i;
      uint8_t *payload = reply;
      // reply payload: isMaster (1 byte) + connection handle (1 byte)
      // + bonding table (1 byte) + remote address (6 bytes) = 9 bytes in total.
      emberAfCorePrintln("replyLength %d", replyLength);
      for (i = 0; i < replyLength; i += 9) {
        emberAfCorePrintln("**** Connection Info ****");
        emberAfCorePrintln("local node is %s",
                           (payload[0]) ? "master" : "slave");
        emberAfCorePrintln("connection handle 0x%x", payload[1]);
        emberAfCorePrintln("bonding handle = 0x%x", payload[2]);
        emberAfCorePrint("remote address: [%x %x %x %x %x %x]",
                         payload[8], payload[7], payload[6],
                         payload[5], payload[4], payload[3]);
        emberAfCorePrintln("\n*************************");
        payload += 9; // Read the second entry if present
      }
    } else {
      emberAfCorePrintln("No open connection present.");
    }
  } else {
    emberAfCorePrintln("Failed to print connection table.");
  }
}
