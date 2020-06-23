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
// * SeMeterGas_callbacks.c
// *
// * This file contains all application specific code for the
// * gas meter sample application. This application is intended
// * to be used along with the meter mirror application
// * SeMeterMirror
// *
// * Copyright 2007 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/util/common.h"

#define GAS_METER_ENDPOINT 0x01

// *******************************************************************
// * clusterInitCallback
// *
// * Make sure that the simple metering device type on endpoint 1
// * is set to gas meter.
// *
// *
// *******************************************************************
void emberAfClusterInitCallback(uint8_t endpointId,
                                uint16_t clusterId)
{
  uint8_t deviceType = EMBER_ZCL_METERING_DEVICE_TYPE_GAS_METERING;
  if ((endpointId == GAS_METER_ENDPOINT)
      && (clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID)) {
    emberAfWriteAttribute(endpointId,
                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                          ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER,
                          (uint8_t*) &deviceType,
                          ZCL_INT8U_ATTRIBUTE_TYPE
                          );
  }
}

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

/** @brief Process Notification Flags
 *
 * This function is called by the metering server plugin when any of the
 * Notification Attribute Set attributes are read or reported by the metering
 * client.
 *
 * @param attributeId   Ver.: always
 * @param attributeValue   Ver.: always
 */
void emberAfPluginSimpleMeteringServerProcessNotificationFlagsCallback(uint16_t attributeId,
                                                                       uint32_t attributeValue)
{
}
