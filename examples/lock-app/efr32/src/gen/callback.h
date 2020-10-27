
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
void emberAfBasicClusterServerInitCallback(uint8_t endpoint);
void emberAfGroupsClusterServerInitCallback(uint8_t endpoint);
void emberAfIdentifyClusterServerInitCallback(uint8_t endpoint);
void emberAfLevelControlClusterServerInitCallback(uint8_t endpoint);
void emberAfOccupancySensingClusterClientInitCallback(uint8_t endpoint);
void emberAfOnOffClusterServerInitCallback(uint8_t endpoint);
void emberAfOverTheAirBootloadingClusterClientInitCallback(uint8_t endpoint);
void emberAfScenesClusterServerInitCallback(uint8_t endpoint);
void emberAfZLLCommissioningClusterServerInitCallback(uint8_t endpoint);

// CLuster Commands Callback

/**
 * @brief Groups Cluster AddGroup Command callback
 * @param groupId
 * @param groupName
 */

bool emberAfGroupsClusterAddGroupCallback(uint16_t groupId, uint8_t * groupName);

/**
 * @brief Groups Cluster AddGroupIfIdentifying Command callback
 * @param groupId
 * @param groupName
 */

bool emberAfGroupsClusterAddGroupIfIdentifyingCallback(uint16_t groupId, uint8_t * groupName);

/**
 * @brief Groups Cluster GetGroupMembership Command callback
 * @param groupCount
 * @param groupList
 */

bool emberAfGroupsClusterGetGroupMembershipCallback(uint8_t groupCount, uint16_t groupList);

/**
 * @brief Groups Cluster RemoveAllGroups Command callback
 */

bool emberAfGroupsClusterRemoveAllGroupsCallback(void);

/**
 * @brief Groups Cluster RemoveGroup Command callback
 * @param groupId
 */

bool emberAfGroupsClusterRemoveGroupCallback(uint16_t groupId);

/**
 * @brief Groups Cluster ViewGroup Command callback
 * @param groupId
 */

bool emberAfGroupsClusterViewGroupCallback(uint16_t groupId);

/**
 * @brief Identify Cluster Identify Command callback
 * @param identifyTime
 */

bool emberAfIdentifyClusterIdentifyCallback(uint16_t identifyTime);

/**
 * @brief Identify Cluster IdentifyQuery Command callback
 */

bool emberAfIdentifyClusterIdentifyQueryCallback(void);

/**
 * @brief Identify Cluster TriggerEffect Command callback
 * @param effectId
 * @param effectVariant
 */

bool emberAfIdentifyClusterTriggerEffectCallback(EmberAfIdentifyEffectIdentifier effectId,
                                                 EmberAfIdentifyEffectVariant effectVariant);

/**
 * @brief Level Control Cluster Move Command callback
 * @param moveMode
 * @param rate
 * @param optionMask
 * @param optionOverride
 */

bool emberAfLevelControlClusterMoveCallback(EmberAfMoveMode moveMode, uint8_t rate, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Level Control Cluster MoveToLevel Command callback
 * @param level
 * @param transitionTime
 * @param optionMask
 * @param optionOverride
 */

bool emberAfLevelControlClusterMoveToLevelCallback(uint8_t level, uint16_t transitionTime, uint8_t optionMask,
                                                   uint8_t optionOverride);

/**
 * @brief Level Control Cluster MoveToLevelWithOnOff Command callback
 * @param level
 * @param transitionTime
 */

bool emberAfLevelControlClusterMoveToLevelWithOnOffCallback(uint8_t level, uint16_t transitionTime);

/**
 * @brief Level Control Cluster MoveWithOnOff Command callback
 * @param moveMode
 * @param rate
 */

bool emberAfLevelControlClusterMoveWithOnOffCallback(EmberAfMoveMode moveMode, uint8_t rate);

/**
 * @brief Level Control Cluster Step Command callback
 * @param stepMode
 * @param stepSize
 * @param transitionTime
 * @param optionMask
 * @param optionOverride
 */

bool emberAfLevelControlClusterStepCallback(EmberAfStepMode stepMode, uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask,
                                            uint8_t optionOverride);

/**
 * @brief Level Control Cluster StepWithOnOff Command callback
 * @param stepMode
 * @param stepSize
 * @param transitionTime
 */

bool emberAfLevelControlClusterStepWithOnOffCallback(EmberAfStepMode stepMode, uint8_t stepSize, uint16_t transitionTime);

/**
 * @brief Level Control Cluster Stop Command callback
 * @param optionMask
 * @param optionOverride
 */

bool emberAfLevelControlClusterStopCallback(uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Level Control Cluster StopWithOnOff Command callback
 */

bool emberAfLevelControlClusterStopWithOnOffCallback(void);

/**
 * @brief On/off Cluster Off Command callback
 */

bool emberAfOnOffClusterOffCallback(void);

/**
 * @brief On/off Cluster OffWithEffect Command callback
 * @param effectId
 * @param effectVariant
 */

bool emberAfOnOffClusterOffWithEffectCallback(EmberAfOnOffEffectIdentifier effectId, uint8_t effectVariant);

/**
 * @brief On/off Cluster On Command callback
 */

bool emberAfOnOffClusterOnCallback(void);

/**
 * @brief On/off Cluster OnWithRecallGlobalScene Command callback
 */

bool emberAfOnOffClusterOnWithRecallGlobalSceneCallback(void);

/**
 * @brief On/off Cluster OnWithTimedOff Command callback
 * @param onOffControl
 * @param onTime
 * @param offWaitTime
 */

bool emberAfOnOffClusterOnWithTimedOffCallback(uint8_t onOffControl, uint16_t onTime, uint16_t offWaitTime);

/**
 * @brief On/off Cluster Toggle Command callback
 */

bool emberAfOnOffClusterToggleCallback(void);

/**
 * @brief Scenes Cluster AddScene Command callback
 * @param groupId
 * @param sceneId
 * @param transitionTime
 * @param sceneName
 * @param extensionFieldSets
 */

bool emberAfScenesClusterAddSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime, uint8_t * sceneName,
                                          EmberAfSceneExtensionFieldSet extensionFieldSets);

/**
 * @brief Scenes Cluster CopyScene Command callback
 * @param mode
 * @param groupIdFrom
 * @param sceneIdFrom
 * @param groupIdTo
 * @param sceneIdTo
 */

bool emberAfScenesClusterCopySceneCallback(uint8_t mode, uint16_t groupIdFrom, uint8_t sceneIdFrom, uint16_t groupIdTo,
                                           uint8_t sceneIdTo);

/**
 * @brief Scenes Cluster EnhancedAddScene Command callback
 * @param groupId
 * @param sceneId
 * @param transitionTime
 * @param sceneName
 * @param extensionFieldSets
 */

bool emberAfScenesClusterEnhancedAddSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime, uint8_t * sceneName,
                                                  EmberAfSceneExtensionFieldSet extensionFieldSets);

/**
 * @brief Scenes Cluster EnhancedViewScene Command callback
 * @param groupId
 * @param sceneId
 */

bool emberAfScenesClusterEnhancedViewSceneCallback(uint16_t groupId, uint8_t sceneId);

/**
 * @brief Scenes Cluster GetSceneMembership Command callback
 * @param groupId
 */

bool emberAfScenesClusterGetSceneMembershipCallback(uint16_t groupId);

/**
 * @brief Scenes Cluster RecallScene Command callback
 * @param groupId
 * @param sceneId
 * @param transitionTime
 */

bool emberAfScenesClusterRecallSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime);

/**
 * @brief Scenes Cluster RemoveAllScenes Command callback
 * @param groupId
 */

bool emberAfScenesClusterRemoveAllScenesCallback(uint16_t groupId);

/**
 * @brief Scenes Cluster RemoveScene Command callback
 * @param groupId
 * @param sceneId
 */

bool emberAfScenesClusterRemoveSceneCallback(uint16_t groupId, uint8_t sceneId);

/**
 * @brief Scenes Cluster StoreScene Command callback
 * @param groupId
 * @param sceneId
 */

bool emberAfScenesClusterStoreSceneCallback(uint16_t groupId, uint8_t sceneId);

/**
 * @brief Scenes Cluster ViewScene Command callback
 * @param groupId
 * @param sceneId
 */

bool emberAfScenesClusterViewSceneCallback(uint16_t groupId, uint8_t sceneId);

/**
 * @brief ZLL Commissioning Cluster DeviceInformationRequest Command callback
 * @param transaction
 * @param startIndex
 */

bool emberAfZLLCommissioningClusterDeviceInformationRequestCallback(uint32_t transaction, uint8_t startIndex);

/**
 * @brief ZLL Commissioning Cluster GetEndpointListRequest Command callback
 * @param startIndex
 */

bool emberAfZLLCommissioningClusterGetEndpointListRequestCallback(uint8_t startIndex);

/**
 * @brief ZLL Commissioning Cluster GetGroupIdentifiersRequest Command callback
 * @param startIndex
 */

bool emberAfZLLCommissioningClusterGetGroupIdentifiersRequestCallback(uint8_t startIndex);

/**
 * @brief ZLL Commissioning Cluster IdentifyRequest Command callback
 * @param transaction
 * @param identifyDuration
 */

bool emberAfZLLCommissioningClusterIdentifyRequestCallback(uint32_t transaction, uint16_t identifyDuration);

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

bool emberAfZLLCommissioningClusterNetworkJoinEndDeviceRequestCallback(
    uint32_t transaction, uint8_t * extendedPanId, EmberAfKeyIndex keyIndex,
    /* TYPE WARNING: security_key defaults to */ uint8_t * encryptedNetworkKey, uint8_t networkUpdateId, uint8_t logicalChannel,
    uint16_t panId, uint16_t networkAddress, uint16_t groupIdentifiersBegin, uint16_t groupIdentifiersEnd,
    uint16_t freeNetworkAddressRangeBegin, uint16_t freeNetworkAddressRangeEnd, uint16_t freeGroupIdentifierRangeBegin,
    uint16_t freeGroupIdentifierRangeEnd);

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

bool emberAfZLLCommissioningClusterNetworkJoinRouterRequestCallback(
    uint32_t transaction, uint8_t * extendedPanId, EmberAfKeyIndex keyIndex,
    /* TYPE WARNING: security_key defaults to */ uint8_t * encryptedNetworkKey, uint8_t networkUpdateId, uint8_t logicalChannel,
    uint16_t panId, uint16_t networkAddress, uint16_t groupIdentifiersBegin, uint16_t groupIdentifiersEnd,
    uint16_t freeNetworkAddressRangeBegin, uint16_t freeNetworkAddressRangeEnd, uint16_t freeGroupIdentifierRangeBegin,
    uint16_t freeGroupIdentifierRangeEnd);

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

bool emberAfZLLCommissioningClusterNetworkStartRequestCallback(
    uint32_t transaction, uint8_t * extendedPanId, EmberAfKeyIndex keyIndex,
    /* TYPE WARNING: security_key defaults to */ uint8_t * encryptedNetworkKey, uint8_t logicalChannel, uint16_t panId,
    uint16_t networkAddress, uint16_t groupIdentifiersBegin, uint16_t groupIdentifiersEnd, uint16_t freeNetworkAddressRangeBegin,
    uint16_t freeNetworkAddressRangeEnd, uint16_t freeGroupIdentifierRangeBegin, uint16_t freeGroupIdentifierRangeEnd,
    uint8_t * initiatorIeeeAddress, uint16_t initiatorNetworkAddress);

/**
 * @brief ZLL Commissioning Cluster NetworkUpdateRequest Command callback
 * @param transaction
 * @param extendedPanId
 * @param networkUpdateId
 * @param logicalChannel
 * @param panId
 * @param networkAddress
 */

bool emberAfZLLCommissioningClusterNetworkUpdateRequestCallback(uint32_t transaction, uint8_t * extendedPanId,
                                                                uint8_t networkUpdateId, uint8_t logicalChannel, uint16_t panId,
                                                                uint16_t networkAddress);

/**
 * @brief ZLL Commissioning Cluster ResetToFactoryNewRequest Command callback
 * @param transaction
 */

bool emberAfZLLCommissioningClusterResetToFactoryNewRequestCallback(uint32_t transaction);

/**
 * @brief ZLL Commissioning Cluster ScanRequest Command callback
 * @param transaction
 * @param zigbeeInformation
 * @param zllInformation
 */

bool emberAfZLLCommissioningClusterScanRequestCallback(uint32_t transaction, uint8_t zigbeeInformation, uint8_t zllInformation);

#endif // ZCL_CALLBACK_HEADER
