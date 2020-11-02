
  /*
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


#ifndef ZCL_CALLBACK_HEADER
#define ZCL_CALLBACK_HEADER

#include "af-structs.h"
#include "af-types.h"

/** @brief Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 */
void emberAfClusterInitCallback(uint8_t endpoint, EmberAfClusterId clusterId);

// Cluster Init Functions
void emberAfBarrierControlClusterInitCallback(uint8_t endpoint);
void emberAfBasicClusterInitCallback(uint8_t endpoint);
void emberAfColorControlClusterInitCallback(uint8_t endpoint);
void emberAfDoorLockClusterInitCallback(uint8_t endpoint);
void emberAfGroupsClusterInitCallback(uint8_t endpoint);
void emberAfIASZoneClusterInitCallback(uint8_t endpoint);
void emberAfIdentifyClusterInitCallback(uint8_t endpoint);
void emberAfLevelControlClusterInitCallback(uint8_t endpoint);
void emberAfOnOffClusterInitCallback(uint8_t endpoint);
void emberAfOnOffSwitchConfigurationClusterInitCallback(uint8_t endpoint);
void emberAfScenesClusterInitCallback(uint8_t endpoint);
void emberAfTemperatureMeasurementClusterInitCallback(uint8_t endpoint);


// CLuster Commands Callback

/**
* @brief Barrier Control Cluster BarrierControlGoToPercent Command callback
* @param percentOpen
*/

bool emberAfBarrierControlClusterBarrierControlGoToPercentCallback( uint8_t percentOpen);


/**
* @brief Barrier Control Cluster BarrierControlStop Command callback
*/

bool emberAfBarrierControlClusterBarrierControlStopCallback(void);


/**
* @brief Color Control Cluster MoveColor Command callback
* @param rateX
* @param rateY
* @param optionsMask
* @param optionsOverride
*/

bool emberAfColorControlClusterMoveColorCallback( int16_t rateX,  int16_t rateY,  uint8_t optionsMask,  uint8_t optionsOverride);


/**
* @brief Color Control Cluster MoveToColor Command callback
* @param colorX
* @param colorY
* @param transitionTime
* @param optionsMask
* @param optionsOverride
*/

bool emberAfColorControlClusterMoveToColorCallback( uint16_t colorX,  uint16_t colorY,  uint16_t transitionTime,  uint8_t optionsMask,  uint8_t optionsOverride);


/**
* @brief Color Control Cluster StepColor Command callback
* @param stepX
* @param stepY
* @param transitionTime
* @param optionsMask
* @param optionsOverride
*/

bool emberAfColorControlClusterStepColorCallback( int16_t stepX,  int16_t stepY,  uint16_t transitionTime,  uint8_t optionsMask,  uint8_t optionsOverride);


/**
* @brief Door Lock Cluster LockDoor Command callback
* @param pIN
*/

bool emberAfDoorLockClusterLockDoorCallback( uint8_t * PIN);


/**
* @brief Door Lock Cluster UnlockDoor Command callback
* @param pIN
*/

bool emberAfDoorLockClusterUnlockDoorCallback( uint8_t * PIN);


/**
* @brief Groups Cluster AddGroupResponse Command callback
* @param status
* @param groupId
*/

bool emberAfGroupsClusterAddGroupResponseCallback( EmberAfStatus status,  uint16_t groupId);


/**
* @brief Groups Cluster GetGroupMembershipResponse Command callback
* @param capacity
* @param groupCount
* @param groupList
*/

bool emberAfGroupsClusterGetGroupMembershipResponseCallback( uint8_t capacity,  uint8_t groupCount,  uint16_t groupList);


/**
* @brief Groups Cluster RemoveGroupResponse Command callback
* @param status
* @param groupId
*/

bool emberAfGroupsClusterRemoveGroupResponseCallback( EmberAfStatus status,  uint16_t groupId);


/**
* @brief Groups Cluster ViewGroupResponse Command callback
* @param status
* @param groupId
* @param groupName
*/

bool emberAfGroupsClusterViewGroupResponseCallback( EmberAfStatus status,  uint16_t groupId,  uint8_t * groupName);


/**
* @brief Groups Cluster AddGroup Command callback
* @param groupId
* @param groupName
*/

bool emberAfGroupsClusterAddGroupCallback( uint16_t groupId,  uint8_t * groupName);


/**
* @brief Groups Cluster AddGroupIfIdentifying Command callback
* @param groupId
* @param groupName
*/

bool emberAfGroupsClusterAddGroupIfIdentifyingCallback( uint16_t groupId,  uint8_t * groupName);


/**
* @brief Groups Cluster GetGroupMembership Command callback
* @param groupCount
* @param groupList
*/

bool emberAfGroupsClusterGetGroupMembershipCallback( uint8_t groupCount,  uint16_t groupList);


/**
* @brief Groups Cluster RemoveAllGroups Command callback
*/

bool emberAfGroupsClusterRemoveAllGroupsCallback(void);


/**
* @brief Groups Cluster RemoveGroup Command callback
* @param groupId
*/

bool emberAfGroupsClusterRemoveGroupCallback( uint16_t groupId);


/**
* @brief Groups Cluster ViewGroup Command callback
* @param groupId
*/

bool emberAfGroupsClusterViewGroupCallback( uint16_t groupId);


/**
* @brief IAS Zone Cluster ZoneEnrollRequest Command callback
* @param zoneType
* @param manufacturerCode
*/

bool emberAfIASZoneClusterZoneEnrollRequestCallback( EmberAfIasZoneType zoneType,  uint16_t manufacturerCode);


/**
* @brief IAS Zone Cluster ZoneStatusChangeNotification Command callback
* @param zoneStatus
* @param extendedStatus
* @param zoneId
* @param delay
*/

bool emberAfIASZoneClusterZoneStatusChangeNotificationCallback( uint16_t zoneStatus,  uint8_t extendedStatus,  uint8_t zoneId,  uint16_t delay);


/**
* @brief IAS Zone Cluster ZoneEnrollResponse Command callback
* @param enrollResponseCode
* @param zoneId
*/

bool emberAfIASZoneClusterZoneEnrollResponseCallback( EmberAfIasEnrollResponseCode enrollResponseCode,  uint8_t zoneId);


/**
* @brief Identify Cluster IdentifyQueryResponse Command callback
* @param timeout
*/

bool emberAfIdentifyClusterIdentifyQueryResponseCallback( uint16_t timeout);


/**
* @brief Identify Cluster Identify Command callback
* @param identifyTime
*/

bool emberAfIdentifyClusterIdentifyCallback( uint16_t identifyTime);


/**
* @brief Identify Cluster IdentifyQuery Command callback
*/

bool emberAfIdentifyClusterIdentifyQueryCallback(void);


/**
* @brief Level Control Cluster Move Command callback
* @param moveMode
* @param rate
* @param optionMask
* @param optionOverride
*/

bool emberAfLevelControlClusterMoveCallback( EmberAfMoveMode moveMode,  uint8_t rate,  uint8_t optionMask,  uint8_t optionOverride);


/**
* @brief Level Control Cluster MoveToLevel Command callback
* @param level
* @param transitionTime
* @param optionMask
* @param optionOverride
*/

bool emberAfLevelControlClusterMoveToLevelCallback( uint8_t level,  uint16_t transitionTime,  uint8_t optionMask,  uint8_t optionOverride);


/**
* @brief Level Control Cluster MoveToLevelWithOnOff Command callback
* @param level
* @param transitionTime
*/

bool emberAfLevelControlClusterMoveToLevelWithOnOffCallback( uint8_t level,  uint16_t transitionTime);


/**
* @brief Level Control Cluster MoveWithOnOff Command callback
* @param moveMode
* @param rate
*/

bool emberAfLevelControlClusterMoveWithOnOffCallback( EmberAfMoveMode moveMode,  uint8_t rate);


/**
* @brief Level Control Cluster Step Command callback
* @param stepMode
* @param stepSize
* @param transitionTime
* @param optionMask
* @param optionOverride
*/

bool emberAfLevelControlClusterStepCallback( EmberAfStepMode stepMode,  uint8_t stepSize,  uint16_t transitionTime,  uint8_t optionMask,  uint8_t optionOverride);


/**
* @brief Level Control Cluster StepWithOnOff Command callback
* @param stepMode
* @param stepSize
* @param transitionTime
*/

bool emberAfLevelControlClusterStepWithOnOffCallback( EmberAfStepMode stepMode,  uint8_t stepSize,  uint16_t transitionTime);


/**
* @brief Level Control Cluster Stop Command callback
* @param optionMask
* @param optionOverride
*/

bool emberAfLevelControlClusterStopCallback( uint8_t optionMask,  uint8_t optionOverride);


/**
* @brief Level Control Cluster StopWithOnOff Command callback
*/

bool emberAfLevelControlClusterStopWithOnOffCallback(void);


/**
* @brief On/off Cluster Off Command callback
*/

bool emberAfOnOffClusterOffCallback(void);


/**
* @brief On/off Cluster On Command callback
*/

bool emberAfOnOffClusterOnCallback(void);


/**
* @brief On/off Cluster Toggle Command callback
*/

bool emberAfOnOffClusterToggleCallback(void);


/**
* @brief Scenes Cluster AddSceneResponse Command callback
* @param status
* @param groupId
* @param sceneId
*/

bool emberAfScenesClusterAddSceneResponseCallback( EmberAfStatus status,  uint16_t groupId,  uint8_t sceneId);


/**
* @brief Scenes Cluster GetSceneMembershipResponse Command callback
* @param status
* @param capacity
* @param groupId
* @param sceneCount
* @param sceneList
*/

bool emberAfScenesClusterGetSceneMembershipResponseCallback( EmberAfStatus status,  uint8_t capacity,  uint16_t groupId,  uint8_t sceneCount,  uint8_t sceneList);


/**
* @brief Scenes Cluster RemoveAllScenesResponse Command callback
* @param status
* @param groupId
*/

bool emberAfScenesClusterRemoveAllScenesResponseCallback( EmberAfStatus status,  uint16_t groupId);


/**
* @brief Scenes Cluster RemoveSceneResponse Command callback
* @param status
* @param groupId
* @param sceneId
*/

bool emberAfScenesClusterRemoveSceneResponseCallback( EmberAfStatus status,  uint16_t groupId,  uint8_t sceneId);


/**
* @brief Scenes Cluster StoreSceneResponse Command callback
* @param status
* @param groupId
* @param sceneId
*/

bool emberAfScenesClusterStoreSceneResponseCallback( EmberAfStatus status,  uint16_t groupId,  uint8_t sceneId);


/**
* @brief Scenes Cluster ViewSceneResponse Command callback
* @param status
* @param groupId
* @param sceneId
* @param transitionTime
* @param sceneName
* @param extensionFieldSets
*/

bool emberAfScenesClusterViewSceneResponseCallback( EmberAfStatus status,  uint16_t groupId,  uint8_t sceneId,  uint16_t transitionTime,  uint8_t * sceneName,  EmberAfSceneExtensionFieldSet extensionFieldSets);


/**
* @brief Scenes Cluster AddScene Command callback
* @param groupId
* @param sceneId
* @param transitionTime
* @param sceneName
* @param extensionFieldSets
*/

bool emberAfScenesClusterAddSceneCallback( uint16_t groupId,  uint8_t sceneId,  uint16_t transitionTime,  uint8_t * sceneName,  EmberAfSceneExtensionFieldSet extensionFieldSets);


/**
* @brief Scenes Cluster GetSceneMembership Command callback
* @param groupId
*/

bool emberAfScenesClusterGetSceneMembershipCallback( uint16_t groupId);


/**
* @brief Scenes Cluster RecallScene Command callback
* @param groupId
* @param sceneId
* @param transitionTime
*/

bool emberAfScenesClusterRecallSceneCallback( uint16_t groupId,  uint8_t sceneId,  uint16_t transitionTime);


/**
* @brief Scenes Cluster RemoveAllScenes Command callback
* @param groupId
*/

bool emberAfScenesClusterRemoveAllScenesCallback( uint16_t groupId);


/**
* @brief Scenes Cluster RemoveScene Command callback
* @param groupId
* @param sceneId
*/

bool emberAfScenesClusterRemoveSceneCallback( uint16_t groupId,  uint8_t sceneId);


/**
* @brief Scenes Cluster StoreScene Command callback
* @param groupId
* @param sceneId
*/

bool emberAfScenesClusterStoreSceneCallback( uint16_t groupId,  uint8_t sceneId);


/**
* @brief Scenes Cluster ViewScene Command callback
* @param groupId
* @param sceneId
*/

bool emberAfScenesClusterViewSceneCallback( uint16_t groupId,  uint8_t sceneId);



#endif //ZCL_CALLBACK_HEADER