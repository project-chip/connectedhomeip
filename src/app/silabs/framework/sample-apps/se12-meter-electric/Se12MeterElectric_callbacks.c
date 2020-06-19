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
 * @brief
 *******************************************************************************
   ******************************************************************************/

// *******************************************************************
// * SeMeterElectric_callbacks.c
// *
// * This file contains all application specific code for the
// * electric meter sample application. This application is intended
// * to be used along with the meter mirror application
// * SeMeterMirror
// *
// * Copyright 2007 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/util/common.h"
#include "app/framework/plugin/tunneling-server/tunneling-server.h"

// *******************************************************************
// * simpleMeteringClusterMirrorRemovedCallback
// *
// * This callback simply prints out the endpoint from which
// * the mirror was removed.
// *
// *******************************************************************
bool emberAfSimpleMeteringClusterMirrorRemovedCallback(uint16_t endpointId)
{
  if (endpointId == 0xffff) {
    emberAfCorePrintln("%premove FAILED", "Mirror ");
  } else {
    emberAfCorePrintln("%pREMOVED from %x", "Mirror ", endpointId);
  }
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

// *******************************************************************
// * simpleMeteringClusterRequestMirrorResponseCallback
// *
// * This callback simply prints out the endpoint to which the
// * mirror was added.
// *
// * A more complex implementation of this sample
// * application would start writing attributes by sending
// * global attribute writes to the mirrored endpoint.
// * I have not implemented a mechanism for
// * this and instead write the attributes using the cli commands
// * such as:
// *
// * zcl global write 0x0702 0x0306 0x18 {04}
// * send 0 1 3
// *
// *******************************************************************
bool emberAfSimpleMeteringClusterRequestMirrorResponseCallback(uint16_t endpointId)
{
  if (endpointId == 0xffff) {
    emberAfCorePrintln("%padd FAILED", "Mirror ");
  } else {
    emberAfCorePrintln("%pADDED on %x", "Mirror ", endpointId);
  }
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Finished
 *
 * This callback is fired when the network-find plugin is finished with the
 * forming or joining process.  The result of the operation will be returned
 * in the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
}

/** @brief Join
 *
 * This callback is called by the plugin when a joinable network has been
 * found.  If the application returns true, the plugin will attempt to join
 * the network.  Otherwise, the plugin will ignore the network and continue
 * searching.  Applications can use this callback to implement a network
 * blacklist.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
bool emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork *networkFound,
                                          uint8_t lqi,
                                          int8_t rssi)
{
  return true;
}

/** @brief Select File Descriptors
 *
 * This function is called when the Gateway plugin will do a select() call to
 * yield the processor until it has a timed event that needs to execute.  The
 * function implementor may add additional file descriptors that the
 * application will monitor with select() for data ready.  These file
 * descriptors must be read file descriptors.  The number of file descriptors
 * added must be returned by the function (0 for none added).
 *
 * @param list A pointer to a list of File descriptors that the function
 * implementor may append to  Ver.: always
 * @param maxSize The maximum number of elements that the function implementor
 * may add.  Ver.: always
 */
int emberAfPluginGatewaySelectFileDescriptorsCallback(int* list,
                                                      int maxSize)
{
  return 0;
}

/** @brief Broadcast Sent
 *
 * This function is called when a new MTORR broadcast has been successfully
 * sent by the concentrator plugin.
 *
 */
void emberAfPluginConcentratorBroadcastSentCallback(void)
{
}

/** @brief Button Event
 *
 * This allows another module to get notification when a button is pressed and
 * released but the button joining plugin did not handle it.  This callback is
 * NOT called in ISR context so there are no restrictions on what code can
 * execute.
 *
 * @param buttonNumber The button number that was pressed.  Ver.: always
 * @param buttonPressDurationMs The length of time button was held down before
 * it was released.  Ver.: always
 */
void emberAfPluginButtonJoiningButtonEventCallback(uint8_t buttonNumber,
                                                   uint32_t buttonPressDurationMs)
{
}

/** @brief Data Received
 *
 * This function is called by the Tunneling server plugin whenever data is
 * received from a client through a tunnel.
 *
 * @param tunnelId The identifier of the tunnel through which the data was
 * received.  Ver.: always
 * @param data Buffer containing the raw octets of the data.  Ver.: always
 * @param dataLen The length in octets of the data.  Ver.: always
 */
void emberAfPluginTunnelingServerDataReceivedCallback(uint16_t tunnelId,
                                                      uint8_t * data,
                                                      uint16_t dataLen)
{
  emberAfTunnelingClusterPrint("ServerDataReceived:%x,[", tunnelId);
  emberAfTunnelingClusterPrintBuffer(data, dataLen, false);
  emberAfTunnelingClusterPrintln("]");

  EmberAfStatus status = emberAfPluginTunnelingServerTransferData(tunnelId,
                                                                  data,
                                                                  dataLen);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfTunnelingClusterPrintln("%p 0x%x", "Unable to send tunneling message", status);
  }
}

/** @brief Is Protocol Supported
 *
 * This function is called by the Tunneling server plugin whenever a Request
 * Tunnel command is received.  The application should return true if the
 * protocol is supported and false otherwise.
 *
 * @param protocolId The identifier of the metering communication protocol for
 * which the tunnel is requested.  Ver.: always
 * @param manufacturerCode The manufacturer code for manufacturer-defined
 * protocols or 0xFFFF in unused.  Ver.: always
 */
bool emberAfPluginTunnelingServerIsProtocolSupportedCallback(uint8_t protocolId,
                                                             uint16_t manufacturerCode)
{
  uint8_t GB_HGRP = 6;
  if (protocolId == GB_HGRP && manufacturerCode == 0xFFFF) {
    return true;
  } else {
    return false;
  }
}
