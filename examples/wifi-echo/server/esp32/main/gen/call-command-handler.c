
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

#include <stdint.h>

#include "af-structs.h"
#include "call-command-handler.h"
#include "callback.h"
#include "command-id.h"
#include "util.h"

EmberAfStatus emberAfBarrierControlClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfBasicClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfBasicClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfColorControlClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfDoorLockClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfGroupsClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfGroupsClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfIASZoneClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfIASZoneClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfIdentifyClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfIdentifyClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfLevelControlClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfOnOffClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfOnOffSwitchConfigurationClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfScenesClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfScenesClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfTemperatureMeasurementClusterServerCommandParse(EmberAfClusterCommand * cmd);


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
        case ZCL_BASIC_CLUSTER_ID :
            result = emberAfBasicClusterClientCommandParse(cmd);
            break;
        case ZCL_GROUPS_CLUSTER_ID :
            result = emberAfGroupsClusterClientCommandParse(cmd);
            break;
        case ZCL_IAS_ZONE_CLUSTER_ID :
            result = emberAfIASZoneClusterClientCommandParse(cmd);
            break;
        case ZCL_IDENTIFY_CLUSTER_ID :
            result = emberAfIdentifyClusterClientCommandParse(cmd);
            break;
        case ZCL_SCENES_CLUSTER_ID :
            result = emberAfScenesClusterClientCommandParse(cmd);
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
        case ZCL_BARRIER_CONTROL_CLUSTER_ID :
            result = emberAfBarrierControlClusterServerCommandParse(cmd);
            break;
        case ZCL_BASIC_CLUSTER_ID :
            result = emberAfBasicClusterServerCommandParse(cmd);
            break;
        case ZCL_COLOR_CONTROL_CLUSTER_ID :
            result = emberAfColorControlClusterServerCommandParse(cmd);
            break;
        case ZCL_DOOR_LOCK_CLUSTER_ID :
            result = emberAfDoorLockClusterServerCommandParse(cmd);
            break;
        case ZCL_GROUPS_CLUSTER_ID :
            result = emberAfGroupsClusterServerCommandParse(cmd);
            break;
        case ZCL_IAS_ZONE_CLUSTER_ID :
            result = emberAfIASZoneClusterServerCommandParse(cmd);
            break;
        case ZCL_IDENTIFY_CLUSTER_ID :
            result = emberAfIdentifyClusterServerCommandParse(cmd);
            break;
        case ZCL_LEVEL_CONTROL_CLUSTER_ID :
            result = emberAfLevelControlClusterServerCommandParse(cmd);
            break;
        case ZCL_ON_OFF_CLUSTER_ID :
            result = emberAfOnOffClusterServerCommandParse(cmd);
            break;
        case ZCL_ON_OFF_SWITCH_CONFIG_CLUSTER_ID :
            result = emberAfOnOffSwitchConfigurationClusterServerCommandParse(cmd);
            break;
        case ZCL_SCENES_CLUSTER_ID :
            result = emberAfScenesClusterServerCommandParse(cmd);
            break;
        case ZCL_TEMP_MEASUREMENT_CLUSTER_ID :
            result = emberAfTemperatureMeasurementClusterServerCommandParse(cmd);
            break;

        default:
            // Unrecognized cluster ID, error status will apply.
            break;
        }
    }
    return result;
}

// Cluster specific command parsing

EmberAfStatus emberAfBarrierControlClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_BARRIER_CONTROL_GO_TO_PERCENT_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint8_t * percentOpen = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfBarrierControlClusterBarrierControlGoToPercentCallback( *percentOpen);
            break;
        }
        case ZCL_BARRIER_CONTROL_STOP_COMMAND_ID: {
            wasHandled = emberAfBarrierControlClusterBarrierControlStopCallback();
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
EmberAfStatus emberAfBasicClusterClientCommandParse(EmberAfClusterCommand * cmd)
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
EmberAfStatus emberAfColorControlClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_MOVE_COLOR_COMMAND_ID: {
            uint32_t argOffset = 0;
            int16_t * rateX = (int16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(int16_t);
            int16_t * rateY = (int16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(int16_t);
            uint8_t * optionsMask = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint8_t * optionsOverride = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfColorControlClusterMoveColorCallback( *rateX,  *rateY,  *optionsMask,  *optionsOverride);
            break;
        }
        case ZCL_MOVE_TO_COLOR_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * colorX = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint16_t * colorY = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint16_t * transitionTime = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * optionsMask = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint8_t * optionsOverride = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfColorControlClusterMoveToColorCallback( *colorX,  *colorY,  *transitionTime,  *optionsMask,  *optionsOverride);
            break;
        }
        case ZCL_STEP_COLOR_COMMAND_ID: {
            uint32_t argOffset = 0;
            int16_t * stepX = (int16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(int16_t);
            int16_t * stepY = (int16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(int16_t);
            uint16_t * transitionTime = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * optionsMask = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint8_t * optionsOverride = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfColorControlClusterStepColorCallback( *stepX,  *stepY,  *transitionTime,  *optionsMask,  *optionsOverride);
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
EmberAfStatus emberAfDoorLockClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_LOCK_DOOR_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint8_t * * PIN = (uint8_t * *)(cmd->buffer + argOffset);

            wasHandled = emberAfDoorLockClusterLockDoorCallback( *PIN);
            break;
        }
        case ZCL_UNLOCK_DOOR_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint8_t * * PIN = (uint8_t * *)(cmd->buffer + argOffset);

            wasHandled = emberAfDoorLockClusterUnlockDoorCallback( *PIN);
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
EmberAfStatus emberAfGroupsClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_ADD_GROUP_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfStatus * status = (EmberAfStatus *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfStatus);
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterAddGroupResponseCallback( *status,  *groupId);
            break;
        }
        case ZCL_GET_GROUP_MEMBERSHIP_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint8_t * capacity = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint8_t * groupCount = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint16_t * groupList = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterGetGroupMembershipResponseCallback( *capacity,  *groupCount,  *groupList);
            break;
        }
        case ZCL_REMOVE_GROUP_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfStatus * status = (EmberAfStatus *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfStatus);
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterRemoveGroupResponseCallback( *status,  *groupId);
            break;
        }
        case ZCL_VIEW_GROUP_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfStatus * status = (EmberAfStatus *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfStatus);
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * * groupName = (uint8_t * *)(cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterViewGroupResponseCallback( *status,  *groupId,  *groupName);
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
EmberAfStatus emberAfGroupsClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_ADD_GROUP_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * * groupName = (uint8_t * *)(cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterAddGroupCallback( *groupId,  *groupName);
            break;
        }
        case ZCL_ADD_GROUP_IF_IDENTIFYING_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * * groupName = (uint8_t * *)(cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterAddGroupIfIdentifyingCallback( *groupId,  *groupName);
            break;
        }
        case ZCL_GET_GROUP_MEMBERSHIP_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint8_t * groupCount = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint16_t * groupList = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterGetGroupMembershipCallback( *groupCount,  *groupList);
            break;
        }
        case ZCL_REMOVE_ALL_GROUPS_COMMAND_ID: {
            wasHandled = emberAfGroupsClusterRemoveAllGroupsCallback();
            break;
        }
        case ZCL_REMOVE_GROUP_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterRemoveGroupCallback( *groupId);
            break;
        }
        case ZCL_VIEW_GROUP_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfGroupsClusterViewGroupCallback( *groupId);
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
EmberAfStatus emberAfIASZoneClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_ZONE_ENROLL_REQUEST_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfIasZoneType * zoneType = (EmberAfIasZoneType *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfIasZoneType);
            uint16_t * manufacturerCode = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfIASZoneClusterZoneEnrollRequestCallback( *zoneType,  *manufacturerCode);
            break;
        }
        case ZCL_ZONE_STATUS_CHANGE_NOTIFICATION_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * zoneStatus = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * extendedStatus = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint8_t * zoneId = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint16_t * delay = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfIASZoneClusterZoneStatusChangeNotificationCallback( *zoneStatus,  *extendedStatus,  *zoneId,  *delay);
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
EmberAfStatus emberAfIASZoneClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_ZONE_ENROLL_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfIasEnrollResponseCode * enrollResponseCode = (EmberAfIasEnrollResponseCode *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfIasEnrollResponseCode);
            uint8_t * zoneId = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfIASZoneClusterZoneEnrollResponseCallback( *enrollResponseCode,  *zoneId);
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
EmberAfStatus emberAfIdentifyClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_IDENTIFY_QUERY_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * timeout = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfIdentifyClusterIdentifyQueryResponseCallback( *timeout);
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
            uint32_t argOffset = 0;
            uint16_t * identifyTime = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfIdentifyClusterIdentifyCallback( *identifyTime);
            break;
        }
        case ZCL_IDENTIFY_QUERY_COMMAND_ID: {
            wasHandled = emberAfIdentifyClusterIdentifyQueryCallback();
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
            uint32_t argOffset = 0;
            EmberAfMoveMode * moveMode = (EmberAfMoveMode *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfMoveMode);
            uint8_t * rate = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint8_t * optionMask = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint8_t * optionOverride = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterMoveCallback( *moveMode,  *rate,  *optionMask,  *optionOverride);
            break;
        }
        case ZCL_MOVE_TO_LEVEL_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint8_t * level = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * optionMask = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint8_t * optionOverride = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterMoveToLevelCallback( *level,  *transitionTime,  *optionMask,  *optionOverride);
            break;
        }
        case ZCL_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint8_t * level = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterMoveToLevelWithOnOffCallback( *level,  *transitionTime);
            break;
        }
        case ZCL_MOVE_WITH_ON_OFF_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfMoveMode * moveMode = (EmberAfMoveMode *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfMoveMode);
            uint8_t * rate = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterMoveWithOnOffCallback( *moveMode,  *rate);
            break;
        }
        case ZCL_STEP_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfStepMode * stepMode = (EmberAfStepMode *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfStepMode);
            uint8_t * stepSize = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * optionMask = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint8_t * optionOverride = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterStepCallback( *stepMode,  *stepSize,  *transitionTime,  *optionMask,  *optionOverride);
            break;
        }
        case ZCL_STEP_WITH_ON_OFF_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfStepMode * stepMode = (EmberAfStepMode *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfStepMode);
            uint8_t * stepSize = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterStepWithOnOffCallback( *stepMode,  *stepSize,  *transitionTime);
            break;
        }
        case ZCL_STOP_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint8_t * optionMask = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint8_t * optionOverride = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfLevelControlClusterStopCallback( *optionMask,  *optionOverride);
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
        case ZCL_ON_COMMAND_ID: {
            wasHandled = emberAfOnOffClusterOnCallback();
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
EmberAfStatus emberAfOnOffSwitchConfigurationClusterServerCommandParse(EmberAfClusterCommand * cmd)
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
EmberAfStatus emberAfScenesClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_ADD_SCENE_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfStatus * status = (EmberAfStatus *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfStatus);
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterAddSceneResponseCallback( *status,  *groupId,  *sceneId);
            break;
        }
        case ZCL_GET_SCENE_MEMBERSHIP_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfStatus * status = (EmberAfStatus *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfStatus);
            uint8_t * capacity = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * sceneCount = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint8_t * sceneList = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterGetSceneMembershipResponseCallback( *status,  *capacity,  *groupId,  *sceneCount,  *sceneList);
            break;
        }
        case ZCL_REMOVE_ALL_SCENES_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfStatus * status = (EmberAfStatus *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfStatus);
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterRemoveAllScenesResponseCallback( *status,  *groupId);
            break;
        }
        case ZCL_REMOVE_SCENE_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfStatus * status = (EmberAfStatus *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfStatus);
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterRemoveSceneResponseCallback( *status,  *groupId,  *sceneId);
            break;
        }
        case ZCL_STORE_SCENE_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfStatus * status = (EmberAfStatus *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfStatus);
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterStoreSceneResponseCallback( *status,  *groupId,  *sceneId);
            break;
        }
        case ZCL_VIEW_SCENE_RESPONSE_COMMAND_ID: {
            uint32_t argOffset = 0;
            EmberAfStatus * status = (EmberAfStatus *)(cmd->buffer + argOffset);
            argOffset+= sizeof(EmberAfStatus);
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * * sceneName = (uint8_t * *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t *);
            EmberAfSceneExtensionFieldSet * extensionFieldSets = (EmberAfSceneExtensionFieldSet *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterViewSceneResponseCallback( *status,  *groupId,  *sceneId,  *transitionTime,  *sceneName,  *extensionFieldSets);
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
EmberAfStatus emberAfScenesClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {

        case ZCL_ADD_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * * sceneName = (uint8_t * *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t *);
            EmberAfSceneExtensionFieldSet * extensionFieldSets = (EmberAfSceneExtensionFieldSet *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterAddSceneCallback( *groupId,  *sceneId,  *transitionTime,  *sceneName,  *extensionFieldSets);
            break;
        }
        case ZCL_GET_SCENE_MEMBERSHIP_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterGetSceneMembershipCallback( *groupId);
            break;
        }
        case ZCL_RECALL_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint8_t);
            uint16_t * transitionTime = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterRecallSceneCallback( *groupId,  *sceneId,  *transitionTime);
            break;
        }
        case ZCL_REMOVE_ALL_SCENES_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterRemoveAllScenesCallback( *groupId);
            break;
        }
        case ZCL_REMOVE_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterRemoveSceneCallback( *groupId,  *sceneId);
            break;
        }
        case ZCL_STORE_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterStoreSceneCallback( *groupId,  *sceneId);
            break;
        }
        case ZCL_VIEW_SCENE_COMMAND_ID: {
            uint32_t argOffset = 0;
            uint16_t * groupId = (uint16_t *)(cmd->buffer + argOffset);
            argOffset+= sizeof(uint16_t);
            uint8_t * sceneId = (uint8_t *)(cmd->buffer + argOffset);

            wasHandled = emberAfScenesClusterViewSceneCallback( *groupId,  *sceneId);
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
EmberAfStatus emberAfTemperatureMeasurementClusterServerCommandParse(EmberAfClusterCommand * cmd)
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
