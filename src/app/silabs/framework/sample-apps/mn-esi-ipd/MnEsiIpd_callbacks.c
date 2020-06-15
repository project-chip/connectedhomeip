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

// Copyright 2007 - 2012 by Ember Corporation. All rights reserved.
//
//

// This callback file is created for your convenience. You may add application code
// to this file. If you regenerate this file over a previous version, the previous
// version will be overwritten and any code you have added will be lost.

#include "app/framework/include/af.h"

/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of emberTrustCenterJoinHandler or
 * ezspTrustCenterJoinHandler. This callback provides the same arguments
 * passed to the TrustCenterJoinHandler. For more information about the
 * TrustCenterJoinHandler please see documentation included in
 * stack/include/trust-center.h.
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
  emberAfRegistrationPrintln("New node joined, shortID=0x%2x", newNodeId);
}

/** @brief Registration
 *
 * This callback is called when the device joins a network and the process of
 * registration is complete. This callback provides a success value of true if
 * the registration process was successful and a value of false if
 * registration failed.
 *
 * @param success true if registration succeeded, false otherwise.  Ver.:
 * always
 */
void emberAfRegistrationCallback(bool success)
{
  emberAfRegistrationPrintln("Registration %p", success ? "complete" : "failed");
}

/** @brief Price Acknowledgement
 *
 *
 * @param providerId   Ver.: always
 * @param issuerEventId   Ver.: always
 * @param priceAckTime   Ver.: always
 * @param control   Ver.: always
 */
bool emberAfPriceClusterPriceAcknowledgementCallback(uint32_t providerId,
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
  return true;
}

/** @brief Event Action
 *
 * This function is called by the demand response and load control client
 * plugin whenever an event status changes within the DRLC event table.  The
 * list of possible event status values is defined by the ZCL spec and is
 * listed in the Application Framework's generated enums located in enums.h.
 * For example, an event status may be:
 * AMI_EVENT_STATUS_LOAD_CONTROL_EVENT_COMMAND_RX indicating that a properly
 * formatted event was received; AMI_EVENT_STATUS_EVENT_STARTED indicating
 * that an event has started; AMI_EVENT_STATUS_THE_EVENT_HAS_BEEN_CANCELED,
 * indicating that the event was canceled.  This callback is intended to give
 * the device an opportunity to take action on the event in question.  For
 * instance if an event starts, the device should take the appropriate event
 * action on the hardware.  This callback returns a bool, if returned value
 * is true, then a notification will be send over the air automatically to the
 * originator of the event.  If it is false, then nothing will be sent back to
 * the originator of the event.  Please note that in order for your
 * application to be ZigBee compliant, a notification must be sent over the
 * air to the originator of the event, so a value of false should only be
 * returned if your application code takes care of sending this message or
 * there is some other reason a message does not need to be sent by the
 * framework.
 *
 * @param loadControlEvent Actual event  Ver.: always
 * @param eventStatus Status of event  Ver.: always
 * @param sequenceNumber Sequence number  Ver.: always
 */
bool emberAfPluginDrlcEventActionCallback(EmberAfLoadControlEvent * loadControlEvent,
                                          EmberAfAmiEventStatus eventStatus,
                                          uint8_t sequenceNumber)
{
  return true;
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

/** @brief Display Message
 *
 * This function is called by the Messaging client plugin whenever the
 * application should display a message.
 *
 * @param message The message that should be displayed.  Ver.: always
 */
void emberAfPluginMessagingClientDisplayMessageCallback(EmberAfPluginMessagingClientMessage * message)
{
  emberAfMessagingClusterPrint("MSG DISPLAY: ");
  emberAfMessagingClusterPrintString(message->message);
  emberAfMessagingClusterPrintln("");
}

/** @brief Cancel Message
 *
 * This function is called by the Messaging client plugin whenever the
 * application should stop displaying a message.
 *
 * @param message The message that should no longer be displayed.  Ver.:
 * always
 */
void emberAfPluginMessagingClientCancelMessageCallback(EmberAfPluginMessagingClientMessage * message)
{
  emberAfMessagingClusterPrint("MSG CANCEL: ");
  emberAfMessagingClusterPrintString(message->message);
  emberAfMessagingClusterPrintln("");
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

/** @brief Request Mirror
 *
 * This function is called by the Simple Metering client plugin whenever a
 * Request Mirror command is received.  The application should return the
 * endpoint to which the mirror has been assigned.  If no mirror could be
 * assigned, the application should return 0xFFFF.
 *
 * @param requestingDeviceIeeeAddress   Ver.: always
 */
uint16_t emberAfPluginSimpleMeteringClientRequestMirrorCallback(EmberEUI64 requestingDeviceIeeeAddress)
{
  return 0xFFFF;
}

/** @brief Remove Mirror
 *
 * This function is called by the Simple Metering client plugin whenever a
 * Remove Mirror command is received.  The application should return the
 * endpoint on which the mirror has been removed.  If the mirror could not be
 * removed, the application should return 0xFFFF.
 *
 * @param requestingDeviceIeeeAddress   Ver.: always
 */
uint16_t emberAfPluginSimpleMeteringClientRemoveMirrorCallback(EmberEUI64 requestingDeviceIeeeAddress)
{
  return 0xFFFF;
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

/** @brief Poll Completed
 *
 * This function is called by the application framework after a poll is
 * completed.
 *
 * @param status Return status of a completed poll operation  Ver.: always
 */
void emberAfPluginEndDeviceSupportPollCompletedCallback(EmberStatus status)
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
