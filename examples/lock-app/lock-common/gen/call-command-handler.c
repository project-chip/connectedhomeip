
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

#include <stdint.h>

#include "af-structs.h"
#include "call-command-handler.h"
#include "callback.h"
#include "command-id.h"
#include "util.h"

EmberAfStatus emberAfBasicClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfGroupsClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfIdentifyClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfLevelControlClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfOccupancySensingClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfOnOffClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfOverTheAirBootloadingClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfScenesClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfZLLCommissioningClusterServerCommandParse(EmberAfClusterCommand * cmd);

static EmberAfStatus status(bool wasHandled, bool clusterExists, bool mfgSpecific)
{
    if (wasHandled)
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    else if (mfgSpecific)
    {
        return EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND;
    }
    else if (clusterExists)
    {
        return EMBER_ZCL_STATUS_UNSUP_COMMAND;
    }
    else
    {
        return EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER;
    }
}

// Main command parsing controller.
EmberAfStatus emberAfClusterSpecificCommandParse(EmberAfClusterCommand * cmd)
{
    EmberAfStatus result = status(false, false, cmd->mfgSpecific);
    if (cmd->direction == (uint8_t) ZCL_DIRECTION_SERVER_TO_CLIENT &&
        emberAfContainsClientWithMfgCode(cmd->apsFrame->destinationEndpoint, cmd->apsFrame->clusterId, cmd->mfgCode))
    {
        switch (cmd->apsFrame->clusterId)
        {
        case ZCL_OCCUPANCY_SENSING_CLUSTER_ID:
            result = emberAfOccupancySensingClusterClientCommandParse(cmd);
            break;
        case ZCL_OTA_BOOTLOAD_CLUSTER_ID:
            result = emberAfOverTheAirBootloadingClusterClientCommandParse(cmd);
            break;
        default:
            // Unrecognized cluster ID, error status will apply.
            break;
        }
    }
    else if (cmd->direction == (uint8_t) ZCL_DIRECTION_CLIENT_TO_SERVER &&
             emberAfContainsServerWithMfgCode(cmd->apsFrame->destinationEndpoint, cmd->apsFrame->clusterId, cmd->mfgCode))
    {
        switch (cmd->apsFrame->clusterId)
        {
        case ZCL_BASIC_CLUSTER_ID:
            result = emberAfBasicClusterServerCommandParse(cmd);
            break;
        case ZCL_GROUPS_CLUSTER_ID:
            result = emberAfGroupsClusterServerCommandParse(cmd);
            break;
        case ZCL_IDENTIFY_CLUSTER_ID:
            result = emberAfIdentifyClusterServerCommandParse(cmd);
            break;
        case ZCL_LEVEL_CONTROL_CLUSTER_ID:
            result = emberAfLevelControlClusterServerCommandParse(cmd);
            break;
        case ZCL_ON_OFF_CLUSTER_ID:
            result = emberAfOnOffClusterServerCommandParse(cmd);
            break;
        case ZCL_SCENES_CLUSTER_ID:
            result = emberAfScenesClusterServerCommandParse(cmd);
            break;
        case ZCL_ZLL_COMMISSIONING_CLUSTER_ID:
            result = emberAfZLLCommissioningClusterServerCommandParse(cmd);
            break;

        default:
            // Unrecognized cluster ID, error status will apply.
            break;
        }
    }
    return result;
}

// Cluster specific command parsing

EmberAfStatus emberAfBasicClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfGroupsClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_ADD_GROUP_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t ** groupName = (uint8_t **) (cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterAddGroupCallback(*groupId, *groupName);
            break;
        }
        case ZCL_ADD_GROUP_IF_IDENTIFYING_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t ** groupName = (uint8_t **) (cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterAddGroupIfIdentifyingCallback(*groupId, *groupName);
            break;
        }
        case ZCL_GET_GROUP_MEMBERSHIP_COMMAND_ID: {
            uint32_t argOffset   = 0;
            uint8_t * groupCount = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * groupList = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterGetGroupMembershipCallback(*groupCount, *groupList);
            break;
        }
        case ZCL_REMOVE_ALL_GROUPS_COMMAND_ID: {
            wasHandled = emberAfGroupsClusterRemoveAllGroupsCallback();
            break;
        }
        case ZCL_REMOVE_GROUP_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterRemoveGroupCallback(*groupId);
            break;
        }
        case ZCL_VIEW_GROUP_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterViewGroupCallback(*groupId);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfIdentifyClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_IDENTIFY_COMMAND_ID: {
            uint32_t argOffset      = 0;
            uint16_t * identifyTime = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfIdentifyClusterIdentifyCallback(*identifyTime);
            break;
        }
        case ZCL_IDENTIFY_QUERY_COMMAND_ID: {
            wasHandled = emberAfIdentifyClusterIdentifyQueryCallback();
            break;
        }
        case ZCL_TRIGGER_EFFECT_COMMAND_ID: {
            uint32_t argOffset                         = 0;
            EmberAfIdentifyEffectIdentifier * effectId = (EmberAfIdentifyEffectIdentifier *) (cmd->buffer + argOffset);
            argOffset += sizeof(EmberAfIdentifyEffectIdentifier);
            EmberAfIdentifyEffectVariant * effectVariant = (EmberAfIdentifyEffectVariant *) (cmd->buffer + argOffset);

            wasHandled = emberAfIdentifyClusterTriggerEffectCallback(*effectId, *effectVariant);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfLevelControlClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_MOVE_COMMAND_ID: {
            uint32_t argOffset         = 0;
            EmberAfMoveMode * moveMode = (EmberAfMoveMode *) (cmd->buffer + argOffset);
            argOffset += sizeof(EmberAfMoveMode);
            uint8_t * rate = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint8_t * optionMask = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint8_t * optionOverride = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterMoveCallback(*moveMode, *rate, *optionMask, *optionOverride);
            break;
        }
        case ZCL_MOVE_TO_LEVEL_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint8_t * level    = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t * optionMask = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint8_t * optionOverride = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterMoveToLevelCallback(*level, *transitionTime, *optionMask, *optionOverride);
            break;
        }
        case ZCL_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint8_t * level    = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterMoveToLevelWithOnOffCallback(*level, *transitionTime);
            break;
        }
        case ZCL_MOVE_WITH_ON_OFF_COMMAND_ID: {
            uint32_t argOffset         = 0;
            EmberAfMoveMode * moveMode = (EmberAfMoveMode *) (cmd->buffer + argOffset);
            argOffset += sizeof(EmberAfMoveMode);
            uint8_t * rate = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterMoveWithOnOffCallback(*moveMode, *rate);
            break;
        }
        case ZCL_STEP_COMMAND_ID: {
            uint32_t argOffset         = 0;
            EmberAfStepMode * stepMode = (EmberAfStepMode *) (cmd->buffer + argOffset);
            argOffset += sizeof(EmberAfStepMode);
            uint8_t * stepSize = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t * optionMask = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint8_t * optionOverride = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled =
                emberAfLevelControlClusterStepCallback(*stepMode, *stepSize, *transitionTime, *optionMask, *optionOverride);
            break;
        }
        case ZCL_STEP_WITH_ON_OFF_COMMAND_ID: {
            uint32_t argOffset         = 0;
            EmberAfStepMode * stepMode = (EmberAfStepMode *) (cmd->buffer + argOffset);
            argOffset += sizeof(EmberAfStepMode);
            uint8_t * stepSize = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterStepWithOnOffCallback(*stepMode, *stepSize, *transitionTime);
            break;
        }
        case ZCL_STOP_COMMAND_ID: {
            uint32_t argOffset   = 0;
            uint8_t * optionMask = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint8_t * optionOverride = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterStopCallback(*optionMask, *optionOverride);
            break;
        }
        case ZCL_STOP_WITH_ON_OFF_COMMAND_ID: {
            wasHandled = emberAfLevelControlClusterStopWithOnOffCallback();
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfOccupancySensingClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfOnOffClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_OFF_COMMAND_ID: {
            wasHandled = emberAfOnOffClusterOffCallback();
            break;
        }
        case ZCL_OFF_WITH_EFFECT_COMMAND_ID: {
            uint32_t argOffset                      = 0;
            EmberAfOnOffEffectIdentifier * effectId = (EmberAfOnOffEffectIdentifier *) (cmd->buffer + argOffset);
            argOffset += sizeof(EmberAfOnOffEffectIdentifier);
            uint8_t * effectVariant = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfOnOffClusterOffWithEffectCallback(*effectId, *effectVariant);
            break;
        }
        case ZCL_ON_COMMAND_ID: {
            wasHandled = emberAfOnOffClusterOnCallback();
            break;
        }
        case ZCL_ON_WITH_RECALL_GLOBAL_SCENE_COMMAND_ID: {
            wasHandled = emberAfOnOffClusterOnWithRecallGlobalSceneCallback();
            break;
        }
        case ZCL_ON_WITH_TIMED_OFF_COMMAND_ID: {
            uint32_t argOffset     = 0;
            uint8_t * onOffControl = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * onTime = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * offWaitTime = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfOnOffClusterOnWithTimedOffCallback(*onOffControl, *onTime, *offWaitTime);
            break;
        }
        case ZCL_TOGGLE_COMMAND_ID: {
            wasHandled = emberAfOnOffClusterToggleCallback();
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfOverTheAirBootloadingClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfScenesClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_ADD_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t ** sceneName = (uint8_t **) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t *);
            EmberAfSceneExtensionFieldSet * extensionFieldSets = (EmberAfSceneExtensionFieldSet *) (cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterAddSceneCallback(*groupId, *sceneId, *transitionTime, *sceneName, *extensionFieldSets);
            break;
        }
        case ZCL_COPY_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint8_t * mode     = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * groupIdFrom = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t * sceneIdFrom = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * groupIdTo = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t * sceneIdTo = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterCopySceneCallback(*mode, *groupIdFrom, *sceneIdFrom, *groupIdTo, *sceneIdTo);
            break;
        }
        case ZCL_ENHANCED_ADD_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t ** sceneName = (uint8_t **) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t *);
            EmberAfSceneExtensionFieldSet * extensionFieldSets = (EmberAfSceneExtensionFieldSet *) (cmd->buffer + argOffset);

            wasHandled =
                emberAfScenesClusterEnhancedAddSceneCallback(*groupId, *sceneId, *transitionTime, *sceneName, *extensionFieldSets);
            break;
        }
        case ZCL_ENHANCED_VIEW_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterEnhancedViewSceneCallback(*groupId, *sceneId);
            break;
        }
        case ZCL_GET_SCENE_MEMBERSHIP_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterGetSceneMembershipCallback(*groupId);
            break;
        }
        case ZCL_RECALL_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterRecallSceneCallback(*groupId, *sceneId, *transitionTime);
            break;
        }
        case ZCL_REMOVE_ALL_SCENES_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterRemoveAllScenesCallback(*groupId);
            break;
        }
        case ZCL_REMOVE_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterRemoveSceneCallback(*groupId, *sceneId);
            break;
        }
        case ZCL_STORE_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterStoreSceneCallback(*groupId, *sceneId);
            break;
        }
        case ZCL_VIEW_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterViewSceneCallback(*groupId, *sceneId);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfZLLCommissioningClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID: {
            uint32_t argOffset     = 0;
            uint32_t * transaction = (uint32_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint32_t);
            uint8_t * startIndex = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfZLLCommissioningClusterDeviceInformationRequestCallback(*transaction, *startIndex);
            break;
        }
        case ZCL_GET_ENDPOINT_LIST_REQUEST_COMMAND_ID: {
            uint32_t argOffset   = 0;
            uint8_t * startIndex = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfZLLCommissioningClusterGetEndpointListRequestCallback(*startIndex);
            break;
        }
        case ZCL_GET_GROUP_IDENTIFIERS_REQUEST_COMMAND_ID: {
            uint32_t argOffset   = 0;
            uint8_t * startIndex = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfZLLCommissioningClusterGetGroupIdentifiersRequestCallback(*startIndex);
            break;
        }
        case ZCL_IDENTIFY_REQUEST_COMMAND_ID: {
            uint32_t argOffset     = 0;
            uint32_t * transaction = (uint32_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint32_t);
            uint16_t * identifyDuration = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfZLLCommissioningClusterIdentifyRequestCallback(*transaction, *identifyDuration);
            break;
        }
        case ZCL_NETWORK_JOIN_END_DEVICE_REQUEST_COMMAND_ID: {
            uint32_t argOffset     = 0;
            uint32_t * transaction = (uint32_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint32_t);
            uint8_t ** extendedPanId = (uint8_t **) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t *);
            EmberAfKeyIndex * keyIndex = (EmberAfKeyIndex *) (cmd->buffer + argOffset);
            argOffset += sizeof(EmberAfKeyIndex);
            /* TYPE WARNING: security_key defaults to */ uint8_t ** encryptedNetworkKey =
                (/* TYPE WARNING: security_key defaults to */ uint8_t **) (cmd->buffer + argOffset);
            argOffset += sizeof(/* TYPE WARNING: security_key defaults to */ uint8_t *);
            uint8_t * networkUpdateId = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint8_t * logicalChannel = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * panId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * networkAddress = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * groupIdentifiersBegin = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * groupIdentifiersEnd = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeNetworkAddressRangeBegin = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeNetworkAddressRangeEnd = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeGroupIdentifierRangeBegin = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeGroupIdentifierRangeEnd = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfZLLCommissioningClusterNetworkJoinEndDeviceRequestCallback(
                *transaction, *extendedPanId, *keyIndex, *encryptedNetworkKey, *networkUpdateId, *logicalChannel, *panId,
                *networkAddress, *groupIdentifiersBegin, *groupIdentifiersEnd, *freeNetworkAddressRangeBegin,
                *freeNetworkAddressRangeEnd, *freeGroupIdentifierRangeBegin, *freeGroupIdentifierRangeEnd);
            break;
        }
        case ZCL_NETWORK_JOIN_ROUTER_REQUEST_COMMAND_ID: {
            uint32_t argOffset     = 0;
            uint32_t * transaction = (uint32_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint32_t);
            uint8_t ** extendedPanId = (uint8_t **) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t *);
            EmberAfKeyIndex * keyIndex = (EmberAfKeyIndex *) (cmd->buffer + argOffset);
            argOffset += sizeof(EmberAfKeyIndex);
            /* TYPE WARNING: security_key defaults to */ uint8_t ** encryptedNetworkKey =
                (/* TYPE WARNING: security_key defaults to */ uint8_t **) (cmd->buffer + argOffset);
            argOffset += sizeof(/* TYPE WARNING: security_key defaults to */ uint8_t *);
            uint8_t * networkUpdateId = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint8_t * logicalChannel = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * panId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * networkAddress = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * groupIdentifiersBegin = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * groupIdentifiersEnd = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeNetworkAddressRangeBegin = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeNetworkAddressRangeEnd = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeGroupIdentifierRangeBegin = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeGroupIdentifierRangeEnd = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfZLLCommissioningClusterNetworkJoinRouterRequestCallback(
                *transaction, *extendedPanId, *keyIndex, *encryptedNetworkKey, *networkUpdateId, *logicalChannel, *panId,
                *networkAddress, *groupIdentifiersBegin, *groupIdentifiersEnd, *freeNetworkAddressRangeBegin,
                *freeNetworkAddressRangeEnd, *freeGroupIdentifierRangeBegin, *freeGroupIdentifierRangeEnd);
            break;
        }
        case ZCL_NETWORK_START_REQUEST_COMMAND_ID: {
            uint32_t argOffset     = 0;
            uint32_t * transaction = (uint32_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint32_t);
            uint8_t ** extendedPanId = (uint8_t **) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t *);
            EmberAfKeyIndex * keyIndex = (EmberAfKeyIndex *) (cmd->buffer + argOffset);
            argOffset += sizeof(EmberAfKeyIndex);
            /* TYPE WARNING: security_key defaults to */ uint8_t ** encryptedNetworkKey =
                (/* TYPE WARNING: security_key defaults to */ uint8_t **) (cmd->buffer + argOffset);
            argOffset += sizeof(/* TYPE WARNING: security_key defaults to */ uint8_t *);
            uint8_t * logicalChannel = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * panId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * networkAddress = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * groupIdentifiersBegin = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * groupIdentifiersEnd = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeNetworkAddressRangeBegin = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeNetworkAddressRangeEnd = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeGroupIdentifierRangeBegin = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * freeGroupIdentifierRangeEnd = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint8_t ** initiatorIeeeAddress = (uint8_t **) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t *);
            uint16_t * initiatorNetworkAddress = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfZLLCommissioningClusterNetworkStartRequestCallback(
                *transaction, *extendedPanId, *keyIndex, *encryptedNetworkKey, *logicalChannel, *panId, *networkAddress,
                *groupIdentifiersBegin, *groupIdentifiersEnd, *freeNetworkAddressRangeBegin, *freeNetworkAddressRangeEnd,
                *freeGroupIdentifierRangeBegin, *freeGroupIdentifierRangeEnd, *initiatorIeeeAddress, *initiatorNetworkAddress);
            break;
        }
        case ZCL_NETWORK_UPDATE_REQUEST_COMMAND_ID: {
            uint32_t argOffset     = 0;
            uint32_t * transaction = (uint32_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint32_t);
            uint8_t ** extendedPanId = (uint8_t **) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t *);
            uint8_t * networkUpdateId = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint8_t * logicalChannel = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint16_t * panId = (uint16_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint16_t);
            uint16_t * networkAddress = (uint16_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfZLLCommissioningClusterNetworkUpdateRequestCallback(*transaction, *extendedPanId, *networkUpdateId,
                                                                                    *logicalChannel, *panId, *networkAddress);
            break;
        }
        case ZCL_RESET_TO_FACTORY_NEW_REQUEST_COMMAND_ID: {
            uint32_t argOffset     = 0;
            uint32_t * transaction = (uint32_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfZLLCommissioningClusterResetToFactoryNewRequestCallback(*transaction);
            break;
        }
        case ZCL_SCAN_REQUEST_COMMAND_ID: {
            uint32_t argOffset     = 0;
            uint32_t * transaction = (uint32_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint32_t);
            uint8_t * zigbeeInformation = (uint8_t *) (cmd->buffer + argOffset);
            argOffset += sizeof(uint8_t);
            uint8_t * zllInformation = (uint8_t *) (cmd->buffer + argOffset);

            wasHandled = emberAfZLLCommissioningClusterScanRequestCallback(*transaction, *zigbeeInformation, *zllInformation);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
