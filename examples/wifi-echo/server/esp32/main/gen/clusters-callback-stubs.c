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

/** @brief Barrier Control Cluster Barrier Control Stop
 *
 *
 *
 */
bool emberAfBarrierControlClusterBarrierControlStopCallback(void)
{
    return false;
}

/** @brief Barrier Control Cluster Barrier Control Go To Percent
 *
 *
 *
 * @param percentOpen   Ver.: always
 */
bool emberAfBarrierControlClusterBarrierControlGoToPercentCallback(uint8_t percentOpen)
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

/** @brief Color Control Cluster Move Color
 *
 *
 *
 * @param rateX   Ver.: always
 * @param rateY   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterMoveColorCallback(int16_t rateX, int16_t rateY, uint8_t optionsMask, uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Move Color Temperature
 *
 *
 *
 * @param moveMode   Ver.: always
 * @param rate   Ver.: always
 * @param colorTemperatureMinimum   Ver.: always
 * @param colorTemperatureMaximum   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterMoveColorTemperatureCallback(uint8_t moveMode, uint16_t rate, uint16_t colorTemperatureMinimum,
                                                            uint16_t colorTemperatureMaximum, uint8_t optionsMask,
                                                            uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Move To Color
 *
 *
 *
 * @param colorX   Ver.: always
 * @param colorY   Ver.: always
 * @param transitionTime   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterMoveToColorCallback(uint16_t colorX, uint16_t colorY, uint16_t transitionTime, uint8_t optionsMask,
                                                   uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Move Hue
 *
 *
 *
 * @param moveMode   Ver.: always
 * @param rate   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterMoveHueCallback(uint8_t moveMode, uint8_t rate, uint8_t optionsMask, uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Move Saturation
 *
 *
 *
 * @param moveMode   Ver.: always
 * @param rate   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterMoveSaturationCallback(uint8_t moveMode, uint8_t rate, uint8_t optionsMask, uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Move To Color Temperature
 *
 *
 *
 * @param colorTemperature   Ver.: always
 * @param transitionTime   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterMoveToColorTemperatureCallback(uint16_t colorTemperature, uint16_t transitionTime,
                                                              uint8_t optionsMask, uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Move To Hue And Saturation
 *
 *
 *
 * @param hue   Ver.: always
 * @param saturation   Ver.: always
 * @param transitionTime   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterMoveToHueAndSaturationCallback(uint8_t hue, uint8_t saturation, uint16_t transitionTime,
                                                              uint8_t optionsMask, uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Move To Hue
 *
 *
 *
 * @param hue   Ver.: always
 * @param direction   Ver.: always
 * @param transitionTime   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterMoveToHueCallback(uint8_t hue, uint8_t direction, uint16_t transitionTime, uint8_t optionsMask,
                                                 uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Move To Saturation
 *
 *
 *
 * @param saturation   Ver.: always
 * @param transitionTime   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterMoveToSaturationCallback(uint8_t saturation, uint16_t transitionTime, uint8_t optionsMask,
                                                        uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Step Color
 *
 *
 *
 * @param stepX   Ver.: always
 * @param stepY   Ver.: always
 * @param transitionTime   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterStepColorCallback(int16_t stepX, int16_t stepY, uint16_t transitionTime, uint8_t optionsMask,
                                                 uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Step Color Temperature
 *
 *
 *
 * @param stepMode   Ver.: always
 * @param stepSize   Ver.: always
 * @param transitionTime   Ver.: always
 * @param colorTemperatureMinimum   Ver.: always
 * @param colorTemperatureMaximum   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterStepColorTemperatureCallback(uint8_t stepMode, uint16_t stepSize, uint16_t transitionTime,
                                                            uint16_t colorTemperatureMinimum, uint16_t colorTemperatureMaximum,
                                                            uint8_t optionsMask, uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Step Hue
 *
 *
 *
 * @param stepMode   Ver.: always
 * @param stepSize   Ver.: always
 * @param transitionTime   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterStepHueCallback(uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime, uint8_t optionsMask,
                                               uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Step Saturation
 *
 *
 *
 * @param stepMode   Ver.: always
 * @param stepSize   Ver.: always
 * @param transitionTime   Ver.: always
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterStepSaturationCallback(uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime,
                                                      uint8_t optionsMask, uint8_t optionsOverride)
{
    return false;
}

/** @brief Color Control Cluster Stop Move Step
 *
 *
 *
 * @param optionsMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionsOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfColorControlClusterStopMoveStepCallback(uint8_t optionsMask, uint8_t optionsOverride)
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

/** @brief Groups Cluster Add Group
 *
 *
 *
 * @param groupId   Ver.: always
 * @param groupName   Ver.: always
 */
bool emberAfGroupsClusterAddGroupCallback(uint16_t groupId, uint8_t * groupName)
{
    return false;
}

/** @brief Groups Cluster Add Group If Identifying
 *
 *
 *
 * @param groupId   Ver.: always
 * @param groupName   Ver.: always
 */
bool emberAfGroupsClusterAddGroupIfIdentifyingCallback(uint16_t groupId, uint8_t * groupName)
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

/** @brief Groups Cluster Get Group Membership
 *
 *
 *
 * @param groupCount   Ver.: always
 * @param groupList   Ver.: always
 */
bool emberAfGroupsClusterGetGroupMembershipCallback(uint8_t groupCount, uint8_t * groupList)
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

/** @brief Groups Cluster Remove All Groups
 *
 *
 *
 */
bool emberAfGroupsClusterRemoveAllGroupsCallback(void)
{
    return false;
}

/** @brief Groups Cluster Remove Group
 *
 *
 *
 * @param groupId   Ver.: always
 */
bool emberAfGroupsClusterRemoveGroupCallback(uint16_t groupId)
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

/** @brief Groups Cluster View Group
 *
 *
 *
 * @param groupId   Ver.: always
 */
bool emberAfGroupsClusterViewGroupCallback(uint16_t groupId)
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

/** @brief Identify Cluster Identify
 *
 *
 *
 * @param identifyTime   Ver.: always
 */
// bool emberAfIdentifyClusterIdentifyCallback(uint16_t identifyTime)
// {
//     return false;
// }

/** @brief Identify Cluster Identify Query
 *
 *
 *
 */
// bool emberAfIdentifyClusterIdentifyQueryCallback(void)
// {
//     return false;
// }

/** @brief Level Control Cluster Move
 *
 *
 *
 * @param moveMode   Ver.: always
 * @param rate   Ver.: always
 * @param optionMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfLevelControlClusterMoveCallback(uint8_t moveMode, uint8_t rate, uint8_t optionMask, uint8_t optionOverride)
{
    return false;
}

/** @brief Level Control Cluster Move To Level
 *
 *
 *
 * @param level   Ver.: always
 * @param transitionTime   Ver.: always
 * @param optionMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfLevelControlClusterMoveToLevelCallback(uint8_t level, uint16_t transitionTime, uint8_t optionMask,
                                                   uint8_t optionOverride)
{
    return false;
}

/** @brief Level Control Cluster Move To Level With On Off
 *
 *
 *
 * @param level   Ver.: always
 * @param transitionTime   Ver.: always
 */
bool emberAfLevelControlClusterMoveToLevelWithOnOffCallback(uint8_t level, uint16_t transitionTime)
{
    return false;
}

/** @brief Level Control Cluster Move With On Off
 *
 *
 *
 * @param moveMode   Ver.: always
 * @param rate   Ver.: always
 */
bool emberAfLevelControlClusterMoveWithOnOffCallback(uint8_t moveMode, uint8_t rate)
{
    return false;
}

/** @brief Level Control Cluster Step
 *
 *
 *
 * @param stepMode   Ver.: always
 * @param stepSize   Ver.: always
 * @param transitionTime   Ver.: always
 * @param optionMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfLevelControlClusterStepCallback(uint8_t stepMode, uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask,
                                            uint8_t optionOverride)
{
    return false;
}

/** @brief Level Control Cluster Stop
 *
 *
 *
 * @param optionMask   Ver.: since zcl6-errata-14-0129-15
 * @param optionOverride   Ver.: since zcl6-errata-14-0129-15
 */
bool emberAfLevelControlClusterStopCallback(uint8_t optionMask, uint8_t optionOverride)
{
    return false;
}

/** @brief Level Control Cluster Step With On Off
 *
 *
 *
 * @param stepMode   Ver.: always
 * @param stepSize   Ver.: always
 * @param transitionTime   Ver.: always
 */
bool emberAfLevelControlClusterStepWithOnOffCallback(uint8_t stepMode, uint8_t stepSize, uint16_t transitionTime)
{
    return false;
}

/** @brief Level Control Cluster Stop With On Off
 *
 *
 *
 */
bool emberAfLevelControlClusterStopWithOnOffCallback(void)
{
    return false;
}

/** @brief Scenes Cluster Add Scene
 *
 *
 *
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 * @param transitionTime   Ver.: always
 * @param sceneName   Ver.: always
 * @param extensionFieldSets   Ver.: always
 */
bool emberAfScenesClusterAddSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime, uint8_t * sceneName,
                                          uint8_t * extensionFieldSets)
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

/** @brief Scenes Cluster Get Scene Membership
 *
 *
 *
 * @param groupId   Ver.: always
 */
bool emberAfScenesClusterGetSceneMembershipCallback(uint16_t groupId)
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

/** @brief Scenes Cluster Recall Scene
 *
 *
 *
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 * @param transitionTime   Ver.: since zcl-7.0-07-5123-07
 */
bool emberAfScenesClusterRecallSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime)
{
    return false;
}

/** @brief Scenes Cluster Remove All Scenes
 *
 *
 *
 * @param groupId   Ver.: always
 */
bool emberAfScenesClusterRemoveAllScenesCallback(uint16_t groupId)
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

/** @brief Scenes Cluster Remove Scene
 *
 *
 *
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
bool emberAfScenesClusterRemoveSceneCallback(uint16_t groupId, uint8_t sceneId)
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

/** @brief Scenes Cluster Store Scene
 *
 *
 *
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
bool emberAfScenesClusterStoreSceneCallback(uint16_t groupId, uint8_t sceneId)
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

/** @brief Scenes Cluster View Scene
 *
 *
 *
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
bool emberAfScenesClusterViewSceneCallback(uint16_t groupId, uint8_t sceneId)
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
void emberAfColorControlClusterServerInitCallback(uint8_t endpoint) {}

void emberAfDoorLockClusterServerAttributeChangedCallback(uint8_t endpoint, EmberAfAttributeId attributeId) {}
void emberAfGroupsClusterServerInitCallback(uint8_t endpoint) {}
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
// void emberAfIdentifyClusterServerAttributeChangedCallback(uint8_t endpoint, EmberAfAttributeId attributeId) {}
// void emberAfIdentifyClusterServerInitCallback(uint8_t endpoint) {}

void emberAfLevelControlClusterServerInitCallback(uint8_t endpoint) {}
void emberAfPluginBarrierControlServerInitCallback(void) {}

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
void emberAfScenesClusterServerInitCallback(uint8_t endpoint) {}

void emberAfPluginIasZoneClientZdoCallback(EmberNodeId emberNodeId, EmberApsFrame * apsFrame, uint8_t * message, uint16_t length) {}

void emberAfPluginIasZoneClientWriteAttributesResponseCallback(EmberAfClusterId clusterId, uint8_t * buffer, uint16_t bufLen) {}

void emberAfPluginIasZoneClientReadAttributesResponseCallback(EmberAfClusterId clusterId, uint8_t * buffer, uint16_t bufLen) {}
