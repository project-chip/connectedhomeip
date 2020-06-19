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

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "app/framework/include/af.h"

/** @brief Registration
 *
 * This callback is called when the device joins a network and the process of
 * registration is complete. This callback provides a success value of true if
 * the registration process was successful and a value of false if registration
 * failed.
 *
 * @param success true if registration succeeded, false otherwise.  Ver.: always
 */
void emberAfRegistrationCallback(bool success)
{
}

/** @brief Price Acknowledgement
 *
 *
 *
 * @param providerId   Ver.: always
 * @param issuerEventId   Ver.: always
 * @param priceAckTime   Ver.: always
 * @param control   Ver.: always
 */
/*bool emberAfPriceClusterPriceAcknowledgementCallback(uint32_t providerId,
                                                        uint32_t issuerEventId,
                                                        uint32_t priceAckTime,
                                                        uint8_t control)
   {
   emberAfPriceClusterPrintln("RX: PriceAcknowledgement 0x%4x, 0x%4x, 0x%4x, 0x%x",
                             providerId,
                             issuerEventId,
                             priceAckTime,
                             control);
   emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
   return false;
   }*/

/** @brief Price Started
 *
 * This function is called by the Price client plugin whenever a price starts.
 *
 * @param price The price that has started.  Ver.: always
 */
void emberAfPluginPriceClientPriceStartedCallback(EmberAfPluginPriceClientPrice *price)
{
  emberAfPriceClusterPrint("Price \"");
  emberAfPriceClusterPrintString(price->rateLabel);
  emberAfPriceClusterPrintln("\" (0x%4x) has started", price->issuerEventId);
  emberAfPriceClusterFlush();
}

/** @brief Price Expired
 *
 * This function is called by the Price client plugin whenever a price
 * expires.
 *
 * @param price The price that has expired.  Ver.: always
 */
void emberAfPluginPriceClientPriceExpiredCallback(EmberAfPluginPriceClientPrice *price)
{
  emberAfPriceClusterPrint("Price \"");
  emberAfPriceClusterPrintString(price->rateLabel);
  emberAfPriceClusterPrintln("\" (0x%4x) has expired", price->issuerEventId);
  emberAfPriceClusterFlush();
}

/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of emberTrustCenterJoinHandler or ezspTrustCenterJoinHandler.
 * This callback provides the same arguments passed to the
 * TrustCenterJoinHandler. For more information about the TrustCenterJoinHandler
 * please see documentation included in stack/include/trust-center.h.
 *
 * @param newNodeId   Ver.: always
 * @param newNodeEui64   Ver.: always
 * @param parentOfNewNode   Ver.: always
 * @param status   Ver.: always
 * @param decision   Ver.: always
 */
void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
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

/** @brief Broadcast Sent
 *
 * This function is called when a new MTORR broadcast has been successfully sent
 * by the concentrator plugin.
 *
 */
void emberAfPluginConcentratorBroadcastSentCallback(void)
{
}

/** @brief Pre Message Received
 *
 * This function is called by the Interpan plugin when an interpan message is
 * received but has not yet been handled by the plugin or the framework.  The
 * application should return true if the message was handled.
 *
 * @param header   Ver.: always
 * @param msgLen   Ver.: always
 * @param message   Ver.: always
 */
bool emberAfPluginInterpanPreMessageReceivedCallback(const EmberAfInterpanHeader * header,
                                                     uint8_t msgLen,
                                                     uint8_t * message)
{
  return false;
}

/** @brief Finished
 *
 * This callback is fired when the network-find plugin is finished with the
 * forming or joining process.  The result of the operation will be returned in
 * the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
}

/** @brief Get Radio Power For Channel
 *
 * This callback is called by the framework when it is setting the radio power
 * during the discovery process. The framework will set the radio power
 * depending on what is returned by this callback.
 *
 * @param channel   Ver.: always
 */
int8_t emberAfPluginNetworkFindGetRadioPowerForChannelCallback(uint8_t channel)
{
  return EMBER_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER;
}

/** @brief Join
 *
 * This callback is called by the plugin when a joinable network has been found.
 *  If the application returns true, the plugin will attempt to join the
 * network.  Otherwise, the plugin will ignore the network and continue
 * searching.  Applications can use this callback to implement a network
 * blacklist.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
bool emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork * networkFound,
                                          uint8_t lqi,
                                          int8_t rssi)
{
  return true;
}

/** @brief Request Mirror Response
 *
 *
 *
 * @param endpointId   Ver.: always
 */
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

/** @brief Mirror Removed
 *
 *
 *
 * @param endpointId   Ver.: always
 */
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
  if (attributeId == ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID) {
    if (attributeValue & 0x00000004) {
      uint16_t attributeIds = ZCL_IDENTIFY_TIME_ATTRIBUTE_ID;
      emberAfSimpleMeteringClusterPrintln("Time cluster info");
      emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_TIME_CLUSTER_ID,
                                                           &attributeIds,
                                                           sizeof(attributeIds));
      emberAfSendResponse();
    }
    if (attributeValue & 0x00040000) {
      emberAfFillCommandSimpleMeteringClusterGetNotifiedMessage(0x01,
                                                                ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                                                                0x00040000);
      emberAfSendResponse();
    }
  } else if (attributeId == ZCL_NOTIFICATION_FLAGS_2_ATTRIBUTE_ID) {
    if (attributeValue & 0x00000001) {
      emberAfFillCommandPriceClusterGetCurrentPrice(0x00);
      emberAfSendResponse();
    }
  }
}

/** @brief Simple Metering Cluster Get Notified Message
 *
 *
 *
 * @param notificationScheme   Ver.: always
 * @param notificationFlagAttributeId   Ver.: always
 * @param notificationFlagsN   Ver.: always
 */
bool emberAfSimpleMeteringClusterGetNotifiedMessageCallback(uint8_t notificationScheme,
                                                            uint16_t notificationFlagAttributeId,
                                                            uint32_t notificationFlagsN)
{
  if (notificationScheme == 0x01) {
    if (notificationFlagAttributeId == ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID) {
      if (notificationFlagsN & 0x00040000) {
        //Send a change supply command.
        emberAfFillCommandSimpleMeteringClusterChangeSupply(0x44556677,
                                                            0x12345678,
                                                            0x00000000,
                                                            0x00000000,
                                                            0x02,
                                                            0x00);
        emberAfSendResponse();
      }
    }
  }
  return true;
}
