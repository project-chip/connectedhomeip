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

// Copyright 2007 - 2011 by Ember Corporation. All rights reserved.
//
//

// This callback file is created for your convenience. You may add application code
// to this file. If you regenerate this file over a previous version, the previous
// version will be overwritten and any code you have added will be lost.

#include "app/framework/include/af.h"

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
