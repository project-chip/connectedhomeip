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
}

/** @brief Mirror Removed
 *
 *
 * @param endpointId   Ver.: always
 */
bool emberAfSimpleMeteringClusterMirrorRemovedCallback(uint16_t endpointId)
{
  return false;
}

/** @brief Request Mirror Response
 *
 *
 * @param endpointId   Ver.: always
 */
bool emberAfSimpleMeteringClusterRequestMirrorResponseCallback(uint16_t endpointId)
{
  return false;
}

/** @brief Ok To Sleep
 *
 * This function is called by the Idle/Sleep plugin before sleeping.  The
 * application should return true if the device may sleep or false otherwise.
 *
 * @param durationMs The maximum duration in milliseconds that the device will
 * sleep.  Ver.: always
 */
bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  return true;
}

/** @brief Wake Up
 *
 * This function is called by the Idle/Sleep plugin after sleeping.
 *
 * @param durationMs The duration in milliseconds that the device slept.
 * Ver.: always
 */
void emberAfPluginIdleSleepWakeUpCallback(uint32_t durationMs)
{
}

/** @brief Ok To Idle
 *
 * This function is called by the Idle/Sleep plugin before idling.  The
 * application should return true if the device may idle or false otherwise.
 *
 */
bool emberAfPluginIdleSleepOkToIdleCallback(void)
{
  return true;
}

/** @brief Active
 *
 * This function is called by the Idle/Sleep plugin after idling.
 *
 */
void emberAfPluginIdleSleepActiveCallback(void)
{
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

/** @brief Display Message
 *
 * This function is called by the Messaging client plugin whenever the
 * application should display a message.
 *
 * @param message The message that should be displayed.  Ver.: always
 */
void emberAfPluginMessagingClientDisplayMessageCallback(EmberAfPluginMessagingClientMessage * message)
{
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
