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
#include "app/framework/plugin/diagnostic-server/diagnostic-server.h"

/** @brief External Attribute Write
 *
 * This callback is called for whenever an attribute is written, if this
 * attribute was tagged to be stored externally. Function should return
 * EMBER_ZCL_STATUS_SUCCESS if write was successful and
 * EMBER_ZCL_STATUS_FAILURE if not. Upon successful exit, the data
 * from the passed buffer should be stored as the current value of the
 * attribute. Important to note is, that if this method ever returns false,
 * then it is possible to break contract of the write-undivided-attributes
 * global command. That command checks for ranges and attribute existence, but
 * if this method returns false, when the attribute is finally written, then
 * the contract of writing all or no attributes will be broken.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeMetadata   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param buffer   Ver.: always
 */
EmberAfStatus emberAfExternalAttributeWriteCallback(uint8_t endpoint,
                                                    EmberAfClusterId clusterId,
                                                    EmberAfAttributeMetadata * attributeMetadata,
                                                    uint16_t manufacturerCode,
                                                    uint8_t * buffer)
{
  return EMBER_ZCL_STATUS_FAILURE;
}

/** @brief External Attribute Read
 *
 * If an attribute is tagged to be stored externally, this function will be
 * called whenever that attribute is read. This function should return
 * EMBER_ZCL_STATUS_SUCCESS if read was successful and
 * EMBER_ZCL_STATUS_FAILURE if not. The passed buffer should be populated
 * with the associated attributes value.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeMetadata   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param buffer   Ver.: always
 */
EmberAfStatus emberAfExternalAttributeReadCallback(uint8_t endpoint,
                                                   EmberAfClusterId clusterId,
                                                   EmberAfAttributeMetadata * attributeMetadata,
                                                   uint16_t manufacturerCode,
                                                   uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
  if (clusterId == ZCL_DIAGNOSTICS_CLUSTER_ID) {
    if (emberAfReadDiagnosticAttribute(attributeMetadata, buffer)) {
      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }
  return EMBER_ZCL_STATUS_FAILURE;
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

/** @brief Activate Door Lock
 *
 * This function is provided by the door lock server plugin. It is
 *
 * @param activate True if the lock should move to the locked position, false
 * if it should move to the unlocked position  Ver.: always
 */
bool emberAfPluginDoorLockServerActivateDoorLockCallback(bool activate)
{
  return true;
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

/** @brief Group Names Supported
 *
 * This function is called by the framework when it is necessary to determine
 * whether or not group names are supported.
 *
 * @param endpoint The endpoint. Ver.: always
 */
bool emberAfPluginGroupsServerGroupNamesSupportedCallback(uint8_t endpoint)
{
  return false;
}

/** @brief Get Group Name
 *
 * This function is called by the framework to query for a group name based
 * on the endpoint and the group ID.
 *
 * @param endpoint The endpoint. Ver.: always
 * @param groupId  The group ID. Ver.: always
 * @param groupName Pointer to the group name. Ver.: always
 */
void emberAfPluginGroupsServerGetGroupNameCallback(uint8_t endpoint,
                                                   uint16_t groupId,
                                                   uint8_t *groupName)
{
}

/** @brief Set Group Name
 *
 * This function is called by the framework to set a group name based
 * on the endpoint and the group ID.
 *
 * @param endpoint The endpoint. Ver.: always
 * @param groupId  The group ID. Ver.: always
 * @param groupName Pointer to the group name. Ver.: always
 */
void emberAfPluginGroupsServerSetGroupNameCallback(uint8_t endpoint,
                                                   uint16_t groupId,
                                                   uint8_t *groupName)
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

/** @brief Counter Rollover
 *
 * This function is called every time a counter exceeds its threshold.
 *
 * @param type The counter that rolled over  Ver.: always
 */
void emberAfPluginCountersRolloverCallback(EmberCounterType type)
{
  emberAfCorePrintln("Counter %u rolled over", type);
}

bool emberAfPluginEm4EnterCallback(uint32_t* sleepDurationMs)
{
  if (emberOkToHibernate()) {
    *sleepDurationMs = 8000; //~8 sec- this changes by the radio to an approximation
    emberAfCorePrintln("\n\nHIBERNATING\n\n");
    return true;
  }
  return false;
}
