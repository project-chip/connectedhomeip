
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

#include "callback.h"
#include "cluster-id.h"

// Cluster Init Functions
void emberAfClusterInitCallback(uint8_t endpoint, EmberAfClusterId clusterId)
{
    switch (clusterId)
    {
    case ZCL_BASIC_CLUSTER_ID :
        emberAfBasicClusterServerInitCallback(endpoint);
        break;
    case ZCL_GROUPS_CLUSTER_ID :
        emberAfGroupsClusterServerInitCallback(endpoint);
        break;
    case ZCL_IDENTIFY_CLUSTER_ID :
        emberAfIdentifyClusterServerInitCallback(endpoint);
        break;
    case ZCL_LEVEL_CONTROL_CLUSTER_ID :
        emberAfLevelControlClusterServerInitCallback(endpoint);
        break;
    case ZCL_OCCUPANCY_SENSING_CLUSTER_ID :
        emberAfOccupancySensingClusterClientInitCallback(endpoint);
        break;
    case ZCL_ON_OFF_CLUSTER_ID :
        emberAfOnOffClusterServerInitCallback(endpoint);
        break;
    case ZCL_OTA_BOOTLOAD_CLUSTER_ID :
        emberAfOverTheAirBootloadingClusterClientInitCallback(endpoint);
        break;
    case ZCL_SCENES_CLUSTER_ID :
        emberAfScenesClusterServerInitCallback(endpoint);
        break;
    case ZCL_ZLL_COMMISSIONING_CLUSTER_ID :
        emberAfZLLCommissioningClusterServerInitCallback(endpoint);
        break;

    default:
        // Unrecognized cluster ID
        break;
    }
}

void __attribute__((weak)) emberAfBasicClusterServerInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfGroupsClusterServerInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfIdentifyClusterServerInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfLevelControlClusterServerInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfOccupancySensingClusterClientInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfOnOffClusterServerInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfOverTheAirBootloadingClusterClientInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfScenesClusterServerInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfZLLCommissioningClusterServerInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}

// Cluster Command callback

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
* @brief Identify Cluster TriggerEffect Command callback
* @param effectId
* @param effectVariant
*/

bool __attribute__((weak))  emberAfIdentifyClusterTriggerEffectCallback( EmberAfIdentifyEffectIdentifier effectId,  EmberAfIdentifyEffectVariant effectVariant)
{
    // To prevent warning
    (void) effectId;
    (void) effectVariant;

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
* @brief On/off Cluster OffWithEffect Command callback
* @param effectId
* @param effectVariant
*/

bool __attribute__((weak))  emberAfOnOffClusterOffWithEffectCallback( EmberAfOnOffEffectIdentifier effectId,  uint8_t effectVariant)
{
    // To prevent warning
    (void) effectId;
    (void) effectVariant;

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
* @brief On/off Cluster OnWithRecallGlobalScene Command callback
*/

bool __attribute__((weak))  emberAfOnOffClusterOnWithRecallGlobalSceneCallback(void)
{
    return false;
}

/**
* @brief On/off Cluster OnWithTimedOff Command callback
* @param onOffControl
* @param onTime
* @param offWaitTime
*/

bool __attribute__((weak))  emberAfOnOffClusterOnWithTimedOffCallback( uint8_t onOffControl,  uint16_t onTime,  uint16_t offWaitTime)
{
    // To prevent warning
    (void) onOffControl;
    (void) onTime;
    (void) offWaitTime;

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
* @brief Scenes Cluster CopyScene Command callback
* @param mode
* @param groupIdFrom
* @param sceneIdFrom
* @param groupIdTo
* @param sceneIdTo
*/

bool __attribute__((weak))  emberAfScenesClusterCopySceneCallback( uint8_t mode,  uint16_t groupIdFrom,  uint8_t sceneIdFrom,  uint16_t groupIdTo,  uint8_t sceneIdTo)
{
    // To prevent warning
    (void) mode;
    (void) groupIdFrom;
    (void) sceneIdFrom;
    (void) groupIdTo;
    (void) sceneIdTo;

    return false;
}

/**
* @brief Scenes Cluster EnhancedAddScene Command callback
* @param groupId
* @param sceneId
* @param transitionTime
* @param sceneName
* @param extensionFieldSets
*/

bool __attribute__((weak))  emberAfScenesClusterEnhancedAddSceneCallback( uint16_t groupId,  uint8_t sceneId,  uint16_t transitionTime,  uint8_t * sceneName,  EmberAfSceneExtensionFieldSet extensionFieldSets)
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
* @brief Scenes Cluster EnhancedViewScene Command callback
* @param groupId
* @param sceneId
*/

bool __attribute__((weak))  emberAfScenesClusterEnhancedViewSceneCallback( uint16_t groupId,  uint8_t sceneId)
{
    // To prevent warning
    (void) groupId;
    (void) sceneId;

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

/**
* @brief ZLL Commissioning Cluster DeviceInformationRequest Command callback
* @param transaction
* @param startIndex
*/

bool __attribute__((weak))  emberAfZLLCommissioningClusterDeviceInformationRequestCallback( uint32_t transaction,  uint8_t startIndex)
{
    // To prevent warning
    (void) transaction;
    (void) startIndex;

    return false;
}

/**
* @brief ZLL Commissioning Cluster GetEndpointListRequest Command callback
* @param startIndex
*/

bool __attribute__((weak))  emberAfZLLCommissioningClusterGetEndpointListRequestCallback( uint8_t startIndex)
{
    // To prevent warning
    (void) startIndex;

    return false;
}

/**
* @brief ZLL Commissioning Cluster GetGroupIdentifiersRequest Command callback
* @param startIndex
*/

bool __attribute__((weak))  emberAfZLLCommissioningClusterGetGroupIdentifiersRequestCallback( uint8_t startIndex)
{
    // To prevent warning
    (void) startIndex;

    return false;
}

/**
* @brief ZLL Commissioning Cluster IdentifyRequest Command callback
* @param transaction
* @param identifyDuration
*/

bool __attribute__((weak))  emberAfZLLCommissioningClusterIdentifyRequestCallback( uint32_t transaction,  uint16_t identifyDuration)
{
    // To prevent warning
    (void) transaction;
    (void) identifyDuration;

    return false;
}

/**
* @brief ZLL Commissioning Cluster NetworkJoinEndDeviceRequest Command callback
* @param transaction
* @param extendedPanId
* @param keyIndex
* @param encryptedNetworkKey
* @param networkUpdateId
* @param logicalChannel
* @param panId
* @param networkAddress
* @param groupIdentifiersBegin
* @param groupIdentifiersEnd
* @param freeNetworkAddressRangeBegin
* @param freeNetworkAddressRangeEnd
* @param freeGroupIdentifierRangeBegin
* @param freeGroupIdentifierRangeEnd
*/

bool __attribute__((weak))  emberAfZLLCommissioningClusterNetworkJoinEndDeviceRequestCallback( uint32_t transaction,  uint8_t * extendedPanId,  EmberAfKeyIndex keyIndex,  /* TYPE WARNING: security_key defaults to */ uint8_t *  encryptedNetworkKey,  uint8_t networkUpdateId,  uint8_t logicalChannel,  uint16_t panId,  uint16_t networkAddress,  uint16_t groupIdentifiersBegin,  uint16_t groupIdentifiersEnd,  uint16_t freeNetworkAddressRangeBegin,  uint16_t freeNetworkAddressRangeEnd,  uint16_t freeGroupIdentifierRangeBegin,  uint16_t freeGroupIdentifierRangeEnd)
{
    // To prevent warning
    (void) transaction;
    (void) extendedPanId;
    (void) keyIndex;
    (void) encryptedNetworkKey;
    (void) networkUpdateId;
    (void) logicalChannel;
    (void) panId;
    (void) networkAddress;
    (void) groupIdentifiersBegin;
    (void) groupIdentifiersEnd;
    (void) freeNetworkAddressRangeBegin;
    (void) freeNetworkAddressRangeEnd;
    (void) freeGroupIdentifierRangeBegin;
    (void) freeGroupIdentifierRangeEnd;

    return false;
}

/**
* @brief ZLL Commissioning Cluster NetworkJoinRouterRequest Command callback
* @param transaction
* @param extendedPanId
* @param keyIndex
* @param encryptedNetworkKey
* @param networkUpdateId
* @param logicalChannel
* @param panId
* @param networkAddress
* @param groupIdentifiersBegin
* @param groupIdentifiersEnd
* @param freeNetworkAddressRangeBegin
* @param freeNetworkAddressRangeEnd
* @param freeGroupIdentifierRangeBegin
* @param freeGroupIdentifierRangeEnd
*/

bool __attribute__((weak))  emberAfZLLCommissioningClusterNetworkJoinRouterRequestCallback( uint32_t transaction,  uint8_t * extendedPanId,  EmberAfKeyIndex keyIndex,  /* TYPE WARNING: security_key defaults to */ uint8_t *  encryptedNetworkKey,  uint8_t networkUpdateId,  uint8_t logicalChannel,  uint16_t panId,  uint16_t networkAddress,  uint16_t groupIdentifiersBegin,  uint16_t groupIdentifiersEnd,  uint16_t freeNetworkAddressRangeBegin,  uint16_t freeNetworkAddressRangeEnd,  uint16_t freeGroupIdentifierRangeBegin,  uint16_t freeGroupIdentifierRangeEnd)
{
    // To prevent warning
    (void) transaction;
    (void) extendedPanId;
    (void) keyIndex;
    (void) encryptedNetworkKey;
    (void) networkUpdateId;
    (void) logicalChannel;
    (void) panId;
    (void) networkAddress;
    (void) groupIdentifiersBegin;
    (void) groupIdentifiersEnd;
    (void) freeNetworkAddressRangeBegin;
    (void) freeNetworkAddressRangeEnd;
    (void) freeGroupIdentifierRangeBegin;
    (void) freeGroupIdentifierRangeEnd;

    return false;
}

/**
* @brief ZLL Commissioning Cluster NetworkStartRequest Command callback
* @param transaction
* @param extendedPanId
* @param keyIndex
* @param encryptedNetworkKey
* @param logicalChannel
* @param panId
* @param networkAddress
* @param groupIdentifiersBegin
* @param groupIdentifiersEnd
* @param freeNetworkAddressRangeBegin
* @param freeNetworkAddressRangeEnd
* @param freeGroupIdentifierRangeBegin
* @param freeGroupIdentifierRangeEnd
* @param initiatorIeeeAddress
* @param initiatorNetworkAddress
*/

bool __attribute__((weak))  emberAfZLLCommissioningClusterNetworkStartRequestCallback( uint32_t transaction,  uint8_t * extendedPanId,  EmberAfKeyIndex keyIndex,  /* TYPE WARNING: security_key defaults to */ uint8_t *  encryptedNetworkKey,  uint8_t logicalChannel,  uint16_t panId,  uint16_t networkAddress,  uint16_t groupIdentifiersBegin,  uint16_t groupIdentifiersEnd,  uint16_t freeNetworkAddressRangeBegin,  uint16_t freeNetworkAddressRangeEnd,  uint16_t freeGroupIdentifierRangeBegin,  uint16_t freeGroupIdentifierRangeEnd,  uint8_t * initiatorIeeeAddress,  uint16_t initiatorNetworkAddress)
{
    // To prevent warning
    (void) transaction;
    (void) extendedPanId;
    (void) keyIndex;
    (void) encryptedNetworkKey;
    (void) logicalChannel;
    (void) panId;
    (void) networkAddress;
    (void) groupIdentifiersBegin;
    (void) groupIdentifiersEnd;
    (void) freeNetworkAddressRangeBegin;
    (void) freeNetworkAddressRangeEnd;
    (void) freeGroupIdentifierRangeBegin;
    (void) freeGroupIdentifierRangeEnd;
    (void) initiatorIeeeAddress;
    (void) initiatorNetworkAddress;

    return false;
}

/**
* @brief ZLL Commissioning Cluster NetworkUpdateRequest Command callback
* @param transaction
* @param extendedPanId
* @param networkUpdateId
* @param logicalChannel
* @param panId
* @param networkAddress
*/

bool __attribute__((weak))  emberAfZLLCommissioningClusterNetworkUpdateRequestCallback( uint32_t transaction,  uint8_t * extendedPanId,  uint8_t networkUpdateId,  uint8_t logicalChannel,  uint16_t panId,  uint16_t networkAddress)
{
    // To prevent warning
    (void) transaction;
    (void) extendedPanId;
    (void) networkUpdateId;
    (void) logicalChannel;
    (void) panId;
    (void) networkAddress;

    return false;
}

/**
* @brief ZLL Commissioning Cluster ResetToFactoryNewRequest Command callback
* @param transaction
*/

bool __attribute__((weak))  emberAfZLLCommissioningClusterResetToFactoryNewRequestCallback( uint32_t transaction)
{
    // To prevent warning
    (void) transaction;

    return false;
}

/**
* @brief ZLL Commissioning Cluster ScanRequest Command callback
* @param transaction
* @param zigbeeInformation
* @param zllInformation
*/

bool __attribute__((weak))  emberAfZLLCommissioningClusterScanRequestCallback( uint32_t transaction,  uint8_t zigbeeInformation,  uint8_t zllInformation)
{
    // To prevent warning
    (void) transaction;
    (void) zigbeeInformation;
    (void) zllInformation;

    return false;
}
