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

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "af.h"

/** @brief Identify Cluster Identify Query Response
 *
 *
 *
 * @param timeout   Ver.: always
 */
bool emberAfIdentifyClusterIdentifyQueryResponseCallback(uint16_t timeout)
{
    return false;
}

/** @brief Identify Cluster Start Feedback Callback
 *
 *
 *
 * @param endpoint Endpoint id
 * @param identifyTime Identify time
 */
bool emberAfPluginIdentifyStartFeedbackCallback(uint8_t endpoint, uint16_t identifyTime)
{
    return false;
}

/** @brief Identify Cluster Stop Feedback Callback
 *
 *
 *
 * @param endpoint Endpoint id
 */
bool emberAfPluginIdentifyStopFeedbackCallback(uint8_t endpoint)
{
    return false;
}

/** @brief Basic Cluster Reset To Factory Defaults
 *
 *
 *
 */
bool emberAfBasicClusterResetToFactoryDefaultsCallback(void)
{
    return false;
}

/** @brief Door Lock Cluster Clear All Pins
 *
 *
 *
 */
bool emberAfDoorLockClusterClearAllPinsCallback(void)
{
    return false;
}

/** @brief Door Lock Cluster Clear All Rfids
 *
 *
 *
 */
bool emberAfDoorLockClusterClearAllRfidsCallback(void)
{
    return false;
}

/** @brief Door Lock Cluster Clear Holiday Schedule
 *
 *
 *
 * @param scheduleId   Ver.: always
 */
bool emberAfDoorLockClusterClearHolidayScheduleCallback(uint8_t scheduleId)
{
    return false;
}

/** @brief Door Lock Cluster Clear Pin
 *
 *
 *
 * @param userId   Ver.: always
 */
bool emberAfDoorLockClusterClearPinCallback(uint16_t userId)
{
    return false;
}

/** @brief Door Lock Cluster Clear Rfid
 *
 *
 *
 * @param userId   Ver.: always
 */
bool emberAfDoorLockClusterClearRfidCallback(uint16_t userId)
{
    return false;
}

/** @brief Door Lock Cluster Clear Weekday Schedule
 *
 *
 *
 * @param scheduleId   Ver.: always
 * @param userId   Ver.: always
 */
bool emberAfDoorLockClusterClearWeekdayScheduleCallback(uint8_t scheduleId, uint16_t userId)
{
    return false;
}

/** @brief Door Lock Cluster Get Holiday Schedule
 *
 *
 *
 * @param scheduleId   Ver.: always
 */
bool emberAfDoorLockClusterGetHolidayScheduleCallback(uint8_t scheduleId)
{
    return false;
}

/** @brief Door Lock Cluster Get Log Record
 *
 *
 *
 * @param logIndex   Ver.: always
 */
bool emberAfDoorLockClusterGetLogRecordCallback(uint16_t logIndex)
{
    return false;
}

/** @brief Door Lock Cluster Get Pin
 *
 *
 *
 * @param userId   Ver.: always
 */
bool emberAfDoorLockClusterGetPinCallback(uint16_t userId)
{
    return false;
}

/** @brief Door Lock Cluster Get Rfid
 *
 *
 *
 * @param userId   Ver.: always
 */
bool emberAfDoorLockClusterGetRfidCallback(uint16_t userId)
{
    return false;
}

/** @brief Door Lock Cluster Get User Type
 *
 *
 *
 * @param userId   Ver.: always
 */
bool emberAfDoorLockClusterGetUserTypeCallback(uint16_t userId)
{
    return false;
}

/** @brief Door Lock Cluster Get Weekday Schedule
 *
 *
 *
 * @param scheduleId   Ver.: always
 * @param userId   Ver.: always
 */
bool emberAfDoorLockClusterGetWeekdayScheduleCallback(uint8_t scheduleId, uint16_t userId)
{
    return false;
}

/** @brief Door Lock Cluster Get Yearday Schedule
 *
 *
 *
 * @param scheduleId   Ver.: always
 * @param userId   Ver.: always
 */
bool emberAfDoorLockClusterGetYeardayScheduleCallback(uint8_t scheduleId, uint16_t userId)
{
    return false;
}

/** @brief Door Lock Cluster Lock Door
 *
 *
 *
 * @param PIN   Ver.: since ha-1.2-05-3520-29
 */
bool emberAfDoorLockClusterLockDoorCallback(uint8_t * PIN)
{
    return false;
}

/** @brief Door Lock Cluster Set Holiday Schedule
 *
 *
 *
 * @param scheduleId   Ver.: always
 * @param localStartTime   Ver.: always
 * @param localEndTime   Ver.: always
 * @param operatingModeDuringHoliday   Ver.: always
 */
bool emberAfDoorLockClusterSetHolidayScheduleCallback(uint8_t scheduleId, uint32_t localStartTime, uint32_t localEndTime,
                                                      uint8_t operatingModeDuringHoliday)
{
    return false;
}

/** @brief Door Lock Cluster Set Pin
 *
 *
 *
 * @param userId   Ver.: always
 * @param userStatus   Ver.: always
 * @param userType   Ver.: always
 * @param pin   Ver.: always
 */
bool emberAfDoorLockClusterSetPinCallback(uint16_t userId, uint8_t userStatus, uint8_t userType, uint8_t * pin)
{
    return false;
}

/** @brief Door Lock Cluster Set Rfid
 *
 *
 *
 * @param userId   Ver.: always
 * @param userStatus   Ver.: always
 * @param userType   Ver.: always
 * @param id   Ver.: always
 */
bool emberAfDoorLockClusterSetRfidCallback(uint16_t userId, uint8_t userStatus, uint8_t userType, uint8_t * id)
{
    return false;
}

/** @brief Door Lock Cluster Clear Yearday Schedule
 *
 *
 *
 * @param scheduleId   Ver.: always
 * @param userId   Ver.: always
 */
bool emberAfDoorLockClusterClearYeardayScheduleCallback(uint8_t scheduleId, uint16_t userId)
{
    return false;
}

/** @brief Door Lock Cluster Set User Type
 *
 *
 *
 * @param userId   Ver.: always
 * @param userType   Ver.: always
 */
bool emberAfDoorLockClusterSetUserTypeCallback(uint16_t userId, uint8_t userType)
{
    return false;
}

/** @brief Door Lock Cluster Set Weekday Schedule
 *
 *
 *
 * @param scheduleId   Ver.: always
 * @param userId   Ver.: always
 * @param daysMask   Ver.: always
 * @param startHour   Ver.: always
 * @param startMinute   Ver.: always
 * @param endHour   Ver.: always
 * @param endMinute   Ver.: always
 */
bool emberAfDoorLockClusterSetWeekdayScheduleCallback(uint8_t scheduleId, uint16_t userId, uint8_t daysMask, uint8_t startHour,
                                                      uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    return false;
}

/** @brief Door Lock Cluster Set Yearday Schedule
 *
 *
 *
 * @param scheduleId   Ver.: always
 * @param userId   Ver.: always
 * @param localStartTime   Ver.: always
 * @param localEndTime   Ver.: always
 */
bool emberAfDoorLockClusterSetYeardayScheduleCallback(uint8_t scheduleId, uint16_t userId, uint32_t localStartTime,
                                                      uint32_t localEndTime)
{
    return false;
}

/** @brief Door Lock Cluster Unlock Door
 *
 *
 *
 * @param PIN   Ver.: since ha-1.2-05-3520-29
 */
bool emberAfDoorLockClusterUnlockDoorCallback(uint8_t * PIN)
{
    return false;
}

/** @brief Door Lock Cluster Unlock With Timeout
 *
 *
 *
 * @param timeoutInSeconds   Ver.: always
 * @param pin   Ver.: since ha-1.2-05-3520-29
 */
bool emberAfDoorLockClusterUnlockWithTimeoutCallback(uint16_t timeoutInSeconds, uint8_t * pin)
{
    return false;
}

/** @brief Groups Cluster Add Group Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 */
bool emberAfGroupsClusterAddGroupResponseCallback(uint8_t status, uint16_t groupId)
{
    return false;
}

/** @brief Groups Cluster Get Group Membership Response
 *
 *
 *
 * @param capacity   Ver.: always
 * @param groupCount   Ver.: always
 * @param groupList   Ver.: always
 */
bool emberAfGroupsClusterGetGroupMembershipResponseCallback(uint8_t capacity, uint8_t groupCount, uint8_t * groupList)
{
    return false;
}

/** @brief Groups Cluster Remove Group Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 */
bool emberAfGroupsClusterRemoveGroupResponseCallback(uint8_t status, uint16_t groupId)
{
    return false;
}

/** @brief Groups Cluster View Group Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param groupName   Ver.: always
 */
bool emberAfGroupsClusterViewGroupResponseCallback(uint8_t status, uint16_t groupId, uint8_t * groupName)
{
    return false;
}

/** @brief IAS Zone Cluster Zone Enroll Request
 *
 *
 *
 * @param zoneType   Ver.: always
 * @param manufacturerCode   Ver.: always
 */
bool emberAfIasZoneClusterZoneEnrollRequestCallback(uint16_t zoneType, uint16_t manufacturerCode)
{
    return false;
}

/** @brief IAS Zone Cluster Zone Enroll Response
 *
 *
 *
 * @param enrollResponseCode   Ver.: always
 * @param zoneId   Ver.: always
 */
bool emberAfIasZoneClusterZoneEnrollResponseCallback(uint8_t enrollResponseCode, uint8_t zoneId)
{
    return false;
}

/** @brief IAS Zone Cluster Zone Status Change Notification
 *
 *
 *
 * @param zoneStatus   Ver.: always
 * @param extendedStatus   Ver.: always
 * @param zoneId   Ver.: since ha-1.2-05-3520-29
 * @param delay   Ver.: since ha-1.2-05-3520-29
 */
bool emberAfIasZoneClusterZoneStatusChangeNotificationCallback(uint16_t zoneStatus, uint8_t extendedStatus, uint8_t zoneId,
                                                               uint16_t delay)
{
    return false;
}

/** @brief Scenes Cluster Add Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
bool emberAfScenesClusterAddSceneResponseCallback(uint8_t status, uint16_t groupId, uint8_t sceneId)
{
    return false;
}

/** @brief Scenes Cluster Get Scene Membership Response
 *
 *
 *
 * @param status   Ver.: always
 * @param capacity   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneCount   Ver.: always
 * @param sceneList   Ver.: always
 */
bool emberAfScenesClusterGetSceneMembershipResponseCallback(uint8_t status, uint8_t capacity, uint16_t groupId, uint8_t sceneCount,
                                                            uint8_t * sceneList)
{
    return false;
}

/** @brief Scenes Cluster Remove All Scenes Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 */
bool emberAfScenesClusterRemoveAllScenesResponseCallback(uint8_t status, uint16_t groupId)
{
    return false;
}

/** @brief Scenes Cluster Remove Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
bool emberAfScenesClusterRemoveSceneResponseCallback(uint8_t status, uint16_t groupId, uint8_t sceneId)
{
    return false;
}

/** @brief Scenes Cluster Store Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
bool emberAfScenesClusterStoreSceneResponseCallback(uint8_t status, uint16_t groupId, uint8_t sceneId)
{
    return false;
}

/** @brief Scenes Cluster View Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 * @param transitionTime   Ver.: always
 * @param sceneName   Ver.: always
 * @param extensionFieldSets   Ver.: always
 */
bool emberAfScenesClusterViewSceneResponseCallback(uint8_t status, uint16_t groupId, uint8_t sceneId, uint16_t transitionTime,
                                                   uint8_t * sceneName, uint8_t * extensionFieldSets)
{
    return false;
}

// endpoint_config.h callbacks, grep'd from SDK, comment these out as clusters come in

void emberAfDoorLockClusterServerAttributeChangedCallback(uint8_t endpoint, EmberAfAttributeId attributeId) {}
void emberAfIasZoneClusterClientInitCallback(uint8_t endpoint) {}
void emberAfIasZoneClusterServerInitCallback(uint8_t endpoint) {}
void emberAfIasZoneClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint16_t indexOrDestination,
                                                    EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                    EmberStatus status)
{}

EmberAfStatus emberAfIasZoneClusterServerPreAttributeChangedCallback(uint8_t endpoint, EmberAfAttributeId attributeId,
                                                                     EmberAfAttributeType attributeType, uint8_t size,
                                                                     uint8_t * value)
{
    return EMBER_ZCL_STATUS_SUCCESS;
}

void emberAfPluginDoorLockServerInitCallback(void) {}
void emberAfPluginReportingInitCallback(void) {}
void emberAfPollControlClusterServerAttributeChangedCallback(uint8_t endpoint, EmberAfAttributeId attributeId) {}
void emberAfPollControlClusterServerInitCallback(uint8_t endpoint) {}
void emberAfPluginPollControlServerStackStatusCallback(EmberStatus status) {}
EmberAfStatus emberAfPollControlClusterServerPreAttributeChangedCallback(uint8_t endpoint, EmberAfAttributeId attributeId,
                                                                         EmberAfAttributeType attributeType, uint8_t size,
                                                                         uint8_t * value)
{
    return EMBER_ZCL_STATUS_SUCCESS;
}

void emberAfPluginIasZoneClientZdoCallback(EmberNodeId emberNodeId, EmberApsFrame * apsFrame, uint8_t * message, uint16_t length) {}

void emberAfPluginIasZoneClientWriteAttributesResponseCallback(EmberAfClusterId clusterId, uint8_t * buffer, uint16_t bufLen) {}

void emberAfPluginIasZoneClientReadAttributesResponseCallback(EmberAfClusterId clusterId, uint8_t * buffer, uint16_t bufLen) {}
