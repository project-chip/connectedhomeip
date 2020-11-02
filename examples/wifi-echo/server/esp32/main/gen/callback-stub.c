
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


#include "callback.h"
#include "cluster-id.h"

// Cluster Init Functions
void emberAfClusterInitCallback(uint8_t endpoint, EmberAfClusterId clusterId)
{
    switch (clusterId)
    {
     case ZCL_BARRIER_CONTROL_CLUSTER_ID :
        emberAfBarrierControlClusterInitCallback(endpoint);
        break;
     case ZCL_BASIC_CLUSTER_ID :
        emberAfBasicClusterInitCallback(endpoint);
        break;
     case ZCL_COLOR_CONTROL_CLUSTER_ID :
        emberAfColorControlClusterInitCallback(endpoint);
        break;
     case ZCL_DOOR_LOCK_CLUSTER_ID :
        emberAfDoorLockClusterInitCallback(endpoint);
        break;
     case ZCL_GROUPS_CLUSTER_ID :
        emberAfGroupsClusterInitCallback(endpoint);
        break;
     case ZCL_IAS_ZONE_CLUSTER_ID :
        emberAfIASZoneClusterInitCallback(endpoint);
        break;
     case ZCL_IDENTIFY_CLUSTER_ID :
        emberAfIdentifyClusterInitCallback(endpoint);
        break;
     case ZCL_LEVEL_CONTROL_CLUSTER_ID :
        emberAfLevelControlClusterInitCallback(endpoint);
        break;
     case ZCL_ON_OFF_CLUSTER_ID :
        emberAfOnOffClusterInitCallback(endpoint);
        break;
     case ZCL_ON_OFF_SWITCH_CONFIG_CLUSTER_ID :
        emberAfOnOffSwitchConfigurationClusterInitCallback(endpoint);
        break;
     case ZCL_SCENES_CLUSTER_ID :
        emberAfScenesClusterInitCallback(endpoint);
        break;
     case ZCL_TEMP_MEASUREMENT_CLUSTER_ID :
        emberAfTemperatureMeasurementClusterInitCallback(endpoint);
        break;

    default:
        // Unrecognized cluster ID
        break;
    }
}

void __attribute__((weak)) emberAfBarrierControlClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfBasicClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfColorControlClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfDoorLockClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfGroupsClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfIASZoneClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfIdentifyClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfLevelControlClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfOnOffClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfOnOffSwitchConfigurationClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfScenesClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfTemperatureMeasurementClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}

// Cluster Command callback

/**
* @brief Barrier Control Cluster BarrierControlGoToPercent Command callback
* @param percentOpen
*/

bool __attribute__((weak))  emberAfBarrierControlClusterBarrierControlGoToPercentCallback( uint8_t percentOpen)
{
    // To prevent warning
    (void) percentOpen;

    return false;
}

/**
* @brief Barrier Control Cluster BarrierControlStop Command callback
*/

bool __attribute__((weak))  emberAfBarrierControlClusterBarrierControlStopCallback(void)
{
    return false;
}

/**
* @brief Color Control Cluster MoveColor Command callback
* @param rateX
* @param rateY
* @param optionsMask
* @param optionsOverride
*/

bool __attribute__((weak))  emberAfColorControlClusterMoveColorCallback( int16_t rateX,  int16_t rateY,  uint8_t optionsMask,  uint8_t optionsOverride)
{
    // To prevent warning
    (void) rateX;
    (void) rateY;
    (void) optionsMask;
    (void) optionsOverride;

    return false;
}

/**
* @brief Color Control Cluster MoveToColor Command callback
* @param colorX
* @param colorY
* @param transitionTime
* @param optionsMask
* @param optionsOverride
*/

bool __attribute__((weak))  emberAfColorControlClusterMoveToColorCallback( uint16_t colorX,  uint16_t colorY,  uint16_t transitionTime,  uint8_t optionsMask,  uint8_t optionsOverride)
{
    // To prevent warning
    (void) colorX;
    (void) colorY;
    (void) transitionTime;
    (void) optionsMask;
    (void) optionsOverride;

    return false;
}

/**
* @brief Color Control Cluster StepColor Command callback
* @param stepX
* @param stepY
* @param transitionTime
* @param optionsMask
* @param optionsOverride
*/

bool __attribute__((weak))  emberAfColorControlClusterStepColorCallback( int16_t stepX,  int16_t stepY,  uint16_t transitionTime,  uint8_t optionsMask,  uint8_t optionsOverride)
{
    // To prevent warning
    (void) stepX;
    (void) stepY;
    (void) transitionTime;
    (void) optionsMask;
    (void) optionsOverride;

    return false;
}

/**
* @brief Door Lock Cluster LockDoor Command callback
* @param pIN
*/

bool __attribute__((weak))  emberAfDoorLockClusterLockDoorCallback( uint8_t * PIN)
{
    // To prevent warning
    (void) PIN;

    return false;
}

/**
* @brief Door Lock Cluster UnlockDoor Command callback
* @param pIN
*/

bool __attribute__((weak))  emberAfDoorLockClusterUnlockDoorCallback( uint8_t * PIN)
{
    // To prevent warning
    (void) PIN;

    return false;
}

/**
* @brief Groups Cluster AddGroupResponse Command callback
* @param status
* @param groupId
*/

bool __attribute__((weak))  emberAfGroupsClusterAddGroupResponseCallback( EmberAfStatus status,  uint16_t groupId)
{
    // To prevent warning
    (void) status;
    (void) groupId;

    return false;
}

/**
* @brief Groups Cluster GetGroupMembershipResponse Command callback
* @param capacity
* @param groupCount
* @param groupList
*/

bool __attribute__((weak))  emberAfGroupsClusterGetGroupMembershipResponseCallback( uint8_t capacity,  uint8_t groupCount,  uint16_t groupList)
{
    // To prevent warning
    (void) capacity;
    (void) groupCount;
    (void) groupList;

    return false;
}

/**
* @brief Groups Cluster RemoveGroupResponse Command callback
* @param status
* @param groupId
*/

bool __attribute__((weak))  emberAfGroupsClusterRemoveGroupResponseCallback( EmberAfStatus status,  uint16_t groupId)
{
    // To prevent warning
    (void) status;
    (void) groupId;

    return false;
}

/**
* @brief Groups Cluster ViewGroupResponse Command callback
* @param status
* @param groupId
* @param groupName
*/

bool __attribute__((weak))  emberAfGroupsClusterViewGroupResponseCallback( EmberAfStatus status,  uint16_t groupId,  uint8_t * groupName)
{
    // To prevent warning
    (void) status;
    (void) groupId;
    (void) groupName;

    return false;
}

/**
* @brief Groups Cluster AddGroup Command callback
* @param groupId
* @param groupName
*/

bool __attribute__((weak))  emberAfGroupsClusterAddGroupCallback( uint16_t groupId,  uint8_t * groupName)
{
    // To prevent warning
    (void) groupId;
    (void) groupName;

    return false;
}

/**
* @brief Groups Cluster AddGroupIfIdentifying Command callback
* @param groupId
* @param groupName
*/

bool __attribute__((weak))  emberAfGroupsClusterAddGroupIfIdentifyingCallback( uint16_t groupId,  uint8_t * groupName)
{
    // To prevent warning
    (void) groupId;
    (void) groupName;

    return false;
}

/**
* @brief Groups Cluster GetGroupMembership Command callback
* @param groupCount
* @param groupList
*/

bool __attribute__((weak))  emberAfGroupsClusterGetGroupMembershipCallback( uint8_t groupCount,  uint16_t groupList)
{
    // To prevent warning
    (void) groupCount;
    (void) groupList;

    return false;
}

/**
* @brief Groups Cluster RemoveAllGroups Command callback
*/

bool __attribute__((weak))  emberAfGroupsClusterRemoveAllGroupsCallback(void)
{
    return false;
}

/**
* @brief Groups Cluster RemoveGroup Command callback
* @param groupId
*/

bool __attribute__((weak))  emberAfGroupsClusterRemoveGroupCallback( uint16_t groupId)
{
    // To prevent warning
    (void) groupId;

    return false;
}

/**
* @brief Groups Cluster ViewGroup Command callback
* @param groupId
*/

bool __attribute__((weak))  emberAfGroupsClusterViewGroupCallback( uint16_t groupId)
{
    // To prevent warning
    (void) groupId;

    return false;
}

/**
* @brief IAS Zone Cluster ZoneEnrollRequest Command callback
* @param zoneType
* @param manufacturerCode
*/

bool __attribute__((weak))  emberAfIASZoneClusterZoneEnrollRequestCallback( EmberAfIasZoneType zoneType,  uint16_t manufacturerCode)
{
    // To prevent warning
    (void) zoneType;
    (void) manufacturerCode;

    return false;
}

/**
* @brief IAS Zone Cluster ZoneStatusChangeNotification Command callback
* @param zoneStatus
* @param extendedStatus
* @param zoneId
* @param delay
*/

bool __attribute__((weak))  emberAfIASZoneClusterZoneStatusChangeNotificationCallback( uint16_t zoneStatus,  uint8_t extendedStatus,  uint8_t zoneId,  uint16_t delay)
{
    // To prevent warning
    (void) zoneStatus;
    (void) extendedStatus;
    (void) zoneId;
    (void) delay;

    return false;
}

/**
* @brief IAS Zone Cluster ZoneEnrollResponse Command callback
* @param enrollResponseCode
* @param zoneId
*/

bool __attribute__((weak))  emberAfIASZoneClusterZoneEnrollResponseCallback( EmberAfIasEnrollResponseCode enrollResponseCode,  uint8_t zoneId)
{
    // To prevent warning
    (void) enrollResponseCode;
    (void) zoneId;

    return false;
}

/**
* @brief Identify Cluster IdentifyQueryResponse Command callback
* @param timeout
*/

bool __attribute__((weak))  emberAfIdentifyClusterIdentifyQueryResponseCallback( uint16_t timeout)
{
    // To prevent warning
    (void) timeout;

    return false;
}

/**
* @brief Identify Cluster Identify Command callback
* @param identifyTime
*/

bool __attribute__((weak))  emberAfIdentifyClusterIdentifyCallback( uint16_t identifyTime)
{
    // To prevent warning
    (void) identifyTime;

    return false;
}

/**
* @brief Identify Cluster IdentifyQuery Command callback
*/

bool __attribute__((weak))  emberAfIdentifyClusterIdentifyQueryCallback(void)
{
    return false;
}

/**
* @brief Level Control Cluster Move Command callback
* @param moveMode
* @param rate
* @param optionMask
* @param optionOverride
*/

bool __attribute__((weak))  emberAfLevelControlClusterMoveCallback( EmberAfMoveMode moveMode,  uint8_t rate,  uint8_t optionMask,  uint8_t optionOverride)
{
    // To prevent warning
    (void) moveMode;
    (void) rate;
    (void) optionMask;
    (void) optionOverride;

    return false;
}

/**
* @brief Level Control Cluster MoveToLevel Command callback
* @param level
* @param transitionTime
* @param optionMask
* @param optionOverride
*/

bool __attribute__((weak))  emberAfLevelControlClusterMoveToLevelCallback( uint8_t level,  uint16_t transitionTime,  uint8_t optionMask,  uint8_t optionOverride)
{
    // To prevent warning
    (void) level;
    (void) transitionTime;
    (void) optionMask;
    (void) optionOverride;

    return false;
}

/**
* @brief Level Control Cluster MoveToLevelWithOnOff Command callback
* @param level
* @param transitionTime
*/

bool __attribute__((weak))  emberAfLevelControlClusterMoveToLevelWithOnOffCallback( uint8_t level,  uint16_t transitionTime)
{
    // To prevent warning
    (void) level;
    (void) transitionTime;

    return false;
}

/**
* @brief Level Control Cluster MoveWithOnOff Command callback
* @param moveMode
* @param rate
*/

bool __attribute__((weak))  emberAfLevelControlClusterMoveWithOnOffCallback( EmberAfMoveMode moveMode,  uint8_t rate)
{
    // To prevent warning
    (void) moveMode;
    (void) rate;

    return false;
}

/**
* @brief Level Control Cluster Step Command callback
* @param stepMode
* @param stepSize
* @param transitionTime
* @param optionMask
* @param optionOverride
*/

bool __attribute__((weak))  emberAfLevelControlClusterStepCallback( EmberAfStepMode stepMode,  uint8_t stepSize,  uint16_t transitionTime,  uint8_t optionMask,  uint8_t optionOverride)
{
    // To prevent warning
    (void) stepMode;
    (void) stepSize;
    (void) transitionTime;
    (void) optionMask;
    (void) optionOverride;

    return false;
}

/**
* @brief Level Control Cluster StepWithOnOff Command callback
* @param stepMode
* @param stepSize
* @param transitionTime
*/

bool __attribute__((weak))  emberAfLevelControlClusterStepWithOnOffCallback( EmberAfStepMode stepMode,  uint8_t stepSize,  uint16_t transitionTime)
{
    // To prevent warning
    (void) stepMode;
    (void) stepSize;
    (void) transitionTime;

    return false;
}

/**
* @brief Level Control Cluster Stop Command callback
* @param optionMask
* @param optionOverride
*/

bool __attribute__((weak))  emberAfLevelControlClusterStopCallback( uint8_t optionMask,  uint8_t optionOverride)
{
    // To prevent warning
    (void) optionMask;
    (void) optionOverride;

    return false;
}

/**
* @brief Level Control Cluster StopWithOnOff Command callback
*/

bool __attribute__((weak))  emberAfLevelControlClusterStopWithOnOffCallback(void)
{
    return false;
}

/**
* @brief On/off Cluster Off Command callback
*/

bool __attribute__((weak))  emberAfOnOffClusterOffCallback(void)
{
    return false;
}

/**
* @brief On/off Cluster On Command callback
*/

bool __attribute__((weak))  emberAfOnOffClusterOnCallback(void)
{
    return false;
}

/**
* @brief On/off Cluster Toggle Command callback
*/

bool __attribute__((weak))  emberAfOnOffClusterToggleCallback(void)
{
    return false;
}

/**
* @brief Scenes Cluster AddSceneResponse Command callback
* @param status
* @param groupId
* @param sceneId
*/

bool __attribute__((weak))  emberAfScenesClusterAddSceneResponseCallback( EmberAfStatus status,  uint16_t groupId,  uint8_t sceneId)
{
    // To prevent warning
    (void) status;
    (void) groupId;
    (void) sceneId;

    return false;
}

/**
* @brief Scenes Cluster GetSceneMembershipResponse Command callback
* @param status
* @param capacity
* @param groupId
* @param sceneCount
* @param sceneList
*/

bool __attribute__((weak))  emberAfScenesClusterGetSceneMembershipResponseCallback( EmberAfStatus status,  uint8_t capacity,  uint16_t groupId,  uint8_t sceneCount,  uint8_t sceneList)
{
    // To prevent warning
    (void) status;
    (void) capacity;
    (void) groupId;
    (void) sceneCount;
    (void) sceneList;

    return false;
}

/**
* @brief Scenes Cluster RemoveAllScenesResponse Command callback
* @param status
* @param groupId
*/

bool __attribute__((weak))  emberAfScenesClusterRemoveAllScenesResponseCallback( EmberAfStatus status,  uint16_t groupId)
{
    // To prevent warning
    (void) status;
    (void) groupId;

    return false;
}

/**
* @brief Scenes Cluster RemoveSceneResponse Command callback
* @param status
* @param groupId
* @param sceneId
*/

bool __attribute__((weak))  emberAfScenesClusterRemoveSceneResponseCallback( EmberAfStatus status,  uint16_t groupId,  uint8_t sceneId)
{
    // To prevent warning
    (void) status;
    (void) groupId;
    (void) sceneId;

    return false;
}

/**
* @brief Scenes Cluster StoreSceneResponse Command callback
* @param status
* @param groupId
* @param sceneId
*/

bool __attribute__((weak))  emberAfScenesClusterStoreSceneResponseCallback( EmberAfStatus status,  uint16_t groupId,  uint8_t sceneId)
{
    // To prevent warning
    (void) status;
    (void) groupId;
    (void) sceneId;

    return false;
}

/**
* @brief Scenes Cluster ViewSceneResponse Command callback
* @param status
* @param groupId
* @param sceneId
* @param transitionTime
* @param sceneName
* @param extensionFieldSets
*/

bool __attribute__((weak))  emberAfScenesClusterViewSceneResponseCallback( EmberAfStatus status,  uint16_t groupId,  uint8_t sceneId,  uint16_t transitionTime,  uint8_t * sceneName,  EmberAfSceneExtensionFieldSet extensionFieldSets)
{
    // To prevent warning
    (void) status;
    (void) groupId;
    (void) sceneId;
    (void) transitionTime;
    (void) sceneName;
    (void) extensionFieldSets;

    return false;
}

/**
* @brief Scenes Cluster AddScene Command callback
* @param groupId
* @param sceneId
* @param transitionTime
* @param sceneName
* @param extensionFieldSets
*/

bool __attribute__((weak))  emberAfScenesClusterAddSceneCallback( uint16_t groupId,  uint8_t sceneId,  uint16_t transitionTime,  uint8_t * sceneName,  EmberAfSceneExtensionFieldSet extensionFieldSets)
{
    // To prevent warning
    (void) groupId;
    (void) sceneId;
    (void) transitionTime;
    (void) sceneName;
    (void) extensionFieldSets;

    return false;
}

/**
* @brief Scenes Cluster GetSceneMembership Command callback
* @param groupId
*/

bool __attribute__((weak))  emberAfScenesClusterGetSceneMembershipCallback( uint16_t groupId)
{
    // To prevent warning
    (void) groupId;

    return false;
}

/**
* @brief Scenes Cluster RecallScene Command callback
* @param groupId
* @param sceneId
* @param transitionTime
*/

bool __attribute__((weak))  emberAfScenesClusterRecallSceneCallback( uint16_t groupId,  uint8_t sceneId,  uint16_t transitionTime)
{
    // To prevent warning
    (void) groupId;
    (void) sceneId;
    (void) transitionTime;

    return false;
}

/**
* @brief Scenes Cluster RemoveAllScenes Command callback
* @param groupId
*/

bool __attribute__((weak))  emberAfScenesClusterRemoveAllScenesCallback( uint16_t groupId)
{
    // To prevent warning
    (void) groupId;

    return false;
}

/**
* @brief Scenes Cluster RemoveScene Command callback
* @param groupId
* @param sceneId
*/

bool __attribute__((weak))  emberAfScenesClusterRemoveSceneCallback( uint16_t groupId,  uint8_t sceneId)
{
    // To prevent warning
    (void) groupId;
    (void) sceneId;

    return false;
}

/**
* @brief Scenes Cluster StoreScene Command callback
* @param groupId
* @param sceneId
*/

bool __attribute__((weak))  emberAfScenesClusterStoreSceneCallback( uint16_t groupId,  uint8_t sceneId)
{
    // To prevent warning
    (void) groupId;
    (void) sceneId;

    return false;
}

/**
* @brief Scenes Cluster ViewScene Command callback
* @param groupId
* @param sceneId
*/

bool __attribute__((weak))  emberAfScenesClusterViewSceneCallback( uint16_t groupId,  uint8_t sceneId)
{
    // To prevent warning
    (void) groupId;
    (void) sceneId;

    return false;
}

