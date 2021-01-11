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

// Prevent multiple inclusion
#pragma once

/** @brief Command description for ReadAttributes
 *
 * Command: ReadAttributes
 * @param attributeIds ATTRIBUTE_ID []
 * @param attributeIdsLen int
 */
#define emberAfFillCommandGlobalReadAttributes(clusterId, attributeIds, attributeIdsLen)                                           \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_READ_ATTRIBUTES_COMMAND_ID, "b", attributeIds, attributeIdsLen);

/** @brief Command description for ReadAttributesResponse
 *
 * Command: ReadAttributesResponse
 * @param readAttributeStatusRecords ReadAttributeStatusRecord []
 * @param readAttributeStatusRecordsLen int
 */
#define emberAfFillCommandGlobalReadAttributesResponse(clusterId, readAttributeStatusRecords, readAttributeStatusRecordsLen)       \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID, "b", readAttributeStatusRecords,           \
                              readAttributeStatusRecordsLen);

/** @brief Command description for WriteAttributes
 *
 * Command: WriteAttributes
 * @param writeAttributeRecords WriteAttributeRecord []
 * @param writeAttributeRecordsLen int
 */
#define emberAfFillCommandGlobalWriteAttributes(clusterId, writeAttributeRecords, writeAttributeRecordsLen)                        \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_WRITE_ATTRIBUTES_COMMAND_ID, "b", writeAttributeRecords,                        \
                              writeAttributeRecordsLen);

/** @brief Command description for WriteAttributesUndivided
 *
 * Command: WriteAttributesUndivided
 * @param writeAttributeRecords WriteAttributeRecord []
 * @param writeAttributeRecordsLen int
 */
#define emberAfFillCommandGlobalWriteAttributesUndivided(clusterId, writeAttributeRecords, writeAttributeRecordsLen)               \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_WRITE_ATTRIBUTES_UNDIVIDED_COMMAND_ID, "b", writeAttributeRecords,              \
                              writeAttributeRecordsLen);

/** @brief Command description for WriteAttributesResponse
 *
 * Command: WriteAttributesResponse
 * @param writeAttributeStatusRecords WriteAttributeStatusRecord []
 * @param writeAttributeStatusRecordsLen int
 */
#define emberAfFillCommandGlobalWriteAttributesResponse(clusterId, writeAttributeStatusRecords, writeAttributeStatusRecordsLen)    \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_WRITE_ATTRIBUTES_RESPONSE_COMMAND_ID, "b", writeAttributeStatusRecords,         \
                              writeAttributeStatusRecordsLen);

/** @brief Command description for WriteAttributesNoResponse
 *
 * Command: WriteAttributesNoResponse
 * @param writeAttributeRecords WriteAttributeRecord []
 * @param writeAttributeRecordsLen int
 */
#define emberAfFillCommandGlobalWriteAttributesNoResponse(clusterId, writeAttributeRecords, writeAttributeRecordsLen)              \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_WRITE_ATTRIBUTES_NO_RESPONSE_COMMAND_ID, "b", writeAttributeRecords,            \
                              writeAttributeRecordsLen);

/** @brief Command description for ConfigureReporting
 *
 * Command: ConfigureReporting
 * @param configureReportingRecords ConfigureReportingRecord []
 * @param configureReportingRecordsLen int
 */
#define emberAfFillCommandGlobalConfigureReporting(clusterId, configureReportingRecords, configureReportingRecordsLen)             \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_CONFIGURE_REPORTING_COMMAND_ID, "b", configureReportingRecords,                 \
                              configureReportingRecordsLen);

/** @brief Command description for ConfigureReportingResponse
 *
 * Command: ConfigureReportingResponse
 * @param configureReportingStatusRecords ConfigureReportingStatusRecord []
 * @param configureReportingStatusRecordsLen int
 */
#define emberAfFillCommandGlobalConfigureReportingResponse(clusterId, configureReportingStatusRecords,                             \
                                                           configureReportingStatusRecordsLen)                                     \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_CONFIGURE_REPORTING_RESPONSE_COMMAND_ID, "b", configureReportingStatusRecords,  \
                              configureReportingStatusRecordsLen);

/** @brief Command description for ReadReportingConfiguration
 *
 * Command: ReadReportingConfiguration
 * @param readReportingConfigurationAttributeRecords ReadReportingConfigurationAttributeRecord []
 * @param readReportingConfigurationAttributeRecordsLen int
 */
#define emberAfFillCommandGlobalReadReportingConfiguration(clusterId, readReportingConfigurationAttributeRecords,                  \
                                                           readReportingConfigurationAttributeRecordsLen)                          \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_READ_REPORTING_CONFIGURATION_COMMAND_ID, "b",                                   \
                              readReportingConfigurationAttributeRecords, readReportingConfigurationAttributeRecordsLen);

/** @brief Command description for ReadReportingConfigurationResponse
 *
 * Command: ReadReportingConfigurationResponse
 * @param readReportingConfigurationRecords ReadReportingConfigurationRecord []
 * @param readReportingConfigurationRecordsLen int
 */
#define emberAfFillCommandGlobalReadReportingConfigurationResponse(clusterId, readReportingConfigurationRecords,                   \
                                                                   readReportingConfigurationRecordsLen)                           \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_READ_REPORTING_CONFIGURATION_RESPONSE_COMMAND_ID, "b",                          \
                              readReportingConfigurationRecords, readReportingConfigurationRecordsLen);

/** @brief Command description for ReportAttributes
 *
 * Command: ReportAttributes
 * @param reportAttributeRecords ReportAttributeRecord []
 * @param reportAttributeRecordsLen int
 */
#define emberAfFillCommandGlobalReportAttributes(clusterId, reportAttributeRecords, reportAttributeRecordsLen)                     \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_REPORT_ATTRIBUTES_COMMAND_ID, "b", reportAttributeRecords,                      \
                              reportAttributeRecordsLen);

/** @brief Command description for DefaultResponse
 *
 * Command: DefaultResponse
 * @param commandId INT8U
 * @param status Status
 */
#define emberAfFillCommandGlobalDefaultResponse(clusterId, commandId, status)                                                      \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_DEFAULT_RESPONSE_COMMAND_ID, "uu", commandId, status);

/** @brief Command description for DiscoverAttributes
 *
 * Command: DiscoverAttributes
 * @param startId ATTRIBUTE_ID
 * @param maxAttributeIds INT8U
 */
#define emberAfFillCommandGlobalDiscoverAttributes(clusterId, startId, maxAttributeIds)                                            \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID, "uu", startId, maxAttributeIds);

/** @brief Command description for DiscoverAttributesResponse
 *
 * Command: DiscoverAttributesResponse
 * @param discoveryComplete INT8U
 * @param discoverAttributesInfoRecords DiscoverAttributesInfoRecord []
 * @param discoverAttributesInfoRecordsLen int
 */
#define emberAfFillCommandGlobalDiscoverAttributesResponse(clusterId, discoveryComplete, discoverAttributesInfoRecords,            \
                                                           discoverAttributesInfoRecordsLen)                                       \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_DISCOVER_ATTRIBUTES_RESPONSE_COMMAND_ID, "ub", discoveryComplete,               \
                              discoverAttributesInfoRecords, discoverAttributesInfoRecordsLen);

/** @brief Command description for ReadAttributesStructured
 *
 * Command: ReadAttributesStructured
 * @param readStructuredAttributeRecords ReadStructuredAttributeRecord []
 * @param readStructuredAttributeRecordsLen int
 */
#define emberAfFillCommandGlobalReadAttributesStructured(clusterId, readStructuredAttributeRecords,                                \
                                                         readStructuredAttributeRecordsLen)                                        \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_READ_ATTRIBUTES_STRUCTURED_COMMAND_ID, "b", readStructuredAttributeRecords,     \
                              readStructuredAttributeRecordsLen);

/** @brief Command description for WriteAttributesStructured
 *
 * Command: WriteAttributesStructured
 * @param writeStructuredAttributeRecords WriteStructuredAttributeRecord []
 * @param writeStructuredAttributeRecordsLen int
 */
#define emberAfFillCommandGlobalWriteAttributesStructured(clusterId, writeStructuredAttributeRecords,                              \
                                                          writeStructuredAttributeRecordsLen)                                      \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_WRITE_ATTRIBUTES_STRUCTURED_COMMAND_ID, "b", writeStructuredAttributeRecords,   \
                              writeStructuredAttributeRecordsLen);

/** @brief Command description for WriteAttributesStructuredResponse
 *
 * Command: WriteAttributesStructuredResponse
 * @param writeStructuredAttributeStatusRecords WriteStructuredAttributeStatusRecord []
 * @param writeStructuredAttributeStatusRecordsLen int
 */
#define emberAfFillCommandGlobalWriteAttributesStructuredResponse(clusterId, writeStructuredAttributeStatusRecords,                \
                                                                  writeStructuredAttributeStatusRecordsLen)                        \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_WRITE_ATTRIBUTES_STRUCTURED_RESPONSE_COMMAND_ID, "b",                           \
                              writeStructuredAttributeStatusRecords, writeStructuredAttributeStatusRecordsLen);

/** @brief Command description for DiscoverCommandsReceived
 *
 * Command: DiscoverCommandsReceived
 * @param startCommandId INT8U
 * @param maxCommandIds INT8U
 */
#define emberAfFillCommandGlobalDiscoverCommandsReceived(clusterId, startCommandId, maxCommandIds)                                 \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_DISCOVER_COMMANDS_RECEIVED_COMMAND_ID, "uu", startCommandId, maxCommandIds);

/** @brief Command description for DiscoverCommandsReceivedResponse
 *
 * Command: DiscoverCommandsReceivedResponse
 * @param discoveryComplete INT8U
 * @param commandIds INT8U []
 * @param commandIdsLen int
 */
#define emberAfFillCommandGlobalDiscoverCommandsReceivedResponse(clusterId, discoveryComplete, commandIds, commandIdsLen)          \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_DISCOVER_COMMANDS_RECEIVED_RESPONSE_COMMAND_ID, "ub", discoveryComplete,        \
                              commandIds, commandIdsLen);

/** @brief Command description for DiscoverCommandsGenerated
 *
 * Command: DiscoverCommandsGenerated
 * @param startCommandId INT8U
 * @param maxCommandIds INT8U
 */
#define emberAfFillCommandGlobalDiscoverCommandsGenerated(clusterId, startCommandId, maxCommandIds)                                \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_DISCOVER_COMMANDS_GENERATED_COMMAND_ID, "uu", startCommandId, maxCommandIds);

/** @brief Command description for DiscoverCommandsGeneratedResponse
 *
 * Command: DiscoverCommandsGeneratedResponse
 * @param discoveryComplete INT8U
 * @param commandIds INT8U []
 * @param commandIdsLen int
 */
#define emberAfFillCommandGlobalDiscoverCommandsGeneratedResponse(clusterId, discoveryComplete, commandIds, commandIdsLen)         \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_DISCOVER_COMMANDS_GENERATED_RESPONSE_COMMAND_ID, "ub", discoveryComplete,       \
                              commandIds, commandIdsLen);

/** @brief Command description for DiscoverAttributesExtended
 *
 * Command: DiscoverAttributesExtended
 * @param startId ATTRIBUTE_ID
 * @param maxAttributeIds INT8U
 */
#define emberAfFillCommandGlobalDiscoverAttributesExtended(clusterId, startId, maxAttributeIds)                                    \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_DISCOVER_ATTRIBUTES_EXTENDED_COMMAND_ID, "uu", startId, maxAttributeIds);

/** @brief Command description for DiscoverAttributesExtendedResponse
 *
 * Command: DiscoverAttributesExtendedResponse
 * @param discoveryComplete INT8U
 * @param extendedDiscoverAttributesInfoRecords ExtendedDiscoverAttributesInfoRecord []
 * @param extendedDiscoverAttributesInfoRecordsLen int
 */
#define emberAfFillCommandGlobalDiscoverAttributesExtendedResponse(                                                                \
    clusterId, discoveryComplete, extendedDiscoverAttributesInfoRecords, extendedDiscoverAttributesInfoRecordsLen)                 \
    emberAfFillExternalBuffer(mask, clusterId, ZCL_DISCOVER_ATTRIBUTES_EXTENDED_RESPONSE_COMMAND_ID, "ub", discoveryComplete,      \
                              extendedDiscoverAttributesInfoRecords, extendedDiscoverAttributesInfoRecordsLen);

/** @brief Command description for ResetToFactoryDefaults
 *
 * Command: ResetToFactoryDefaults
 */
#define emberAfFillCommandBasicClusterResetToFactoryDefaults()                                                                     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_RESET_TO_FACTORY_DEFAULTS_COMMAND_ID, "", );

/** @brief Command description for Identify
 *
 * Command: Identify
 * @param identifyTime INT16U
 * @param timeout INT16U
 */
#define emberAfFillCommandIdentifyClusterIdentify(identifyTime, timeout)                                                           \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_IDENTIFY_COMMAND_ID, "uu", identifyTime, timeout);

/** @brief Command description for IdentifyQuery
 *
 * Command: IdentifyQuery
 */
#define emberAfFillCommandIdentifyClusterIdentifyQuery()                                                                           \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_IDENTIFY_QUERY_COMMAND_ID, "", );

/** @brief Command description for EZModeInvoke
 *
 * Command: EZModeInvoke
 * @param action BITMAP8
 */
#define emberAfFillCommandIdentifyClusterEZModeInvoke(action)                                                                      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_EZ_MODE_INVOKE_COMMAND_ID, "u", action);

/** @brief Command description for UpdateCommissionState
 *
 * Command: UpdateCommissionState
 * @param action ENUM8
 * @param commissionStateMask BITMAP8
 */
#define emberAfFillCommandIdentifyClusterUpdateCommissionState(action, commissionStateMask)                                        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_UPDATE_COMMISSION_STATE_COMMAND_ID, "uu", action, commissionStateMask);

/** @brief Command description for TriggerEffect
 *
 * Command: TriggerEffect
 * @param effectId IdentifyEffectIdentifier
 * @param effectVariant IdentifyEffectVariant
 */
#define emberAfFillCommandIdentifyClusterTriggerEffect(effectId, effectVariant)                                                    \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_TRIGGER_EFFECT_COMMAND_ID, "uu", effectId, effectVariant);

/** @brief Command description for AddGroup
 *
 * Command: AddGroup
 * @param groupId INT16U
 * @param status Status
 * @param groupName CHAR_STRING
 * @param groupId INT16U
 */
#define emberAfFillCommandGroupsClusterAddGroup(groupId, status, groupName, groupId)                                               \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_ADD_GROUP_COMMAND_ID, "uuuu", groupId, status, groupName, groupId);

/** @brief Command description for ViewGroup
 *
 * Command: ViewGroup
 * @param groupId INT16U
 * @param status Status
 * @param groupId INT16U
 * @param groupName CHAR_STRING
 */
#define emberAfFillCommandGroupsClusterViewGroup(groupId, status, groupId, groupName)                                              \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_VIEW_GROUP_COMMAND_ID, "uuuu", groupId, status, groupId, groupName);

/** @brief Command description for GetGroupMembership
 *
 * Command: GetGroupMembership
 * @param groupCount INT8U
 * @param capacity INT8U
 * @param groupList INT16U []
 * @param groupListLen int
 * @param groupCount INT8U
 * @param groupList INT16U []
 * @param groupListLen int
 */
#define emberAfFillCommandGroupsClusterGetGroupMembership(groupCount, capacity, groupList, groupListLen, groupCount, groupList,    \
                                                          groupListLen)                                                            \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_GROUP_MEMBERSHIP_COMMAND_ID, "uubub", groupCount, capacity, groupList, groupListLen,         \
                              groupCount, groupList, groupListLen);

/** @brief Command description for RemoveGroup
 *
 * Command: RemoveGroup
 * @param groupId INT16U
 * @param status Status
 * @param groupId INT16U
 */
#define emberAfFillCommandGroupsClusterRemoveGroup(groupId, status, groupId)                                                       \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_REMOVE_GROUP_COMMAND_ID, "uuu", groupId, status, groupId);

/** @brief Command description for RemoveAllGroups
 *
 * Command: RemoveAllGroups
 */
#define emberAfFillCommandGroupsClusterRemoveAllGroups()                                                                           \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_REMOVE_ALL_GROUPS_COMMAND_ID, "", );

/** @brief Command description for AddGroupIfIdentifying
 *
 * Command: AddGroupIfIdentifying
 * @param groupId INT16U
 * @param groupName CHAR_STRING
 */
#define emberAfFillCommandGroupsClusterAddGroupIfIdentifying(groupId, groupName)                                                   \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_ADD_GROUP_IF_IDENTIFYING_COMMAND_ID, "uu", groupId, groupName);

/** @brief Command description for AddScene
 *
 * Command: AddScene
 * @param groupId INT16U
 * @param status Status
 * @param sceneId INT8U
 * @param groupId INT16U
 * @param transitionTime INT16U
 * @param sceneId INT8U
 * @param sceneName CHAR_STRING
 * @param extensionFieldSets SceneExtensionFieldSet []
 * @param extensionFieldSetsLen int
 */
#define emberAfFillCommandScenesClusterAddScene(groupId, status, sceneId, groupId, transitionTime, sceneId, sceneName,             \
                                                extensionFieldSets, extensionFieldSetsLen)                                         \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_ADD_SCENE_COMMAND_ID, "uuuuuuub", groupId, status, sceneId, groupId, transitionTime, sceneId,    \
                              sceneName, extensionFieldSets, extensionFieldSetsLen);

/** @brief Command description for ViewScene
 *
 * Command: ViewScene
 * @param groupId INT16U
 * @param status Status
 * @param sceneId INT8U
 * @param groupId INT16U
 * @param sceneId INT8U
 * @param transitionTime INT16U
 * @param sceneName CHAR_STRING
 * @param extensionFieldSets SceneExtensionFieldSet []
 * @param extensionFieldSetsLen int
 */
#define emberAfFillCommandScenesClusterViewScene(groupId, status, sceneId, groupId, sceneId, transitionTime, sceneName,            \
                                                 extensionFieldSets, extensionFieldSetsLen)                                        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_VIEW_SCENE_COMMAND_ID, "uuuuuuub", groupId, status, sceneId, groupId, sceneId, transitionTime,   \
                              sceneName, extensionFieldSets, extensionFieldSetsLen);

/** @brief Command description for RemoveScene
 *
 * Command: RemoveScene
 * @param groupId INT16U
 * @param status Status
 * @param sceneId INT8U
 * @param groupId INT16U
 * @param sceneId INT8U
 */
#define emberAfFillCommandScenesClusterRemoveScene(groupId, status, sceneId, groupId, sceneId)                                     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_REMOVE_SCENE_COMMAND_ID, "uuuuu", groupId, status, sceneId, groupId, sceneId);

/** @brief Command description for RemoveAllScenes
 *
 * Command: RemoveAllScenes
 * @param groupId INT16U
 * @param status Status
 * @param groupId INT16U
 */
#define emberAfFillCommandScenesClusterRemoveAllScenes(groupId, status, groupId)                                                   \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_REMOVE_ALL_SCENES_COMMAND_ID, "uuu", groupId, status, groupId);

/** @brief Command description for StoreScene
 *
 * Command: StoreScene
 * @param groupId INT16U
 * @param status Status
 * @param sceneId INT8U
 * @param groupId INT16U
 * @param sceneId INT8U
 */
#define emberAfFillCommandScenesClusterStoreScene(groupId, status, sceneId, groupId, sceneId)                                      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_STORE_SCENE_COMMAND_ID, "uuuuu", groupId, status, sceneId, groupId, sceneId);

/** @brief Command description for RecallScene
 *
 * Command: RecallScene
 * @param groupId INT16U
 * @param sceneId INT8U
 * @param transitionTime INT16U
 */
#define emberAfFillCommandScenesClusterRecallScene(groupId, sceneId, transitionTime)                                               \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_RECALL_SCENE_COMMAND_ID, "uuu", groupId, sceneId, transitionTime);

/** @brief Command description for GetSceneMembership
 *
 * Command: GetSceneMembership
 * @param groupId INT16U
 * @param status Status
 * @param capacity INT8U
 * @param groupId INT16U
 * @param sceneCount INT8U
 * @param sceneList INT8U []
 * @param sceneListLen int
 */
#define emberAfFillCommandScenesClusterGetSceneMembership(groupId, status, capacity, groupId, sceneCount, sceneList, sceneListLen) \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_SCENE_MEMBERSHIP_COMMAND_ID, "uuuuub", groupId, status, capacity, groupId, sceneCount,       \
                              sceneList, sceneListLen);

/** @brief Command description for EnhancedAddScene
 *
 * Command: EnhancedAddScene
 * @param groupId INT16U
 * @param status Status
 * @param sceneId INT8U
 * @param groupId INT16U
 * @param transitionTime INT16U
 * @param sceneId INT8U
 * @param sceneName CHAR_STRING
 * @param extensionFieldSets SceneExtensionFieldSet []
 * @param extensionFieldSetsLen int
 */
#define emberAfFillCommandScenesClusterEnhancedAddScene(groupId, status, sceneId, groupId, transitionTime, sceneId, sceneName,     \
                                                        extensionFieldSets, extensionFieldSetsLen)                                 \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_ENHANCED_ADD_SCENE_COMMAND_ID, "uuuuuuub", groupId, status, sceneId, groupId, transitionTime,    \
                              sceneId, sceneName, extensionFieldSets, extensionFieldSetsLen);

/** @brief Command description for EnhancedViewScene
 *
 * Command: EnhancedViewScene
 * @param groupId INT16U
 * @param status Status
 * @param sceneId INT8U
 * @param groupId INT16U
 * @param sceneId INT8U
 * @param transitionTime INT16U
 * @param sceneName CHAR_STRING
 * @param extensionFieldSets SceneExtensionFieldSet []
 * @param extensionFieldSetsLen int
 */
#define emberAfFillCommandScenesClusterEnhancedViewScene(groupId, status, sceneId, groupId, sceneId, transitionTime, sceneName,    \
                                                         extensionFieldSets, extensionFieldSetsLen)                                \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_ENHANCED_VIEW_SCENE_COMMAND_ID, "uuuuuuub", groupId, status, sceneId, groupId, sceneId,          \
                              transitionTime, sceneName, extensionFieldSets, extensionFieldSetsLen);

/** @brief Command description for CopyScene
 *
 * Command: CopyScene
 * @param mode ScenesCopyMode
 * @param status Status
 * @param groupIdFrom INT16U
 * @param groupIdFrom INT16U
 * @param sceneIdFrom INT8U
 * @param sceneIdFrom INT8U
 * @param groupIdTo INT16U
 * @param sceneIdTo INT8U
 */
#define emberAfFillCommandScenesClusterCopyScene(mode, status, groupIdFrom, groupIdFrom, sceneIdFrom, sceneIdFrom, groupIdTo,      \
                                                 sceneIdTo)                                                                        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_COPY_SCENE_COMMAND_ID, "uuuuuuuu", mode, status, groupIdFrom, groupIdFrom, sceneIdFrom,          \
                              sceneIdFrom, groupIdTo, sceneIdTo);

/** @brief Command description for Off
 *
 * Command: Off
 */
#define emberAfFillCommandOn                                                                                                       \
    /                                                                                                                              \
        offClusterOff() emberAfFillExternalBuffer(mask,                                                                            \
                                                                                                                                   \
                                                  ZCL_OFF_COMMAND_ID, "", );

/** @brief Command description for On
 *
 * Command: On
 */
#define emberAfFillCommandOn                                                                                                       \
    /                                                                                                                              \
        offClusterOn() emberAfFillExternalBuffer(mask,                                                                             \
                                                                                                                                   \
                                                 ZCL_ON_COMMAND_ID, "", );

/** @brief Command description for Toggle
 *
 * Command: Toggle
 */
#define emberAfFillCommandOn                                                                                                       \
    /                                                                                                                              \
        offClusterToggle() emberAfFillExternalBuffer(mask,                                                                         \
                                                                                                                                   \
                                                     ZCL_TOGGLE_COMMAND_ID, "", );

/** @brief Command description for OffWithEffect
 *
 * Command: OffWithEffect
 * @param effectId OnOffEffectIdentifier
 * @param effectVariant ENUM8
 */
#define emberAfFillCommandOn                                                                                                       \
    /                                                                                                                              \
        offClusterOffWithEffect(effectId, effectVariant)                                                                           \
            emberAfFillExternalBuffer(mask,                                                                                        \
                                                                                                                                   \
                                      ZCL_OFF_WITH_EFFECT_COMMAND_ID, "uu", effectId, effectVariant);

/** @brief Command description for OnWithRecallGlobalScene
 *
 * Command: OnWithRecallGlobalScene
 */
#define emberAfFillCommandOn                                                                                                       \
    /                                                                                                                              \
        offClusterOnWithRecallGlobalScene() emberAfFillExternalBuffer(mask,                                                        \
                                                                                                                                   \
                                                                      ZCL_ON_WITH_RECALL_GLOBAL_SCENE_COMMAND_ID, "", );

/** @brief Command description for OnWithTimedOff
 *
 * Command: OnWithTimedOff
 * @param onOffControl OnOffControl
 * @param onTime INT16U
 * @param offWaitTime INT16U
 */
#define emberAfFillCommandOn                                                                                                       \
    /                                                                                                                              \
        offClusterOnWithTimedOff(onOffControl, onTime, offWaitTime)                                                                \
            emberAfFillExternalBuffer(mask,                                                                                        \
                                                                                                                                   \
                                      ZCL_ON_WITH_TIMED_OFF_COMMAND_ID, "uuu", onOffControl, onTime, offWaitTime);

/** @brief Command description for MoveToLevel
 *
 * Command: MoveToLevel
 * @param level INT8U
 * @param transitionTime INT16U
 * @param optionMask BITMAP8
 * @param optionOverride BITMAP8
 */
#define emberAfFillCommandLevel                                                                                                    \
    ControlClusterMoveToLevel(level, transitionTime, optionMask, optionOverride)                                                   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_MOVE_TO_LEVEL_COMMAND_ID, "uuuu", level, transitionTime, optionMask, optionOverride);

/** @brief Command description for Move
 *
 * Command: Move
 * @param moveMode MoveMode
 * @param rate INT8U
 * @param optionMask BITMAP8
 * @param optionOverride BITMAP8
 */
#define emberAfFillCommandLevel                                                                                                    \
    ControlClusterMove(moveMode, rate, optionMask, optionOverride)                                                                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_MOVE_COMMAND_ID, "uuuu", moveMode, rate, optionMask, optionOverride);

/** @brief Command description for Step
 *
 * Command: Step
 * @param stepMode StepMode
 * @param stepSize INT8U
 * @param transitionTime INT16U
 * @param optionMask BITMAP8
 * @param optionOverride BITMAP8
 */
#define emberAfFillCommandLevel                                                                                                    \
    ControlClusterStep(stepMode, stepSize, transitionTime, optionMask, optionOverride)                                             \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_STEP_COMMAND_ID, "uuuuu", stepMode, stepSize, transitionTime, optionMask, optionOverride);

/** @brief Command description for Stop
 *
 * Command: Stop
 * @param optionMask BITMAP8
 * @param optionOverride BITMAP8
 */
#define emberAfFillCommandLevel                                                                                                    \
    ControlClusterStop(optionMask, optionOverride)                                                                                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_STOP_COMMAND_ID, "uu", optionMask, optionOverride);

/** @brief Command description for MoveToLevelWithOnOff
 *
 * Command: MoveToLevelWithOnOff
 * @param level INT8U
 * @param transitionTime INT16U
 */
#define emberAfFillCommandLevel                                                                                                    \
    ControlClusterMoveToLevelWithOnOff(level, transitionTime)                                                                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID, "uu", level, transitionTime);

/** @brief Command description for MoveWithOnOff
 *
 * Command: MoveWithOnOff
 * @param moveMode MoveMode
 * @param rate INT8U
 */
#define emberAfFillCommandLevel                                                                                                    \
    ControlClusterMoveWithOnOff(moveMode, rate) emberAfFillExternalBuffer(mask,                                                    \
                                                                                                                                   \
                                                                          ZCL_MOVE_WITH_ON_OFF_COMMAND_ID, "uu", moveMode, rate);

/** @brief Command description for StepWithOnOff
 *
 * Command: StepWithOnOff
 * @param stepMode StepMode
 * @param stepSize INT8U
 * @param transitionTime INT16U
 */
#define emberAfFillCommandLevel                                                                                                    \
    ControlClusterStepWithOnOff(stepMode, stepSize, transitionTime)                                                                \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_STEP_WITH_ON_OFF_COMMAND_ID, "uuu", stepMode, stepSize, transitionTime);

/** @brief Command description for StopWithOnOff
 *
 * Command: StopWithOnOff
 */
#define emberAfFillCommandLevel                                                                                                    \
    ControlClusterStopWithOnOff() emberAfFillExternalBuffer(mask,                                                                  \
                                                                                                                                   \
                                                            ZCL_STOP_WITH_ON_OFF_COMMAND_ID, "", );

/** @brief Command description for ResetAlarm
 *
 * Command: ResetAlarm
 * @param alarmCode ENUM8
 * @param alarmCode ENUM8
 * @param clusterId CLUSTER_ID
 * @param clusterId CLUSTER_ID
 */
#define emberAfFillCommandAlarmsClusterResetAlarm(alarmCode, alarmCode, clusterId, clusterId)                                      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_RESET_ALARM_COMMAND_ID, "uuuu", alarmCode, alarmCode, clusterId, clusterId);

/** @brief Command description for ResetAllAlarms
 *
 * Command: ResetAllAlarms
 * @param status Status
 * @param alarmCode ENUM8
 * @param clusterId CLUSTER_ID
 * @param timeStamp INT32U
 */
#define emberAfFillCommandAlarmsClusterResetAllAlarms(status, alarmCode, clusterId, timeStamp)                                     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_RESET_ALL_ALARMS_COMMAND_ID, "uuuu", status, alarmCode, clusterId, timeStamp);

/** @brief Command description for GetAlarm
 *
 * Command: GetAlarm
 */
#define emberAfFillCommandAlarmsClusterGetAlarm()                                                                                  \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_ALARM_COMMAND_ID, "", );

/** @brief Command description for ResetAlarmLog
 *
 * Command: ResetAlarmLog
 */
#define emberAfFillCommandAlarmsClusterResetAlarmLog()                                                                             \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_RESET_ALARM_LOG_COMMAND_ID, "", );

/** @brief Command description for SetAbsoluteLocation
 *
 * Command: SetAbsoluteLocation
 * @param coordinate1 INT16S
 * @param status Status
 * @param coordinate2 INT16S
 * @param power INT16S
 * @param coordinate3 INT16S
 * @param pathLossExponent INT16U
 * @param power INT16S
 * @param calculationPeriod INT16U
 * @param pathLossExponent INT16U
 * @param numberRssiMeasurements INT8U
 * @param reportingPeriod INT16U
 */
#define emberAfFillCommandRSSI                                                                                                     \
    LocationClusterSetAbsoluteLocation(coordinate1, status, coordinate2, power, coordinate3, pathLossExponent, power,              \
                                       calculationPeriod, pathLossExponent, numberRssiMeasurements, reportingPeriod)               \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_ABSOLUTE_LOCATION_COMMAND_ID, "uuuuuuuuuuu", coordinate1, status, coordinate2, power,    \
                                  coordinate3, pathLossExponent, power, calculationPeriod, pathLossExponent,                       \
                                  numberRssiMeasurements, reportingPeriod);

/** @brief Command description for SetDeviceConfiguration
 *
 * Command: SetDeviceConfiguration
 * @param power INT16S
 * @param status Status
 * @param pathLossExponent INT16U
 * @param locationType LocationType
 * @param calculationPeriod INT16U
 * @param coordinate1 INT16S
 * @param numberRssiMeasurements INT8U
 * @param coordinate2 INT16S
 * @param reportingPeriod INT16U
 * @param coordinate3 INT16S
 * @param power INT16S
 * @param pathLossExponent INT16U
 * @param locationMethod LocationMethod
 * @param qualityMeasure INT8U
 * @param locationAge INT16U
 */
#define emberAfFillCommandRSSI                                                                                                     \
    LocationClusterSetDeviceConfiguration(power, status, pathLossExponent, locationType, calculationPeriod, coordinate1,           \
                                          numberRssiMeasurements, coordinate2, reportingPeriod, coordinate3, power,                \
                                          pathLossExponent, locationMethod, qualityMeasure, locationAge)                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_DEVICE_CONFIGURATION_COMMAND_ID, "uuuuuuuuuuuuuuu", power, status, pathLossExponent,     \
                                  locationType, calculationPeriod, coordinate1, numberRssiMeasurements, coordinate2,               \
                                  reportingPeriod, coordinate3, power, pathLossExponent, locationMethod, qualityMeasure,           \
                                  locationAge);

/** @brief Command description for GetDeviceConfiguration
 *
 * Command: GetDeviceConfiguration
 * @param targetAddress IEEE_ADDRESS
 * @param locationType LocationType
 * @param coordinate1 INT16S
 * @param coordinate2 INT16S
 * @param coordinate3 INT16S
 * @param power INT16S
 * @param pathLossExponent INT16U
 * @param locationMethod LocationMethod
 * @param qualityMeasure INT8U
 * @param locationAge INT16U
 */
#define emberAfFillCommandRSSI                                                                                                     \
    LocationClusterGetDeviceConfiguration(targetAddress, locationType, coordinate1, coordinate2, coordinate3, power,               \
                                          pathLossExponent, locationMethod, qualityMeasure, locationAge)                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_DEVICE_CONFIGURATION_COMMAND_ID, "uuuuuuuuuu", targetAddress, locationType, coordinate1, \
                                  coordinate2, coordinate3, power, pathLossExponent, locationMethod, qualityMeasure, locationAge);

/** @brief Command description for GetLocationData
 *
 * Command: GetLocationData
 * @param flags GetLocationDataFlags
 * @param locationType LocationType
 * @param numberResponses INT8U
 * @param coordinate1 INT16S
 * @param targetAddress IEEE_ADDRESS
 * @param coordinate2 INT16S
 * @param coordinate3 INT16S
 * @param qualityMeasure INT8U
 * @param locationAge INT16U
 */
#define emberAfFillCommandRSSI                                                                                                     \
    LocationClusterGetLocationData(flags, locationType, numberResponses, coordinate1, targetAddress, coordinate2, coordinate3,     \
                                   qualityMeasure, locationAge)                                                                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_LOCATION_DATA_COMMAND_ID, "uuuuuuuuu", flags, locationType, numberResponses,             \
                                  coordinate1, targetAddress, coordinate2, coordinate3, qualityMeasure, locationAge);

/** @brief Command description for RssiResponse
 *
 * Command: RssiResponse
 * @param replyingDevice IEEE_ADDRESS
 * @param locationType LocationType
 * @param coordinate1 INT16S
 * @param coordinate2 INT16S
 * @param coordinate3 INT16S
 * @param rssi INT8S
 * @param numberRssiMeasurements INT8U
 */
#define emberAfFillCommandRSSI                                                                                                     \
    LocationClusterRssiResponse(replyingDevice, locationType, coordinate1, coordinate2, coordinate3, rssi, numberRssiMeasurements) \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_RSSI_RESPONSE_COMMAND_ID, "uuuuuuu", replyingDevice, locationType, coordinate1, coordinate2, \
                                  coordinate3, rssi, numberRssiMeasurements);

/** @brief Command description for RssiRequest
 *
 * Command: RssiRequest
 * @param targetAddress IEEE_ADDRESS
 * @param numberRssiMeasurements INT8U
 * @param calculationPeriod INT16U
 */
#define emberAfFillCommandRSSI                                                                                                     \
    LocationClusterRssiRequest(targetAddress, numberRssiMeasurements, calculationPeriod)                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_RSSI_REQUEST_COMMAND_ID, "uuu", targetAddress, numberRssiMeasurements, calculationPeriod);

/** @brief Command description for AnchorNodeAnnounce
 *
 * Command: AnchorNodeAnnounce
 * @param anchorNodeIeeeAddress IEEE_ADDRESS
 * @param measuringDevice IEEE_ADDRESS
 * @param coordinate1 INT16S
 * @param neighbors INT8U
 * @param coordinate2 INT16S
 * @param neighborsInfo NeighborInfo []
 * @param neighborsInfoLen int
 * @param coordinate3 INT16S
 */
#define emberAfFillCommandRSSI                                                                                                     \
    LocationClusterAnchorNodeAnnounce(anchorNodeIeeeAddress, measuringDevice, coordinate1, neighbors, coordinate2, neighborsInfo,  \
                                      neighborsInfoLen, coordinate3)                                                               \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ANCHOR_NODE_ANNOUNCE_COMMAND_ID, "uuuuubu", anchorNodeIeeeAddress, measuringDevice,          \
                                  coordinate1, neighbors, coordinate2, neighborsInfo, neighborsInfoLen, coordinate3);

/** @brief Command description for RequestOwnLocation
 *
 * Command: RequestOwnLocation
 * @param blindNode IEEE_ADDRESS
 */
#define emberAfFillCommandRSSI                                                                                                     \
    LocationClusterRequestOwnLocation(blindNode) emberAfFillExternalBuffer(mask,                                                   \
                                                                                                                                   \
                                                                           ZCL_REQUEST_OWN_LOCATION_COMMAND_ID, "u", blindNode);

/** @brief Command description for RestartDevice
 *
 * Command: RestartDevice
 * @param options RestartOptions
 * @param status ENUM8
 * @param delay INT8U
 * @param jitter INT8U
 */
#define emberAfFillCommandCommissioningClusterRestartDevice(options, status, delay, jitter)                                        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_RESTART_DEVICE_COMMAND_ID, "uuuu", options, status, delay, jitter);

/** @brief Command description for SaveStartupParameters
 *
 * Command: SaveStartupParameters
 * @param options BITMAP8
 * @param status ENUM8
 * @param index INT8U
 */
#define emberAfFillCommandCommissioningClusterSaveStartupParameters(options, status, index)                                        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SAVE_STARTUP_PARAMETERS_COMMAND_ID, "uuu", options, status, index);

/** @brief Command description for RestoreStartupParameters
 *
 * Command: RestoreStartupParameters
 * @param options BITMAP8
 * @param status ENUM8
 * @param index INT8U
 */
#define emberAfFillCommandCommissioningClusterRestoreStartupParameters(options, status, index)                                     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_RESTORE_STARTUP_PARAMETERS_COMMAND_ID, "uuu", options, status, index);

/** @brief Command description for ResetStartupParameters
 *
 * Command: ResetStartupParameters
 * @param options ResetOptions
 * @param status ENUM8
 * @param index INT8U
 */
#define emberAfFillCommandCommissioningClusterResetStartupParameters(options, status, index)                                       \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_RESET_STARTUP_PARAMETERS_COMMAND_ID, "uuu", options, status, index);

/** @brief Command description for TransferPartitionedFrame
 *
 * Command: TransferPartitionedFrame
 * @param fragmentationOptions BITMAP8
 * @param ackOptions BITMAP8
 * @param partitionedIndicatorAndFrame INT8U []
 * @param partitionedIndicatorAndFrameLen int
 * @param firstFrameIdAndNackList INT8U []
 * @param firstFrameIdAndNackListLen int
 */
#define emberAfFillCommandPartitionClusterTransferPartitionedFrame(fragmentationOptions, ackOptions, partitionedIndicatorAndFrame, \
                                                                   partitionedIndicatorAndFrameLen, firstFrameIdAndNackList,       \
                                                                   firstFrameIdAndNackListLen)                                     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_TRANSFER_PARTITIONED_FRAME_COMMAND_ID, "uubb", fragmentationOptions, ackOptions,                 \
                              partitionedIndicatorAndFrame, partitionedIndicatorAndFrameLen, firstFrameIdAndNackList,              \
                              firstFrameIdAndNackListLen);

/** @brief Command description for ReadHandshakeParam
 *
 * Command: ReadHandshakeParam
 * @param partitionedClusterId CLUSTER_ID
 * @param partitionedClusterId CLUSTER_ID
 * @param attributeList ATTRIBUTE_ID []
 * @param attributeListLen int
 * @param readAttributeStatusRecords ReadAttributeStatusRecord []
 * @param readAttributeStatusRecordsLen int
 */
#define emberAfFillCommandPartitionClusterReadHandshakeParam(partitionedClusterId, partitionedClusterId, attributeList,            \
                                                             attributeListLen, readAttributeStatusRecords,                         \
                                                             readAttributeStatusRecordsLen)                                        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_READ_HANDSHAKE_PARAM_COMMAND_ID, "uubb", partitionedClusterId, partitionedClusterId,             \
                              attributeList, attributeListLen, readAttributeStatusRecords, readAttributeStatusRecordsLen);

/** @brief Command description for WriteHandshakeParam
 *
 * Command: WriteHandshakeParam
 * @param partitionedClusterId CLUSTER_ID
 * @param writeAttributeRecords WriteAttributeRecord []
 * @param writeAttributeRecordsLen int
 */
#define emberAfFillCommandPartitionClusterWriteHandshakeParam(partitionedClusterId, writeAttributeRecords,                         \
                                                              writeAttributeRecordsLen)                                            \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_WRITE_HANDSHAKE_PARAM_COMMAND_ID, "ub", partitionedClusterId, writeAttributeRecords,             \
                              writeAttributeRecordsLen);

/** @brief Command description for ImageNotify
 *
 * Command: ImageNotify
 * @param payloadType ENUM8
 * @param queryJitter INT8U
 * @param manufacturerId INT16U
 * @param imageType INT16U
 * @param newFileVersion INT32U
 */
#define emberAfFillCommandOver                                                                                                     \
    the Air BootloadingClusterImageNotify(payloadType, queryJitter, manufacturerId, imageType, newFileVersion)                     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_IMAGE_NOTIFY_COMMAND_ID, "uuuuu", payloadType, queryJitter, manufacturerId, imageType,       \
                                  newFileVersion);

/** @brief Command description for QueryNextImageRequest
 *
 * Command: QueryNextImageRequest
 * @param fieldControl INT8U
 * @param manufacturerId INT16U
 * @param imageType INT16U
 * @param currentFileVersion INT32U
 * @param hardwareVersion INT16U
 */
#define emberAfFillCommandOver                                                                                                     \
    the Air BootloadingClusterQueryNextImageRequest(fieldControl, manufacturerId, imageType, currentFileVersion, hardwareVersion)  \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_QUERY_NEXT_IMAGE_REQUEST_COMMAND_ID, "uuuuu", fieldControl, manufacturerId, imageType,       \
                                  currentFileVersion, hardwareVersion);

/** @brief Command description for QueryNextImageResponse
 *
 * Command: QueryNextImageResponse
 * @param status Status
 * @param manufacturerId INT16U
 * @param imageType INT16U
 * @param fileVersion INT32U
 * @param imageSize INT32U
 */
#define emberAfFillCommandOver                                                                                                     \
    the Air BootloadingClusterQueryNextImageResponse(status, manufacturerId, imageType, fileVersion, imageSize)                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_QUERY_NEXT_IMAGE_RESPONSE_COMMAND_ID, "uuuuu", status, manufacturerId, imageType,            \
                                  fileVersion, imageSize);

/** @brief Command description for ImageBlockRequest
 *
 * Command: ImageBlockRequest
 * @param fieldControl INT8U
 * @param manufacturerId INT16U
 * @param imageType INT16U
 * @param fileVersion INT32U
 * @param fileOffset INT32U
 * @param maxDataSize INT8U
 * @param requestNodeAddress IEEE_ADDRESS
 */
#define emberAfFillCommandOver                                                                                                     \
    the Air BootloadingClusterImageBlockRequest(fieldControl, manufacturerId, imageType, fileVersion, fileOffset, maxDataSize,     \
                                                requestNodeAddress)                                                                \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID, "uuuuuuu", fieldControl, manufacturerId, imageType,          \
                                  fileVersion, fileOffset, maxDataSize, requestNodeAddress);

/** @brief Command description for ImagePageRequest
 *
 * Command: ImagePageRequest
 * @param fieldControl INT8U
 * @param manufacturerId INT16U
 * @param imageType INT16U
 * @param fileVersion INT32U
 * @param fileOffset INT32U
 * @param maxDataSize INT8U
 * @param pageSize INT16U
 * @param responseSpacing INT16U
 * @param requestNodeAddress IEEE_ADDRESS
 */
#define emberAfFillCommandOver                                                                                                     \
    the Air BootloadingClusterImagePageRequest(fieldControl, manufacturerId, imageType, fileVersion, fileOffset, maxDataSize,      \
                                               pageSize, responseSpacing, requestNodeAddress)                                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID, "uuuuuuuuu", fieldControl, manufacturerId, imageType,         \
                                  fileVersion, fileOffset, maxDataSize, pageSize, responseSpacing, requestNodeAddress);

/** @brief Command description for ImageBlockResponse
 *
 * Command: ImageBlockResponse
 * @param status Status
 * @param manufacturerId INT16U
 * @param imageType INT16U
 * @param fileVersion INT32U
 * @param fileOffset INT32U
 * @param dataSize INT8U
 * @param imageData INT8U []
 * @param imageDataLen int
 */
#define emberAfFillCommandOver                                                                                                     \
    the Air BootloadingClusterImageBlockResponse(status, manufacturerId, imageType, fileVersion, fileOffset, dataSize, imageData,  \
                                                 imageDataLen)                                                                     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_IMAGE_BLOCK_RESPONSE_COMMAND_ID, "uuuuuub", status, manufacturerId, imageType, fileVersion,  \
                                  fileOffset, dataSize, imageData, imageDataLen);

/** @brief Command description for UpgradeEndRequest
 *
 * Command: UpgradeEndRequest
 * @param status Status
 * @param manufacturerId INT16U
 * @param imageType INT16U
 * @param fileVersion INT32U
 */
#define emberAfFillCommandOver                                                                                                     \
    the Air BootloadingClusterUpgradeEndRequest(status, manufacturerId, imageType, fileVersion)                                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_UPGRADE_END_REQUEST_COMMAND_ID, "uuuu", status, manufacturerId, imageType, fileVersion);

/** @brief Command description for UpgradeEndResponse
 *
 * Command: UpgradeEndResponse
 * @param manufacturerId INT16U
 * @param imageType INT16U
 * @param fileVersion INT32U
 * @param currentTime UTC_TIME
 * @param upgradeTime UTC_TIME
 */
#define emberAfFillCommandOver                                                                                                     \
    the Air BootloadingClusterUpgradeEndResponse(manufacturerId, imageType, fileVersion, currentTime, upgradeTime)                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_UPGRADE_END_RESPONSE_COMMAND_ID, "uuuuu", manufacturerId, imageType, fileVersion,            \
                                  currentTime, upgradeTime);

/** @brief Command description for QuerySpecificFileRequest
 *
 * Command: QuerySpecificFileRequest
 * @param requestNodeAddress IEEE_ADDRESS
 * @param manufacturerId INT16U
 * @param imageType INT16U
 * @param fileVersion INT32U
 * @param currentZigbeeStackVersion INT16U
 */
#define emberAfFillCommandOver                                                                                                     \
    the Air BootloadingClusterQuerySpecificFileRequest(requestNodeAddress, manufacturerId, imageType, fileVersion,                 \
                                                       currentZigbeeStackVersion)                                                  \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_QUERY_SPECIFIC_FILE_REQUEST_COMMAND_ID, "uuuuu", requestNodeAddress, manufacturerId,         \
                                  imageType, fileVersion, currentZigbeeStackVersion);

/** @brief Command description for QuerySpecificFileResponse
 *
 * Command: QuerySpecificFileResponse
 * @param status Status
 * @param manufacturerId INT16U
 * @param imageType INT16U
 * @param fileVersion INT32U
 * @param imageSize INT32U
 */
#define emberAfFillCommandOver                                                                                                     \
    the Air BootloadingClusterQuerySpecificFileResponse(status, manufacturerId, imageType, fileVersion, imageSize)                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_QUERY_SPECIFIC_FILE_RESPONSE_COMMAND_ID, "uuuuu", status, manufacturerId, imageType,         \
                                  fileVersion, imageSize);

/** @brief Command description for PowerProfileRequest
 *
 * Command: PowerProfileRequest
 * @param powerProfileId INT8U
 * @param totalProfileNum INT8U
 * @param powerProfileId INT8U
 * @param numOfTransferredPhases INT8U
 * @param transferredPhases TransferredPhase []
 * @param transferredPhasesLen int
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterPowerProfileRequest(powerProfileId, totalProfileNum, powerProfileId, numOfTransferredPhases, transferredPhases,  \
                                      transferredPhasesLen)                                                                        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_POWER_PROFILE_REQUEST_COMMAND_ID, "uuuub", powerProfileId, totalProfileNum, powerProfileId,  \
                                  numOfTransferredPhases, transferredPhases, transferredPhasesLen);

/** @brief Command description for PowerProfileStateRequest
 *
 * Command: PowerProfileStateRequest
 * @param totalProfileNum INT8U
 * @param powerProfileId INT8U
 * @param numOfTransferredPhases INT8U
 * @param transferredPhases TransferredPhase []
 * @param transferredPhasesLen int
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterPowerProfileStateRequest(totalProfileNum, powerProfileId, numOfTransferredPhases, transferredPhases,             \
                                           transferredPhasesLen)                                                                   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_POWER_PROFILE_STATE_REQUEST_COMMAND_ID, "uuub", totalProfileNum, powerProfileId,             \
                                  numOfTransferredPhases, transferredPhases, transferredPhasesLen);

/** @brief Command description for GetPowerProfilePriceResponse
 *
 * Command: GetPowerProfilePriceResponse
 * @param powerProfileId INT8U
 * @param powerProfileCount INT8U
 * @param currency INT16U
 * @param powerProfileRecords PowerProfileRecord []
 * @param powerProfileRecordsLen int
 * @param price INT32U
 * @param priceTrailingDigit INT8U
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterGetPowerProfilePriceResponse(powerProfileId, powerProfileCount, currency, powerProfileRecords,                   \
                                               powerProfileRecordsLen, price, priceTrailingDigit)                                  \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_POWER_PROFILE_PRICE_RESPONSE_COMMAND_ID, "uuubuu", powerProfileId, powerProfileCount,    \
                                  currency, powerProfileRecords, powerProfileRecordsLen, price, priceTrailingDigit);

/** @brief Command description for GetOverallSchedulePriceResponse
 *
 * Command: GetOverallSchedulePriceResponse
 * @param currency INT16U
 * @param powerProfileId INT8U
 * @param price INT32U
 * @param priceTrailingDigit INT8U
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterGetOverallSchedulePriceResponse(currency, powerProfileId, price, priceTrailingDigit) emberAfFillExternalBuffer(  \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_GET_OVERALL_SCHEDULE_PRICE_RESPONSE_COMMAND_ID, "uuuu", currency, powerProfileId, price, priceTrailingDigit);

/** @brief Command description for EnergyPhasesScheduleNotification
 *
 * Command: EnergyPhasesScheduleNotification
 * @param powerProfileId INT8U
 * @param powerProfileCount INT8U
 * @param numOfScheduledPhases INT8U
 * @param powerProfileRecords PowerProfileRecord []
 * @param powerProfileRecordsLen int
 * @param scheduledPhases ScheduledPhase []
 * @param scheduledPhasesLen int
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterEnergyPhasesScheduleNotification(powerProfileId, powerProfileCount, numOfScheduledPhases, powerProfileRecords,   \
                                                   powerProfileRecordsLen, scheduledPhases, scheduledPhasesLen)                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENERGY_PHASES_SCHEDULE_NOTIFICATION_COMMAND_ID, "uuubb", powerProfileId, powerProfileCount,  \
                                  numOfScheduledPhases, powerProfileRecords, powerProfileRecordsLen, scheduledPhases,              \
                                  scheduledPhasesLen);

/** @brief Command description for GetOverallSchedulePrice
 *
 * Command: GetOverallSchedulePrice
 * @param powerProfileId INT8U
 * @param numOfScheduledPhases INT8U
 * @param scheduledPhases ScheduledPhase []
 * @param scheduledPhasesLen int
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterGetOverallSchedulePrice(powerProfileId, numOfScheduledPhases, scheduledPhases, scheduledPhasesLen)               \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_OVERALL_SCHEDULE_PRICE_COMMAND_ID, "uub", powerProfileId, numOfScheduledPhases,          \
                                  scheduledPhases, scheduledPhasesLen);

/** @brief Command description for PowerProfileScheduleConstraintsRequest
 *
 * Command: PowerProfileScheduleConstraintsRequest
 * @param powerProfileId INT8U
 * @param powerProfileId INT8U
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterPowerProfileScheduleConstraintsRequest(powerProfileId, powerProfileId) emberAfFillExternalBuffer(                \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_POWER_PROFILE_SCHEDULE_CONSTRAINTS_REQUEST_COMMAND_ID, "uu", powerProfileId, powerProfileId);

/** @brief Command description for EnergyPhasesScheduleStateRequest
 *
 * Command: EnergyPhasesScheduleStateRequest
 * @param powerProfileId INT8U
 * @param powerProfileId INT8U
 * @param numOfScheduledPhases INT8U
 * @param scheduledPhases ScheduledPhase []
 * @param scheduledPhasesLen int
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterEnergyPhasesScheduleStateRequest(powerProfileId, powerProfileId, numOfScheduledPhases, scheduledPhases,          \
                                                   scheduledPhasesLen)                                                             \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENERGY_PHASES_SCHEDULE_STATE_REQUEST_COMMAND_ID, "uuub", powerProfileId, powerProfileId,     \
                                  numOfScheduledPhases, scheduledPhases, scheduledPhasesLen);

/** @brief Command description for GetPowerProfilePriceExtendedResponse
 *
 * Command: GetPowerProfilePriceExtendedResponse
 * @param powerProfileId INT8U
 * @param powerProfileId INT8U
 * @param currency INT16U
 * @param numOfScheduledPhases INT8U
 * @param price INT32U
 * @param scheduledPhases ScheduledPhase []
 * @param scheduledPhasesLen int
 * @param priceTrailingDigit INT8U
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterGetPowerProfilePriceExtendedResponse(powerProfileId, powerProfileId, currency, numOfScheduledPhases, price,      \
                                                       scheduledPhases, scheduledPhasesLen, priceTrailingDigit)                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_POWER_PROFILE_PRICE_EXTENDED_RESPONSE_COMMAND_ID, "uuuuubu", powerProfileId,             \
                                  powerProfileId, currency, numOfScheduledPhases, price, scheduledPhases, scheduledPhasesLen,      \
                                  priceTrailingDigit);

/** @brief Command description for PowerProfileScheduleConstraintsNotification
 *
 * Command: PowerProfileScheduleConstraintsNotification
 * @param powerProfileId INT8U
 * @param startAfter INT16U
 * @param stopBefore INT16U
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterPowerProfileScheduleConstraintsNotification(powerProfileId, startAfter, stopBefore) emberAfFillExternalBuffer(   \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_POWER_PROFILE_SCHEDULE_CONSTRAINTS_NOTIFICATION_COMMAND_ID, "uuu", powerProfileId, startAfter, stopBefore);

/** @brief Command description for PowerProfileScheduleConstraintsResponse
 *
 * Command: PowerProfileScheduleConstraintsResponse
 * @param powerProfileId INT8U
 * @param startAfter INT16U
 * @param stopBefore INT16U
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterPowerProfileScheduleConstraintsResponse(powerProfileId, startAfter, stopBefore) emberAfFillExternalBuffer(       \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_POWER_PROFILE_SCHEDULE_CONSTRAINTS_RESPONSE_COMMAND_ID, "uuu", powerProfileId, startAfter, stopBefore);

/** @brief Command description for GetPowerProfilePriceExtended
 *
 * Command: GetPowerProfilePriceExtended
 * @param options BITMAP8
 * @param powerProfileId INT8U
 * @param powerProfileStartTime INT16U
 */
#define emberAfFillCommandPower                                                                                                    \
    ProfileClusterGetPowerProfilePriceExtended(options, powerProfileId, powerProfileStartTime) emberAfFillExternalBuffer(          \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_GET_POWER_PROFILE_PRICE_EXTENDED_COMMAND_ID, "uuu", options, powerProfileId, powerProfileStartTime);

/** @brief Command description for ExecutionOfACommand
 *
 * Command: ExecutionOfACommand
 * @param commandId CommandIdentification
 * @param applianceStatus ApplianceStatus
 * @param remoteEnableFlagsAndDeviceStatus2 RemoteEnableFlagsAndDeviceStatus2
 * @param applianceStatus2 INT24U
 */
#define emberAfFillCommandAppliance                                                                                                \
    ControlClusterExecutionOfACommand(commandId, applianceStatus, remoteEnableFlagsAndDeviceStatus2, applianceStatus2)             \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_EXECUTION_OF_A_COMMAND_COMMAND_ID, "uuuu", commandId, applianceStatus,                       \
                                  remoteEnableFlagsAndDeviceStatus2, applianceStatus2);

/** @brief Command description for SignalState
 *
 * Command: SignalState
 * @param applianceStatus ApplianceStatus
 * @param remoteEnableFlagsAndDeviceStatus2 RemoteEnableFlagsAndDeviceStatus2
 * @param applianceStatus2 INT24U
 */
#define emberAfFillCommandAppliance                                                                                                \
    ControlClusterSignalState(applianceStatus, remoteEnableFlagsAndDeviceStatus2, applianceStatus2) emberAfFillExternalBuffer(     \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_SIGNAL_STATE_COMMAND_ID, "uuu", applianceStatus, remoteEnableFlagsAndDeviceStatus2, applianceStatus2);

/** @brief Command description for WriteFunctions
 *
 * Command: WriteFunctions
 * @param functionId INT16U
 * @param functionDataType ENUM8
 * @param functionData INT8U []
 * @param functionDataLen int
 */
#define emberAfFillCommandAppliance                                                                                                \
    ControlClusterWriteFunctions(functionId, functionDataType, functionData, functionDataLen) emberAfFillExternalBuffer(           \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_WRITE_FUNCTIONS_COMMAND_ID, "uub", functionId, functionDataType, functionData, functionDataLen);

/** @brief Command description for OverloadPauseResume
 *
 * Command: OverloadPauseResume
 */
#define emberAfFillCommandAppliance                                                                                                \
    ControlClusterOverloadPauseResume() emberAfFillExternalBuffer(mask,                                                            \
                                                                                                                                   \
                                                                  ZCL_OVERLOAD_PAUSE_RESUME_COMMAND_ID, "", );

/** @brief Command description for OverloadPause
 *
 * Command: OverloadPause
 */
#define emberAfFillCommandAppliance                                                                                                \
    ControlClusterOverloadPause() emberAfFillExternalBuffer(mask,                                                                  \
                                                                                                                                   \
                                                            ZCL_OVERLOAD_PAUSE_COMMAND_ID, "", );

/** @brief Command description for OverloadWarning
 *
 * Command: OverloadWarning
 * @param warningEvent WarningEvent
 */
#define emberAfFillCommandAppliance                                                                                                \
    ControlClusterOverloadWarning(warningEvent) emberAfFillExternalBuffer(mask,                                                    \
                                                                                                                                   \
                                                                          ZCL_OVERLOAD_WARNING_COMMAND_ID, "u", warningEvent);

/** @brief Command description for CheckIn
 *
 * Command: CheckIn
 * @param startFastPolling BOOLEAN
 * @param fastPollTimeout INT16U
 */
#define emberAfFillCommandPoll                                                                                                     \
    ControlClusterCheckIn(startFastPolling, fastPollTimeout)                                                                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_CHECK_IN_COMMAND_ID, "uu", startFastPolling, fastPollTimeout);

/** @brief Command description for FastPollStop
 *
 * Command: FastPollStop
 */
#define emberAfFillCommandPoll                                                                                                     \
    ControlClusterFastPollStop() emberAfFillExternalBuffer(mask,                                                                   \
                                                                                                                                   \
                                                           ZCL_FAST_POLL_STOP_COMMAND_ID, "", );

/** @brief Command description for SetLongPollInterval
 *
 * Command: SetLongPollInterval
 * @param newLongPollInterval INT32U
 */
#define emberAfFillCommandPoll                                                                                                     \
    ControlClusterSetLongPollInterval(newLongPollInterval)                                                                         \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_LONG_POLL_INTERVAL_COMMAND_ID, "u", newLongPollInterval);

/** @brief Command description for SetShortPollInterval
 *
 * Command: SetShortPollInterval
 * @param newShortPollInterval INT16U
 */
#define emberAfFillCommandPoll                                                                                                     \
    ControlClusterSetShortPollInterval(newShortPollInterval)                                                                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_SHORT_POLL_INTERVAL_COMMAND_ID, "u", newShortPollInterval);

/** @brief Command description for GpNotification
 *
 * Command: GpNotification
 * @param options GpNotificationOption
 * @param options GpNotificationResponseOption
 * @param gpdSrcId INT32U
 * @param gpdSrcId INT32U
 * @param gpdIeee IEEE_ADDRESS
 * @param gpdIeee IEEE_ADDRESS
 * @param gpdEndpoint INT8U
 * @param endpoint INT8U
 * @param gpdSecurityFrameCounter INT32U
 * @param gpdSecurityFrameCounter INT32U
 * @param gpdCommandId INT8U
 * @param gpdCommandPayload OCTET_STRING
 * @param gppShortAddress INT16U
 * @param gppDistance INT8U
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpNotification(options, options, gpdSrcId, gpdSrcId, gpdIeee, gpdIeee, gpdEndpoint, endpoint,                      \
                               gpdSecurityFrameCounter, gpdSecurityFrameCounter, gpdCommandId, gpdCommandPayload, gppShortAddress, \
                               gppDistance)                                                                                        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GP_NOTIFICATION_COMMAND_ID, "uuuuuuuuuuuuuu", options, options, gpdSrcId, gpdSrcId, gpdIeee, \
                                  gpdIeee, gpdEndpoint, endpoint, gpdSecurityFrameCounter, gpdSecurityFrameCounter, gpdCommandId,  \
                                  gpdCommandPayload, gppShortAddress, gppDistance);

/** @brief Command description for GpPairingSearch
 *
 * Command: GpPairingSearch
 * @param options GpPairingSearchOption
 * @param options GpPairingOption
 * @param gpdSrcId INT32U
 * @param gpdSrcId INT32U
 * @param gpdIeee IEEE_ADDRESS
 * @param gpdIeee IEEE_ADDRESS
 * @param endpoint INT8U
 * @param endpoint INT8U
 * @param sinkIeeeAddress IEEE_ADDRESS
 * @param sinkNwkAddress INT16U
 * @param sinkGroupId INT16U
 * @param deviceId GpDeviceId
 * @param gpdSecurityFrameCounter INT32U
 * @param gpdKey SECURITY_KEY
 * @param assignedAlias INT16U
 * @param groupcastRadius INT8U
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpPairingSearch(options, options, gpdSrcId, gpdSrcId, gpdIeee, gpdIeee, endpoint, endpoint, sinkIeeeAddress,       \
                                sinkNwkAddress, sinkGroupId, deviceId, gpdSecurityFrameCounter, gpdKey, assignedAlias,             \
                                groupcastRadius)                                                                                   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GP_PAIRING_SEARCH_COMMAND_ID, "uuuuuuuuuuuuuuuu", options, options, gpdSrcId, gpdSrcId,      \
                                  gpdIeee, gpdIeee, endpoint, endpoint, sinkIeeeAddress, sinkNwkAddress, sinkGroupId, deviceId,    \
                                  gpdSecurityFrameCounter, gpdKey, assignedAlias, groupcastRadius);

/** @brief Command description for GpProxyCommissioningMode
 *
 * Command: GpProxyCommissioningMode
 * @param options GpProxyCommissioningModeOption
 * @param commissioningWindow INT16U
 * @param channel INT8U
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpProxyCommissioningMode(options, commissioningWindow, channel)                                                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GP_PROXY_COMMISSIONING_MODE_COMMAND_ID, "uuu", options, commissioningWindow, channel);

/** @brief Command description for GpTunnelingStop
 *
 * Command: GpTunnelingStop
 * @param options GpTunnelingStopOption
 * @param gpdSrcId INT32U
 * @param gpdIeee IEEE_ADDRESS
 * @param endpoint INT8U
 * @param gpdSecurityFrameCounter INT32U
 * @param gppShortAddress INT16U
 * @param gppDistance INT8S
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpTunnelingStop(options, gpdSrcId, gpdIeee, endpoint, gpdSecurityFrameCounter, gppShortAddress, gppDistance)       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GP_TUNNELING_STOP_COMMAND_ID, "uuuuuuu", options, gpdSrcId, gpdIeee, endpoint,               \
                                  gpdSecurityFrameCounter, gppShortAddress, gppDistance);

/** @brief Command description for GpCommissioningNotification
 *
 * Command: GpCommissioningNotification
 * @param options GpCommissioningNotificationOption
 * @param gpdSrcId INT32U
 * @param gpdIeee IEEE_ADDRESS
 * @param endpoint INT8U
 * @param gpdSecurityFrameCounter INT32U
 * @param gpdCommandId INT8U
 * @param gpdCommandPayload OCTET_STRING
 * @param gppShortAddress INT16U
 * @param gppLink INT8U
 * @param mic INT32U
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpCommissioningNotification(options, gpdSrcId, gpdIeee, endpoint, gpdSecurityFrameCounter, gpdCommandId,           \
                                            gpdCommandPayload, gppShortAddress, gppLink, mic)                                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GP_COMMISSIONING_NOTIFICATION_COMMAND_ID, "uuuuuuuuuu", options, gpdSrcId, gpdIeee,          \
                                  endpoint, gpdSecurityFrameCounter, gpdCommandId, gpdCommandPayload, gppShortAddress, gppLink,    \
                                  mic);

/** @brief Command description for GpSinkCommissioningMode
 *
 * Command: GpSinkCommissioningMode
 * @param options GpSinkCommissioningModeOptions
 * @param gpmAddrForSecurity INT16U
 * @param gpmAddrForPairing INT16U
 * @param sinkEndpoint INT8U
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpSinkCommissioningMode(options, gpmAddrForSecurity, gpmAddrForPairing, sinkEndpoint) emberAfFillExternalBuffer(   \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_GP_SINK_COMMISSIONING_MODE_COMMAND_ID, "uuuu", options, gpmAddrForSecurity, gpmAddrForPairing, sinkEndpoint);

/** @brief Command description for GpResponse
 *
 * Command: GpResponse
 * @param options GpResponseOption
 * @param tempMasterShortAddress INT16U
 * @param tempMasterTxChannel BITMAP8
 * @param gpdSrcId INT32U
 * @param gpdIeee IEEE_ADDRESS
 * @param endpoint INT8U
 * @param gpdCommandId INT8U
 * @param gpdCommandPayload OCTET_STRING
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpResponse(options, tempMasterShortAddress, tempMasterTxChannel, gpdSrcId, gpdIeee, endpoint, gpdCommandId,        \
                           gpdCommandPayload)                                                                                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GP_RESPONSE_COMMAND_ID, "uuuuuuuu", options, tempMasterShortAddress, tempMasterTxChannel,    \
                                  gpdSrcId, gpdIeee, endpoint, gpdCommandId, gpdCommandPayload);

/** @brief Command description for GpTranslationTableUpdate
 *
 * Command: GpTranslationTableUpdate
 * @param options GpTranslationTableUpdateOption
 * @param gpdSrcId INT32U
 * @param gpdIeee IEEE_ADDRESS
 * @param endpoint INT8U
 * @param translations GpTranslationTableUpdateTranslation []
 * @param translationsLen int
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpTranslationTableUpdate(options, gpdSrcId, gpdIeee, endpoint, translations, translationsLen)                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GP_TRANSLATION_TABLE_UPDATE_COMMAND_ID, "uuuub", options, gpdSrcId, gpdIeee, endpoint,       \
                                  translations, translationsLen);

/** @brief Command description for GpTranslationTableRequest
 *
 * Command: GpTranslationTableRequest
 * @param startIndex INT8U
 * @param status GpTranslationTableResponseStatus
 * @param options GpTranslationTableResponseOption
 * @param totalNumberOfEntries INT8U
 * @param startIndex INT8U
 * @param entriesCount INT8U
 * @param translationTableList INT8U []
 * @param translationTableListLen int
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpTranslationTableRequest(startIndex, status, options, totalNumberOfEntries, startIndex, entriesCount,             \
                                          translationTableList, translationTableListLen)                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GP_TRANSLATION_TABLE_REQUEST_COMMAND_ID, "uuuuuub", startIndex, status, options,             \
                                  totalNumberOfEntries, startIndex, entriesCount, translationTableList, translationTableListLen);

/** @brief Command description for GpPairingConfiguration
 *
 * Command: GpPairingConfiguration
 * @param actions GpPairingConfigurationActions
 * @param options GpPairingConfigurationOption
 * @param gpdSrcId INT32U
 * @param gpdIeee IEEE_ADDRESS
 * @param endpoint INT8U
 * @param deviceId INT8U
 * @param groupListCount INT8U
 * @param groupList GpPairingConfigurationGroupList []
 * @param groupListLen int
 * @param gpdAssignedAlias INT16U
 * @param groupcastRadius INT8U
 * @param securityOptions INT8U
 * @param gpdSecurityFrameCounter INT32U
 * @param gpdSecurityKey SECURITY_KEY
 * @param numberOfPairedEndpoints INT8U
 * @param pairedEndpoints INT8U []
 * @param pairedEndpointsLen int
 * @param applicationInformation GpApplicationInformation
 * @param manufacturerId INT16U
 * @param modeId INT16U
 * @param numberOfGpdCommands INT8U
 * @param gpdCommandIdList INT8U []
 * @param gpdCommandIdListLen int
 * @param clusterIdListCount INT8U
 * @param clusterListServer INT16U []
 * @param clusterListServerLen int
 * @param clusterListClient INT16U []
 * @param clusterListClientLen int
 * @param switchInformationLength INT8U
 * @param switchConfiguration INT8U
 * @param currentContactStatus INT8U
 * @param totalNumberOfReports INT8U
 * @param numberOfReports INT8U
 * @param reportDescriptor INT8U []
 * @param reportDescriptorLen int
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpPairingConfiguration(                                                                                            \
        actions, options, gpdSrcId, gpdIeee, endpoint, deviceId, groupListCount, groupList, groupListLen, gpdAssignedAlias,        \
        groupcastRadius, securityOptions, gpdSecurityFrameCounter, gpdSecurityKey, numberOfPairedEndpoints, pairedEndpoints,       \
        pairedEndpointsLen, applicationInformation, manufacturerId, modeId, numberOfGpdCommands, gpdCommandIdList,                 \
        gpdCommandIdListLen, clusterIdListCount, clusterListServer, clusterListServerLen, clusterListClient, clusterListClientLen, \
        switchInformationLength, switchConfiguration, currentContactStatus, totalNumberOfReports, numberOfReports,                 \
        reportDescriptor, reportDescriptorLen)                                                                                     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GP_PAIRING_CONFIGURATION_COMMAND_ID, "uuuuuuubuuuuuubuuuububbuuuuub", actions, options,      \
                                  gpdSrcId, gpdIeee, endpoint, deviceId, groupListCount, groupList, groupListLen,                  \
                                  gpdAssignedAlias, groupcastRadius, securityOptions, gpdSecurityFrameCounter, gpdSecurityKey,     \
                                  numberOfPairedEndpoints, pairedEndpoints, pairedEndpointsLen, applicationInformation,            \
                                  manufacturerId, modeId, numberOfGpdCommands, gpdCommandIdList, gpdCommandIdListLen,              \
                                  clusterIdListCount, clusterListServer, clusterListServerLen, clusterListClient,                  \
                                  clusterListClientLen, switchInformationLength, switchConfiguration, currentContactStatus,        \
                                  totalNumberOfReports, numberOfReports, reportDescriptor, reportDescriptorLen);

/** @brief Command description for GpSinkTableRequest
 *
 * Command: GpSinkTableRequest
 * @param options GpSinkTableRequestOptions
 * @param status ENUM8
 * @param gpdSrcId INT32U
 * @param totalNumberofNonEmptySinkTableEntries INT8U
 * @param gpdIeee INT64U
 * @param startIndex INT8U
 * @param endpoint INT8U
 * @param sinkTableEntriesCount INT8U
 * @param index INT8U
 * @param sinkTableEntries INT8U []
 * @param sinkTableEntriesLen int
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpSinkTableRequest(options, status, gpdSrcId, totalNumberofNonEmptySinkTableEntries, gpdIeee, startIndex,          \
                                   endpoint, sinkTableEntriesCount, index, sinkTableEntries, sinkTableEntriesLen)                  \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GP_SINK_TABLE_REQUEST_COMMAND_ID, "uuuuuuuuub", options, status, gpdSrcId,                   \
                                  totalNumberofNonEmptySinkTableEntries, gpdIeee, startIndex, endpoint, sinkTableEntriesCount,     \
                                  index, sinkTableEntries, sinkTableEntriesLen);

/** @brief Command description for GpProxyTableResponse
 *
 * Command: GpProxyTableResponse
 * @param status GpProxyTableResponseStatus
 * @param options GpProxyTableRequestOptions
 * @param totalNumberOfNonEmptyProxyTableEntries INT8U
 * @param gpdSrcId INT32U
 * @param startIndex INT8U
 * @param gpdIeee INT64U
 * @param entriesCount INT8U
 * @param endpoint INT8U
 * @param proxyTableEntries INT8U []
 * @param proxyTableEntriesLen int
 * @param index INT8U
 */
#define emberAfFillCommandGreen                                                                                                    \
    PowerClusterGpProxyTableResponse(status, options, totalNumberOfNonEmptyProxyTableEntries, gpdSrcId, startIndex, gpdIeee,       \
                                     entriesCount, endpoint, proxyTableEntries, proxyTableEntriesLen, index)                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GP_PROXY_TABLE_RESPONSE_COMMAND_ID, "uuuuuuuubu", status, options,                           \
                                  totalNumberOfNonEmptyProxyTableEntries, gpdSrcId, startIndex, gpdIeee, entriesCount, endpoint,   \
                                  proxyTableEntries, proxyTableEntriesLen, index);

/** @brief Command description for LockDoor
 *
 * Command: LockDoor
 * @param PIN CHAR_STRING
 * @param status INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterLockDoor(PIN, status) emberAfFillExternalBuffer(mask,                                                               \
                                                                                                                                   \
                                                               ZCL_LOCK_DOOR_COMMAND_ID, "uu", PIN, status);

/** @brief Command description for UnlockDoor
 *
 * Command: UnlockDoor
 * @param PIN CHAR_STRING
 * @param status INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterUnlockDoor(PIN, status) emberAfFillExternalBuffer(mask,                                                             \
                                                                                                                                   \
                                                                 ZCL_UNLOCK_DOOR_COMMAND_ID, "uu", PIN, status);

/** @brief Command description for Toggle
 *
 * Command: Toggle
 * @param pin CHAR_STRING
 * @param status INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterToggle(pin, status) emberAfFillExternalBuffer(mask,                                                                 \
                                                                                                                                   \
                                                             ZCL_TOGGLE_COMMAND_ID, "uu", pin, status);

/** @brief Command description for UnlockWithTimeout
 *
 * Command: UnlockWithTimeout
 * @param timeoutInSeconds INT16U
 * @param status INT8U
 * @param pin CHAR_STRING
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterUnlockWithTimeout(timeoutInSeconds, status, pin)                                                                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_UNLOCK_WITH_TIMEOUT_COMMAND_ID, "uuu", timeoutInSeconds, status, pin);

/** @brief Command description for GetLogRecord
 *
 * Command: GetLogRecord
 * @param logIndex INT16U
 * @param logEntryId INT16U
 * @param timestamp INT32U
 * @param eventType ENUM8
 * @param source INT8U
 * @param eventIdOrAlarmCode INT8U
 * @param userId INT16U
 * @param pin CHAR_STRING
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterGetLogRecord(logIndex, logEntryId, timestamp, eventType, source, eventIdOrAlarmCode, userId, pin)                   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_LOG_RECORD_COMMAND_ID, "uuuuuuuu", logIndex, logEntryId, timestamp, eventType, source,   \
                                  eventIdOrAlarmCode, userId, pin);

/** @brief Command description for SetPin
 *
 * Command: SetPin
 * @param userId INT16U
 * @param status DoorLockSetPinOrIdStatus
 * @param userStatus DoorLockUserStatus
 * @param userType DoorLockUserType
 * @param pin CHAR_STRING
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterSetPin(userId, status, userStatus, userType, pin)                                                                   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_PIN_COMMAND_ID, "uuuuu", userId, status, userStatus, userType, pin);

/** @brief Command description for GetPin
 *
 * Command: GetPin
 * @param userId INT16U
 * @param userId INT16U
 * @param userStatus DoorLockUserStatus
 * @param userType DoorLockUserType
 * @param pin CHAR_STRING
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterGetPin(userId, userId, userStatus, userType, pin)                                                                   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_PIN_COMMAND_ID, "uuuuu", userId, userId, userStatus, userType, pin);

/** @brief Command description for ClearPin
 *
 * Command: ClearPin
 * @param userId INT16U
 * @param status INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterClearPin(userId, status) emberAfFillExternalBuffer(mask,                                                            \
                                                                                                                                   \
                                                                  ZCL_CLEAR_PIN_COMMAND_ID, "uu", userId, status);

/** @brief Command description for ClearAllPins
 *
 * Command: ClearAllPins
 * @param status INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterClearAllPins(status) emberAfFillExternalBuffer(mask,                                                                \
                                                                                                                                   \
                                                              ZCL_CLEAR_ALL_PINS_COMMAND_ID, "u", status);

/** @brief Command description for SetUserStatus
 *
 * Command: SetUserStatus
 * @param userId INT16U
 * @param status INT8U
 * @param userStatus INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterSetUserStatus(userId, status, userStatus)                                                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_USER_STATUS_COMMAND_ID, "uuu", userId, status, userStatus);

/** @brief Command description for GetUserStatus
 *
 * Command: GetUserStatus
 * @param userId INT16U
 * @param userId INT16U
 * @param status INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterGetUserStatus(userId, userId, status)                                                                               \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_USER_STATUS_COMMAND_ID, "uuu", userId, userId, status);

/** @brief Command description for SetWeekdaySchedule
 *
 * Command: SetWeekdaySchedule
 * @param scheduleId INT8U
 * @param status INT8U
 * @param userId INT16U
 * @param daysMask DoorLockDayOfWeek
 * @param startHour INT8U
 * @param startMinute INT8U
 * @param endHour INT8U
 * @param endMinute INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterSetWeekdaySchedule(scheduleId, status, userId, daysMask, startHour, startMinute, endHour, endMinute)                \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_WEEKDAY_SCHEDULE_COMMAND_ID, "uuuuuuuu", scheduleId, status, userId, daysMask,           \
                                  startHour, startMinute, endHour, endMinute);

/** @brief Command description for GetWeekdaySchedule
 *
 * Command: GetWeekdaySchedule
 * @param scheduleId INT8U
 * @param scheduleId INT8U
 * @param userId INT16U
 * @param userId INT16U
 * @param status INT8U
 * @param daysMask INT8U
 * @param startHour INT8U
 * @param startMinute INT8U
 * @param endHour INT8U
 * @param endMinute INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterGetWeekdaySchedule(scheduleId, scheduleId, userId, userId, status, daysMask, startHour, startMinute, endHour,       \
                                  endMinute)                                                                                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_WEEKDAY_SCHEDULE_COMMAND_ID, "uuuuuuuuuu", scheduleId, scheduleId, userId, userId,       \
                                  status, daysMask, startHour, startMinute, endHour, endMinute);

/** @brief Command description for ClearWeekdaySchedule
 *
 * Command: ClearWeekdaySchedule
 * @param scheduleId INT8U
 * @param status INT8U
 * @param userId INT16U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterClearWeekdaySchedule(scheduleId, status, userId)                                                                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_CLEAR_WEEKDAY_SCHEDULE_COMMAND_ID, "uuu", scheduleId, status, userId);

/** @brief Command description for SetYeardaySchedule
 *
 * Command: SetYeardaySchedule
 * @param scheduleId INT8U
 * @param status INT8U
 * @param userId INT16U
 * @param localStartTime INT32U
 * @param localEndTime INT32U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterSetYeardaySchedule(scheduleId, status, userId, localStartTime, localEndTime) emberAfFillExternalBuffer(             \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_SET_YEARDAY_SCHEDULE_COMMAND_ID, "uuuuu", scheduleId, status, userId, localStartTime, localEndTime);

/** @brief Command description for GetYeardaySchedule
 *
 * Command: GetYeardaySchedule
 * @param scheduleId INT8U
 * @param scheduleId INT8U
 * @param userId INT16U
 * @param userId INT16U
 * @param status INT8U
 * @param localStartTime INT32U
 * @param localEndTime INT32U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterGetYeardaySchedule(scheduleId, scheduleId, userId, userId, status, localStartTime, localEndTime)                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_YEARDAY_SCHEDULE_COMMAND_ID, "uuuuuuu", scheduleId, scheduleId, userId, userId, status,  \
                                  localStartTime, localEndTime);

/** @brief Command description for ClearYeardaySchedule
 *
 * Command: ClearYeardaySchedule
 * @param scheduleId INT8U
 * @param status INT8U
 * @param userId INT16U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterClearYeardaySchedule(scheduleId, status, userId)                                                                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_CLEAR_YEARDAY_SCHEDULE_COMMAND_ID, "uuu", scheduleId, status, userId);

/** @brief Command description for SetHolidaySchedule
 *
 * Command: SetHolidaySchedule
 * @param scheduleId INT8U
 * @param status INT8U
 * @param localStartTime INT32U
 * @param localEndTime INT32U
 * @param operatingModeDuringHoliday ENUM8
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterSetHolidaySchedule(scheduleId, status, localStartTime, localEndTime, operatingModeDuringHoliday)                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_HOLIDAY_SCHEDULE_COMMAND_ID, "uuuuu", scheduleId, status, localStartTime, localEndTime,  \
                                  operatingModeDuringHoliday);

/** @brief Command description for GetHolidaySchedule
 *
 * Command: GetHolidaySchedule
 * @param scheduleId INT8U
 * @param scheduleId INT8U
 * @param status INT8U
 * @param localStartTime INT32U
 * @param localEndTime INT32U
 * @param operatingModeDuringHoliday ENUM8
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterGetHolidaySchedule(scheduleId, scheduleId, status, localStartTime, localEndTime, operatingModeDuringHoliday)        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_HOLIDAY_SCHEDULE_COMMAND_ID, "uuuuuu", scheduleId, scheduleId, status, localStartTime,   \
                                  localEndTime, operatingModeDuringHoliday);

/** @brief Command description for ClearHolidaySchedule
 *
 * Command: ClearHolidaySchedule
 * @param scheduleId INT8U
 * @param status INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterClearHolidaySchedule(scheduleId, status)                                                                            \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_CLEAR_HOLIDAY_SCHEDULE_COMMAND_ID, "uu", scheduleId, status);

/** @brief Command description for SetUserType
 *
 * Command: SetUserType
 * @param userId INT16U
 * @param status INT8U
 * @param userType DoorLockUserType
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterSetUserType(userId, status, userType)                                                                               \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_USER_TYPE_COMMAND_ID, "uuu", userId, status, userType);

/** @brief Command description for GetUserType
 *
 * Command: GetUserType
 * @param userId INT16U
 * @param userId INT16U
 * @param userType DoorLockUserType
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterGetUserType(userId, userId, userType)                                                                               \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_USER_TYPE_COMMAND_ID, "uuu", userId, userId, userType);

/** @brief Command description for SetRfid
 *
 * Command: SetRfid
 * @param userId INT16U
 * @param status DoorLockSetPinOrIdStatus
 * @param userStatus DoorLockUserStatus
 * @param userType DoorLockUserType
 * @param id CHAR_STRING
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterSetRfid(userId, status, userStatus, userType, id)                                                                   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_RFID_COMMAND_ID, "uuuuu", userId, status, userStatus, userType, id);

/** @brief Command description for GetRfid
 *
 * Command: GetRfid
 * @param userId INT16U
 * @param userId INT16U
 * @param userStatus DoorLockUserStatus
 * @param userType DoorLockUserType
 * @param rfid CHAR_STRING
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterGetRfid(userId, userId, userStatus, userType, rfid)                                                                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_RFID_COMMAND_ID, "uuuuu", userId, userId, userStatus, userType, rfid);

/** @brief Command description for ClearRfid
 *
 * Command: ClearRfid
 * @param userId INT16U
 * @param status INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterClearRfid(userId, status) emberAfFillExternalBuffer(mask,                                                           \
                                                                                                                                   \
                                                                   ZCL_CLEAR_RFID_COMMAND_ID, "uu", userId, status);

/** @brief Command description for ClearAllRfids
 *
 * Command: ClearAllRfids
 * @param status INT8U
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterClearAllRfids(status) emberAfFillExternalBuffer(mask,                                                               \
                                                                                                                                   \
                                                               ZCL_CLEAR_ALL_RFIDS_COMMAND_ID, "u", status);

/** @brief Command description for OperationEventNotification
 *
 * Command: OperationEventNotification
 * @param source INT8U
 * @param eventCode DoorLockOperationEventCode
 * @param userId INT16U
 * @param pin CHAR_STRING
 * @param timeStamp INT32U
 * @param data CHAR_STRING
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterOperationEventNotification(source, eventCode, userId, pin, timeStamp, data) emberAfFillExternalBuffer(              \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_OPERATION_EVENT_NOTIFICATION_COMMAND_ID, "uuuuuu", source, eventCode, userId, pin, timeStamp, data);

/** @brief Command description for ProgrammingEventNotification
 *
 * Command: ProgrammingEventNotification
 * @param source INT8U
 * @param eventCode DoorLockProgrammingEventCode
 * @param userId INT16U
 * @param pin CHAR_STRING
 * @param userType DoorLockUserType
 * @param userStatus DoorLockUserStatus
 * @param timeStamp INT32U
 * @param data CHAR_STRING
 */
#define emberAfFillCommandDoor                                                                                                     \
    LockClusterProgrammingEventNotification(source, eventCode, userId, pin, userType, userStatus, timeStamp, data)                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_PROGRAMMING_EVENT_NOTIFICATION_COMMAND_ID, "uuuuuuuu", source, eventCode, userId, pin,       \
                                  userType, userStatus, timeStamp, data);

/** @brief Command description for WindowCoveringUpOpen
 *
 * Command: WindowCoveringUpOpen
 */
#define emberAfFillCommandWindow                                                                                                   \
    CoveringClusterWindowCoveringUpOpen() emberAfFillExternalBuffer(mask,                                                          \
                                                                                                                                   \
                                                                    ZCL_WINDOW_COVERING_UP_OPEN_COMMAND_ID, "", );

/** @brief Command description for WindowCoveringDownClose
 *
 * Command: WindowCoveringDownClose
 */
#define emberAfFillCommandWindow                                                                                                   \
    CoveringClusterWindowCoveringDownClose() emberAfFillExternalBuffer(mask,                                                       \
                                                                                                                                   \
                                                                       ZCL_WINDOW_COVERING_DOWN_CLOSE_COMMAND_ID, "", );

/** @brief Command description for WindowCoveringStop
 *
 * Command: WindowCoveringStop
 */
#define emberAfFillCommandWindow                                                                                                   \
    CoveringClusterWindowCoveringStop() emberAfFillExternalBuffer(mask,                                                            \
                                                                                                                                   \
                                                                  ZCL_WINDOW_COVERING_STOP_COMMAND_ID, "", );

/** @brief Command description for WindowCoveringGoToLiftValue
 *
 * Command: WindowCoveringGoToLiftValue
 * @param liftValue INT16U
 */
#define emberAfFillCommandWindow                                                                                                   \
    CoveringClusterWindowCoveringGoToLiftValue(liftValue)                                                                          \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_WINDOW_COVERING_GO_TO_LIFT_VALUE_COMMAND_ID, "u", liftValue);

/** @brief Command description for WindowCoveringGoToLiftPercentage
 *
 * Command: WindowCoveringGoToLiftPercentage
 * @param percentageLiftValue INT8U
 */
#define emberAfFillCommandWindow                                                                                                   \
    CoveringClusterWindowCoveringGoToLiftPercentage(percentageLiftValue)                                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_WINDOW_COVERING_GO_TO_LIFT_PERCENTAGE_COMMAND_ID, "u", percentageLiftValue);

/** @brief Command description for WindowCoveringGoToTiltValue
 *
 * Command: WindowCoveringGoToTiltValue
 * @param tiltValue INT16U
 */
#define emberAfFillCommandWindow                                                                                                   \
    CoveringClusterWindowCoveringGoToTiltValue(tiltValue)                                                                          \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_WINDOW_COVERING_GO_TO_TILT_VALUE_COMMAND_ID, "u", tiltValue);

/** @brief Command description for WindowCoveringGoToTiltPercentage
 *
 * Command: WindowCoveringGoToTiltPercentage
 * @param percentageTiltValue INT8U
 */
#define emberAfFillCommandWindow                                                                                                   \
    CoveringClusterWindowCoveringGoToTiltPercentage(percentageTiltValue)                                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_WINDOW_COVERING_GO_TO_TILT_PERCENTAGE_COMMAND_ID, "u", percentageTiltValue);

/** @brief Command description for BarrierControlGoToPercent
 *
 * Command: BarrierControlGoToPercent
 * @param percentOpen INT8U
 */
#define emberAfFillCommandBarrier                                                                                                  \
    ControlClusterBarrierControlGoToPercent(percentOpen)                                                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_BARRIER_CONTROL_GO_TO_PERCENT_COMMAND_ID, "u", percentOpen);

/** @brief Command description for BarrierControlStop
 *
 * Command: BarrierControlStop
 */
#define emberAfFillCommandBarrier                                                                                                  \
    ControlClusterBarrierControlStop() emberAfFillExternalBuffer(mask,                                                             \
                                                                                                                                   \
                                                                 ZCL_BARRIER_CONTROL_STOP_COMMAND_ID, "", );

/** @brief Command description for SetpointRaiseLower
 *
 * Command: SetpointRaiseLower
 * @param mode SetpointAdjustMode
 * @param numberOfTransitionsForSequence ENUM8
 * @param amount INT8S
 * @param dayOfWeekForSequence DayOfWeek
 * @param modeForSequence ModeForSequence
 * @param payload INT8U []
 * @param payloadLen int
 */
#define emberAfFillCommandThermostatClusterSetpointRaiseLower(mode, numberOfTransitionsForSequence, amount, dayOfWeekForSequence,  \
                                                              modeForSequence, payload, payloadLen)                                \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SETPOINT_RAISE_LOWER_COMMAND_ID, "uuuuub", mode, numberOfTransitionsForSequence, amount,         \
                              dayOfWeekForSequence, modeForSequence, payload, payloadLen);

/** @brief Command description for SetWeeklySchedule
 *
 * Command: SetWeeklySchedule
 * @param numberOfTransitionsForSequence ENUM8
 * @param timeOfDay INT16U
 * @param dayOfWeekForSequence DayOfWeek
 * @param relayStatus BITMAP16
 * @param modeForSequence ModeForSequence
 * @param localTemperature INT16S
 * @param payload INT8U []
 * @param payloadLen int
 * @param humidityInPercentage INT8U
 * @param setpoint INT16S
 * @param unreadEntries INT16U
 */
#define emberAfFillCommandThermostatClusterSetWeeklySchedule(numberOfTransitionsForSequence, timeOfDay, dayOfWeekForSequence,      \
                                                             relayStatus, modeForSequence, localTemperature, payload, payloadLen,  \
                                                             humidityInPercentage, setpoint, unreadEntries)                        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SET_WEEKLY_SCHEDULE_COMMAND_ID, "uuuuuubuuu", numberOfTransitionsForSequence, timeOfDay,         \
                              dayOfWeekForSequence, relayStatus, modeForSequence, localTemperature, payload, payloadLen,           \
                              humidityInPercentage, setpoint, unreadEntries);

/** @brief Command description for GetWeeklySchedule
 *
 * Command: GetWeeklySchedule
 * @param daysToReturn DayOfWeek
 * @param modeToReturn ModeForSequence
 */
#define emberAfFillCommandThermostatClusterGetWeeklySchedule(daysToReturn, modeToReturn)                                           \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_WEEKLY_SCHEDULE_COMMAND_ID, "uu", daysToReturn, modeToReturn);

/** @brief Command description for ClearWeeklySchedule
 *
 * Command: ClearWeeklySchedule
 */
#define emberAfFillCommandThermostatClusterClearWeeklySchedule()                                                                   \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CLEAR_WEEKLY_SCHEDULE_COMMAND_ID, "", );

/** @brief Command description for GetRelayStatusLog
 *
 * Command: GetRelayStatusLog
 */
#define emberAfFillCommandThermostatClusterGetRelayStatusLog()                                                                     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_RELAY_STATUS_LOG_COMMAND_ID, "", );

/** @brief Command description for MoveToHue
 *
 * Command: MoveToHue
 * @param hue INT8U
 * @param direction HueDirection
 * @param transitionTime INT16U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterMoveToHue(hue, direction, transitionTime, optionsMask, optionsOverride) emberAfFillExternalBuffer(               \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_MOVE_TO_HUE_COMMAND_ID, "uuuuu", hue, direction, transitionTime, optionsMask, optionsOverride);

/** @brief Command description for MoveHue
 *
 * Command: MoveHue
 * @param moveMode HueMoveMode
 * @param rate INT8U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterMoveHue(moveMode, rate, optionsMask, optionsOverride)                                                            \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_MOVE_HUE_COMMAND_ID, "uuuu", moveMode, rate, optionsMask, optionsOverride);

/** @brief Command description for StepHue
 *
 * Command: StepHue
 * @param stepMode HueStepMode
 * @param stepSize INT8U
 * @param transitionTime INT8U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterStepHue(stepMode, stepSize, transitionTime, optionsMask, optionsOverride) emberAfFillExternalBuffer(             \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_STEP_HUE_COMMAND_ID, "uuuuu", stepMode, stepSize, transitionTime, optionsMask, optionsOverride);

/** @brief Command description for MoveToSaturation
 *
 * Command: MoveToSaturation
 * @param saturation INT8U
 * @param transitionTime INT16U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterMoveToSaturation(saturation, transitionTime, optionsMask, optionsOverride) emberAfFillExternalBuffer(            \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_MOVE_TO_SATURATION_COMMAND_ID, "uuuu", saturation, transitionTime, optionsMask, optionsOverride);

/** @brief Command description for MoveSaturation
 *
 * Command: MoveSaturation
 * @param moveMode SaturationMoveMode
 * @param rate INT8U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterMoveSaturation(moveMode, rate, optionsMask, optionsOverride)                                                     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_MOVE_SATURATION_COMMAND_ID, "uuuu", moveMode, rate, optionsMask, optionsOverride);

/** @brief Command description for StepSaturation
 *
 * Command: StepSaturation
 * @param stepMode SaturationStepMode
 * @param stepSize INT8U
 * @param transitionTime INT8U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterStepSaturation(stepMode, stepSize, transitionTime, optionsMask, optionsOverride) emberAfFillExternalBuffer(      \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_STEP_SATURATION_COMMAND_ID, "uuuuu", stepMode, stepSize, transitionTime, optionsMask, optionsOverride);

/** @brief Command description for MoveToHueAndSaturation
 *
 * Command: MoveToHueAndSaturation
 * @param hue INT8U
 * @param saturation INT8U
 * @param transitionTime INT16U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterMoveToHueAndSaturation(hue, saturation, transitionTime, optionsMask, optionsOverride) emberAfFillExternalBuffer( \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_MOVE_TO_HUE_AND_SATURATION_COMMAND_ID, "uuuuu", hue, saturation, transitionTime, optionsMask, optionsOverride);

/** @brief Command description for MoveToColor
 *
 * Command: MoveToColor
 * @param colorX INT16U
 * @param colorY INT16U
 * @param transitionTime INT16U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterMoveToColor(colorX, colorY, transitionTime, optionsMask, optionsOverride) emberAfFillExternalBuffer(             \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_MOVE_TO_COLOR_COMMAND_ID, "uuuuu", colorX, colorY, transitionTime, optionsMask, optionsOverride);

/** @brief Command description for MoveColor
 *
 * Command: MoveColor
 * @param rateX INT16S
 * @param rateY INT16S
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterMoveColor(rateX, rateY, optionsMask, optionsOverride)                                                            \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_MOVE_COLOR_COMMAND_ID, "uuuu", rateX, rateY, optionsMask, optionsOverride);

/** @brief Command description for StepColor
 *
 * Command: StepColor
 * @param stepX INT16S
 * @param stepY INT16S
 * @param transitionTime INT16U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterStepColor(stepX, stepY, transitionTime, optionsMask, optionsOverride)                                            \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_STEP_COLOR_COMMAND_ID, "uuuuu", stepX, stepY, transitionTime, optionsMask, optionsOverride);

/** @brief Command description for MoveToColorTemperature
 *
 * Command: MoveToColorTemperature
 * @param colorTemperature INT16U
 * @param transitionTime INT16U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterMoveToColorTemperature(colorTemperature, transitionTime, optionsMask, optionsOverride)                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_MOVE_TO_COLOR_TEMPERATURE_COMMAND_ID, "uuuu", colorTemperature, transitionTime, optionsMask, \
                                  optionsOverride);

/** @brief Command description for EnhancedMoveToHue
 *
 * Command: EnhancedMoveToHue
 * @param enhancedHue INT16U
 * @param direction HueDirection
 * @param transitionTime INT16U
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterEnhancedMoveToHue(enhancedHue, direction, transitionTime)                                                        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENHANCED_MOVE_TO_HUE_COMMAND_ID, "uuu", enhancedHue, direction, transitionTime);

/** @brief Command description for EnhancedMoveHue
 *
 * Command: EnhancedMoveHue
 * @param moveMode HueMoveMode
 * @param rate INT16U
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterEnhancedMoveHue(moveMode, rate)                                                                                  \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENHANCED_MOVE_HUE_COMMAND_ID, "uu", moveMode, rate);

/** @brief Command description for EnhancedStepHue
 *
 * Command: EnhancedStepHue
 * @param stepMode HueStepMode
 * @param stepSize INT16U
 * @param transitionTime INT16U
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterEnhancedStepHue(stepMode, stepSize, transitionTime)                                                              \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENHANCED_STEP_HUE_COMMAND_ID, "uuu", stepMode, stepSize, transitionTime);

/** @brief Command description for EnhancedMoveToHueAndSaturation
 *
 * Command: EnhancedMoveToHueAndSaturation
 * @param enhancedHue INT16U
 * @param saturation INT8U
 * @param transitionTime INT16U
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterEnhancedMoveToHueAndSaturation(enhancedHue, saturation, transitionTime) emberAfFillExternalBuffer(               \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_ENHANCED_MOVE_TO_HUE_AND_SATURATION_COMMAND_ID, "uuu", enhancedHue, saturation, transitionTime);

/** @brief Command description for ColorLoopSet
 *
 * Command: ColorLoopSet
 * @param updateFlags ColorLoopUpdateFlags
 * @param action ColorLoopAction
 * @param direction ColorLoopDirection
 * @param time INT16U
 * @param startHue INT16U
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterColorLoopSet(updateFlags, action, direction, time, startHue)                                                     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_COLOR_LOOP_SET_COMMAND_ID, "uuuuu", updateFlags, action, direction, time, startHue);

/** @brief Command description for StopMoveStep
 *
 * Command: StopMoveStep
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterStopMoveStep(optionsMask, optionsOverride)                                                                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_STOP_MOVE_STEP_COMMAND_ID, "uu", optionsMask, optionsOverride);

/** @brief Command description for MoveColorTemperature
 *
 * Command: MoveColorTemperature
 * @param moveMode HueMoveMode
 * @param rate INT16U
 * @param colorTemperatureMinimum INT16U
 * @param colorTemperatureMaximum INT16U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterMoveColorTemperature(moveMode, rate, colorTemperatureMinimum, colorTemperatureMaximum, optionsMask,              \
                                       optionsOverride)                                                                            \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_MOVE_COLOR_TEMPERATURE_COMMAND_ID, "uuuuuu", moveMode, rate, colorTemperatureMinimum,        \
                                  colorTemperatureMaximum, optionsMask, optionsOverride);

/** @brief Command description for StepColorTemperature
 *
 * Command: StepColorTemperature
 * @param stepMode HueStepMode
 * @param stepSize INT16U
 * @param transitionTime INT16U
 * @param colorTemperatureMinimum INT16U
 * @param colorTemperatureMaximum INT16U
 * @param optionsMask BITMAP8
 * @param optionsOverride BITMAP8
 */
#define emberAfFillCommandColor                                                                                                    \
    ControlClusterStepColorTemperature(stepMode, stepSize, transitionTime, colorTemperatureMinimum, colorTemperatureMaximum,       \
                                       optionsMask, optionsOverride)                                                               \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_STEP_COLOR_TEMPERATURE_COMMAND_ID, "uuuuuuu", stepMode, stepSize, transitionTime,            \
                                  colorTemperatureMinimum, colorTemperatureMaximum, optionsMask, optionsOverride);

/** @brief Command description for ZoneEnrollResponse
 *
 * Command: ZoneEnrollResponse
 * @param enrollResponseCode IasEnrollResponseCode
 * @param zoneStatus IasZoneStatus
 * @param zoneId INT8U
 * @param extendedStatus BITMAP8
 * @param zoneId INT8U
 * @param delay INT16U
 */
#define emberAfFillCommandIAS                                                                                                      \
    ZoneClusterZoneEnrollResponse(enrollResponseCode, zoneStatus, zoneId, extendedStatus, zoneId, delay)                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ZONE_ENROLL_RESPONSE_COMMAND_ID, "uuuuuu", enrollResponseCode, zoneStatus, zoneId,           \
                                  extendedStatus, zoneId, delay);

/** @brief Command description for InitiateNormalOperationMode
 *
 * Command: InitiateNormalOperationMode
 * @param zoneType IasZoneType
 * @param manufacturerCode INT16U
 */
#define emberAfFillCommandIAS                                                                                                      \
    ZoneClusterInitiateNormalOperationMode(zoneType, manufacturerCode)                                                             \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_INITIATE_NORMAL_OPERATION_MODE_COMMAND_ID, "uu", zoneType, manufacturerCode);

/** @brief Command description for InitiateNormalOperationModeResponse
 *
 * Command: InitiateNormalOperationModeResponse
 * @param testModeDuration INT8U
 * @param currentZoneSensitivityLevel INT8U
 */
#define emberAfFillCommandIAS                                                                                                      \
    ZoneClusterInitiateNormalOperationModeResponse(testModeDuration, currentZoneSensitivityLevel) emberAfFillExternalBuffer(       \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_INITIATE_NORMAL_OPERATION_MODE_RESPONSE_COMMAND_ID, "uu", testModeDuration, currentZoneSensitivityLevel);

/** @brief Command description for InitiateTestModeResponse
 *
 * Command: InitiateTestModeResponse
 */
#define emberAfFillCommandIAS                                                                                                      \
    ZoneClusterInitiateTestModeResponse() emberAfFillExternalBuffer(mask,                                                          \
                                                                                                                                   \
                                                                    ZCL_INITIATE_TEST_MODE_RESPONSE_COMMAND_ID, "", );

/** @brief Command description for Arm
 *
 * Command: Arm
 * @param armMode IasAceArmMode
 * @param armNotification IasAceArmNotification
 * @param armDisarmCode CHAR_STRING
 * @param zoneId INT8U
 */
#define emberAfFillCommandIAS                                                                                                      \
    ACEClusterArm(armMode, armNotification, armDisarmCode, zoneId)                                                                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ARM_COMMAND_ID, "uuuu", armMode, armNotification, armDisarmCode, zoneId);

/** @brief Command description for Bypass
 *
 * Command: Bypass
 * @param numberOfZones INT8U
 * @param section0 BITMAP16
 * @param zoneIds INT8U []
 * @param zoneIdsLen int
 * @param section1 BITMAP16
 * @param armDisarmCode CHAR_STRING
 * @param section2 BITMAP16
 * @param section3 BITMAP16
 * @param section4 BITMAP16
 * @param section5 BITMAP16
 * @param section6 BITMAP16
 * @param section7 BITMAP16
 * @param section8 BITMAP16
 * @param section9 BITMAP16
 * @param section10 BITMAP16
 * @param section11 BITMAP16
 * @param section12 BITMAP16
 * @param section13 BITMAP16
 * @param section14 BITMAP16
 * @param section15 BITMAP16
 */
#define emberAfFillCommandIAS                                                                                                      \
    ACEClusterBypass(numberOfZones, section0, zoneIds, zoneIdsLen, section1, armDisarmCode, section2, section3, section4,          \
                     section5, section6, section7, section8, section9, section10, section11, section12, section13, section14,      \
                     section15)                                                                                                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_BYPASS_COMMAND_ID, "uubuuuuuuuuuuuuuuuu", numberOfZones, section0, zoneIds, zoneIdsLen,      \
                                  section1, armDisarmCode, section2, section3, section4, section5, section6, section7, section8,   \
                                  section9, section10, section11, section12, section13, section14, section15);

/** @brief Command description for Emergency
 *
 * Command: Emergency
 * @param zoneId INT8U
 * @param zoneType IasZoneType
 * @param ieeeAddress IEEE_ADDRESS
 * @param zoneLabel CHAR_STRING
 */
#define emberAfFillCommandIAS                                                                                                      \
    ACEClusterEmergency(zoneId, zoneType, ieeeAddress, zoneLabel)                                                                  \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_EMERGENCY_COMMAND_ID, "uuuu", zoneId, zoneType, ieeeAddress, zoneLabel);

/** @brief Command description for Fire
 *
 * Command: Fire
 * @param zoneId INT8U
 * @param zoneStatus ENUM16
 * @param audibleNotification IasAceAudibleNotification
 * @param zoneLabel CHAR_STRING
 */
#define emberAfFillCommandIAS                                                                                                      \
    ACEClusterFire(zoneId, zoneStatus, audibleNotification, zoneLabel)                                                             \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_FIRE_COMMAND_ID, "uuuu", zoneId, zoneStatus, audibleNotification, zoneLabel);

/** @brief Command description for Panic
 *
 * Command: Panic
 * @param panelStatus IasAcePanelStatus
 * @param secondsRemaining INT8U
 * @param audibleNotification IasAceAudibleNotification
 * @param alarmStatus IasAceAlarmStatus
 */
#define emberAfFillCommandIAS                                                                                                      \
    ACEClusterPanic(panelStatus, secondsRemaining, audibleNotification, alarmStatus)                                               \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_PANIC_COMMAND_ID, "uuuu", panelStatus, secondsRemaining, audibleNotification, alarmStatus);

/** @brief Command description for GetZoneIdMap
 *
 * Command: GetZoneIdMap
 * @param panelStatus IasAcePanelStatus
 * @param secondsRemaining INT8U
 * @param audibleNotification IasAceAudibleNotification
 * @param alarmStatus IasAceAlarmStatus
 */
#define emberAfFillCommandIAS                                                                                                      \
    ACEClusterGetZoneIdMap(panelStatus, secondsRemaining, audibleNotification, alarmStatus) emberAfFillExternalBuffer(             \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_GET_ZONE_ID_MAP_COMMAND_ID, "uuuu", panelStatus, secondsRemaining, audibleNotification, alarmStatus);

/** @brief Command description for GetZoneInformation
 *
 * Command: GetZoneInformation
 * @param zoneId INT8U
 * @param numberOfZones INT8U
 * @param zoneIds INT8U []
 * @param zoneIdsLen int
 */
#define emberAfFillCommandIAS                                                                                                      \
    ACEClusterGetZoneInformation(zoneId, numberOfZones, zoneIds, zoneIdsLen)                                                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_ZONE_INFORMATION_COMMAND_ID, "uub", zoneId, numberOfZones, zoneIds, zoneIdsLen);

/** @brief Command description for GetPanelStatus
 *
 * Command: GetPanelStatus
 * @param numberOfZones INT8U
 * @param bypassResult IasAceBypassResult []
 * @param bypassResultLen int
 */
#define emberAfFillCommandIAS                                                                                                      \
    ACEClusterGetPanelStatus(numberOfZones, bypassResult, bypassResultLen)                                                         \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_PANEL_STATUS_COMMAND_ID, "ub", numberOfZones, bypassResult, bypassResultLen);

/** @brief Command description for GetBypassedZoneList
 *
 * Command: GetBypassedZoneList
 * @param zoneStatusComplete BOOLEAN
 * @param numberOfZones INT8U
 * @param zoneStatusResult IasAceZoneStatusResult []
 * @param zoneStatusResultLen int
 */
#define emberAfFillCommandIAS                                                                                                      \
    ACEClusterGetBypassedZoneList(zoneStatusComplete, numberOfZones, zoneStatusResult, zoneStatusResultLen)                        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_BYPASSED_ZONE_LIST_COMMAND_ID, "uub", zoneStatusComplete, numberOfZones,                 \
                                  zoneStatusResult, zoneStatusResultLen);

/** @brief Command description for GetZoneStatus
 *
 * Command: GetZoneStatus
 * @param startingZoneId INT8U
 * @param maxNumberOfZoneIds INT8U
 * @param zoneStatusMaskFlag BOOLEAN
 * @param zoneStatusMask BITMAP16
 */
#define emberAfFillCommandIAS                                                                                                      \
    ACEClusterGetZoneStatus(startingZoneId, maxNumberOfZoneIds, zoneStatusMaskFlag, zoneStatusMask) emberAfFillExternalBuffer(     \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_GET_ZONE_STATUS_COMMAND_ID, "uuuu", startingZoneId, maxNumberOfZoneIds, zoneStatusMaskFlag, zoneStatusMask);

/** @brief Command description for StartWarning
 *
 * Command: StartWarning
 * @param warningInfo WarningInfo
 * @param warningDuration INT16U
 * @param strobeDutyCycle INT8U
 * @param strobeLevel ENUM8
 */
#define emberAfFillCommandIAS                                                                                                      \
    WDClusterStartWarning(warningInfo, warningDuration, strobeDutyCycle, strobeLevel) emberAfFillExternalBuffer(                   \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_START_WARNING_COMMAND_ID, "uuuu", warningInfo, warningDuration, strobeDutyCycle, strobeLevel);

/** @brief Command description for Squawk
 *
 * Command: Squawk
 * @param squawkInfo SquawkInfo
 */
#define emberAfFillCommandIAS                                                                                                      \
    WDClusterSquawk(squawkInfo) emberAfFillExternalBuffer(mask,                                                                    \
                                                                                                                                   \
                                                          ZCL_SQUAWK_COMMAND_ID, "u", squawkInfo);

/** @brief Command description for MatchProtocolAddress
 *
 * Command: MatchProtocolAddress
 * @param protocolAddress OCTET_STRING
 * @param deviceIeeeAddress IEEE_ADDRESS
 * @param protocolAddress OCTET_STRING
 */
#define emberAfFillCommandGeneric                                                                                                  \
    TunnelClusterMatchProtocolAddress(protocolAddress, deviceIeeeAddress, protocolAddress) emberAfFillExternalBuffer(              \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_MATCH_PROTOCOL_ADDRESS_COMMAND_ID, "uuu", protocolAddress, deviceIeeeAddress, protocolAddress);

/** @brief Command description for AdvertiseProtocolAddress
 *
 * Command: AdvertiseProtocolAddress
 * @param protocolAddress OCTET_STRING
 */
#define emberAfFillCommandGeneric                                                                                                  \
    TunnelClusterAdvertiseProtocolAddress(protocolAddress)                                                                         \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ADVERTISE_PROTOCOL_ADDRESS_COMMAND_ID, "u", protocolAddress);

/** @brief Command description for TransferNpdu
 *
 * Command: TransferNpdu
 * @param npdu DATA8 []
 * @param npduLen int
 */
#define emberAfFillCommandBACnet                                                                                                   \
    Protocol TunnelClusterTransferNpdu(npdu, npduLen) emberAfFillExternalBuffer(mask,                                              \
                                                                                                                                   \
                                                                                ZCL_TRANSFER_NPDU_COMMAND_ID, "b", npdu, npduLen);

/** @brief Command description for TransferAPDU
 *
 * Command: TransferAPDU
 * @param apdu OCTET_STRING
 */
#define emberAfFillCommand11073                                                                                                    \
    Protocol TunnelClusterTransferAPDU(apdu) emberAfFillExternalBuffer(mask,                                                       \
                                                                                                                                   \
                                                                       ZCL_TRANSFER_APDU_COMMAND_ID, "u", apdu);

/** @brief Command description for ConnectRequest
 *
 * Command: ConnectRequest
 * @param connectControl 11073ConnectRequestConnectControl
 * @param idleTimeout INT16U
 * @param managerTarget IEEE_ADDRESS
 * @param managerEndpoint INT8U
 */
#define emberAfFillCommand11073                                                                                                    \
    Protocol TunnelClusterConnectRequest(connectControl, idleTimeout, managerTarget, managerEndpoint) emberAfFillExternalBuffer(   \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_CONNECT_REQUEST_COMMAND_ID, "uuuu", connectControl, idleTimeout, managerTarget, managerEndpoint);

/** @brief Command description for DisconnectRequest
 *
 * Command: DisconnectRequest
 * @param managerIEEEAddress IEEE_ADDRESS
 */
#define emberAfFillCommand11073                                                                                                    \
    Protocol TunnelClusterDisconnectRequest(managerIEEEAddress)                                                                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DISCONNECT_REQUEST_COMMAND_ID, "u", managerIEEEAddress);

/** @brief Command description for ConnectStatusNotification
 *
 * Command: ConnectStatusNotification
 * @param connectStatus 11073TunnelConnectionStatus
 */
#define emberAfFillCommand11073                                                                                                    \
    Protocol TunnelClusterConnectStatusNotification(connectStatus)                                                                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_CONNECT_STATUS_NOTIFICATION_COMMAND_ID, "u", connectStatus);

/** @brief Command description for TransferApdu
 *
 * Command: TransferApdu
 * @param apdu OCTET_STRING
 */
#define emberAfFillCommandISO                                                                                                      \
    7816 Protocol TunnelClusterTransferApdu(apdu) emberAfFillExternalBuffer(mask,                                                  \
                                                                                                                                   \
                                                                            ZCL_TRANSFER_APDU_COMMAND_ID, "u", apdu);

/** @brief Command description for InsertSmartCard
 *
 * Command: InsertSmartCard
 */
#define emberAfFillCommandISO                                                                                                      \
    7816 Protocol TunnelClusterInsertSmartCard() emberAfFillExternalBuffer(mask,                                                   \
                                                                                                                                   \
                                                                           ZCL_INSERT_SMART_CARD_COMMAND_ID, "", );

/** @brief Command description for ExtractSmartCard
 *
 * Command: ExtractSmartCard
 */
#define emberAfFillCommandISO                                                                                                      \
    7816 Protocol TunnelClusterExtractSmartCard() emberAfFillExternalBuffer(mask,                                                  \
                                                                                                                                   \
                                                                            ZCL_EXTRACT_SMART_CARD_COMMAND_ID, "", );

/** @brief Command description for PublishPrice
 *
 * Command: PublishPrice
 * @param providerId INT32U
 * @param commandOptions AmiCommandOptions
 * @param rateLabel OCTET_STRING
 * @param issuerEventId INT32U
 * @param currentTime UTC_TIME
 * @param unitOfMeasure AmiUnitOfMeasure
 * @param currency INT16U
 * @param priceTrailingDigitAndPriceTier PriceTrailingDigitAndPriceTier
 * @param numberOfPriceTiersAndRegisterTier PriceNumberOfPriceTiersAndRegisterTier
 * @param startTime UTC_TIME
 * @param durationInMinutes INT16U
 * @param price INT32U
 * @param priceRatio INT8U
 * @param generationPrice INT32U
 * @param generationPriceRatio INT8U
 * @param alternateCostDelivered INT32U
 * @param alternateCostUnit AlternateCostUnit
 * @param alternateCostTrailingDigit AlternateCostTrailingDigit
 * @param numberOfBlockThresholds INT8U
 * @param priceControl PriceControlMask
 * @param numberOfGenerationTiers INT8U
 * @param generationTier GenerationTier
 * @param extendedNumberOfPriceTiers ExtendedNumberOfPriceTiers
 * @param extendedPriceTier ExtendedPriceTier
 * @param extendedRegisterTier ExtendedRegisterTier
 */
#define emberAfFillCommandPriceClusterPublishPrice(                                                                                \
    providerId, commandOptions, rateLabel, issuerEventId, currentTime, unitOfMeasure, currency, priceTrailingDigitAndPriceTier,    \
    numberOfPriceTiersAndRegisterTier, startTime, durationInMinutes, price, priceRatio, generationPrice, generationPriceRatio,     \
    alternateCostDelivered, alternateCostUnit, alternateCostTrailingDigit, numberOfBlockThresholds, priceControl,                  \
    numberOfGenerationTiers, generationTier, extendedNumberOfPriceTiers, extendedPriceTier, extendedRegisterTier)                  \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_PRICE_COMMAND_ID, "uuuuuuuuuuuuuuuuuuuuuuuuu", providerId, commandOptions, rateLabel,    \
                              issuerEventId, currentTime, unitOfMeasure, currency, priceTrailingDigitAndPriceTier,                 \
                              numberOfPriceTiersAndRegisterTier, startTime, durationInMinutes, price, priceRatio, generationPrice, \
                              generationPriceRatio, alternateCostDelivered, alternateCostUnit, alternateCostTrailingDigit,         \
                              numberOfBlockThresholds, priceControl, numberOfGenerationTiers, generationTier,                      \
                              extendedNumberOfPriceTiers, extendedPriceTier, extendedRegisterTier);

/** @brief Command description for PublishBlockPeriod
 *
 * Command: PublishBlockPeriod
 * @param providerId INT32U
 * @param startTime UTC_TIME
 * @param issuerEventId INT32U
 * @param numberOfEvents INT8U
 * @param blockPeriodStartTime UTC_TIME
 * @param blockPeriodDuration INT24U
 * @param numberOfPriceTiersAndNumberOfBlockThresholds BITMAP8
 * @param blockPeriodControl BlockPeriodControl
 * @param blockPeriodDurationType BlockPeriodDurationType
 * @param tariffType TariffType
 * @param tariffResolutionPeriod TariffResolutionPeriod
 */
#define emberAfFillCommandPriceClusterPublishBlockPeriod(                                                                          \
    providerId, startTime, issuerEventId, numberOfEvents, blockPeriodStartTime, blockPeriodDuration,                               \
    numberOfPriceTiersAndNumberOfBlockThresholds, blockPeriodControl, blockPeriodDurationType, tariffType, tariffResolutionPeriod) \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_BLOCK_PERIOD_COMMAND_ID, "uuuuuuuuuuu", providerId, startTime, issuerEventId,            \
                              numberOfEvents, blockPeriodStartTime, blockPeriodDuration,                                           \
                              numberOfPriceTiersAndNumberOfBlockThresholds, blockPeriodControl, blockPeriodDurationType,           \
                              tariffType, tariffResolutionPeriod);

/** @brief Command description for PublishConversionFactor
 *
 * Command: PublishConversionFactor
 * @param issuerEventId INT32U
 * @param providerId INT32U
 * @param startTime UTC_TIME
 * @param issuerEventId INT32U
 * @param conversionFactor INT32U
 * @param priceAckTime UTC_TIME
 * @param conversionFactorTrailingDigit ConversionFactorTrailingDigit
 * @param control PriceControlMask
 */
#define emberAfFillCommandPriceClusterPublishConversionFactor(                                                                     \
    issuerEventId, providerId, startTime, issuerEventId, conversionFactor, priceAckTime, conversionFactorTrailingDigit, control)   \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_CONVERSION_FACTOR_COMMAND_ID, "uuuuuuuu", issuerEventId, providerId, startTime,          \
                              issuerEventId, conversionFactor, priceAckTime, conversionFactorTrailingDigit, control);

/** @brief Command description for PublishCalorificValue
 *
 * Command: PublishCalorificValue
 * @param issuerEventId INT32U
 * @param startTime UTC_TIME
 * @param startTime UTC_TIME
 * @param numberOfEvents INT8U
 * @param calorificValue INT32U
 * @param tariffType TariffType
 * @param calorificValueUnit CalorificValueUnit
 * @param calorificValueTrailingDigit CalorificValueTrailingDigit
 */
#define emberAfFillCommandPriceClusterPublishCalorificValue(issuerEventId, startTime, startTime, numberOfEvents, calorificValue,   \
                                                            tariffType, calorificValueUnit, calorificValueTrailingDigit)           \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_CALORIFIC_VALUE_COMMAND_ID, "uuuuuuuu", issuerEventId, startTime, startTime,             \
                              numberOfEvents, calorificValue, tariffType, calorificValueUnit, calorificValueTrailingDigit);

/** @brief Command description for PublishTariffInformation
 *
 * Command: PublishTariffInformation
 * @param providerId INT32U
 * @param earliestStartTime UTC_TIME
 * @param issuerEventId INT32U
 * @param minIssuerEventId INT32U
 * @param issuerTariffId INT32U
 * @param numberOfCommands INT8U
 * @param startTime UTC_TIME
 * @param tariffTypeChargingScheme TariffTypeChargingScheme
 * @param tariffLabel OCTET_STRING
 * @param numberOfPriceTiersInUse INT8U
 * @param numberOfBlockThresholdsInUse INT8U
 * @param unitOfMeasure AmiUnitOfMeasure
 * @param currency INT16U
 * @param priceTrailingDigit PriceTrailingDigit
 * @param standingCharge INT32U
 * @param tierBlockMode TierBlockMode
 * @param blockThresholdMultiplier INT24U
 * @param blockThresholdDivisor INT24U
 */
#define emberAfFillCommandPriceClusterPublishTariffInformation(                                                                    \
    providerId, earliestStartTime, issuerEventId, minIssuerEventId, issuerTariffId, numberOfCommands, startTime,                   \
    tariffTypeChargingScheme, tariffLabel, numberOfPriceTiersInUse, numberOfBlockThresholdsInUse, unitOfMeasure, currency,         \
    priceTrailingDigit, standingCharge, tierBlockMode, blockThresholdMultiplier, blockThresholdDivisor)                            \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_TARIFF_INFORMATION_COMMAND_ID, "uuuuuuuuuuuuuuuuuu", providerId, earliestStartTime,      \
                              issuerEventId, minIssuerEventId, issuerTariffId, numberOfCommands, startTime,                        \
                              tariffTypeChargingScheme, tariffLabel, numberOfPriceTiersInUse, numberOfBlockThresholdsInUse,        \
                              unitOfMeasure, currency, priceTrailingDigit, standingCharge, tierBlockMode,                          \
                              blockThresholdMultiplier, blockThresholdDivisor);

/** @brief Command description for PublishPriceMatrix
 *
 * Command: PublishPriceMatrix
 * @param providerId INT32U
 * @param earliestStartTime UTC_TIME
 * @param issuerEventId INT32U
 * @param minIssuerEventId INT32U
 * @param startTime UTC_TIME
 * @param numberOfCommands INT8U
 * @param issuerTariffId INT32U
 * @param commandIndex INT8U
 * @param numberOfCommands INT8U
 * @param subPayloadControl PriceMatrixSubPayloadControl
 * @param payload PriceMatrixSubPayload []
 * @param payloadLen int
 */
#define emberAfFillCommandPriceClusterPublishPriceMatrix(providerId, earliestStartTime, issuerEventId, minIssuerEventId,           \
                                                         startTime, numberOfCommands, issuerTariffId, commandIndex,                \
                                                         numberOfCommands, subPayloadControl, payload, payloadLen)                 \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_PRICE_MATRIX_COMMAND_ID, "uuuuuuuuuub", providerId, earliestStartTime, issuerEventId,    \
                              minIssuerEventId, startTime, numberOfCommands, issuerTariffId, commandIndex, numberOfCommands,       \
                              subPayloadControl, payload, payloadLen);

/** @brief Command description for PublishBlockThresholds
 *
 * Command: PublishBlockThresholds
 * @param providerId INT32U
 * @param earliestStartTime UTC_TIME
 * @param issuerEventId INT32U
 * @param minIssuerEventId INT32U
 * @param startTime UTC_TIME
 * @param numberOfCommands INT8U
 * @param issuerTariffId INT32U
 * @param tariffType TariffType
 * @param commandIndex INT8U
 * @param numberOfCommands INT8U
 * @param subPayloadControl BlockThresholdSubPayloadControl
 * @param payload BlockThresholdSubPayload []
 * @param payloadLen int
 */
#define emberAfFillCommandPriceClusterPublishBlockThresholds(                                                                      \
    providerId, earliestStartTime, issuerEventId, minIssuerEventId, startTime, numberOfCommands, issuerTariffId, tariffType,       \
    commandIndex, numberOfCommands, subPayloadControl, payload, payloadLen)                                                        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_BLOCK_THRESHOLDS_COMMAND_ID, "uuuuuuuuuuub", providerId, earliestStartTime,              \
                              issuerEventId, minIssuerEventId, startTime, numberOfCommands, issuerTariffId, tariffType,            \
                              commandIndex, numberOfCommands, subPayloadControl, payload, payloadLen);

/** @brief Command description for PublishCO2Value
 *
 * Command: PublishCO2Value
 * @param providerId INT32U
 * @param issuerTariffId INT32U
 * @param issuerEventId INT32U
 * @param startTime UTC_TIME
 * @param tariffType TariffType
 * @param cO2Value INT32U
 * @param cO2ValueUnit CO2Unit
 * @param cO2ValueTrailingDigit CO2TrailingDigit
 */
#define emberAfFillCommandPriceClusterPublishCO2Value(providerId, issuerTariffId, issuerEventId, startTime, tariffType, cO2Value,  \
                                                      cO2ValueUnit, cO2ValueTrailingDigit)                                         \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_CO2_VALUE_COMMAND_ID, "uuuuuuuu", providerId, issuerTariffId, issuerEventId, startTime,  \
                              tariffType, cO2Value, cO2ValueUnit, cO2ValueTrailingDigit);

/** @brief Command description for PublishTierLabels
 *
 * Command: PublishTierLabels
 * @param providerId INT32U
 * @param issuerTariffId INT32U
 * @param issuerEventId INT32U
 * @param issuerTariffId INT32U
 * @param commandIndex INT8U
 * @param numberOfCommands INT8U
 * @param numberOfLabels INT8U
 * @param tierLabelsPayload INT8U []
 * @param tierLabelsPayloadLen int
 */
#define emberAfFillCommandPriceClusterPublishTierLabels(providerId, issuerTariffId, issuerEventId, issuerTariffId, commandIndex,   \
                                                        numberOfCommands, numberOfLabels, tierLabelsPayload, tierLabelsPayloadLen) \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_TIER_LABELS_COMMAND_ID, "uuuuuuub", providerId, issuerTariffId, issuerEventId,           \
                              issuerTariffId, commandIndex, numberOfCommands, numberOfLabels, tierLabelsPayload,                   \
                              tierLabelsPayloadLen);

/** @brief Command description for PublishBillingPeriod
 *
 * Command: PublishBillingPeriod
 * @param providerId INT32U
 * @param earliestStartTime UTC_TIME
 * @param issuerEventId INT32U
 * @param minIssuerEventId INT32U
 * @param billingPeriodStartTime UTC_TIME
 * @param numberOfCommands INT8U
 * @param billingPeriodDuration BillingPeriodDuration
 * @param tariffType TariffType
 * @param billingPeriodDurationType BillingPeriodDurationType
 * @param tariffType TariffType
 */
#define emberAfFillCommandPriceClusterPublishBillingPeriod(providerId, earliestStartTime, issuerEventId, minIssuerEventId,         \
                                                           billingPeriodStartTime, numberOfCommands, billingPeriodDuration,        \
                                                           tariffType, billingPeriodDurationType, tariffType)                      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_BILLING_PERIOD_COMMAND_ID, "uuuuuuuuuu", providerId, earliestStartTime, issuerEventId,   \
                              minIssuerEventId, billingPeriodStartTime, numberOfCommands, billingPeriodDuration, tariffType,       \
                              billingPeriodDurationType, tariffType);

/** @brief Command description for PublishConsolidatedBill
 *
 * Command: PublishConsolidatedBill
 * @param providerId INT32U
 * @param issuerTariffId INT32U
 * @param issuerEventId INT32U
 * @param billingPeriodStartTime UTC_TIME
 * @param billingPeriodDuration BillingPeriodDuration
 * @param billingPeriodDurationType BillingPeriodDurationType
 * @param tariffType TariffType
 * @param consolidatedBill INT32U
 * @param currency INT16U
 * @param billTrailingDigit BillTrailingDigit
 */
#define emberAfFillCommandPriceClusterPublishConsolidatedBill(providerId, issuerTariffId, issuerEventId, billingPeriodStartTime,   \
                                                              billingPeriodDuration, billingPeriodDurationType, tariffType,        \
                                                              consolidatedBill, currency, billTrailingDigit)                       \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_CONSOLIDATED_BILL_COMMAND_ID, "uuuuuuuuuu", providerId, issuerTariffId, issuerEventId,   \
                              billingPeriodStartTime, billingPeriodDuration, billingPeriodDurationType, tariffType,                \
                              consolidatedBill, currency, billTrailingDigit);

/** @brief Command description for PublishCppEvent
 *
 * Command: PublishCppEvent
 * @param providerId INT32U
 * @param earliestStartTime UTC_TIME
 * @param issuerEventId INT32U
 * @param minIssuerEventId INT32U
 * @param startTime UTC_TIME
 * @param numberOfCommands INT8U
 * @param durationInMinutes INT16U
 * @param tariffType TariffType
 * @param tariffType TariffType
 * @param cppPriceTier CppPriceTier
 * @param cppAuth PublishCppEventCppAuth
 */
#define emberAfFillCommandPriceClusterPublishCppEvent(providerId, earliestStartTime, issuerEventId, minIssuerEventId, startTime,   \
                                                      numberOfCommands, durationInMinutes, tariffType, tariffType, cppPriceTier,   \
                                                      cppAuth)                                                                     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_CPP_EVENT_COMMAND_ID, "uuuuuuuuuuu", providerId, earliestStartTime, issuerEventId,       \
                              minIssuerEventId, startTime, numberOfCommands, durationInMinutes, tariffType, tariffType,            \
                              cppPriceTier, cppAuth);

/** @brief Command description for PublishCreditPayment
 *
 * Command: PublishCreditPayment
 * @param providerId INT32U
 * @param earliestStartTime UTC_TIME
 * @param issuerEventId INT32U
 * @param minIssuerEventId INT32U
 * @param creditPaymentDueDate UTC_TIME
 * @param numberOfCommands INT8U
 * @param creditPaymentOverDueAmount INT32U
 * @param tariffType TariffType
 * @param creditPaymentStatus CreditPaymentStatus
 * @param creditPayment INT32U
 * @param creditPaymentDate UTC_TIME
 * @param creditPaymentRef OCTET_STRING
 */
#define emberAfFillCommandPriceClusterPublishCreditPayment(                                                                        \
    providerId, earliestStartTime, issuerEventId, minIssuerEventId, creditPaymentDueDate, numberOfCommands,                        \
    creditPaymentOverDueAmount, tariffType, creditPaymentStatus, creditPayment, creditPaymentDate, creditPaymentRef)               \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_CREDIT_PAYMENT_COMMAND_ID, "uuuuuuuuuuuu", providerId, earliestStartTime, issuerEventId, \
                              minIssuerEventId, creditPaymentDueDate, numberOfCommands, creditPaymentOverDueAmount, tariffType,    \
                              creditPaymentStatus, creditPayment, creditPaymentDate, creditPaymentRef);

/** @brief Command description for PublishCurrencyConversion
 *
 * Command: PublishCurrencyConversion
 * @param providerId INT32U
 * @param issuerEventId INT32U
 * @param issuerEventId INT32U
 * @param cppAuth CppEventResponseCppAuth
 * @param startTime UTC_TIME
 * @param oldCurrency INT16U
 * @param newCurrency INT16U
 * @param conversionFactor INT32U
 * @param conversionFactorTrailingDigit ConversionFactorTrailingDigit
 * @param currencyChangeControlFlags CurrencyChangeControl
 */
#define emberAfFillCommandPriceClusterPublishCurrencyConversion(providerId, issuerEventId, issuerEventId, cppAuth, startTime,      \
                                                                oldCurrency, newCurrency, conversionFactor,                        \
                                                                conversionFactorTrailingDigit, currencyChangeControlFlags)         \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_CURRENCY_CONVERSION_COMMAND_ID, "uuuuuuuuuu", providerId, issuerEventId, issuerEventId,  \
                              cppAuth, startTime, oldCurrency, newCurrency, conversionFactor, conversionFactorTrailingDigit,       \
                              currencyChangeControlFlags);

/** @brief Command description for CancelTariff
 *
 * Command: CancelTariff
 * @param providerId INT32U
 * @param latestEndTime UTC_TIME
 * @param issuerTariffId INT32U
 * @param numberOfRecords INT8U
 * @param tariffType TariffType
 */
#define emberAfFillCommandPriceClusterCancelTariff(providerId, latestEndTime, issuerTariffId, numberOfRecords, tariffType)         \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CANCEL_TARIFF_COMMAND_ID, "uuuuu", providerId, latestEndTime, issuerTariffId, numberOfRecords,   \
                              tariffType);

/** @brief Command description for GetCurrencyConversionCommand
 *
 * Command: GetCurrencyConversionCommand
 */
#define emberAfFillCommandPriceClusterGetCurrencyConversionCommand()                                                               \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_CURRENCY_CONVERSION_COMMAND_COMMAND_ID, "", );

/** @brief Command description for GetTariffCancellation
 *
 * Command: GetTariffCancellation
 */
#define emberAfFillCommandPriceClusterGetTariffCancellation()                                                                      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_TARIFF_CANCELLATION_COMMAND_ID, "", );

/** @brief Command description for LoadControlEvent
 *
 * Command: LoadControlEvent
 * @param issuerEventId INT32U
 * @param issuerEventId INT32U
 * @param deviceClass AmiDeviceClass
 * @param eventStatus AmiEventStatus
 * @param utilityEnrollmentGroup INT8U
 * @param eventStatusTime UTC_TIME
 * @param startTime UTC_TIME
 * @param criticalityLevelApplied AmiCriticalityLevel
 * @param durationInMinutes INT16U
 * @param coolingTemperatureSetPointApplied INT16U
 * @param criticalityLevel AmiCriticalityLevel
 * @param heatingTemperatureSetPointApplied INT16U
 * @param coolingTemperatureOffset INT8U
 * @param averageLoadAdjustmentPercentageApplied INT8S
 * @param heatingTemperatureOffset INT8U
 * @param dutyCycleApplied INT8U
 * @param coolingTemperatureSetPoint INT16S
 * @param eventControl AmiEventControl
 * @param heatingTemperatureSetPoint INT16S
 * @param signatureType SignatureType
 * @param averageLoadAdjustmentPercentage INT8S
 * @param signature Signature
 * @param dutyCycle INT8U
 * @param eventControl AmiEventControl
 */
#define emberAfFillCommandDemand                                                                                                   \
    Response and Load ControlClusterLoadControlEvent(                                                                              \
        issuerEventId, issuerEventId, deviceClass, eventStatus, utilityEnrollmentGroup, eventStatusTime, startTime,                \
        criticalityLevelApplied, durationInMinutes, coolingTemperatureSetPointApplied, criticalityLevel,                           \
        heatingTemperatureSetPointApplied, coolingTemperatureOffset, averageLoadAdjustmentPercentageApplied,                       \
        heatingTemperatureOffset, dutyCycleApplied, coolingTemperatureSetPoint, eventControl, heatingTemperatureSetPoint,          \
        signatureType, averageLoadAdjustmentPercentage, signature, dutyCycle, eventControl)                                        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_LOAD_CONTROL_EVENT_COMMAND_ID, "uuuuuuuuuuuuuuuuuuuuuuuu", issuerEventId, issuerEventId,     \
                                  deviceClass, eventStatus, utilityEnrollmentGroup, eventStatusTime, startTime,                    \
                                  criticalityLevelApplied, durationInMinutes, coolingTemperatureSetPointApplied, criticalityLevel, \
                                  heatingTemperatureSetPointApplied, coolingTemperatureOffset,                                     \
                                  averageLoadAdjustmentPercentageApplied, heatingTemperatureOffset, dutyCycleApplied,              \
                                  coolingTemperatureSetPoint, eventControl, heatingTemperatureSetPoint, signatureType,             \
                                  averageLoadAdjustmentPercentage, signature, dutyCycle, eventControl);

/** @brief Command description for CancelLoadControlEvent
 *
 * Command: CancelLoadControlEvent
 * @param issuerEventId INT32U
 * @param startTime UTC_TIME
 * @param deviceClass AmiDeviceClass
 * @param numberOfEvents INT8U
 * @param utilityEnrollmentGroup INT8U
 * @param issuerEventId INT32U
 * @param cancelControl AmiCancelControl
 * @param effectiveTime UTC_TIME
 */
#define emberAfFillCommandDemand                                                                                                   \
    Response and Load ControlClusterCancelLoadControlEvent(issuerEventId, startTime, deviceClass, numberOfEvents,                  \
                                                           utilityEnrollmentGroup, issuerEventId, cancelControl, effectiveTime)    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_CANCEL_LOAD_CONTROL_EVENT_COMMAND_ID, "uuuuuuuu", issuerEventId, startTime, deviceClass,     \
                                  numberOfEvents, utilityEnrollmentGroup, issuerEventId, cancelControl, effectiveTime);

/** @brief Command description for CancelAllLoadControlEvents
 *
 * Command: CancelAllLoadControlEvents
 * @param cancelControl AmiCancelControl
 */
#define emberAfFillCommandDemand                                                                                                   \
    Response and Load ControlClusterCancelAllLoadControlEvents(cancelControl)                                                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_CANCEL_ALL_LOAD_CONTROL_EVENTS_COMMAND_ID, "u", cancelControl);

/** @brief Command description for GetProfileResponse
 *
 * Command: GetProfileResponse
 * @param endTime UTC_TIME
 * @param intervalChannel AmiIntervalChannel
 * @param status AmiGetProfileStatus
 * @param endTime UTC_TIME
 * @param profileIntervalPeriod AmiIntervalPeriod
 * @param numberOfPeriods INT8U
 * @param numberOfPeriodsDelivered INT8U
 * @param intervals INT24U []
 * @param intervalsLen int
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterGetProfileResponse(endTime, intervalChannel, status, endTime, profileIntervalPeriod, numberOfPeriods,           \
                                      numberOfPeriodsDelivered, intervals, intervalsLen)                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_PROFILE_RESPONSE_COMMAND_ID, "uuuuuuub", endTime, intervalChannel, status, endTime,      \
                                  profileIntervalPeriod, numberOfPeriods, numberOfPeriodsDelivered, intervals, intervalsLen);

/** @brief Command description for RequestMirror
 *
 * Command: RequestMirror
 * @param endpointId INT16U
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterRequestMirror(endpointId) emberAfFillExternalBuffer(mask,                                                       \
                                                                                                                                   \
                                                                       ZCL_REQUEST_MIRROR_COMMAND_ID, "u", endpointId);

/** @brief Command description for RemoveMirror
 *
 * Command: RemoveMirror
 * @param endpointId INT16U
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterRemoveMirror(endpointId) emberAfFillExternalBuffer(mask,                                                        \
                                                                                                                                   \
                                                                      ZCL_REMOVE_MIRROR_COMMAND_ID, "u", endpointId);

/** @brief Command description for RequestFastPollModeResponse
 *
 * Command: RequestFastPollModeResponse
 * @param appliedUpdatePeriod INT8U
 * @param fastPollUpdatePeriod INT8U
 * @param fastPollModeEndtime UTC_TIME
 * @param duration INT8U
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterRequestFastPollModeResponse(appliedUpdatePeriod, fastPollUpdatePeriod, fastPollModeEndtime, duration)           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_REQUEST_FAST_POLL_MODE_RESPONSE_COMMAND_ID, "uuuu", appliedUpdatePeriod,                     \
                                  fastPollUpdatePeriod, fastPollModeEndtime, duration);

/** @brief Command description for ScheduleSnapshotResponse
 *
 * Command: ScheduleSnapshotResponse
 * @param issuerEventId INT32U
 * @param issuerEventId INT32U
 * @param snapshotResponsePayload SnapshotResponsePayload []
 * @param snapshotResponsePayloadLen int
 * @param commandIndex INT8U
 * @param commandCount INT8U
 * @param snapshotSchedulePayload SnapshotSchedulePayload []
 * @param snapshotSchedulePayloadLen int
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterScheduleSnapshotResponse(issuerEventId, issuerEventId, snapshotResponsePayload, snapshotResponsePayloadLen,     \
                                            commandIndex, commandCount, snapshotSchedulePayload, snapshotSchedulePayloadLen)       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SCHEDULE_SNAPSHOT_RESPONSE_COMMAND_ID, "uubuub", issuerEventId, issuerEventId,               \
                                  snapshotResponsePayload, snapshotResponsePayloadLen, commandIndex, commandCount,                 \
                                  snapshotSchedulePayload, snapshotSchedulePayloadLen);

/** @brief Command description for TakeSnapshotResponse
 *
 * Command: TakeSnapshotResponse
 * @param snapshotId INT32U
 * @param snapshotCause SnapshotCause
 * @param snapshotConfirmation SnapshotConfirmation
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterTakeSnapshotResponse(snapshotId, snapshotCause, snapshotConfirmation)                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_TAKE_SNAPSHOT_RESPONSE_COMMAND_ID, "uuu", snapshotId, snapshotCause, snapshotConfirmation);

/** @brief Command description for PublishSnapshot
 *
 * Command: PublishSnapshot
 * @param snapshotId INT32U
 * @param earliestStartTime UTC_TIME
 * @param snapshotTime UTC_TIME
 * @param latestEndTime UTC_TIME
 * @param totalSnapshotsFound INT8U
 * @param snapshotOffset INT8U
 * @param commandIndex INT8U
 * @param snapshotCause SnapshotCause
 * @param totalCommands INT8U
 * @param snapshotCause SnapshotCause
 * @param snapshotPayloadType SnapshotPayloadType
 * @param snapshotPayload INT8U []
 * @param snapshotPayloadLen int
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterPublishSnapshot(snapshotId, earliestStartTime, snapshotTime, latestEndTime, totalSnapshotsFound,                \
                                   snapshotOffset, commandIndex, snapshotCause, totalCommands, snapshotCause, snapshotPayloadType, \
                                   snapshotPayload, snapshotPayloadLen)                                                            \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_PUBLISH_SNAPSHOT_COMMAND_ID, "uuuuuuuuuuub", snapshotId, earliestStartTime, snapshotTime,    \
                                  latestEndTime, totalSnapshotsFound, snapshotOffset, commandIndex, snapshotCause, totalCommands,  \
                                  snapshotCause, snapshotPayloadType, snapshotPayload, snapshotPayloadLen);

/** @brief Command description for GetSampledDataResponse
 *
 * Command: GetSampledDataResponse
 * @param sampleId INT16U
 * @param issuerEventId INT32U
 * @param sampleStartTime UTC_TIME
 * @param startSamplingTime UTC_TIME
 * @param sampleType SampleType
 * @param sampleType SampleType
 * @param sampleRequestInterval INT16U
 * @param sampleRequestInterval INT16U
 * @param numberOfSamples INT16U
 * @param maxNumberOfSamples INT16U
 * @param samples INT24U []
 * @param samplesLen int
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterGetSampledDataResponse(sampleId, issuerEventId, sampleStartTime, startSamplingTime, sampleType, sampleType,     \
                                          sampleRequestInterval, sampleRequestInterval, numberOfSamples, maxNumberOfSamples,       \
                                          samples, samplesLen)                                                                     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_SAMPLED_DATA_RESPONSE_COMMAND_ID, "uuuuuuuuuub", sampleId, issuerEventId,                \
                                  sampleStartTime, startSamplingTime, sampleType, sampleType, sampleRequestInterval,               \
                                  sampleRequestInterval, numberOfSamples, maxNumberOfSamples, samples, samplesLen);

/** @brief Command description for ConfigureMirror
 *
 * Command: ConfigureMirror
 * @param issuerEventId INT32U
 * @param sampleId INT16U
 * @param reportingInterval INT24U
 * @param earliestSampleTime UTC_TIME
 * @param mirrorNotificationReporting BOOLEAN
 * @param sampleType SampleType
 * @param notificationScheme INT8U
 * @param numberOfSamples INT16U
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterConfigureMirror(issuerEventId, sampleId, reportingInterval, earliestSampleTime, mirrorNotificationReporting,    \
                                   sampleType, notificationScheme, numberOfSamples)                                                \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_CONFIGURE_MIRROR_COMMAND_ID, "uuuuuuuu", issuerEventId, sampleId, reportingInterval,         \
                                  earliestSampleTime, mirrorNotificationReporting, sampleType, notificationScheme,                 \
                                  numberOfSamples);

/** @brief Command description for ConfigureNotificationScheme
 *
 * Command: ConfigureNotificationScheme
 * @param issuerEventId INT32U
 * @param notificationScheme INT8U
 * @param notificationScheme INT8U
 * @param notificationFlags BITMAP32 []
 * @param notificationFlagsLen int
 * @param notificationFlagOrder BITMAP32
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterConfigureNotificationScheme(issuerEventId, notificationScheme, notificationScheme, notificationFlags,           \
                                               notificationFlagsLen, notificationFlagOrder)                                        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_CONFIGURE_NOTIFICATION_SCHEME_COMMAND_ID, "uuubu", issuerEventId, notificationScheme,        \
                                  notificationScheme, notificationFlags, notificationFlagsLen, notificationFlagOrder);

/** @brief Command description for ConfigureNotificationFlags
 *
 * Command: ConfigureNotificationFlags
 * @param issuerEventId INT32U
 * @param providerId INT32U
 * @param notificationScheme INT8U
 * @param issuerEventId INT32U
 * @param notificationFlagAttributeId INT16U
 * @param clusterId INT16U
 * @param manufacturerCode INT16U
 * @param numberOfCommands INT8U
 * @param commandIds INT8U []
 * @param commandIdsLen int
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterConfigureNotificationFlags(issuerEventId, providerId, notificationScheme, issuerEventId,                        \
                                              notificationFlagAttributeId, clusterId, manufacturerCode, numberOfCommands,          \
                                              commandIds, commandIdsLen)                                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_CONFIGURE_NOTIFICATION_FLAGS_COMMAND_ID, "uuuuuuuub", issuerEventId, providerId,             \
                                  notificationScheme, issuerEventId, notificationFlagAttributeId, clusterId, manufacturerCode,     \
                                  numberOfCommands, commandIds, commandIdsLen);

/** @brief Command description for GetNotifiedMessage
 *
 * Command: GetNotifiedMessage
 * @param notificationScheme INT8U
 * @param providerId INT32U
 * @param notificationFlagAttributeId INT16U
 * @param issuerEventId INT32U
 * @param notificationFlagsN BITMAP32
 * @param requestDateTime UTC_TIME
 * @param implementationDateTime UTC_TIME
 * @param proposedSupplyStatus MeteringSupplyStatus
 * @param supplyControlBits SupplyControlBits
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterGetNotifiedMessage(notificationScheme, providerId, notificationFlagAttributeId, issuerEventId,                  \
                                      notificationFlagsN, requestDateTime, implementationDateTime, proposedSupplyStatus,           \
                                      supplyControlBits)                                                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_NOTIFIED_MESSAGE_COMMAND_ID, "uuuuuuuuu", notificationScheme, providerId,                \
                                  notificationFlagAttributeId, issuerEventId, notificationFlagsN, requestDateTime,                 \
                                  implementationDateTime, proposedSupplyStatus, supplyControlBits);

/** @brief Command description for SupplyStatusResponse
 *
 * Command: SupplyStatusResponse
 * @param providerId INT32U
 * @param proposedSupplyStatus ProposedSupplyStatus
 * @param issuerEventId INT32U
 * @param implementationDateTime UTC_TIME
 * @param supplyStatus MeteringSupplyStatus
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterSupplyStatusResponse(providerId, proposedSupplyStatus, issuerEventId, implementationDateTime, supplyStatus)     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SUPPLY_STATUS_RESPONSE_COMMAND_ID, "uuuuu", providerId, proposedSupplyStatus, issuerEventId, \
                                  implementationDateTime, supplyStatus);

/** @brief Command description for StartSamplingResponse
 *
 * Command: StartSamplingResponse
 * @param sampleId INT16U
 * @param issuerEventId INT32U
 * @param supplyTamperState SupplyStatus
 * @param supplyDepletionState SupplyStatus
 * @param supplyUncontrolledFlowState SupplyStatus
 * @param loadLimitSupplyState SupplyStatus
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterStartSamplingResponse(sampleId, issuerEventId, supplyTamperState, supplyDepletionState,                         \
                                         supplyUncontrolledFlowState, loadLimitSupplyState)                                        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_START_SAMPLING_RESPONSE_COMMAND_ID, "uuuuuu", sampleId, issuerEventId, supplyTamperState,    \
                                  supplyDepletionState, supplyUncontrolledFlowState, loadLimitSupplyState);

/** @brief Command description for SetUncontrolledFlowThreshold
 *
 * Command: SetUncontrolledFlowThreshold
 * @param providerId INT32U
 * @param issuerEventId INT32U
 * @param uncontrolledFlowThreshold INT16U
 * @param unitOfMeasure AmiUnitOfMeasure
 * @param multiplier INT16U
 * @param divisor INT16U
 * @param stabilisationPeriod INT8U
 * @param measurementPeriod INT16U
 */
#define emberAfFillCommandSimple                                                                                                   \
    MeteringClusterSetUncontrolledFlowThreshold(providerId, issuerEventId, uncontrolledFlowThreshold, unitOfMeasure, multiplier,   \
                                                divisor, stabilisationPeriod, measurementPeriod)                                   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_UNCONTROLLED_FLOW_THRESHOLD_COMMAND_ID, "uuuuuuuu", providerId, issuerEventId,           \
                                  uncontrolledFlowThreshold, unitOfMeasure, multiplier, divisor, stabilisationPeriod,              \
                                  measurementPeriod);

/** @brief Command description for GetLastMessage
 *
 * Command: GetLastMessage
 * @param messageId INT32U
 * @param messageControl MessagingControlMask
 * @param startTime UTC_TIME
 * @param durationInMinutes INT16U
 * @param message CHAR_STRING
 * @param optionalExtendedMessageControl MessagingExtendedControlMask
 */
#define emberAfFillCommandMessagingClusterGetLastMessage(messageId, messageControl, startTime, durationInMinutes, message,         \
                                                         optionalExtendedMessageControl)                                           \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_LAST_MESSAGE_COMMAND_ID, "uuuuuu", messageId, messageControl, startTime, durationInMinutes,  \
                              message, optionalExtendedMessageControl);

/** @brief Command description for CancelMessage
 *
 * Command: CancelMessage
 * @param messageId INT32U
 * @param messageId INT32U
 * @param messageControl MessagingControlMask
 * @param confirmationTime UTC_TIME
 * @param messageConfirmationControl BITMAP8
 * @param messageResponse OCTET_STRING
 */
#define emberAfFillCommandMessagingClusterCancelMessage(messageId, messageId, messageControl, confirmationTime,                    \
                                                        messageConfirmationControl, messageResponse)                               \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CANCEL_MESSAGE_COMMAND_ID, "uuuuuu", messageId, messageId, messageControl, confirmationTime,     \
                              messageConfirmationControl, messageResponse);

/** @brief Command description for DisplayProtectedMessage
 *
 * Command: DisplayProtectedMessage
 * @param messageId INT32U
 * @param earliestImplementationTime UTC_TIME
 * @param messageControl MessagingControlMask
 * @param startTime UTC_TIME
 * @param durationInMinutes INT16U
 * @param message CHAR_STRING
 * @param optionalExtendedMessageControl MessagingExtendedControlMask
 */
#define emberAfFillCommandMessagingClusterDisplayProtectedMessage(                                                                 \
    messageId, earliestImplementationTime, messageControl, startTime, durationInMinutes, message, optionalExtendedMessageControl)  \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_DISPLAY_PROTECTED_MESSAGE_COMMAND_ID, "uuuuuuu", messageId, earliestImplementationTime,          \
                              messageControl, startTime, durationInMinutes, message, optionalExtendedMessageControl);

/** @brief Command description for CancelAllMessages
 *
 * Command: CancelAllMessages
 * @param implementationDateTime UTC_TIME
 */
#define emberAfFillCommandMessagingClusterCancelAllMessages(implementationDateTime)                                                \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CANCEL_ALL_MESSAGES_COMMAND_ID, "u", implementationDateTime);

/** @brief Command description for RequestTunnel
 *
 * Command: RequestTunnel
 * @param protocolId INT8U
 * @param tunnelId INT16U
 * @param manufacturerCode INT16U
 * @param tunnelStatus TunnelingTunnelStatus
 * @param flowControlSupport BOOLEAN
 * @param maximumIncomingTransferSize INT16U
 * @param maximumIncomingTransferSize INT16U
 */
#define emberAfFillCommandTunnelingClusterRequestTunnel(protocolId, tunnelId, manufacturerCode, tunnelStatus, flowControlSupport,  \
                                                        maximumIncomingTransferSize, maximumIncomingTransferSize)                  \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_REQUEST_TUNNEL_COMMAND_ID, "uuuuuuu", protocolId, tunnelId, manufacturerCode, tunnelStatus,      \
                              flowControlSupport, maximumIncomingTransferSize, maximumIncomingTransferSize);

/** @brief Command description for CloseTunnel
 *
 * Command: CloseTunnel
 * @param tunnelId INT16U
 * @param tunnelId INT16U
 * @param data INT8U []
 * @param dataLen int
 */
#define emberAfFillCommandTunnelingClusterCloseTunnel(tunnelId, tunnelId, data, dataLen)                                           \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CLOSE_TUNNEL_COMMAND_ID, "uub", tunnelId, tunnelId, data, dataLen);

/** @brief Command description for TransferDataClientToServer
 *
 * Command: TransferDataClientToServer
 * @param tunnelId INT16U
 * @param tunnelId INT16U
 * @param data INT8U []
 * @param dataLen int
 * @param transferDataStatus TunnelingTransferDataStatus
 */
#define emberAfFillCommandTunnelingClusterTransferDataClientToServer(tunnelId, tunnelId, data, dataLen, transferDataStatus)        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_TRANSFER_DATA_CLIENT_TO_SERVER_COMMAND_ID, "uubu", tunnelId, tunnelId, data, dataLen,            \
                              transferDataStatus);

/** @brief Command description for TransferDataErrorClientToServer
 *
 * Command: TransferDataErrorClientToServer
 * @param tunnelId INT16U
 * @param tunnelId INT16U
 * @param transferDataStatus TunnelingTransferDataStatus
 * @param numberOfBytesLeft INT16U
 */
#define emberAfFillCommandTunnelingClusterTransferDataErrorClientToServer(tunnelId, tunnelId, transferDataStatus,                  \
                                                                          numberOfBytesLeft)                                       \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_TRANSFER_DATA_ERROR_CLIENT_TO_SERVER_COMMAND_ID, "uuuu", tunnelId, tunnelId, transferDataStatus, \
                              numberOfBytesLeft);

/** @brief Command description for AckTransferDataClientToServer
 *
 * Command: AckTransferDataClientToServer
 * @param tunnelId INT16U
 * @param tunnelId INT16U
 * @param numberOfBytesLeft INT16U
 * @param numberOfOctetsLeft INT16U
 */
#define emberAfFillCommandTunnelingClusterAckTransferDataClientToServer(tunnelId, tunnelId, numberOfBytesLeft, numberOfOctetsLeft) \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_ACK_TRANSFER_DATA_CLIENT_TO_SERVER_COMMAND_ID, "uuuu", tunnelId, tunnelId, numberOfBytesLeft,    \
                              numberOfOctetsLeft);

/** @brief Command description for ReadyDataClientToServer
 *
 * Command: ReadyDataClientToServer
 * @param tunnelId INT16U
 * @param protocolListComplete BOOLEAN
 * @param numberOfOctetsLeft INT16U
 * @param protocolCount INT8U
 * @param protocolList Protocol []
 * @param protocolListLen int
 */
#define emberAfFillCommandTunnelingClusterReadyDataClientToServer(tunnelId, protocolListComplete, numberOfOctetsLeft,              \
                                                                  protocolCount, protocolList, protocolListLen)                    \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_READY_DATA_CLIENT_TO_SERVER_COMMAND_ID, "uuuub", tunnelId, protocolListComplete,                 \
                              numberOfOctetsLeft, protocolCount, protocolList, protocolListLen);

/** @brief Command description for GetSupportedTunnelProtocols
 *
 * Command: GetSupportedTunnelProtocols
 * @param protocolOffset INT8U
 * @param tunnelId INT16U
 */
#define emberAfFillCommandTunnelingClusterGetSupportedTunnelProtocols(protocolOffset, tunnelId)                                    \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_SUPPORTED_TUNNEL_PROTOCOLS_COMMAND_ID, "uu", protocolOffset, tunnelId);

/** @brief Command description for SelectAvailableEmergencyCredit
 *
 * Command: SelectAvailableEmergencyCredit
 * @param commandIssueDateTime UTC_TIME
 * @param originatingDevice OriginatingDevice
 * @param siteId OCTET_STRING
 * @param meterSerialNumber OCTET_STRING
 */
#define emberAfFillCommandPrepaymentClusterSelectAvailableEmergencyCredit(commandIssueDateTime, originatingDevice, siteId,         \
                                                                          meterSerialNumber)                                       \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SELECT_AVAILABLE_EMERGENCY_CREDIT_COMMAND_ID, "uuuu", commandIssueDateTime, originatingDevice,   \
                              siteId, meterSerialNumber);

/** @brief Command description for PublishPrepaySnapshot
 *
 * Command: PublishPrepaySnapshot
 * @param snapshotId INT32U
 * @param snapshotTime UTC_TIME
 * @param totalSnapshotsFound INT8U
 * @param commandIndex INT8U
 * @param totalNumberOfCommands INT8U
 * @param snapshotCause PrepaySnapshotPayloadCause
 * @param snapshotPayloadType PrepaySnapshotPayloadType
 * @param snapshotPayload INT8U []
 * @param snapshotPayloadLen int
 */
#define emberAfFillCommandPrepaymentClusterPublishPrepaySnapshot(snapshotId, snapshotTime, totalSnapshotsFound, commandIndex,      \
                                                                 totalNumberOfCommands, snapshotCause, snapshotPayloadType,        \
                                                                 snapshotPayload, snapshotPayloadLen)                              \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_PREPAY_SNAPSHOT_COMMAND_ID, "uuuuuuub", snapshotId, snapshotTime, totalSnapshotsFound,   \
                              commandIndex, totalNumberOfCommands, snapshotCause, snapshotPayloadType, snapshotPayload,            \
                              snapshotPayloadLen);

/** @brief Command description for ChangeDebt
 *
 * Command: ChangeDebt
 * @param issuerEventId INT32U
 * @param friendlyCredit FriendlyCredit
 * @param debtLabel OCTET_STRING
 * @param friendlyCreditCalendarId INT32U
 * @param debtAmount INT32U
 * @param emergencyCreditLimit INT32U
 * @param debtRecoveryMethod DebtRecoveryMethod
 * @param emergencyCreditThreshold INT32U
 * @param debtAmountType DebtAmountType
 * @param debtRecoveryStartTime UTC_TIME
 * @param debtRecoveryCollectionTime INT16U
 * @param debtRecoveryFrequency DebtRecoveryFrequency
 * @param debtRecoveryAmount INT32U
 * @param debtRecoveryBalancePercentage INT16U
 */
#define emberAfFillCommandPrepaymentClusterChangeDebt(                                                                             \
    issuerEventId, friendlyCredit, debtLabel, friendlyCreditCalendarId, debtAmount, emergencyCreditLimit, debtRecoveryMethod,      \
    emergencyCreditThreshold, debtAmountType, debtRecoveryStartTime, debtRecoveryCollectionTime, debtRecoveryFrequency,            \
    debtRecoveryAmount, debtRecoveryBalancePercentage)                                                                             \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CHANGE_DEBT_COMMAND_ID, "uuuuuuuuuuuuuu", issuerEventId, friendlyCredit, debtLabel,              \
                              friendlyCreditCalendarId, debtAmount, emergencyCreditLimit, debtRecoveryMethod,                      \
                              emergencyCreditThreshold, debtAmountType, debtRecoveryStartTime, debtRecoveryCollectionTime,         \
                              debtRecoveryFrequency, debtRecoveryAmount, debtRecoveryBalancePercentage);

/** @brief Command description for EmergencyCreditSetup
 *
 * Command: EmergencyCreditSetup
 * @param issuerEventId INT32U
 * @param resultType ResultType
 * @param startTime UTC_TIME
 * @param topUpValue INT32U
 * @param emergencyCreditLimit INT32U
 * @param sourceOfTopUp OriginatingDevice
 * @param emergencyCreditThreshold INT32U
 * @param creditRemaining INT32U
 */
#define emberAfFillCommandPrepaymentClusterEmergencyCreditSetup(issuerEventId, resultType, startTime, topUpValue,                  \
                                                                emergencyCreditLimit, sourceOfTopUp, emergencyCreditThreshold,     \
                                                                creditRemaining)                                                   \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_EMERGENCY_CREDIT_SETUP_COMMAND_ID, "uuuuuuuu", issuerEventId, resultType, startTime, topUpValue, \
                              emergencyCreditLimit, sourceOfTopUp, emergencyCreditThreshold, creditRemaining);

/** @brief Command description for ConsumerTopUp
 *
 * Command: ConsumerTopUp
 * @param originatingDevice OriginatingDevice
 * @param topUpCode OCTET_STRING
 */
#define emberAfFillCommandPrepaymentClusterConsumerTopUp(originatingDevice, topUpCode)                                             \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CONSUMER_TOP_UP_COMMAND_ID, "uu", originatingDevice, topUpCode);

/** @brief Command description for CreditAdjustment
 *
 * Command: CreditAdjustment
 * @param issuerEventId INT32U
 * @param commandIndex INT8U
 * @param startTime UTC_TIME
 * @param totalNumberOfCommands INT8U
 * @param creditAdjustmentType CreditAdjustmentType
 * @param topUpPayload TopUpPayload []
 * @param topUpPayloadLen int
 * @param creditAdjustmentValue INT32U
 */
#define emberAfFillCommandPrepaymentClusterCreditAdjustment(issuerEventId, commandIndex, startTime, totalNumberOfCommands,         \
                                                            creditAdjustmentType, topUpPayload, topUpPayloadLen,                   \
                                                            creditAdjustmentValue)                                                 \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CREDIT_ADJUSTMENT_COMMAND_ID, "uuuuubu", issuerEventId, commandIndex, startTime,                 \
                              totalNumberOfCommands, creditAdjustmentType, topUpPayload, topUpPayloadLen, creditAdjustmentValue);

/** @brief Command description for ChangePaymentMode
 *
 * Command: ChangePaymentMode
 * @param providerId INT32U
 * @param commandIndex INT8U
 * @param issuerEventId INT32U
 * @param totalNumberOfCommands INT8U
 * @param implementationDateTime UTC_TIME
 * @param debtPayload DebtPayload []
 * @param debtPayloadLen int
 * @param proposedPaymentControlConfiguration PaymentControlConfiguration
 * @param cutOffValue INT32U
 */
#define emberAfFillCommandPrepaymentClusterChangePaymentMode(providerId, commandIndex, issuerEventId, totalNumberOfCommands,       \
                                                             implementationDateTime, debtPayload, debtPayloadLen,                  \
                                                             proposedPaymentControlConfiguration, cutOffValue)                     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CHANGE_PAYMENT_MODE_COMMAND_ID, "uuuuubuu", providerId, commandIndex, issuerEventId,             \
                              totalNumberOfCommands, implementationDateTime, debtPayload, debtPayloadLen,                          \
                              proposedPaymentControlConfiguration, cutOffValue);

/** @brief Command description for GetPrepaySnapshot
 *
 * Command: GetPrepaySnapshot
 * @param earliestStartTime UTC_TIME
 * @param latestEndTime UTC_TIME
 * @param snapshotOffset INT8U
 * @param snapshotCause PrepaySnapshotPayloadCause
 */
#define emberAfFillCommandPrepaymentClusterGetPrepaySnapshot(earliestStartTime, latestEndTime, snapshotOffset, snapshotCause)      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_PREPAY_SNAPSHOT_COMMAND_ID, "uuuu", earliestStartTime, latestEndTime, snapshotOffset,        \
                              snapshotCause);

/** @brief Command description for GetTopUpLog
 *
 * Command: GetTopUpLog
 * @param latestEndTime UTC_TIME
 * @param numberOfRecords INT8U
 */
#define emberAfFillCommandPrepaymentClusterGetTopUpLog(latestEndTime, numberOfRecords)                                             \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_TOP_UP_LOG_COMMAND_ID, "uu", latestEndTime, numberOfRecords);

/** @brief Command description for SetLowCreditWarningLevel
 *
 * Command: SetLowCreditWarningLevel
 * @param lowCreditWarningLevel INT32U
 */
#define emberAfFillCommandPrepaymentClusterSetLowCreditWarningLevel(lowCreditWarningLevel)                                         \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SET_LOW_CREDIT_WARNING_LEVEL_COMMAND_ID, "u", lowCreditWarningLevel);

/** @brief Command description for GetDebtRepaymentLog
 *
 * Command: GetDebtRepaymentLog
 * @param latestEndTime UTC_TIME
 * @param numberOfDebts INT8U
 * @param debtType RepaymentDebtType
 */
#define emberAfFillCommandPrepaymentClusterGetDebtRepaymentLog(latestEndTime, numberOfDebts, debtType)                             \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_DEBT_REPAYMENT_LOG_COMMAND_ID, "uuu", latestEndTime, numberOfDebts, debtType);

/** @brief Command description for SetMaximumCreditLimit
 *
 * Command: SetMaximumCreditLimit
 * @param providerId INT32U
 * @param issuerEventId INT32U
 * @param implementationDateTime UTC_TIME
 * @param maximumCreditLevel INT32U
 * @param maximumCreditPerTopUp INT32U
 */
#define emberAfFillCommandPrepaymentClusterSetMaximumCreditLimit(providerId, issuerEventId, implementationDateTime,                \
                                                                 maximumCreditLevel, maximumCreditPerTopUp)                        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SET_MAXIMUM_CREDIT_LIMIT_COMMAND_ID, "uuuuu", providerId, issuerEventId, implementationDateTime, \
                              maximumCreditLevel, maximumCreditPerTopUp);

/** @brief Command description for SetOverallDebtCap
 *
 * Command: SetOverallDebtCap
 * @param providerId INT32U
 * @param issuerEventId INT32U
 * @param implementationDateTime UTC_TIME
 * @param overallDebtCap INT32U
 */
#define emberAfFillCommandPrepaymentClusterSetOverallDebtCap(providerId, issuerEventId, implementationDateTime, overallDebtCap)    \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SET_OVERALL_DEBT_CAP_COMMAND_ID, "uuuu", providerId, issuerEventId, implementationDateTime,      \
                              overallDebtCap);

/** @brief Command description for ReportEventStatus
 *
 * Command: ReportEventStatus
 * @param issuerEventId INT32U
 * @param issuerEventId INT32U
 * @param eventStatus AmiEventStatus
 * @param deviceClass AmiDeviceClass
 * @param eventStatusTime UTC_TIME
 * @param utilityEnrollmentGroup INT8U
 * @param criticalityLevelApplied AmiCriticalityLevel
 * @param actionRequired INT8U
 * @param coolingTemperatureSetPointApplied INT16U
 * @param heatingTemperatureSetPointApplied INT16U
 * @param averageLoadAdjustmentPercentageApplied INT8S
 * @param dutyCycleApplied INT8U
 * @param eventControl AmiEventControl
 */
#define emberAfFillCommandEnergy                                                                                                   \
    ManagementClusterReportEventStatus(issuerEventId, issuerEventId, eventStatus, deviceClass, eventStatusTime,                    \
                                       utilityEnrollmentGroup, criticalityLevelApplied, actionRequired,                            \
                                       coolingTemperatureSetPointApplied, heatingTemperatureSetPointApplied,                       \
                                       averageLoadAdjustmentPercentageApplied, dutyCycleApplied, eventControl)                     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_REPORT_EVENT_STATUS_COMMAND_ID, "uuuuuuuuuuuuu", issuerEventId, issuerEventId, eventStatus,  \
                                  deviceClass, eventStatusTime, utilityEnrollmentGroup, criticalityLevelApplied, actionRequired,   \
                                  coolingTemperatureSetPointApplied, heatingTemperatureSetPointApplied,                            \
                                  averageLoadAdjustmentPercentageApplied, dutyCycleApplied, eventControl);

/** @brief Command description for PublishCalendar
 *
 * Command: PublishCalendar
 * @param providerId INT32U
 * @param earliestStartTime UTC_TIME
 * @param issuerEventId INT32U
 * @param minIssuerEventId INT32U
 * @param issuerCalendarId INT32U
 * @param numberOfCalendars INT8U
 * @param startTime UTC_TIME
 * @param calendarType CalendarType
 * @param calendarType CalendarType
 * @param providerId INT32U
 * @param calendarTimeReference CalendarTimeReference
 * @param calendarName OCTET_STRING
 * @param numberOfSeasons INT8U
 * @param numberOfWeekProfiles INT8U
 * @param numberOfDayProfiles INT8U
 */
#define emberAfFillCommandCalendarClusterPublishCalendar(                                                                          \
    providerId, earliestStartTime, issuerEventId, minIssuerEventId, issuerCalendarId, numberOfCalendars, startTime, calendarType,  \
    calendarType, providerId, calendarTimeReference, calendarName, numberOfSeasons, numberOfWeekProfiles, numberOfDayProfiles)     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_CALENDAR_COMMAND_ID, "uuuuuuuuuuuuuuu", providerId, earliestStartTime, issuerEventId,    \
                              minIssuerEventId, issuerCalendarId, numberOfCalendars, startTime, calendarType, calendarType,        \
                              providerId, calendarTimeReference, calendarName, numberOfSeasons, numberOfWeekProfiles,              \
                              numberOfDayProfiles);

/** @brief Command description for PublishDayProfile
 *
 * Command: PublishDayProfile
 * @param providerId INT32U
 * @param providerId INT32U
 * @param issuerEventId INT32U
 * @param issuerCalendarId INT32U
 * @param issuerCalendarId INT32U
 * @param startDayId INT8U
 * @param dayId INT8U
 * @param numberOfDays INT8U
 * @param totalNumberOfScheduleEntries INT8U
 * @param commandIndex INT8U
 * @param totalNumberOfCommands INT8U
 * @param calendarType CalendarType
 * @param dayScheduleEntries ScheduleEntry []
 * @param dayScheduleEntriesLen int
 */
#define emberAfFillCommandCalendarClusterPublishDayProfile(                                                                        \
    providerId, providerId, issuerEventId, issuerCalendarId, issuerCalendarId, startDayId, dayId, numberOfDays,                    \
    totalNumberOfScheduleEntries, commandIndex, totalNumberOfCommands, calendarType, dayScheduleEntries, dayScheduleEntriesLen)    \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_DAY_PROFILE_COMMAND_ID, "uuuuuuuuuuuub", providerId, providerId, issuerEventId,          \
                              issuerCalendarId, issuerCalendarId, startDayId, dayId, numberOfDays, totalNumberOfScheduleEntries,   \
                              commandIndex, totalNumberOfCommands, calendarType, dayScheduleEntries, dayScheduleEntriesLen);

/** @brief Command description for PublishWeekProfile
 *
 * Command: PublishWeekProfile
 * @param providerId INT32U
 * @param providerId INT32U
 * @param issuerEventId INT32U
 * @param issuerCalendarId INT32U
 * @param issuerCalendarId INT32U
 * @param startWeekId INT8U
 * @param weekId INT8U
 * @param numberOfWeeks INT8U
 * @param dayIdRefMonday INT8U
 * @param dayIdRefTuesday INT8U
 * @param dayIdRefWednesday INT8U
 * @param dayIdRefThursday INT8U
 * @param dayIdRefFriday INT8U
 * @param dayIdRefSaturday INT8U
 * @param dayIdRefSunday INT8U
 */
#define emberAfFillCommandCalendarClusterPublishWeekProfile(                                                                       \
    providerId, providerId, issuerEventId, issuerCalendarId, issuerCalendarId, startWeekId, weekId, numberOfWeeks, dayIdRefMonday, \
    dayIdRefTuesday, dayIdRefWednesday, dayIdRefThursday, dayIdRefFriday, dayIdRefSaturday, dayIdRefSunday)                        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_WEEK_PROFILE_COMMAND_ID, "uuuuuuuuuuuuuuu", providerId, providerId, issuerEventId,       \
                              issuerCalendarId, issuerCalendarId, startWeekId, weekId, numberOfWeeks, dayIdRefMonday,              \
                              dayIdRefTuesday, dayIdRefWednesday, dayIdRefThursday, dayIdRefFriday, dayIdRefSaturday,              \
                              dayIdRefSunday);

/** @brief Command description for PublishSeasons
 *
 * Command: PublishSeasons
 * @param providerId INT32U
 * @param providerId INT32U
 * @param issuerEventId INT32U
 * @param issuerCalendarId INT32U
 * @param issuerCalendarId INT32U
 * @param commandIndex INT8U
 * @param totalNumberOfCommands INT8U
 * @param seasonEntries SeasonEntry []
 * @param seasonEntriesLen int
 */
#define emberAfFillCommandCalendarClusterPublishSeasons(providerId, providerId, issuerEventId, issuerCalendarId, issuerCalendarId, \
                                                        commandIndex, totalNumberOfCommands, seasonEntries, seasonEntriesLen)      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_SEASONS_COMMAND_ID, "uuuuuuub", providerId, providerId, issuerEventId, issuerCalendarId, \
                              issuerCalendarId, commandIndex, totalNumberOfCommands, seasonEntries, seasonEntriesLen);

/** @brief Command description for PublishSpecialDays
 *
 * Command: PublishSpecialDays
 * @param providerId INT32U
 * @param startTime UTC_TIME
 * @param issuerEventId INT32U
 * @param numberOfEvents INT8U
 * @param issuerCalendarId INT32U
 * @param calendarType CalendarType
 * @param startTime UTC_TIME
 * @param providerId INT32U
 * @param calendarType CalendarType
 * @param issuerCalendarId INT32U
 * @param totalNumberOfSpecialDays INT8U
 * @param commandIndex INT8U
 * @param totalNumberOfCommands INT8U
 * @param specialDayEntries SpecialDay []
 * @param specialDayEntriesLen int
 */
#define emberAfFillCommandCalendarClusterPublishSpecialDays(                                                                       \
    providerId, startTime, issuerEventId, numberOfEvents, issuerCalendarId, calendarType, startTime, providerId, calendarType,     \
    issuerCalendarId, totalNumberOfSpecialDays, commandIndex, totalNumberOfCommands, specialDayEntries, specialDayEntriesLen)      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUBLISH_SPECIAL_DAYS_COMMAND_ID, "uuuuuuuuuuuuub", providerId, startTime, issuerEventId,         \
                              numberOfEvents, issuerCalendarId, calendarType, startTime, providerId, calendarType,                 \
                              issuerCalendarId, totalNumberOfSpecialDays, commandIndex, totalNumberOfCommands, specialDayEntries,  \
                              specialDayEntriesLen);

/** @brief Command description for GetCalendarCancellation
 *
 * Command: GetCalendarCancellation
 * @param providerId INT32U
 * @param issuerCalendarId INT32U
 * @param calendarType CalendarType
 */
#define emberAfFillCommandCalendarClusterGetCalendarCancellation(providerId, issuerCalendarId, calendarType)                       \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_CALENDAR_CANCELLATION_COMMAND_ID, "uuu", providerId, issuerCalendarId, calendarType);

/** @brief Command description for GetChangeOfTenancy
 *
 * Command: GetChangeOfTenancy
 * @param providerId INT32U
 * @param issuerEventId INT32U
 * @param tariffType TariffType
 * @param implementationDateTime UTC_TIME
 * @param proposedTenancyChangeControl ProposedChangeControl
 */
#define emberAfFillCommandDevice                                                                                                   \
    ManagementClusterGetChangeOfTenancy(providerId, issuerEventId, tariffType, implementationDateTime,                             \
                                        proposedTenancyChangeControl)                                                              \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_CHANGE_OF_TENANCY_COMMAND_ID, "uuuuu", providerId, issuerEventId, tariffType,            \
                                  implementationDateTime, proposedTenancyChangeControl);

/** @brief Command description for GetChangeOfSupplier
 *
 * Command: GetChangeOfSupplier
 * @param currentProviderId INT32U
 * @param issuerEventId INT32U
 * @param tariffType TariffType
 * @param proposedProviderId INT32U
 * @param providerChangeImplementationTime UTC_TIME
 * @param providerChangeControl ProposedChangeControl
 * @param proposedProviderName OCTET_STRING
 * @param proposedProviderContactDetails OCTET_STRING
 */
#define emberAfFillCommandDevice                                                                                                   \
    ManagementClusterGetChangeOfSupplier(currentProviderId, issuerEventId, tariffType, proposedProviderId,                         \
                                         providerChangeImplementationTime, providerChangeControl, proposedProviderName,            \
                                         proposedProviderContactDetails)                                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_CHANGE_OF_SUPPLIER_COMMAND_ID, "uuuuuuuu", currentProviderId, issuerEventId, tariffType, \
                                  proposedProviderId, providerChangeImplementationTime, providerChangeControl,                     \
                                  proposedProviderName, proposedProviderContactDetails);

/** @brief Command description for RequestNewPassword
 *
 * Command: RequestNewPassword
 * @param passwordType PasswordType
 * @param issuerEventId INT32U
 * @param implementationDateTime UTC_TIME
 * @param durationInMinutes INT16U
 * @param passwordType PasswordType
 * @param password OCTET_STRING
 */
#define emberAfFillCommandDevice                                                                                                   \
    ManagementClusterRequestNewPassword(passwordType, issuerEventId, implementationDateTime, durationInMinutes, passwordType,      \
                                        password)                                                                                  \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_REQUEST_NEW_PASSWORD_COMMAND_ID, "uuuuuu", passwordType, issuerEventId,                      \
                                  implementationDateTime, durationInMinutes, passwordType, password);

/** @brief Command description for GetSiteId
 *
 * Command: GetSiteId
 * @param issuerEventId INT32U
 * @param siteIdTime UTC_TIME
 * @param providerId INT32U
 * @param siteId OCTET_STRING
 */
#define emberAfFillCommandDevice                                                                                                   \
    ManagementClusterGetSiteId(issuerEventId, siteIdTime, providerId, siteId)                                                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_SITE_ID_COMMAND_ID, "uuuu", issuerEventId, siteIdTime, providerId, siteId);

/** @brief Command description for ReportEventConfiguration
 *
 * Command: ReportEventConfiguration
 * @param commandIndex INT8U
 * @param issuerEventId INT32U
 * @param totalCommands INT8U
 * @param startDateTime UTC_TIME
 * @param eventConfigurationPayload EventConfigurationPayload []
 * @param eventConfigurationPayloadLen int
 * @param eventConfiguration EventConfiguration
 * @param configurationControl EventConfigurationControl
 * @param eventConfigurationPayload INT8U []
 * @param eventConfigurationPayloadLen int
 */
#define emberAfFillCommandDevice                                                                                                   \
    ManagementClusterReportEventConfiguration(commandIndex, issuerEventId, totalCommands, startDateTime,                           \
                                              eventConfigurationPayload, eventConfigurationPayloadLen, eventConfiguration,         \
                                              configurationControl, eventConfigurationPayload, eventConfigurationPayloadLen)       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_REPORT_EVENT_CONFIGURATION_COMMAND_ID, "uuuubuub", commandIndex, issuerEventId,              \
                                  totalCommands, startDateTime, eventConfigurationPayload, eventConfigurationPayloadLen,           \
                                  eventConfiguration, configurationControl, eventConfigurationPayload,                             \
                                  eventConfigurationPayloadLen);

/** @brief Command description for GetCIN
 *
 * Command: GetCIN
 * @param eventId INT16U
 */
#define emberAfFillCommandDevice                                                                                                   \
    ManagementClusterGetCIN(eventId) emberAfFillExternalBuffer(mask,                                                               \
                                                                                                                                   \
                                                               ZCL_GET_CIN_COMMAND_ID, "u", eventId);

/** @brief Command description for UpdateCIN
 *
 * Command: UpdateCIN
 * @param issuerEventId INT32U
 * @param implementationTime UTC_TIME
 * @param providerId INT32U
 * @param customerIdNumber OCTET_STRING
 */
#define emberAfFillCommandDevice                                                                                                   \
    ManagementClusterUpdateCIN(issuerEventId, implementationTime, providerId, customerIdNumber) emberAfFillExternalBuffer(         \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_UPDATE_CIN_COMMAND_ID, "uuuu", issuerEventId, implementationTime, providerId, customerIdNumber);

/** @brief Command description for GetEventLog
 *
 * Command: GetEventLog
 * @param eventControlLogId EventControlLogId
 * @param logId EventLogId
 * @param eventId INT16U
 * @param eventId INT16U
 * @param startTime UTC_TIME
 * @param eventTime UTC_TIME
 * @param endTime UTC_TIME
 * @param eventControl EventActionControl
 * @param numberOfEvents INT8U
 * @param eventData OCTET_STRING
 * @param eventOffset INT16U
 */
#define emberAfFillCommandEventsClusterGetEventLog(eventControlLogId, logId, eventId, eventId, startTime, eventTime, endTime,      \
                                                   eventControl, numberOfEvents, eventData, eventOffset)                           \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_EVENT_LOG_COMMAND_ID, "uuuuuuuuuuu", eventControlLogId, logId, eventId, eventId, startTime,  \
                              eventTime, endTime, eventControl, numberOfEvents, eventData, eventOffset);

/** @brief Command description for ClearEventLogRequest
 *
 * Command: ClearEventLogRequest
 * @param logId EventLogId
 * @param totalNumberOfEvents INT16U
 * @param commandIndex INT8U
 * @param totalCommands INT8U
 * @param logPayloadControl NumberOfEventsLogPayloadControl
 * @param logPayload EventLogPayload []
 * @param logPayloadLen int
 */
#define emberAfFillCommandEventsClusterClearEventLogRequest(logId, totalNumberOfEvents, commandIndex, totalCommands,               \
                                                            logPayloadControl, logPayload, logPayloadLen)                          \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CLEAR_EVENT_LOG_REQUEST_COMMAND_ID, "uuuuub", logId, totalNumberOfEvents, commandIndex,          \
                              totalCommands, logPayloadControl, logPayload, logPayloadLen);

/** @brief Command description for ClearEventLogResponse
 *
 * Command: ClearEventLogResponse
 * @param clearedEventsLogs ClearedEventsLogs
 */
#define emberAfFillCommandEventsClusterClearEventLogResponse(clearedEventsLogs)                                                    \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CLEAR_EVENT_LOG_RESPONSE_COMMAND_ID, "u", clearedEventsLogs);

/** @brief Command description for PairingResponse
 *
 * Command: PairingResponse
 * @param pairingInformationVersion INT32U
 * @param localPairingInformationVersion INT32U
 * @param totalNumberOfDevices INT8U
 * @param eui64OfRequestingDevice IEEE_ADDRESS
 * @param commandIndex INT8U
 * @param totalNumberOfCommands INT8U
 * @param eui64s IEEE_ADDRESS []
 * @param eui64sLen int
 */
#define emberAfFillCommandMDU                                                                                                      \
    PairingClusterPairingResponse(pairingInformationVersion, localPairingInformationVersion, totalNumberOfDevices,                 \
                                  eui64OfRequestingDevice, commandIndex, totalNumberOfCommands, eui64s, eui64sLen)                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_PAIRING_RESPONSE_COMMAND_ID, "uuuuuub", pairingInformationVersion,                           \
                                  localPairingInformationVersion, totalNumberOfDevices, eui64OfRequestingDevice, commandIndex,     \
                                  totalNumberOfCommands, eui64s, eui64sLen);

/** @brief Command description for GetSuspendZclMessagesStatus
 *
 * Command: GetSuspendZclMessagesStatus
 * @param period INT8U
 */
#define emberAfFillCommandSub                                                                                                      \
    -GHzClusterGetSuspendZclMessagesStatus(period)                                                                                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_SUSPEND_ZCL_MESSAGES_STATUS_COMMAND_ID, "u", period);

/** @brief Command description for InitiateKeyEstablishmentRequest
 *
 * Command: InitiateKeyEstablishmentRequest
 * @param keyEstablishmentSuite BITMAP16
 * @param requestedKeyEstablishmentSuite BITMAP16
 * @param ephemeralDataGenerateTime INT8U
 * @param ephemeralDataGenerateTime INT8U
 * @param confirmKeyGenerateTime INT8U
 * @param confirmKeyGenerateTime INT8U
 * @param identity Identity
 * @param identity Identity
 */
#define emberAfFillCommandKey                                                                                                      \
    EstablishmentClusterInitiateKeyEstablishmentRequest(keyEstablishmentSuite, requestedKeyEstablishmentSuite,                     \
                                                        ephemeralDataGenerateTime, ephemeralDataGenerateTime,                      \
                                                        confirmKeyGenerateTime, confirmKeyGenerateTime, identity, identity)        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID, "uuuuuuuu", keyEstablishmentSuite,            \
                                  requestedKeyEstablishmentSuite, ephemeralDataGenerateTime, ephemeralDataGenerateTime,            \
                                  confirmKeyGenerateTime, confirmKeyGenerateTime, identity, identity);

/** @brief Command description for EphemeralDataRequest
 *
 * Command: EphemeralDataRequest
 * @param ephemeralData EphemeralData
 * @param ephemeralData EphemeralData
 */
#define emberAfFillCommandKey                                                                                                      \
    EstablishmentClusterEphemeralDataRequest(ephemeralData, ephemeralData)                                                         \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_EPHEMERAL_DATA_REQUEST_COMMAND_ID, "uu", ephemeralData, ephemeralData);

/** @brief Command description for ConfirmKeyDataRequest
 *
 * Command: ConfirmKeyDataRequest
 * @param secureMessageAuthenticationCode Smac
 * @param secureMessageAuthenticationCode Smac
 */
#define emberAfFillCommandKey                                                                                                      \
    EstablishmentClusterConfirmKeyDataRequest(secureMessageAuthenticationCode, secureMessageAuthenticationCode)                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_CONFIRM_KEY_DATA_REQUEST_COMMAND_ID, "uu", secureMessageAuthenticationCode,                  \
                                  secureMessageAuthenticationCode);

/** @brief Command description for TerminateKeyEstablishment
 *
 * Command: TerminateKeyEstablishment
 * @param statusCode AmiKeyEstablishmentStatus
 * @param waitTime INT8U
 * @param keyEstablishmentSuite BITMAP16
 */
#define emberAfFillCommandKey                                                                                                      \
    EstablishmentClusterTerminateKeyEstablishment(statusCode, waitTime, keyEstablishmentSuite)                                     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID, "uuu", statusCode, waitTime, keyEstablishmentSuite);

/** @brief Command description for RequestInformation
 *
 * Command: RequestInformation
 * @param inquiryId ENUM8
 * @param number INT8U
 * @param dataTypeId BITMAP8
 * @param buffer INT8U []
 * @param bufferLen int
 * @param requestInformationPayload INT8U []
 * @param requestInformationPayloadLen int
 */
#define emberAfFillCommandInformationClusterRequestInformation(inquiryId, number, dataTypeId, buffer, bufferLen,                   \
                                                               requestInformationPayload, requestInformationPayloadLen)            \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_REQUEST_INFORMATION_COMMAND_ID, "uuubb", inquiryId, number, dataTypeId, buffer, bufferLen,       \
                              requestInformationPayload, requestInformationPayloadLen);

/** @brief Command description for PushInformationResponse
 *
 * Command: PushInformationResponse
 * @param notificationList Notification []
 * @param notificationListLen int
 * @param contents INT8U []
 * @param contentsLen int
 */
#define emberAfFillCommandInformationClusterPushInformationResponse(notificationList, notificationListLen, contents, contentsLen)  \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PUSH_INFORMATION_RESPONSE_COMMAND_ID, "bb", notificationList, notificationListLen, contents,     \
                              contentsLen);

/** @brief Command description for SendPreference
 *
 * Command: SendPreference
 * @param preferenceType INT16U
 * @param statusFeedbackList Status []
 * @param statusFeedbackListLen int
 * @param preferencePayload INT8U []
 * @param preferencePayloadLen int
 */
#define emberAfFillCommandInformationClusterSendPreference(preferenceType, statusFeedbackList, statusFeedbackListLen,              \
                                                           preferencePayload, preferencePayloadLen)                                \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SEND_PREFERENCE_COMMAND_ID, "ubb", preferenceType, statusFeedbackList, statusFeedbackListLen,    \
                              preferencePayload, preferencePayloadLen);

/** @brief Command description for ServerRequestPreference
 *
 * Command: ServerRequestPreference
 * @param statusFeedback Status
 * @param preferenceType INT16U
 * @param preferencePayload INT8U []
 * @param preferencePayloadLen int
 */
#define emberAfFillCommandInformationClusterServerRequestPreference(statusFeedback, preferenceType, preferencePayload,             \
                                                                    preferencePayloadLen)                                          \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SERVER_REQUEST_PREFERENCE_COMMAND_ID, "uub", statusFeedback, preferenceType, preferencePayload,  \
                              preferencePayloadLen);

/** @brief Command description for Update
 *
 * Command: Update
 * @param accessControl ENUM8
 * @param statusFeedbackList Status []
 * @param statusFeedbackListLen int
 * @param option BITMAP8
 * @param contents INT8U []
 * @param contentsLen int
 */
#define emberAfFillCommandInformationClusterUpdate(accessControl, statusFeedbackList, statusFeedbackListLen, option, contents,     \
                                                   contentsLen)                                                                    \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_UPDATE_COMMAND_ID, "ubub", accessControl, statusFeedbackList, statusFeedbackListLen, option,     \
                              contents, contentsLen);

/** @brief Command description for Delete
 *
 * Command: Delete
 * @param deletionOptions BITMAP8
 * @param notificationList Notification []
 * @param notificationListLen int
 * @param contentIds INT16U []
 * @param contentIdsLen int
 */
#define emberAfFillCommandInformationClusterDelete(deletionOptions, notificationList, notificationListLen, contentIds,             \
                                                   contentIdsLen)                                                                  \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_DELETE_COMMAND_ID, "ubb", deletionOptions, notificationList, notificationListLen, contentIds,    \
                              contentIdsLen);

/** @brief Command description for ConfigureNodeDescription
 *
 * Command: ConfigureNodeDescription
 * @param description CHAR_STRING
 * @param notificationList Notification []
 * @param notificationListLen int
 */
#define emberAfFillCommandInformationClusterConfigureNodeDescription(description, notificationList, notificationListLen)           \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CONFIGURE_NODE_DESCRIPTION_COMMAND_ID, "ub", description, notificationList,                      \
                              notificationListLen);

/** @brief Command description for ConfigureDeliveryEnable
 *
 * Command: ConfigureDeliveryEnable
 * @param enable BOOLEAN
 */
#define emberAfFillCommandInformationClusterConfigureDeliveryEnable(enable)                                                        \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CONFIGURE_DELIVERY_ENABLE_COMMAND_ID, "u", enable);

/** @brief Command description for ConfigurePushInformationTimer
 *
 * Command: ConfigurePushInformationTimer
 * @param timer INT32U
 */
#define emberAfFillCommandInformationClusterConfigurePushInformationTimer(timer)                                                   \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CONFIGURE_PUSH_INFORMATION_TIMER_COMMAND_ID, "u", timer);

/** @brief Command description for ConfigureSetRootId
 *
 * Command: ConfigureSetRootId
 * @param rootId INT16U
 */
#define emberAfFillCommandInformationClusterConfigureSetRootId(rootId)                                                             \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CONFIGURE_SET_ROOT_ID_COMMAND_ID, "u", rootId);

/** @brief Command description for ReadFileRequest
 *
 * Command: ReadFileRequest
 * @param fileIndex INT16U
 * @param writeOptions BITMAP8
 * @param fileStartPositionAndRequestedOctetCount INT8U []
 * @param fileStartPositionAndRequestedOctetCountLen int
 * @param fileSize INT8U []
 * @param fileSizeLen int
 */
#define emberAfFillCommandData                                                                                                     \
    SharingClusterReadFileRequest(fileIndex, writeOptions, fileStartPositionAndRequestedOctetCount,                                \
                                  fileStartPositionAndRequestedOctetCountLen, fileSize, fileSizeLen)                               \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_READ_FILE_REQUEST_COMMAND_ID, "uubb", fileIndex, writeOptions,                               \
                                  fileStartPositionAndRequestedOctetCount, fileStartPositionAndRequestedOctetCountLen, fileSize,   \
                                  fileSizeLen);

/** @brief Command description for ReadRecordRequest
 *
 * Command: ReadRecordRequest
 * @param fileIndex INT16U
 * @param fileIndex INT16U
 * @param fileStartRecordAndRequestedRecordCount INT8U []
 * @param fileStartRecordAndRequestedRecordCountLen int
 * @param fileStartPosition INT32U
 * @param octetCount INT32U
 */
#define emberAfFillCommandData                                                                                                     \
    SharingClusterReadRecordRequest(fileIndex, fileIndex, fileStartRecordAndRequestedRecordCount,                                  \
                                    fileStartRecordAndRequestedRecordCountLen, fileStartPosition, octetCount)                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_READ_RECORD_REQUEST_COMMAND_ID, "uubuu", fileIndex, fileIndex,                               \
                                  fileStartRecordAndRequestedRecordCount, fileStartRecordAndRequestedRecordCountLen,               \
                                  fileStartPosition, octetCount);

/** @brief Command description for WriteFileResponse
 *
 * Command: WriteFileResponse
 * @param status ENUM8
 * @param fileIndex INT16U
 * @param fileIndex INT8U []
 * @param fileIndexLen int
 * @param fileStartRecord INT16U
 * @param recordCount INT16U
 */
#define emberAfFillCommandData                                                                                                     \
    SharingClusterWriteFileResponse(status, fileIndex, fileIndex, fileIndexLen, fileStartRecord, recordCount)                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_WRITE_FILE_RESPONSE_COMMAND_ID, "uubuu", status, fileIndex, fileIndex, fileIndexLen,         \
                                  fileStartRecord, recordCount);

/** @brief Command description for FileTransmission
 *
 * Command: FileTransmission
 * @param transmitOptions BITMAP8
 * @param buffer INT8U []
 * @param bufferLen int
 */
#define emberAfFillCommandData                                                                                                     \
    SharingClusterFileTransmission(transmitOptions, buffer, bufferLen)                                                             \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_FILE_TRANSMISSION_COMMAND_ID, "ub", transmitOptions, buffer, bufferLen);

/** @brief Command description for RecordTransmission
 *
 * Command: RecordTransmission
 * @param transmitOptions BITMAP8
 * @param buffer INT8U []
 * @param bufferLen int
 */
#define emberAfFillCommandData                                                                                                     \
    SharingClusterRecordTransmission(transmitOptions, buffer, bufferLen)                                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_RECORD_TRANSMISSION_COMMAND_ID, "ub", transmitOptions, buffer, bufferLen);

/** @brief Command description for SearchGame
 *
 * Command: SearchGame
 * @param specificGame ENUM8
 * @param gameId INT16U
 * @param gameId INT16U
 * @param gameMaster BOOLEAN
 * @param listOfGame CHAR_STRING
 */
#define emberAfFillCommandGamingClusterSearchGame(specificGame, gameId, gameId, gameMaster, listOfGame)                            \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SEARCH_GAME_COMMAND_ID, "uuuuu", specificGame, gameId, gameId, gameMaster, listOfGame);

/** @brief Command description for JoinGame
 *
 * Command: JoinGame
 * @param gameId INT16U
 * @param commandId INT8U
 * @param joinAsMaster BOOLEAN
 * @param status BITMAP8
 * @param nameOfGame CHAR_STRING
 * @param message CHAR_STRING
 */
#define emberAfFillCommandGamingClusterJoinGame(gameId, commandId, joinAsMaster, status, nameOfGame, message)                      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_JOIN_GAME_COMMAND_ID, "uuuuuu", gameId, commandId, joinAsMaster, status, nameOfGame, message);

/** @brief Command description for StartGame
 *
 * Command: StartGame
 */
#define emberAfFillCommandGamingClusterStartGame()                                                                                 \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_START_GAME_COMMAND_ID, "", );

/** @brief Command description for PauseGame
 *
 * Command: PauseGame
 */
#define emberAfFillCommandGamingClusterPauseGame()                                                                                 \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PAUSE_GAME_COMMAND_ID, "", );

/** @brief Command description for ResumeGame
 *
 * Command: ResumeGame
 */
#define emberAfFillCommandGamingClusterResumeGame()                                                                                \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_RESUME_GAME_COMMAND_ID, "", );

/** @brief Command description for QuitGame
 *
 * Command: QuitGame
 */
#define emberAfFillCommandGamingClusterQuitGame()                                                                                  \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_QUIT_GAME_COMMAND_ID, "", );

/** @brief Command description for EndGame
 *
 * Command: EndGame
 */
#define emberAfFillCommandGamingClusterEndGame()                                                                                   \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_END_GAME_COMMAND_ID, "", );

/** @brief Command description for StartOver
 *
 * Command: StartOver
 */
#define emberAfFillCommandGamingClusterStartOver()                                                                                 \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_START_OVER_COMMAND_ID, "", );

/** @brief Command description for ActionControl
 *
 * Command: ActionControl
 * @param actions BITMAP32
 */
#define emberAfFillCommandGamingClusterActionControl(actions)                                                                      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_ACTION_CONTROL_COMMAND_ID, "u", actions);

/** @brief Command description for DownloadGame
 *
 * Command: DownloadGame
 */
#define emberAfFillCommandGamingClusterDownloadGame()                                                                              \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_DOWNLOAD_GAME_COMMAND_ID, "", );

/** @brief Command description for PathCreation
 *
 * Command: PathCreation
 * @param originatorAddress DATA16
 * @param originatorAddress DATA16
 * @param destinationAddress DATA16
 * @param destinationAddress DATA16
 * @param dataRate INT8U
 * @param dataRate INT8U
 */
#define emberAfFillCommandData                                                                                                     \
    Rate ControlClusterPathCreation(originatorAddress, originatorAddress, destinationAddress, destinationAddress, dataRate,        \
                                    dataRate)                                                                                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_PATH_CREATION_COMMAND_ID, "uuuuuu", originatorAddress, originatorAddress,                    \
                                  destinationAddress, destinationAddress, dataRate, dataRate);

/** @brief Command description for DataRateNotification
 *
 * Command: DataRateNotification
 * @param originatorAddress DATA16
 * @param destinationAddress DATA16
 * @param dataRate INT8U
 */
#define emberAfFillCommandData                                                                                                     \
    Rate ControlClusterDataRateNotification(originatorAddress, destinationAddress, dataRate)                                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DATA_RATE_NOTIFICATION_COMMAND_ID, "uuu", originatorAddress, destinationAddress, dataRate);

/** @brief Command description for PathDeletion
 *
 * Command: PathDeletion
 * @param originatorAddress DATA16
 * @param destinationAddress DATA16
 */
#define emberAfFillCommandData                                                                                                     \
    Rate ControlClusterPathDeletion(originatorAddress, destinationAddress)                                                         \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_PATH_DELETION_COMMAND_ID, "uu", originatorAddress, destinationAddress);

/** @brief Command description for EstablishmentRequest
 *
 * Command: EstablishmentRequest
 * @param flag BITMAP8
 * @param ackNack ENUM8
 * @param codecType ENUM8
 * @param codecType ENUM8
 * @param sampFreq ENUM8
 * @param codecRate ENUM8
 * @param serviceType ENUM8
 * @param buffer INT8U []
 * @param bufferLen int
 */
#define emberAfFillCommandVoice                                                                                                    \
    over ZigBeeClusterEstablishmentRequest(flag, ackNack, codecType, codecType, sampFreq, codecRate, serviceType, buffer,          \
                                           bufferLen)                                                                              \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ESTABLISHMENT_REQUEST_COMMAND_ID, "uuuuuuub", flag, ackNack, codecType, codecType, sampFreq, \
                                  codecRate, serviceType, buffer, bufferLen);

/** @brief Command description for VoiceTransmission
 *
 * Command: VoiceTransmission
 * @param voiceData INT8U []
 * @param voiceDataLen int
 * @param sequenceNumber INT8U
 * @param errorFlag ENUM8
 */
#define emberAfFillCommandVoice                                                                                                    \
    over ZigBeeClusterVoiceTransmission(voiceData, voiceDataLen, sequenceNumber, errorFlag)                                        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_VOICE_TRANSMISSION_COMMAND_ID, "buu", voiceData, voiceDataLen, sequenceNumber, errorFlag);

/** @brief Command description for VoiceTransmissionCompletion
 *
 * Command: VoiceTransmissionCompletion
 * @param controlType ENUM8
 */
#define emberAfFillCommandVoice                                                                                                    \
    over ZigBeeClusterVoiceTransmissionCompletion(controlType)                                                                     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_VOICE_TRANSMISSION_COMPLETION_COMMAND_ID, "u", controlType);

/** @brief Command description for ControlResponse
 *
 * Command: ControlResponse
 * @param ackNack ENUM8
 */
#define emberAfFillCommandVoice                                                                                                    \
    over ZigBeeClusterControlResponse(ackNack) emberAfFillExternalBuffer(mask,                                                     \
                                                                                                                                   \
                                                                         ZCL_CONTROL_RESPONSE_COMMAND_ID, "u", ackNack);

/** @brief Command description for JoinChatRequest
 *
 * Command: JoinChatRequest
 * @param uid INT16U
 * @param status ENUM8
 * @param nickname CHAR_STRING
 * @param cid INT16U
 * @param cid INT16U
 */
#define emberAfFillCommandChattingClusterJoinChatRequest(uid, status, nickname, cid, cid)                                          \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_JOIN_CHAT_REQUEST_COMMAND_ID, "uuuuu", uid, status, nickname, cid, cid);

/** @brief Command description for LeaveChatRequest
 *
 * Command: LeaveChatRequest
 * @param cid INT16U
 * @param status ENUM8
 * @param uid INT16U
 * @param cid INT16U
 * @param chatParticipantList ChatParticipant []
 * @param chatParticipantListLen int
 */
#define emberAfFillCommandChattingClusterLeaveChatRequest(cid, status, uid, cid, chatParticipantList, chatParticipantListLen)      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_LEAVE_CHAT_REQUEST_COMMAND_ID, "uuuub", cid, status, uid, cid, chatParticipantList,              \
                              chatParticipantListLen);

/** @brief Command description for SearchChatRequest
 *
 * Command: SearchChatRequest
 * @param cid INT16U
 * @param uid INT16U
 * @param nickname CHAR_STRING
 */
#define emberAfFillCommandChattingClusterSearchChatRequest(cid, uid, nickname)                                                     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SEARCH_CHAT_REQUEST_COMMAND_ID, "uuu", cid, uid, nickname);

/** @brief Command description for SwitchChairmanResponse
 *
 * Command: SwitchChairmanResponse
 * @param cid INT16U
 * @param cid INT16U
 * @param uid INT16U
 * @param uid INT16U
 * @param nickname CHAR_STRING
 */
#define emberAfFillCommandChattingClusterSwitchChairmanResponse(cid, cid, uid, uid, nickname)                                      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SWITCH_CHAIRMAN_RESPONSE_COMMAND_ID, "uuuuu", cid, cid, uid, uid, nickname);

/** @brief Command description for StartChatRequest
 *
 * Command: StartChatRequest
 * @param name CHAR_STRING
 * @param options BITMAP8
 * @param uid INT16U
 * @param chatRoomList ChatRoom []
 * @param chatRoomListLen int
 * @param nickname CHAR_STRING
 */
#define emberAfFillCommandChattingClusterStartChatRequest(name, options, uid, chatRoomList, chatRoomListLen, nickname)             \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_START_CHAT_REQUEST_COMMAND_ID, "uuubu", name, options, uid, chatRoomList, chatRoomListLen,       \
                              nickname);

/** @brief Command description for ChatMessage
 *
 * Command: ChatMessage
 * @param destinationUid INT16U
 * @param cid INT16U
 * @param sourceUid INT16U
 * @param cid INT16U
 * @param nickname CHAR_STRING
 * @param message CHAR_STRING
 */
#define emberAfFillCommandChattingClusterChatMessage(destinationUid, cid, sourceUid, cid, nickname, message)                       \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_CHAT_MESSAGE_COMMAND_ID, "uuuuuu", destinationUid, cid, sourceUid, cid, nickname, message);

/** @brief Command description for GetNodeInformationRequest
 *
 * Command: GetNodeInformationRequest
 * @param cid INT16U
 * @param cid INT16U
 * @param uid INT16U
 * @param nodeInformationList NodeInformation []
 * @param nodeInformationListLen int
 */
#define emberAfFillCommandChattingClusterGetNodeInformationRequest(cid, cid, uid, nodeInformationList, nodeInformationListLen)     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_NODE_INFORMATION_REQUEST_COMMAND_ID, "uuub", cid, cid, uid, nodeInformationList,             \
                              nodeInformationListLen);

/** @brief Command description for SwitchChairmanNotification
 *
 * Command: SwitchChairmanNotification
 * @param cid INT16U
 * @param uid INT16U
 * @param address DATA16
 * @param endpoint INT8U
 */
#define emberAfFillCommandChattingClusterSwitchChairmanNotification(cid, uid, address, endpoint)                                   \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SWITCH_CHAIRMAN_NOTIFICATION_COMMAND_ID, "uuuu", cid, uid, address, endpoint);

/** @brief Command description for GetNodeInformationResponse
 *
 * Command: GetNodeInformationResponse
 * @param status ENUM8
 * @param cid INT16U
 * @param uid INT16U
 * @param addressEndpointAndNickname INT8U []
 * @param addressEndpointAndNicknameLen int
 */
#define emberAfFillCommandChattingClusterGetNodeInformationResponse(status, cid, uid, addressEndpointAndNickname,                  \
                                                                    addressEndpointAndNicknameLen)                                 \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_GET_NODE_INFORMATION_RESPONSE_COMMAND_ID, "uuub", status, cid, uid, addressEndpointAndNickname,  \
                              addressEndpointAndNicknameLen);

/** @brief Command description for BuyRequest
 *
 * Command: BuyRequest
 * @param userId OCTET_STRING
 * @param serialNumber OCTET_STRING
 * @param userType INT16U
 * @param currency INT32U
 * @param serviceId INT16U
 * @param priceTrailingDigit INT8U
 * @param goodId OCTET_STRING
 * @param price INT32U
 * @param timestamp OCTET_STRING
 * @param transId INT16U
 * @param transStatus ENUM8
 */
#define emberAfFillCommandPaymentClusterBuyRequest(userId, serialNumber, userType, currency, serviceId, priceTrailingDigit,        \
                                                   goodId, price, timestamp, transId, transStatus)                                 \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_BUY_REQUEST_COMMAND_ID, "uuuuuuuuuuu", userId, serialNumber, userType, currency, serviceId,      \
                              priceTrailingDigit, goodId, price, timestamp, transId, transStatus);

/** @brief Command description for AcceptPayment
 *
 * Command: AcceptPayment
 * @param userId OCTET_STRING
 * @param serialNumber OCTET_STRING
 * @param userType INT16U
 * @param currency INT32U
 * @param serviceId INT16U
 * @param priceTrailingDigit INT8U
 * @param goodId OCTET_STRING
 * @param price INT32U
 * @param timestamp OCTET_STRING
 */
#define emberAfFillCommandPaymentClusterAcceptPayment(userId, serialNumber, userType, currency, serviceId, priceTrailingDigit,     \
                                                      goodId, price, timestamp)                                                    \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_ACCEPT_PAYMENT_COMMAND_ID, "uuuuuuuuu", userId, serialNumber, userType, currency, serviceId,     \
                              priceTrailingDigit, goodId, price, timestamp);

/** @brief Command description for PaymentConfirm
 *
 * Command: PaymentConfirm
 * @param serialNumber OCTET_STRING
 * @param serialNumber OCTET_STRING
 * @param transId INT16U
 * @param status ENUM8
 * @param transStatus ENUM8
 */
#define emberAfFillCommandPaymentClusterPaymentConfirm(serialNumber, serialNumber, transId, status, transStatus)                   \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_PAYMENT_CONFIRM_COMMAND_ID, "uuuuu", serialNumber, serialNumber, transId, status, transStatus);

/** @brief Command description for Subscribe
 *
 * Command: Subscribe
 * @param userId OCTET_STRING
 * @param userId OCTET_STRING
 * @param serviceId INT16U
 * @param serviceId INT16U
 * @param serviceProviderId INT16U
 * @param serviceProviderId INT16U
 */
#define emberAfFillCommandBillingClusterSubscribe(userId, userId, serviceId, serviceId, serviceProviderId, serviceProviderId)      \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SUBSCRIBE_COMMAND_ID, "uuuuuu", userId, userId, serviceId, serviceId, serviceProviderId,         \
                              serviceProviderId);

/** @brief Command description for Unsubscribe
 *
 * Command: Unsubscribe
 * @param userId OCTET_STRING
 * @param userId OCTET_STRING
 * @param serviceId INT16U
 * @param serviceId INT16U
 * @param serviceProviderId INT16U
 * @param serviceProviderId INT16U
 * @param timestamp OCTET_STRING
 * @param duration INT16U
 */
#define emberAfFillCommandBillingClusterUnsubscribe(userId, userId, serviceId, serviceId, serviceProviderId, serviceProviderId,    \
                                                    timestamp, duration)                                                           \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_UNSUBSCRIBE_COMMAND_ID, "uuuuuuuu", userId, userId, serviceId, serviceId, serviceProviderId,     \
                              serviceProviderId, timestamp, duration);

/** @brief Command description for StartBillingSession
 *
 * Command: StartBillingSession
 * @param userId OCTET_STRING
 * @param serviceId INT16U
 * @param serviceProviderId INT16U
 */
#define emberAfFillCommandBillingClusterStartBillingSession(userId, serviceId, serviceProviderId)                                  \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_START_BILLING_SESSION_COMMAND_ID, "uuu", userId, serviceId, serviceProviderId);

/** @brief Command description for StopBillingSession
 *
 * Command: StopBillingSession
 * @param userId OCTET_STRING
 * @param serviceId INT16U
 * @param serviceProviderId INT16U
 */
#define emberAfFillCommandBillingClusterStopBillingSession(userId, serviceId, serviceProviderId)                                   \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_STOP_BILLING_SESSION_COMMAND_ID, "uuu", userId, serviceId, serviceProviderId);

/** @brief Command description for BillStatusNotification
 *
 * Command: BillStatusNotification
 * @param userId OCTET_STRING
 * @param status ENUM8
 */
#define emberAfFillCommandBillingClusterBillStatusNotification(userId, status)                                                     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_BILL_STATUS_NOTIFICATION_COMMAND_ID, "uu", userId, status);

/** @brief Command description for SessionKeepAlive
 *
 * Command: SessionKeepAlive
 * @param userId OCTET_STRING
 * @param serviceId INT16U
 * @param serviceProviderId INT16U
 */
#define emberAfFillCommandBillingClusterSessionKeepAlive(userId, serviceId, serviceProviderId)                                     \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_SESSION_KEEP_ALIVE_COMMAND_ID, "uuu", userId, serviceId, serviceProviderId);

/** @brief Command description for GetAlerts
 *
 * Command: GetAlerts
 * @param alertsCount AlertCount
 * @param alertStructures AlertStructure []
 * @param alertStructuresLen int
 */
#define emberAfFillCommandAppliance                                                                                                \
    Events and AlertClusterGetAlerts(alertsCount, alertStructures, alertStructuresLen)                                             \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_ALERTS_COMMAND_ID, "ub", alertsCount, alertStructures, alertStructuresLen);

/** @brief Command description for AlertsNotification
 *
 * Command: AlertsNotification
 * @param alertsCount AlertCount
 * @param alertStructures AlertStructure []
 * @param alertStructuresLen int
 */
#define emberAfFillCommandAppliance                                                                                                \
    Events and AlertClusterAlertsNotification(alertsCount, alertStructures, alertStructuresLen)                                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ALERTS_NOTIFICATION_COMMAND_ID, "ub", alertsCount, alertStructures, alertStructuresLen);

/** @brief Command description for EventsNotification
 *
 * Command: EventsNotification
 * @param eventHeader INT8U
 * @param eventId EventIdentification
 */
#define emberAfFillCommandAppliance                                                                                                \
    Events and AlertClusterEventsNotification(eventHeader, eventId)                                                                \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_EVENTS_NOTIFICATION_COMMAND_ID, "uu", eventHeader, eventId);

/** @brief Command description for LogNotification
 *
 * Command: LogNotification
 * @param timeStamp TIME_OF_DAY
 * @param logId INT32U
 * @param logId INT32U
 * @param logLength INT32U
 * @param logPayload INT8U []
 * @param logPayloadLen int
 */
#define emberAfFillCommandAppliance                                                                                                \
    StatisticsClusterLogNotification(timeStamp, logId, logId, logLength, logPayload, logPayloadLen) emberAfFillExternalBuffer(     \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_LOG_NOTIFICATION_COMMAND_ID, "uuuub", timeStamp, logId, logId, logLength, logPayload, logPayloadLen);

/** @brief Command description for LogQueueRequest
 *
 * Command: LogQueueRequest
 * @param timeStamp TIME_OF_DAY
 * @param logId INT32U
 * @param logLength INT32U
 * @param logPayload INT8U []
 * @param logPayloadLen int
 */
#define emberAfFillCommandAppliance                                                                                                \
    StatisticsClusterLogQueueRequest(timeStamp, logId, logLength, logPayload, logPayloadLen) emberAfFillExternalBuffer(            \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_LOG_QUEUE_REQUEST_COMMAND_ID, "uuub", timeStamp, logId, logLength, logPayload, logPayloadLen);

/** @brief Command description for LogQueueResponse
 *
 * Command: LogQueueResponse
 * @param logQueueSize INT8U
 * @param logIds INT32U []
 * @param logIdsLen int
 */
#define emberAfFillCommandAppliance                                                                                                \
    StatisticsClusterLogQueueResponse(logQueueSize, logIds, logIdsLen)                                                             \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_LOG_QUEUE_RESPONSE_COMMAND_ID, "ub", logQueueSize, logIds, logIdsLen);

/** @brief Command description for StatisticsAvailable
 *
 * Command: StatisticsAvailable
 * @param logQueueSize INT8U
 * @param logIds INT32U []
 * @param logIdsLen int
 */
#define emberAfFillCommandAppliance                                                                                                \
    StatisticsClusterStatisticsAvailable(logQueueSize, logIds, logIdsLen)                                                          \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_STATISTICS_AVAILABLE_COMMAND_ID, "ub", logQueueSize, logIds, logIdsLen);

/** @brief Command description for GetProfileInfoCommand
 *
 * Command: GetProfileInfoCommand
 * @param profileCount INT8U
 * @param profileIntervalPeriod ENUM8
 * @param maxNumberOfIntervals INT8U
 * @param listOfAttributes INT16U []
 * @param listOfAttributesLen int
 */
#define emberAfFillCommandElectrical                                                                                               \
    MeasurementClusterGetProfileInfoCommand(profileCount, profileIntervalPeriod, maxNumberOfIntervals, listOfAttributes,           \
                                            listOfAttributesLen)                                                                   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_PROFILE_INFO_COMMAND_COMMAND_ID, "uuub", profileCount, profileIntervalPeriod,            \
                                  maxNumberOfIntervals, listOfAttributes, listOfAttributesLen);

/** @brief Command description for GetMeasurementProfileResponseCommand
 *
 * Command: GetMeasurementProfileResponseCommand
 * @param startTime INT32U
 * @param attributeId INT16U
 * @param status ENUM8
 * @param startTime INT32U
 * @param profileIntervalPeriod ENUM8
 * @param numberOfIntervals ENUM8
 * @param numberOfIntervalsDelivered INT8U
 * @param attributeId INT16U
 * @param intervals INT8U []
 * @param intervalsLen int
 */
#define emberAfFillCommandElectrical                                                                                               \
    MeasurementClusterGetMeasurementProfileResponseCommand(startTime, attributeId, status, startTime, profileIntervalPeriod,       \
                                                           numberOfIntervals, numberOfIntervalsDelivered, attributeId, intervals,  \
                                                           intervalsLen)                                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_MEASUREMENT_PROFILE_RESPONSE_COMMAND_COMMAND_ID, "uuuuuuuub", startTime, attributeId,    \
                                  status, startTime, profileIntervalPeriod, numberOfIntervals, numberOfIntervalsDelivered,         \
                                  attributeId, intervals, intervalsLen);

/** @brief Command description for ScanRequest
 *
 * Command: ScanRequest
 * @param transaction INT32U
 * @param zigbeeInformation ZigbeeInformation
 * @param zllInformation ZllInformation
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterScanRequest(transaction, zigbeeInformation, zllInformation)                                                \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SCAN_REQUEST_COMMAND_ID, "uuu", transaction, zigbeeInformation, zllInformation);

/** @brief Command description for ScanResponse
 *
 * Command: ScanResponse
 * @param transaction INT32U
 * @param rssiCorrection INT8U
 * @param zigbeeInformation ZigbeeInformation
 * @param zllInformation ZllInformation
 * @param keyBitmask KeyBitmask
 * @param responseId INT32U
 * @param extendedPanId IEEE_ADDRESS
 * @param networkUpdateId INT8U
 * @param logicalChannel INT8U
 * @param panId INT16U
 * @param networkAddress INT16U
 * @param numberOfSubDevices INT8U
 * @param totalGroupIds INT8U
 * @param endpointId INT8U
 * @param profileId INT16U
 * @param deviceId INT16U
 * @param version INT8U
 * @param groupIdCount INT8U
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterScanResponse(transaction, rssiCorrection, zigbeeInformation, zllInformation, keyBitmask, responseId,       \
                                     extendedPanId, networkUpdateId, logicalChannel, panId, networkAddress, numberOfSubDevices,    \
                                     totalGroupIds, endpointId, profileId, deviceId, version, groupIdCount)                        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SCAN_RESPONSE_COMMAND_ID, "uuuuuuuuuuuuuuuuuu", transaction, rssiCorrection,                 \
                                  zigbeeInformation, zllInformation, keyBitmask, responseId, extendedPanId, networkUpdateId,       \
                                  logicalChannel, panId, networkAddress, numberOfSubDevices, totalGroupIds, endpointId, profileId, \
                                  deviceId, version, groupIdCount);

/** @brief Command description for DeviceInformationRequest
 *
 * Command: DeviceInformationRequest
 * @param transaction INT32U
 * @param startIndex INT8U
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterDeviceInformationRequest(transaction, startIndex)                                                          \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID, "uu", transaction, startIndex);

/** @brief Command description for DeviceInformationResponse
 *
 * Command: DeviceInformationResponse
 * @param transaction INT32U
 * @param numberOfSubDevices INT8U
 * @param startIndex INT8U
 * @param deviceInformationRecordCount INT8U
 * @param deviceInformationRecordList DeviceInformationRecord []
 * @param deviceInformationRecordListLen int
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterDeviceInformationResponse(transaction, numberOfSubDevices, startIndex, deviceInformationRecordCount,       \
                                                  deviceInformationRecordList, deviceInformationRecordListLen)                     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DEVICE_INFORMATION_RESPONSE_COMMAND_ID, "uuuub", transaction, numberOfSubDevices,            \
                                  startIndex, deviceInformationRecordCount, deviceInformationRecordList,                           \
                                  deviceInformationRecordListLen);

/** @brief Command description for IdentifyRequest
 *
 * Command: IdentifyRequest
 * @param transaction INT32U
 * @param identifyDuration INT16U
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterIdentifyRequest(transaction, identifyDuration)                                                             \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_IDENTIFY_REQUEST_COMMAND_ID, "uu", transaction, identifyDuration);

/** @brief Command description for ResetToFactoryNewRequest
 *
 * Command: ResetToFactoryNewRequest
 * @param transaction INT32U
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterResetToFactoryNewRequest(transaction)                                                                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_RESET_TO_FACTORY_NEW_REQUEST_COMMAND_ID, "u", transaction);

/** @brief Command description for NetworkStartRequest
 *
 * Command: NetworkStartRequest
 * @param transaction INT32U
 * @param extendedPanId IEEE_ADDRESS
 * @param keyIndex KeyIndex
 * @param encryptedNetworkKey SECURITY_KEY
 * @param logicalChannel INT8U
 * @param panId INT16U
 * @param networkAddress INT16U
 * @param groupIdentifiersBegin INT16U
 * @param groupIdentifiersEnd INT16U
 * @param freeNetworkAddressRangeBegin INT16U
 * @param freeNetworkAddressRangeEnd INT16U
 * @param freeGroupIdentifierRangeBegin INT16U
 * @param freeGroupIdentifierRangeEnd INT16U
 * @param initiatorIeeeAddress IEEE_ADDRESS
 * @param initiatorNetworkAddress INT16U
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterNetworkStartRequest(                                                                                       \
        transaction, extendedPanId, keyIndex, encryptedNetworkKey, logicalChannel, panId, networkAddress, groupIdentifiersBegin,   \
        groupIdentifiersEnd, freeNetworkAddressRangeBegin, freeNetworkAddressRangeEnd, freeGroupIdentifierRangeBegin,              \
        freeGroupIdentifierRangeEnd, initiatorIeeeAddress, initiatorNetworkAddress)                                                \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_NETWORK_START_REQUEST_COMMAND_ID, "uuuuuuuuuuuuuuu", transaction, extendedPanId, keyIndex,   \
                                  encryptedNetworkKey, logicalChannel, panId, networkAddress, groupIdentifiersBegin,               \
                                  groupIdentifiersEnd, freeNetworkAddressRangeBegin, freeNetworkAddressRangeEnd,                   \
                                  freeGroupIdentifierRangeBegin, freeGroupIdentifierRangeEnd, initiatorIeeeAddress,                \
                                  initiatorNetworkAddress);

/** @brief Command description for NetworkStartResponse
 *
 * Command: NetworkStartResponse
 * @param transaction INT32U
 * @param status ZllStatus
 * @param extendedPanId IEEE_ADDRESS
 * @param networkUpdateId INT8U
 * @param logicalChannel INT8U
 * @param panId INT16U
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterNetworkStartResponse(transaction, status, extendedPanId, networkUpdateId, logicalChannel, panId)           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_NETWORK_START_RESPONSE_COMMAND_ID, "uuuuuu", transaction, status, extendedPanId,             \
                                  networkUpdateId, logicalChannel, panId);

/** @brief Command description for NetworkJoinRouterRequest
 *
 * Command: NetworkJoinRouterRequest
 * @param transaction INT32U
 * @param extendedPanId IEEE_ADDRESS
 * @param keyIndex KeyIndex
 * @param encryptedNetworkKey SECURITY_KEY
 * @param networkUpdateId INT8U
 * @param logicalChannel INT8U
 * @param panId INT16U
 * @param networkAddress INT16U
 * @param groupIdentifiersBegin INT16U
 * @param groupIdentifiersEnd INT16U
 * @param freeNetworkAddressRangeBegin INT16U
 * @param freeNetworkAddressRangeEnd INT16U
 * @param freeGroupIdentifierRangeBegin INT16U
 * @param freeGroupIdentifierRangeEnd INT16U
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterNetworkJoinRouterRequest(transaction, extendedPanId, keyIndex, encryptedNetworkKey, networkUpdateId,       \
                                                 logicalChannel, panId, networkAddress, groupIdentifiersBegin,                     \
                                                 groupIdentifiersEnd, freeNetworkAddressRangeBegin, freeNetworkAddressRangeEnd,    \
                                                 freeGroupIdentifierRangeBegin, freeGroupIdentifierRangeEnd)                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_NETWORK_JOIN_ROUTER_REQUEST_COMMAND_ID, "uuuuuuuuuuuuuu", transaction, extendedPanId,        \
                                  keyIndex, encryptedNetworkKey, networkUpdateId, logicalChannel, panId, networkAddress,           \
                                  groupIdentifiersBegin, groupIdentifiersEnd, freeNetworkAddressRangeBegin,                        \
                                  freeNetworkAddressRangeEnd, freeGroupIdentifierRangeBegin, freeGroupIdentifierRangeEnd);

/** @brief Command description for NetworkJoinRouterResponse
 *
 * Command: NetworkJoinRouterResponse
 * @param transaction INT32U
 * @param status ZllStatus
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterNetworkJoinRouterResponse(transaction, status)                                                             \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_NETWORK_JOIN_ROUTER_RESPONSE_COMMAND_ID, "uu", transaction, status);

/** @brief Command description for NetworkJoinEndDeviceRequest
 *
 * Command: NetworkJoinEndDeviceRequest
 * @param transaction INT32U
 * @param extendedPanId IEEE_ADDRESS
 * @param keyIndex KeyIndex
 * @param encryptedNetworkKey SECURITY_KEY
 * @param networkUpdateId INT8U
 * @param logicalChannel INT8U
 * @param panId INT16U
 * @param networkAddress INT16U
 * @param groupIdentifiersBegin INT16U
 * @param groupIdentifiersEnd INT16U
 * @param freeNetworkAddressRangeBegin INT16U
 * @param freeNetworkAddressRangeEnd INT16U
 * @param freeGroupIdentifierRangeBegin INT16U
 * @param freeGroupIdentifierRangeEnd INT16U
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterNetworkJoinEndDeviceRequest(transaction, extendedPanId, keyIndex, encryptedNetworkKey, networkUpdateId,    \
                                                    logicalChannel, panId, networkAddress, groupIdentifiersBegin,                  \
                                                    groupIdentifiersEnd, freeNetworkAddressRangeBegin, freeNetworkAddressRangeEnd, \
                                                    freeGroupIdentifierRangeBegin, freeGroupIdentifierRangeEnd)                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_NETWORK_JOIN_END_DEVICE_REQUEST_COMMAND_ID, "uuuuuuuuuuuuuu", transaction, extendedPanId,    \
                                  keyIndex, encryptedNetworkKey, networkUpdateId, logicalChannel, panId, networkAddress,           \
                                  groupIdentifiersBegin, groupIdentifiersEnd, freeNetworkAddressRangeBegin,                        \
                                  freeNetworkAddressRangeEnd, freeGroupIdentifierRangeBegin, freeGroupIdentifierRangeEnd);

/** @brief Command description for NetworkJoinEndDeviceResponse
 *
 * Command: NetworkJoinEndDeviceResponse
 * @param transaction INT32U
 * @param status ZllStatus
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterNetworkJoinEndDeviceResponse(transaction, status)                                                          \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_NETWORK_JOIN_END_DEVICE_RESPONSE_COMMAND_ID, "uu", transaction, status);

/** @brief Command description for NetworkUpdateRequest
 *
 * Command: NetworkUpdateRequest
 * @param transaction INT32U
 * @param extendedPanId IEEE_ADDRESS
 * @param networkUpdateId INT8U
 * @param logicalChannel INT8U
 * @param panId INT16U
 * @param networkAddress INT16U
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterNetworkUpdateRequest(transaction, extendedPanId, networkUpdateId, logicalChannel, panId, networkAddress)   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_NETWORK_UPDATE_REQUEST_COMMAND_ID, "uuuuuu", transaction, extendedPanId, networkUpdateId,    \
                                  logicalChannel, panId, networkAddress);

/** @brief Command description for EndpointInformation
 *
 * Command: EndpointInformation
 * @param ieeeAddress IEEE_ADDRESS
 * @param networkAddress INT16U
 * @param endpointId INT8U
 * @param profileId INT16U
 * @param deviceId INT16U
 * @param version INT8U
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterEndpointInformation(ieeeAddress, networkAddress, endpointId, profileId, deviceId, version)                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENDPOINT_INFORMATION_COMMAND_ID, "uuuuuu", ieeeAddress, networkAddress, endpointId,          \
                                  profileId, deviceId, version);

/** @brief Command description for GetGroupIdentifiersRequest
 *
 * Command: GetGroupIdentifiersRequest
 * @param startIndex INT8U
 * @param total INT8U
 * @param startIndex INT8U
 * @param count INT8U
 * @param groupInformationRecordList GroupInformationRecord []
 * @param groupInformationRecordListLen int
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterGetGroupIdentifiersRequest(startIndex, total, startIndex, count, groupInformationRecordList,               \
                                                   groupInformationRecordListLen)                                                  \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_GROUP_IDENTIFIERS_REQUEST_COMMAND_ID, "uuuub", startIndex, total, startIndex, count,     \
                                  groupInformationRecordList, groupInformationRecordListLen);

/** @brief Command description for GetEndpointListRequest
 *
 * Command: GetEndpointListRequest
 * @param startIndex INT8U
 * @param total INT8U
 * @param startIndex INT8U
 * @param count INT8U
 * @param endpointInformationRecordList EndpointInformationRecord []
 * @param endpointInformationRecordListLen int
 */
#define emberAfFillCommandZLL                                                                                                      \
    CommissioningClusterGetEndpointListRequest(startIndex, total, startIndex, count, endpointInformationRecordList,                \
                                               endpointInformationRecordListLen)                                                   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_GET_ENDPOINT_LIST_REQUEST_COMMAND_ID, "uuuub", startIndex, total, startIndex, count,         \
                                  endpointInformationRecordList, endpointInformationRecordListLen);

/** @brief Command description for Bind
 *
 * Command: Bind
 * @param nodeId INT64U
 * @param groupId INT16U
 * @param endpointId INT8U
 * @param clusterId CLUSTER_ID
 */
#define emberAfFillCommandBindingClusterBind(nodeId, groupId, endpointId, clusterId)                                               \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_BIND_COMMAND_ID, "uuuu", nodeId, groupId, endpointId, clusterId);

/** @brief Command description for Unbind
 *
 * Command: Unbind
 * @param nodeId INT64U
 * @param groupId INT16U
 * @param endpointId INT8U
 * @param clusterId CLUSTER_ID
 */
#define emberAfFillCommandBindingClusterUnbind(nodeId, groupId, endpointId, clusterId)                                             \
    emberAfFillExternalBuffer(mask,                                                                                                \
                                                                                                                                   \
                              ZCL_UNBIND_COMMAND_ID, "uuuu", nodeId, groupId, endpointId, clusterId);

/** @brief Command description for CommandOne
 *
 * Command: CommandOne
 * @param argOne INT8U
 * @param argOne INT8U
 */
#define emberAfFillCommandSample                                                                                                   \
    Mfg Specific ClusterClusterCommandOne(argOne, argOne)                                                                          \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_COMMAND_ONE_COMMAND_ID, "uu", argOne, argOne);

/** @brief Command description for SetToken
 *
 * Command: SetToken
 * @param token INT16U
 * @param token INT16U
 * @param data OCTET_STRING
 * @param data OCTET_STRING
 */
#define emberAfFillCommandConfiguration                                                                                            \
    ClusterClusterSetToken(token, token, data, data)                                                                               \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_TOKEN_COMMAND_ID, "uuuu", token, token, data, data);

/** @brief Command description for LockTokens
 *
 * Command: LockTokens
 */
#define emberAfFillCommandConfiguration                                                                                            \
    ClusterClusterLockTokens() emberAfFillExternalBuffer(mask,                                                                     \
                                                                                                                                   \
                                                         ZCL_LOCK_TOKENS_COMMAND_ID, "", );

/** @brief Command description for ReadTokens
 *
 * Command: ReadTokens
 * @param token INT16U
 */
#define emberAfFillCommandConfiguration                                                                                            \
    ClusterClusterReadTokens(token) emberAfFillExternalBuffer(mask,                                                                \
                                                                                                                                   \
                                                              ZCL_READ_TOKENS_COMMAND_ID, "u", token);

/** @brief Command description for UnlockTokens
 *
 * Command: UnlockTokens
 * @param data OCTET_STRING
 */
#define emberAfFillCommandConfiguration                                                                                            \
    ClusterClusterUnlockTokens(data) emberAfFillExternalBuffer(mask,                                                               \
                                                                                                                                   \
                                                               ZCL_UNLOCK_TOKENS_COMMAND_ID, "u", data);

/** @brief Command description for stream
 *
 * Command: stream
 * @param channel INT8U
 * @param power INT8S
 * @param time INT16U
 */
#define emberAfFillCommandMFGLIB                                                                                                   \
    ClusterClusterstream(channel, power, time) emberAfFillExternalBuffer(mask,                                                     \
                                                                                                                                   \
                                                                         ZCL_STREAM_COMMAND_ID, "uuu", channel, power, time);

/** @brief Command description for tone
 *
 * Command: tone
 * @param channel INT8U
 * @param power INT8S
 * @param time INT16U
 */
#define emberAfFillCommandMFGLIB                                                                                                   \
    ClusterClustertone(channel, power, time) emberAfFillExternalBuffer(mask,                                                       \
                                                                                                                                   \
                                                                       ZCL_TONE_COMMAND_ID, "uuu", channel, power, time);

/** @brief Command description for rxMode
 *
 * Command: rxMode
 * @param channel INT8U
 * @param power INT8S
 * @param time INT16U
 */
#define emberAfFillCommandMFGLIB                                                                                                   \
    ClusterClusterrxMode(channel, power, time) emberAfFillExternalBuffer(mask,                                                     \
                                                                                                                                   \
                                                                         ZCL_RX_MODE_COMMAND_ID, "uuu", channel, power, time);

/** @brief Command description for EnableApsLinkKeyAuthorization
 *
 * Command: EnableApsLinkKeyAuthorization
 * @param numberExemptClusters INT8U
 * @param clusterId CLUSTER_ID
 * @param clusterId CLUSTER_ID []
 * @param clusterIdLen int
 * @param apsLinkKeyAuthStatus BOOLEAN
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterEnableApsLinkKeyAuthorization(numberExemptClusters, clusterId, clusterId, clusterIdLen,              \
                                                            apsLinkKeyAuthStatus)                                                  \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENABLE_APS_LINK_KEY_AUTHORIZATION_COMMAND_ID, "uubu", numberExemptClusters, clusterId,       \
                                  clusterId, clusterIdLen, apsLinkKeyAuthStatus);

/** @brief Command description for DisableApsLinkKeyAuthorization
 *
 * Command: DisableApsLinkKeyAuthorization
 * @param numberExemptClusters INT8U
 * @param powerNotificationReason WwahPowerNotificationReason
 * @param clusterId CLUSTER_ID []
 * @param clusterIdLen int
 * @param manufacturerId INT16U
 * @param manufacturerReasonLength INT8U
 * @param manufacturerReason INT8U []
 * @param manufacturerReasonLen int
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterDisableApsLinkKeyAuthorization(numberExemptClusters, powerNotificationReason, clusterId,             \
                                                             clusterIdLen, manufacturerId, manufacturerReasonLength,               \
                                                             manufacturerReason, manufacturerReasonLen)                            \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DISABLE_APS_LINK_KEY_AUTHORIZATION_COMMAND_ID, "uubuub", numberExemptClusters,               \
                                  powerNotificationReason, clusterId, clusterIdLen, manufacturerId, manufacturerReasonLength,      \
                                  manufacturerReason, manufacturerReasonLen);

/** @brief Command description for ApsLinkKeyAuthorizationQuery
 *
 * Command: ApsLinkKeyAuthorizationQuery
 * @param clusterId CLUSTER_ID
 * @param powerNotificationReason WwahPowerNotificationReason
 * @param manufacturerId INT16U
 * @param manufacturerReasonLength INT8U
 * @param manufacturerReason INT8U []
 * @param manufacturerReasonLen int
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterApsLinkKeyAuthorizationQuery(clusterId, powerNotificationReason, manufacturerId,                     \
                                                           manufacturerReasonLength, manufacturerReason, manufacturerReasonLen)    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_APS_LINK_KEY_AUTHORIZATION_QUERY_COMMAND_ID, "uuuub", clusterId, powerNotificationReason,    \
                                  manufacturerId, manufacturerReasonLength, manufacturerReason, manufacturerReasonLen);

/** @brief Command description for RequestNewApsLinkKey
 *
 * Command: RequestNewApsLinkKey
 * @param deviceEui64 IEEE_ADDRESS
 * @param deviceShort INT16U
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterRequestNewApsLinkKey(deviceEui64, deviceShort)                                                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_REQUEST_NEW_APS_LINK_KEY_COMMAND_ID, "uu", deviceEui64, deviceShort);

/** @brief Command description for EnableWwahAppEventRetryAlgorithm
 *
 * Command: EnableWwahAppEventRetryAlgorithm
 * @param firstBackoffTimeSeconds INT8U
 * @param numberExemptClusters INT8U
 * @param backoffSeqCommonRatio INT8U
 * @param clusterId CLUSTER_ID []
 * @param clusterIdLen int
 * @param maxBackoffTimeSeconds INT32U
 * @param maxRedeliveryAttempts INT8U
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterEnableWwahAppEventRetryAlgorithm(                                                                    \
        firstBackoffTimeSeconds, numberExemptClusters, backoffSeqCommonRatio, clusterId, clusterIdLen, maxBackoffTimeSeconds,      \
        maxRedeliveryAttempts) emberAfFillExternalBuffer(mask,                                                                     \
                                                                                                                                   \
                                                         ZCL_ENABLE_WWAH_APP_EVENT_RETRY_ALGORITHM_COMMAND_ID, "uuubuu",           \
                                                         firstBackoffTimeSeconds, numberExemptClusters, backoffSeqCommonRatio,     \
                                                         clusterId, clusterIdLen, maxBackoffTimeSeconds, maxRedeliveryAttempts);

/** @brief Command description for DisableWwahAppEventRetryAlgorithm
 *
 * Command: DisableWwahAppEventRetryAlgorithm
 * @param currentPowerMode INT32U
 * @param availablePowerSources INT32U
 * @param currentPowerSource INT32U
 * @param currentPowerSourceLevel INT32U
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterDisableWwahAppEventRetryAlgorithm(currentPowerMode, availablePowerSources, currentPowerSource,       \
                                                                currentPowerSourceLevel)                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DISABLE_WWAH_APP_EVENT_RETRY_ALGORITHM_COMMAND_ID, "uuuu", currentPowerMode,                 \
                                  availablePowerSources, currentPowerSource, currentPowerSourceLevel);

/** @brief Command description for RequestTime
 *
 * Command: RequestTime
 * @param debugReportId INT8U
 * @param debugReportSize INT32U
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterRequestTime(debugReportId, debugReportSize)                                                          \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_REQUEST_TIME_COMMAND_ID, "uu", debugReportId, debugReportSize);

/** @brief Command description for EnableWwahRejoinAlgorithm
 *
 * Command: EnableWwahRejoinAlgorithm
 * @param fastRejoinTimeoutSeconds INT16U
 * @param debugReportId INT8U
 * @param durationBetweenRejoinsSeconds INT16U
 * @param debugReportData INT8U []
 * @param debugReportDataLen int
 * @param fastRejoinFirstBackoffSeconds INT16U
 * @param maxBackoffTimeSeconds INT16U
 * @param maxBackoffIterations INT16U
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterEnableWwahRejoinAlgorithm(fastRejoinTimeoutSeconds, debugReportId, durationBetweenRejoinsSeconds,    \
                                                        debugReportData, debugReportDataLen, fastRejoinFirstBackoffSeconds,        \
                                                        maxBackoffTimeSeconds, maxBackoffIterations)                               \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENABLE_WWAH_REJOIN_ALGORITHM_COMMAND_ID, "uuubuuu", fastRejoinTimeoutSeconds, debugReportId, \
                                  durationBetweenRejoinsSeconds, debugReportData, debugReportDataLen,                              \
                                  fastRejoinFirstBackoffSeconds, maxBackoffTimeSeconds, maxBackoffIterations);

/** @brief Command description for DisableWwahRejoinAlgorithm
 *
 * Command: DisableWwahRejoinAlgorithm
 * @param numberOfClusters INT8U
 * @param clusterId CLUSTER_ID []
 * @param clusterIdLen int
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterDisableWwahRejoinAlgorithm(numberOfClusters, clusterId, clusterIdLen)                                \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DISABLE_WWAH_REJOIN_ALGORITHM_COMMAND_ID, "ub", numberOfClusters, clusterId, clusterIdLen);

/** @brief Command description for SetIasZoneEnrollmentMethod
 *
 * Command: SetIasZoneEnrollmentMethod
 * @param enrollmentMode WwahIasZoneEnrollmentMode
 * @param numberOfBeacons INT8U
 * @param beacon WwahBeaconSurvey []
 * @param beaconLen int
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterSetIasZoneEnrollmentMethod(enrollmentMode, numberOfBeacons, beacon, beaconLen)                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_IAS_ZONE_ENROLLMENT_METHOD_COMMAND_ID, "uub", enrollmentMode, numberOfBeacons, beacon,   \
                                  beaconLen);

/** @brief Command description for ClearBindingTable
 *
 * Command: ClearBindingTable
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterClearBindingTable() emberAfFillExternalBuffer(mask,                                                  \
                                                                                                                                   \
                                                                            ZCL_CLEAR_BINDING_TABLE_COMMAND_ID, "", );

/** @brief Command description for EnablePeriodicRouterCheckIns
 *
 * Command: EnablePeriodicRouterCheckIns
 * @param checkInInterval INT16U
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterEnablePeriodicRouterCheckIns(checkInInterval)                                                        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENABLE_PERIODIC_ROUTER_CHECK_INS_COMMAND_ID, "u", checkInInterval);

/** @brief Command description for DisablePeriodicRouterCheckIns
 *
 * Command: DisablePeriodicRouterCheckIns
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterDisablePeriodicRouterCheckIns()                                                                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DISABLE_PERIODIC_ROUTER_CHECK_INS_COMMAND_ID, "", );

/** @brief Command description for SetMacPollFailureWaitTime
 *
 * Command: SetMacPollFailureWaitTime
 * @param waitTime INT8U
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterSetMacPollFailureWaitTime(waitTime)                                                                  \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_MAC_POLL_FAILURE_WAIT_TIME_COMMAND_ID, "u", waitTime);

/** @brief Command description for SetPendingNetworkUpdate
 *
 * Command: SetPendingNetworkUpdate
 * @param channel INT8U
 * @param panId INT16U
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterSetPendingNetworkUpdate(channel, panId)                                                              \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SET_PENDING_NETWORK_UPDATE_COMMAND_ID, "uu", channel, panId);

/** @brief Command description for RequireApsAcksOnUnicasts
 *
 * Command: RequireApsAcksOnUnicasts
 * @param numberExemptClusters INT8U
 * @param clusterId CLUSTER_ID []
 * @param clusterIdLen int
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterRequireApsAcksOnUnicasts(numberExemptClusters, clusterId, clusterIdLen) emberAfFillExternalBuffer(   \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_REQUIRE_APS_ACKS_ON_UNICASTS_COMMAND_ID, "ub", numberExemptClusters, clusterId, clusterIdLen);

/** @brief Command description for RemoveApsAcksOnUnicastsRequirement
 *
 * Command: RemoveApsAcksOnUnicastsRequirement
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterRemoveApsAcksOnUnicastsRequirement()                                                                 \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_REMOVE_APS_ACKS_ON_UNICASTS_REQUIREMENT_COMMAND_ID, "", );

/** @brief Command description for ApsAckRequirementQuery
 *
 * Command: ApsAckRequirementQuery
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterApsAckRequirementQuery() emberAfFillExternalBuffer(mask,                                             \
                                                                                                                                   \
                                                                                 ZCL_APS_ACK_REQUIREMENT_QUERY_COMMAND_ID, "", );

/** @brief Command description for DebugReportQuery
 *
 * Command: DebugReportQuery
 * @param debugReportId INT8U
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterDebugReportQuery(debugReportId)                                                                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DEBUG_REPORT_QUERY_COMMAND_ID, "u", debugReportId);

/** @brief Command description for SurveyBeacons
 *
 * Command: SurveyBeacons
 * @param standardBeacons BOOLEAN
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterSurveyBeacons(standardBeacons)                                                                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_SURVEY_BEACONS_COMMAND_ID, "u", standardBeacons);

/** @brief Command description for DisableOtaDowngrades
 *
 * Command: DisableOtaDowngrades
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterDisableOtaDowngrades() emberAfFillExternalBuffer(mask,                                               \
                                                                                                                                   \
                                                                               ZCL_DISABLE_OTA_DOWNGRADES_COMMAND_ID, "", );

/** @brief Command description for DisableMgmtLeaveWithoutRejoin
 *
 * Command: DisableMgmtLeaveWithoutRejoin
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterDisableMgmtLeaveWithoutRejoin()                                                                      \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DISABLE_MGMT_LEAVE_WITHOUT_REJOIN_COMMAND_ID, "", );

/** @brief Command description for DisableTouchlinkInterpanMessageSupport
 *
 * Command: DisableTouchlinkInterpanMessageSupport
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterDisableTouchlinkInterpanMessageSupport()                                                             \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DISABLE_TOUCHLINK_INTERPAN_MESSAGE_SUPPORT_COMMAND_ID, "", );

/** @brief Command description for EnableWwahParentClassification
 *
 * Command: EnableWwahParentClassification
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterEnableWwahParentClassification()                                                                     \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENABLE_WWAH_PARENT_CLASSIFICATION_COMMAND_ID, "", );

/** @brief Command description for DisableWwahParentClassification
 *
 * Command: DisableWwahParentClassification
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterDisableWwahParentClassification()                                                                    \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DISABLE_WWAH_PARENT_CLASSIFICATION_COMMAND_ID, "", );

/** @brief Command description for EnableTcSecurityOnNtwkKeyRotation
 *
 * Command: EnableTcSecurityOnNtwkKeyRotation
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterEnableTcSecurityOnNtwkKeyRotation()                                                                  \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENABLE_TC_SECURITY_ON_NTWK_KEY_ROTATION_COMMAND_ID, "", );

/** @brief Command description for EnableWwahBadParentRecovery
 *
 * Command: EnableWwahBadParentRecovery
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterEnableWwahBadParentRecovery()                                                                        \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_ENABLE_WWAH_BAD_PARENT_RECOVERY_COMMAND_ID, "", );

/** @brief Command description for DisableWwahBadParentRecovery
 *
 * Command: DisableWwahBadParentRecovery
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterDisableWwahBadParentRecovery()                                                                       \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DISABLE_WWAH_BAD_PARENT_RECOVERY_COMMAND_ID, "", );

/** @brief Command description for EnableConfigurationMode
 *
 * Command: EnableConfigurationMode
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterEnableConfigurationMode() emberAfFillExternalBuffer(mask,                                            \
                                                                                                                                   \
                                                                                  ZCL_ENABLE_CONFIGURATION_MODE_COMMAND_ID, "", );

/** @brief Command description for DisableConfigurationMode
 *
 * Command: DisableConfigurationMode
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterDisableConfigurationMode()                                                                           \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_DISABLE_CONFIGURATION_MODE_COMMAND_ID, "", );

/** @brief Command description for UseTrustCenterForClusterServer
 *
 * Command: UseTrustCenterForClusterServer
 * @param numberOfClusters INT8U
 * @param clusterId CLUSTER_ID []
 * @param clusterIdLen int
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterUseTrustCenterForClusterServer(numberOfClusters, clusterId, clusterIdLen) emberAfFillExternalBuffer( \
        mask,                                                                                                                      \
                                                                                                                                   \
        ZCL_USE_TRUST_CENTER_FOR_CLUSTER_SERVER_COMMAND_ID, "ub", numberOfClusters, clusterId, clusterIdLen);

/** @brief Command description for TrustCenterForClusterServerQuery
 *
 * Command: TrustCenterForClusterServerQuery
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterTrustCenterForClusterServerQuery()                                                                   \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_TRUST_CENTER_FOR_CLUSTER_SERVER_QUERY_COMMAND_ID, "", );

/** @brief Command description for UseTrustCenterForClusterServerResponse
 *
 * Command: UseTrustCenterForClusterServerResponse
 * @param status Status
 * @param clusterStatusLength INT8U
 * @param clusterStatus WwahClusterStatusToUseTC []
 * @param clusterStatusLen int
 */
#define emberAfFillCommandSL                                                                                                       \
    Works With All HubsClusterUseTrustCenterForClusterServerResponse(status, clusterStatusLength, clusterStatus, clusterStatusLen) \
        emberAfFillExternalBuffer(mask,                                                                                            \
                                                                                                                                   \
                                  ZCL_USE_TRUST_CENTER_FOR_CLUSTER_SERVER_RESPONSE_COMMAND_ID, "uub", status, clusterStatusLength, \
                                  clusterStatus, clusterStatusLen);
