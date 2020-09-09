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
/****************************************************************************
 * @file
 * @brief Routines for the Scenes plugin, which
 *implements the server side of the Scenes cluster.
 *******************************************************************************
 ******************************************************************************/

#include "scenes.h"
#include "app/util/common.h"
#include <app/util/af.h>

#ifdef EMBER_AF_PLUGIN_ZLL_SCENES_SERVER
#include "../zll-scenes-server/zll-scenes-server.h"
#endif

uint8_t emberAfPluginScenesServerEntriesInUse = 0;
#if !defined(EMBER_AF_PLUGIN_SCENES_USE_TOKENS) || defined(EZSP_HOST)
EmberAfSceneTableEntry emberAfPluginScenesServerSceneTable[EMBER_AF_PLUGIN_SCENES_TABLE_SIZE];
#endif

static bool readServerAttribute(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId, const char * name,
                                uint8_t * data, uint8_t size)
{
    bool success = false;
    if (emberAfContainsServer(endpoint, clusterId))
    {
        EmberAfStatus status = emberAfReadServerAttribute(endpoint, clusterId, attributeId, data, size);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            success = true;
        }
        else
        {
            emberAfScenesClusterPrintln("ERR: %ping %p 0x%x", "read", name, status);
        }
    }
    return success;
}

static EmberAfStatus writeServerAttribute(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId,
                                          const char * name, uint8_t * data, EmberAfAttributeType type)
{
    EmberAfStatus status = emberAfWriteServerAttribute(endpoint, clusterId, attributeId, data, type);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfScenesClusterPrintln("ERR: %ping %p 0x%x", "writ", name, status);
    }
    return status;
}

void emberAfScenesClusterServerInitCallback(uint8_t endpoint)
{
#ifdef EMBER_AF_PLUGIN_SCENES_NAME_SUPPORT
    {
        // The high bit of Name Support indicates whether scene names are supported.
        uint8_t nameSupport = BIT(7);
        writeServerAttribute(endpoint, ZCL_SCENES_CLUSTER_ID, ZCL_SCENE_NAME_SUPPORT_ATTRIBUTE_ID, "name support",
                             (uint8_t *) &nameSupport, ZCL_BITMAP8_ATTRIBUTE_TYPE);
    }
#endif
#if !defined(EMBER_AF_PLUGIN_SCENES_USE_TOKENS) || defined(EZSP_HOST)
    {
        uint8_t i;
        for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
        {
            EmberAfSceneTableEntry entry;
            emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
            entry.endpoint = EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
            emberAfPluginScenesServerSaveSceneEntry(entry, i);
        }
        emberAfPluginScenesServerSetNumSceneEntriesInUse(0);
    }
#endif
    emberAfScenesSetSceneCountAttribute(endpoint, emberAfPluginScenesServerNumSceneEntriesInUse());
}

EmberAfStatus emberAfScenesSetSceneCountAttribute(uint8_t endpoint, uint8_t newCount)
{
    return writeServerAttribute(endpoint, ZCL_SCENES_CLUSTER_ID, ZCL_SCENE_COUNT_ATTRIBUTE_ID, "scene count", (uint8_t *) &newCount,
                                ZCL_INT8U_ATTRIBUTE_TYPE);
}

EmberAfStatus emberAfScenesMakeValid(uint8_t endpoint, uint8_t sceneId, uint16_t groupId)
{
    EmberAfStatus status;
    bool valid = true;

    // scene ID
    status = writeServerAttribute(endpoint, ZCL_SCENES_CLUSTER_ID, ZCL_CURRENT_SCENE_ATTRIBUTE_ID, "current scene",
                                  (uint8_t *) &sceneId, ZCL_INT8U_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }

    // group ID
    status = writeServerAttribute(endpoint, ZCL_SCENES_CLUSTER_ID, ZCL_CURRENT_GROUP_ATTRIBUTE_ID, "current group",
                                  (uint8_t *) &groupId, ZCL_INT16U_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }

    status = writeServerAttribute(endpoint, ZCL_SCENES_CLUSTER_ID, ZCL_SCENE_VALID_ATTRIBUTE_ID, "scene valid", (uint8_t *) &valid,
                                  ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    return status;
}

EmberAfStatus emberAfScenesClusterMakeInvalidCallback(uint8_t endpoint)
{
    bool valid = false;
    return writeServerAttribute(endpoint, ZCL_SCENES_CLUSTER_ID, ZCL_SCENE_VALID_ATTRIBUTE_ID, "scene valid", (uint8_t *) &valid,
                                ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}

void emAfPluginScenesServerPrintInfo(void)
{
    uint8_t i;
    EmberAfSceneTableEntry entry;
    emberAfCorePrintln("using 0x%x out of 0x%x table slots", emberAfPluginScenesServerNumSceneEntriesInUse(),
                       EMBER_AF_PLUGIN_SCENES_TABLE_SIZE);
    for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
    {
        emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
        emberAfCorePrint("%x: ", i);
        if (entry.endpoint != EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID)
        {
            emberAfCorePrint("ep %x grp %2x scene %x tt %d", entry.endpoint, entry.groupId, entry.sceneId, entry.transitionTime);
            emberAfCorePrint(".%d", entry.transitionTime100ms);
#ifdef EMBER_AF_PLUGIN_SCENES_NAME_SUPPORT
            emberAfCorePrint(" name(%x)\"", emberAfStringLength(entry.name));
            emberAfCorePrintString(entry.name);
            emberAfCorePrint("\"");
#endif
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
            emberAfCorePrint(" on/off %x", entry.onOffValue);
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
            emberAfCorePrint(" lvl %x", entry.currentLevelValue);
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
            emberAfCorePrint(" therm %2x %2x %x", entry.occupiedCoolingSetpointValue, entry.occupiedHeatingSetpointValue,
                             entry.systemModeValue);
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
            emberAfCorePrint(" color %2x %2x", entry.currentXValue, entry.currentYValue);
            emberAfCorePrint(" %2x %x %x %x %2x", entry.enhancedCurrentHueValue, entry.currentSaturationValue,
                             entry.colorLoopActiveValue, entry.colorLoopDirectionValue, entry.colorLoopTimeValue,
                             entry.colorTemperatureMiredsValue);
            emberAfCoreFlush();
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
            emberAfCorePrint(" door %x", entry.lockStateValue);
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
            emberAfCorePrint(" window %x %x", entry.currentPositionLiftPercentageValue, entry.currentPositionTiltPercentageValue);
#endif
        }
        emberAfCorePrintln("");
    }
}

bool emberAfScenesClusterAddSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime, uint8_t * sceneName,
                                          uint8_t * extensionFieldSets)
{
    return emberAfPluginScenesServerParseAddScene(emberAfCurrentCommand(), groupId, sceneId, transitionTime, sceneName,
                                                  extensionFieldSets);
}

bool emberAfScenesClusterViewSceneCallback(uint16_t groupId, uint8_t sceneId)
{
    return emberAfPluginScenesServerParseViewScene(emberAfCurrentCommand(), groupId, sceneId);
}

bool emberAfScenesClusterRemoveSceneCallback(uint16_t groupId, uint8_t sceneId)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_NOT_FOUND;
    EmberStatus sendStatus;

    emberAfScenesClusterPrintln("RX: RemoveScene 0x%2x, 0x%x", groupId, sceneId);

    // If a group id is specified but this endpoint isn't in it, take no action.
    if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID &&
        !emberAfGroupsClusterEndpointInGroupCallback(emberAfCurrentEndpoint(), groupId))
    {
        status = EMBER_ZCL_STATUS_INVALID_FIELD;
    }
    else
    {
        uint8_t i;
        for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
        {
            EmberAfSceneTableEntry entry;
            emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
            if (entry.endpoint == emberAfCurrentEndpoint() && entry.groupId == groupId && entry.sceneId == sceneId)
            {
                entry.endpoint = EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
                emberAfPluginScenesServerSaveSceneEntry(entry, i);
                emberAfPluginScenesServerDecrNumSceneEntriesInUse();
                emberAfScenesSetSceneCountAttribute(emberAfCurrentEndpoint(), emberAfPluginScenesServerNumSceneEntriesInUse());
                status = EMBER_ZCL_STATUS_SUCCESS;
                break;
            }
        }
    }

    // Remove Scene commands are only responded to when they are addressed to a
    // single device.
    if (emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST || emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST_REPLY)
    {
        emberAfFillCommandScenesClusterRemoveSceneResponse(status, groupId, sceneId);
        sendStatus = emberAfSendResponse();
        if (EMBER_SUCCESS != sendStatus)
        {
            emberAfScenesClusterPrintln("Scenes: failed to send %s response: 0x%x", "remove_scene", sendStatus);
        }
    }
    return true;
}

bool emberAfScenesClusterRemoveAllScenesCallback(uint16_t groupId)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_INVALID_FIELD;
    EmberStatus sendStatus;

    emberAfScenesClusterPrintln("RX: RemoveAllScenes 0x%2x", groupId);

    if (groupId == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID ||
        emberAfGroupsClusterEndpointInGroupCallback(emberAfCurrentEndpoint(), groupId))
    {
        uint8_t i;
        status = EMBER_ZCL_STATUS_SUCCESS;
        for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
        {
            EmberAfSceneTableEntry entry;
            emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
            if (entry.endpoint == emberAfCurrentEndpoint() && entry.groupId == groupId)
            {
                entry.endpoint = EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
                emberAfPluginScenesServerSaveSceneEntry(entry, i);
                emberAfPluginScenesServerDecrNumSceneEntriesInUse();
            }
        }
        emberAfScenesSetSceneCountAttribute(emberAfCurrentEndpoint(), emberAfPluginScenesServerNumSceneEntriesInUse());
    }

    // Remove All Scenes commands are only responded to when they are addressed
    // to a single device.
    if (emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST || emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST_REPLY)
    {
        emberAfFillCommandScenesClusterRemoveAllScenesResponse(status, groupId);
        sendStatus = emberAfSendResponse();
        if (EMBER_SUCCESS != sendStatus)
        {
            emberAfScenesClusterPrintln("Scenes: failed to send %s response: 0x%x", "remove_all_scenes", sendStatus);
        }
    }
    return true;
}

bool emberAfScenesClusterStoreSceneCallback(uint16_t groupId, uint8_t sceneId)
{
    EmberAfStatus status;
    EmberStatus sendStatus;
    emberAfScenesClusterPrintln("RX: StoreScene 0x%2x, 0x%x", groupId, sceneId);
    status = emberAfScenesClusterStoreCurrentSceneCallback(emberAfCurrentEndpoint(), groupId, sceneId);

    // Store Scene commands are only responded to when they are addressed to a
    // single device.
    if (emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST || emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST_REPLY)
    {
        emberAfFillCommandScenesClusterStoreSceneResponse(status, groupId, sceneId);
        sendStatus = emberAfSendResponse();
        if (EMBER_SUCCESS != sendStatus)
        {
            emberAfScenesClusterPrintln("Scenes: failed to send %s response: 0x%x", "store_scene", sendStatus);
        }
    }
    return true;
}

bool emberAfScenesClusterRecallSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime)
{
    // NOTE: TransitionTime field in the RecallScene command is currently
    // ignored. Per Zigbee Alliance ZCL 7 (07-5123-07):
    //
    // "The transition time determines how long the tranition takes from the
    // old cluster state to the new cluster state. It is recommended that, where
    // possible (e.g., it is not possible for attributes with Boolean type),
    // a gradual transition SHOULD take place from the old to the new state
    // over this time. However, the exact transition is manufacturer dependent."
    //
    // The manufacturer-dependent implementation here is to immediately set
    // all attributes to their scene-specified values, without regard to the
    // value of TransitionTime.

    EmberAfStatus status;
    EmberStatus sendStatus;
    emberAfScenesClusterPrintln("RX: RecallScene 0x%2x, 0x%x", groupId, sceneId);
    status = emberAfScenesClusterRecallSavedSceneCallback(emberAfCurrentEndpoint(), groupId, sceneId);
#ifdef EMBER_AF_PLUGIN_ZLL_SCENES_SERVER
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPluginZllScenesServerRecallSceneZllExtensions(emberAfCurrentEndpoint());
    }
#endif
    sendStatus = emberAfSendImmediateDefaultResponse(status);
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfScenesClusterPrintln("Scenes: failed to send %s: 0x%x", "default_response", sendStatus);
    }
    return true;
}

bool emberAfScenesClusterGetSceneMembershipCallback(uint16_t groupId)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    EmberStatus sendStatus;
    uint8_t sceneCount = 0;

    emberAfScenesClusterPrintln("RX: GetSceneMembership 0x%2x", groupId);

    if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID &&
        !emberAfGroupsClusterEndpointInGroupCallback(emberAfCurrentEndpoint(), groupId))
    {
        status = EMBER_ZCL_STATUS_INVALID_FIELD;
    }

    // The status, capacity, and group id are always included in the response, but
    // the scene count and scene list are only included if the group id matched.
    emberAfFillExternalBuffer(
        (ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT | EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES),
        ZCL_SCENES_CLUSTER_ID, ZCL_GET_SCENE_MEMBERSHIP_RESPONSE_COMMAND_ID, "uuv", status,
        (EMBER_AF_PLUGIN_SCENES_TABLE_SIZE - emberAfPluginScenesServerNumSceneEntriesInUse()), // capacity
        groupId);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        uint8_t i, sceneList[EMBER_AF_PLUGIN_SCENES_TABLE_SIZE];
        for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
        {
            EmberAfSceneTableEntry entry;
            emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
            if (entry.endpoint == emberAfCurrentEndpoint() && entry.groupId == groupId)
            {
                sceneList[sceneCount] = entry.sceneId;
                sceneCount++;
            }
        }
        emberAfPutInt8uInResp(sceneCount);
        for (i = 0; i < sceneCount; i++)
        {
            emberAfPutInt8uInResp(sceneList[i]);
        }
    }

    // Get Scene Membership commands are only responded to when they are
    // addressed to a single device or when an entry in the table matches.
    if (emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST || emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST_REPLY ||
        sceneCount != 0)
    {
        sendStatus = emberAfSendResponse();
        if (EMBER_SUCCESS != sendStatus)
        {
            emberAfScenesClusterPrintln("Scenes: failed to send %s response: 0x%x", "get_scene_membership", sendStatus);
        }
    }
    return true;
}

EmberAfStatus emberAfScenesClusterStoreCurrentSceneCallback(uint8_t endpoint, uint16_t groupId, uint8_t sceneId)
{
    EmberAfSceneTableEntry entry;
    uint8_t i, index = EMBER_AF_SCENE_TABLE_NULL_INDEX;

    // If a group id is specified but this endpoint isn't in it, take no action.
    if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID && !emberAfGroupsClusterEndpointInGroupCallback(endpoint, groupId))
    {
        return EMBER_ZCL_STATUS_INVALID_FIELD;
    }

    for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
    {
        emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
        if (entry.endpoint == endpoint && entry.groupId == groupId && entry.sceneId == sceneId)
        {
            index = i;
            break;
        }
        else if (index == EMBER_AF_SCENE_TABLE_NULL_INDEX && entry.endpoint == EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID)
        {
            index = i;
        }
    }

    // If the target index is still zero, the table is full.
    if (index == EMBER_AF_SCENE_TABLE_NULL_INDEX)
    {
        return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
    }

    emberAfPluginScenesServerRetrieveSceneEntry(entry, index);

    // When creating a new entry or refreshing an existing one, the extension
    // fields are updated with the current state of other clusters on the device.
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
    entry.hasOnOffValue = readServerAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, "on/off",
                                              (uint8_t *) &entry.onOffValue, sizeof(entry.onOffValue));
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
    entry.hasCurrentLevelValue =
        readServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, "current level",
                            (uint8_t *) &entry.currentLevelValue, sizeof(entry.currentLevelValue));
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
    entry.hasOccupiedCoolingSetpointValue = readServerAttribute(
        endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID, "occupied cooling setpoint",
        (uint8_t *) &entry.occupiedCoolingSetpointValue, sizeof(entry.occupiedCoolingSetpointValue));
    entry.hasOccupiedHeatingSetpointValue = readServerAttribute(
        endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID, "occupied heating setpoint",
        (uint8_t *) &entry.occupiedHeatingSetpointValue, sizeof(entry.occupiedHeatingSetpointValue));
    entry.hasSystemModeValue = readServerAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_SYSTEM_MODE_ATTRIBUTE_ID, "system mode",
                                                   (uint8_t *) &entry.systemModeValue, sizeof(entry.systemModeValue));
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
    entry.hasCurrentXValue = readServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID,
                                                 "current x", (uint8_t *) &entry.currentXValue, sizeof(entry.currentXValue));
    entry.hasCurrentYValue = readServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID,
                                                 "current y", (uint8_t *) &entry.currentYValue, sizeof(entry.currentYValue));
    entry.hasEnhancedCurrentHueValue = readServerAttribute(
        endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_ENHANCED_CURRENT_HUE_ATTRIBUTE_ID, "enhanced current hue",
        (uint8_t *) &entry.enhancedCurrentHueValue, sizeof(entry.enhancedCurrentHueValue));
    entry.hasCurrentSaturationValue =
        readServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                            "current saturation", (uint8_t *) &entry.currentSaturationValue, sizeof(entry.currentSaturationValue));
    entry.hasColorLoopActiveValue =
        readServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_COLOR_LOOP_ACTIVE_ATTRIBUTE_ID,
                            "color loop active", (uint8_t *) &entry.colorLoopActiveValue, sizeof(entry.colorLoopActiveValue));
    entry.hasColorLoopDirectionValue = readServerAttribute(
        endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_COLOR_LOOP_DIRECTION_ATTRIBUTE_ID, "color loop direction",
        (uint8_t *) &entry.colorLoopDirectionValue, sizeof(entry.colorLoopDirectionValue));
    entry.hasColorLoopTimeValue =
        readServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_COLOR_LOOP_TIME_ATTRIBUTE_ID,
                            "color loop time", (uint8_t *) &entry.colorLoopTimeValue, sizeof(entry.colorLoopTimeValue));
    entry.hasColorTemperatureMiredsValue = readServerAttribute(
        endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID, "color temp mireds",
        (uint8_t *) &entry.colorTemperatureMiredsValue, sizeof(entry.colorTemperatureMiredsValue));
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
    entry.hasLockStateValue = readServerAttribute(endpoint, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_LOCK_STATE_ATTRIBUTE_ID, "lock state",
                                                  (uint8_t *) &entry.lockStateValue, sizeof(entry.lockStateValue));
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
    entry.hasCurrentPositionLiftPercentageValue = readServerAttribute(
        endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_CURRENT_LIFT_PERCENTAGE_ATTRIBUTE_ID, "current position lift percentage",
        (uint8_t *) &entry.currentPositionLiftPercentageValue, sizeof(entry.currentPositionLiftPercentageValue));
    entry.hasCurrentPositionTiltPercentageValue = readServerAttribute(
        endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_CURRENT_TILT_PERCENTAGE_ATTRIBUTE_ID, "current position tilt percentage",
        (uint8_t *) &entry.currentPositionTiltPercentageValue, sizeof(entry.currentPositionTiltPercentageValue));
#endif

    // When creating a new entry, the name is set to the null string (i.e., the
    // length is set to zero) and the transition time is set to zero.  The scene
    // count must be increased and written to the attribute table when adding a
    // new scene.  Otherwise, these fields and the count are left alone.
    if (i != index)
    {
        entry.endpoint = endpoint;
        entry.groupId  = groupId;
        entry.sceneId  = sceneId;
#ifdef EMBER_AF_PLUGIN_SCENES_NAME_SUPPORT
        entry.name[0] = 0;
#endif
        entry.transitionTime      = 0;
        entry.transitionTime100ms = 0;
        emberAfPluginScenesServerIncrNumSceneEntriesInUse();
        emberAfScenesSetSceneCountAttribute(endpoint, emberAfPluginScenesServerNumSceneEntriesInUse());
    }

    // Save the scene entry and mark is as valid by storing its scene and group
    // ids in the attribute table and setting valid to true.
    emberAfPluginScenesServerSaveSceneEntry(entry, index);
    emberAfScenesMakeValid(endpoint, sceneId, groupId);
    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfScenesClusterRecallSavedSceneCallback(uint8_t endpoint, uint16_t groupId, uint8_t sceneId)
{
    if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID && !emberAfGroupsClusterEndpointInGroupCallback(endpoint, groupId))
    {
        return EMBER_ZCL_STATUS_INVALID_FIELD;
    }
    else
    {
        uint8_t i;
        for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
        {
            EmberAfSceneTableEntry entry;
            emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
            if (entry.endpoint == endpoint && entry.groupId == groupId && entry.sceneId == sceneId)
            {
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
                if (entry.hasOnOffValue)
                {
                    writeServerAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, "on/off",
                                         (uint8_t *) &entry.onOffValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
                }
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
                if (entry.hasCurrentLevelValue)
                {
                    writeServerAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, "current level",
                                         (uint8_t *) &entry.currentLevelValue, ZCL_INT8U_ATTRIBUTE_TYPE);
                }
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
                if (entry.hasOccupiedCoolingSetpointValue)
                {
                    writeServerAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID,
                                         "occupied cooling setpoint", (uint8_t *) &entry.occupiedCoolingSetpointValue,
                                         ZCL_INT16S_ATTRIBUTE_TYPE);
                }
                if (entry.hasOccupiedHeatingSetpointValue)
                {
                    writeServerAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID,
                                         "occupied heating setpoint", (uint8_t *) &entry.occupiedHeatingSetpointValue,
                                         ZCL_INT16S_ATTRIBUTE_TYPE);
                }
                if (entry.hasSystemModeValue)
                {
                    writeServerAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_SYSTEM_MODE_ATTRIBUTE_ID, "system mode",
                                         (uint8_t *) &entry.systemModeValue, ZCL_INT8U_ATTRIBUTE_TYPE);
                }
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
                if (entry.hasCurrentXValue)
                {
                    writeServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID,
                                         "current x", (uint8_t *) &entry.currentXValue, ZCL_INT16U_ATTRIBUTE_TYPE);
                }
                if (entry.hasCurrentYValue)
                {
                    writeServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID,
                                         "current y", (uint8_t *) &entry.currentYValue, ZCL_INT16U_ATTRIBUTE_TYPE);
                }

                if (entry.hasEnhancedCurrentHueValue)
                {
                    writeServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID,
                                         ZCL_COLOR_CONTROL_ENHANCED_CURRENT_HUE_ATTRIBUTE_ID, "enhanced current hue",
                                         (uint8_t *) &entry.enhancedCurrentHueValue, ZCL_INT16U_ATTRIBUTE_TYPE);
                }
                if (entry.hasCurrentSaturationValue)
                {
                    writeServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                                         "current saturation", (uint8_t *) &entry.currentSaturationValue, ZCL_INT8U_ATTRIBUTE_TYPE);
                }
                if (entry.hasColorLoopActiveValue)
                {
                    writeServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_COLOR_LOOP_ACTIVE_ATTRIBUTE_ID,
                                         "color loop active", (uint8_t *) &entry.colorLoopActiveValue, ZCL_INT8U_ATTRIBUTE_TYPE);
                }
                if (entry.hasColorLoopDirectionValue)
                {
                    writeServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID,
                                         ZCL_COLOR_CONTROL_COLOR_LOOP_DIRECTION_ATTRIBUTE_ID, "color loop direction",
                                         (uint8_t *) &entry.colorLoopDirectionValue, ZCL_INT8U_ATTRIBUTE_TYPE);
                }
                if (entry.hasColorLoopTimeValue)
                {
                    writeServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_COLOR_LOOP_TIME_ATTRIBUTE_ID,
                                         "color loop time", (uint8_t *) &entry.colorLoopTimeValue, ZCL_INT16U_ATTRIBUTE_TYPE);
                }
                if (entry.hasColorTemperatureMiredsValue)
                {
                    writeServerAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID,
                                         "color temp mireds", (uint8_t *) &entry.colorTemperatureMiredsValue,
                                         ZCL_INT16U_ATTRIBUTE_TYPE);
                }
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
                if (entry.hasLockStateValue)
                {
                    writeServerAttribute(endpoint, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_LOCK_STATE_ATTRIBUTE_ID, "lock state",
                                         (uint8_t *) &entry.lockStateValue, ZCL_INT8U_ATTRIBUTE_TYPE);
                }
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
                if (entry.hasCurrentPositionLiftPercentageValue)
                {
                    writeServerAttribute(endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_CURRENT_LIFT_PERCENTAGE_ATTRIBUTE_ID,
                                         "current position lift percentage", (uint8_t *) &entry.currentPositionLiftPercentageValue,
                                         ZCL_INT8U_ATTRIBUTE_TYPE);
                }
                if (entry.hasCurrentPositionTiltPercentageValue)
                {
                    writeServerAttribute(endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_CURRENT_TILT_PERCENTAGE_ATTRIBUTE_ID,
                                         "current position tilt percentage", (uint8_t *) &entry.currentPositionTiltPercentageValue,
                                         ZCL_INT8U_ATTRIBUTE_TYPE);
                }
#endif
                emberAfScenesMakeValid(endpoint, sceneId, groupId);
                return EMBER_ZCL_STATUS_SUCCESS;
            }
        }
    }

    return EMBER_ZCL_STATUS_NOT_FOUND;
}

void emberAfScenesClusterClearSceneTableCallback(uint8_t endpoint)
{
    uint8_t i, networkIndex = 0 /* emberGetCurrentNetwork() */;
    for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
    {
        EmberAfSceneTableEntry entry;
        emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
        if (entry.endpoint != EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID &&
            (endpoint == entry.endpoint ||
             (endpoint == EMBER_BROADCAST_ENDPOINT && (networkIndex == emberAfNetworkIndexFromEndpoint(entry.endpoint)))))
        {
            entry.endpoint = EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
            emberAfPluginScenesServerSaveSceneEntry(entry, i);
        }
    }
    emberAfPluginScenesServerSetNumSceneEntriesInUse(0);
    if (endpoint == EMBER_BROADCAST_ENDPOINT)
    {
        for (i = 0; i < emberAfEndpointCount(); i++)
        {
            if (emberAfNetworkIndexFromEndpointIndex(i) == networkIndex)
            {
                emberAfScenesSetSceneCountAttribute(emberAfEndpointFromIndex(i), 0);
            }
        }
    }
    else
    {
        emberAfScenesSetSceneCountAttribute(endpoint, 0);
    }
}

bool emberAfPluginScenesServerParseAddScene(const EmberAfClusterCommand * cmd, uint16_t groupId, uint8_t sceneId,
                                            uint16_t transitionTime, uint8_t * sceneName, uint8_t * extensionFieldSets)
{
    EmberAfSceneTableEntry entry;
    EmberAfStatus status;
    EmberStatus sendStatus;
    bool enhanced                    = (cmd->commandId == ZCL_ENHANCED_ADD_SCENE_COMMAND_ID);
    uint16_t extensionFieldSetsLen   = (cmd->bufLen -
                                      (cmd->payloadStartIndex + sizeof(groupId) + sizeof(sceneId) + sizeof(transitionTime) +
                                       emberAfStringLength(sceneName) + 1));
    uint16_t extensionFieldSetsIndex = 0;
    uint8_t endpoint                 = cmd->apsFrame->destinationEndpoint;
    uint8_t i, index = EMBER_AF_SCENE_TABLE_NULL_INDEX;

    emberAfScenesClusterPrint("RX: %pAddScene 0x%2x, 0x%x, 0x%2x, \"", (enhanced ? "Enhanced" : ""), groupId, sceneId,
                              transitionTime);
    emberAfScenesClusterPrintString(sceneName);
    emberAfScenesClusterPrint("\", ");
    emberAfScenesClusterPrintBuffer(extensionFieldSets, extensionFieldSetsLen, false);
    emberAfScenesClusterPrintln("");

    // Add Scene commands can only reference groups to which we belong.
    if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID && !emberAfGroupsClusterEndpointInGroupCallback(endpoint, groupId))
    {
        status = EMBER_ZCL_STATUS_INVALID_FIELD;
        goto kickout;
    }

    for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
    {
        emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
        if (entry.endpoint == endpoint && entry.groupId == groupId && entry.sceneId == sceneId)
        {
            index = i;
            break;
        }
        else if (index == EMBER_AF_SCENE_TABLE_NULL_INDEX && entry.endpoint == EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID)
        {
            index = i;
        }
    }

    // If the target index is still zero, the table is full.
    if (index == EMBER_AF_SCENE_TABLE_NULL_INDEX)
    {
        status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
        goto kickout;
    }

    emberAfPluginScenesServerRetrieveSceneEntry(entry, index);

    // The transition time is specified in seconds in the regular version of the
    // command and tenths of a second in the enhanced version.
    if (enhanced)
    {
        entry.transitionTime      = transitionTime / 10;
        entry.transitionTime100ms = (uint8_t)(transitionTime - entry.transitionTime * 10);
    }
    else
    {
        entry.transitionTime      = transitionTime;
        entry.transitionTime100ms = 0;
    }

#ifdef EMBER_AF_PLUGIN_SCENES_NAME_SUPPORT
    emberAfCopyString(entry.name, sceneName, ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH);
#endif

    // When adding a new scene, wipe out all of the extensions before parsing the
    // extension field sets data.
    if (i != index)
    {
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
        entry.hasOnOffValue = false;
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
        entry.hasCurrentLevelValue = false;
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
        entry.hasOccupiedCoolingSetpointValue = false;
        entry.hasOccupiedHeatingSetpointValue = false;
        entry.hasSystemModeValue              = false;
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
        entry.hasCurrentXValue               = false;
        entry.hasCurrentYValue               = false;
        entry.hasEnhancedCurrentHueValue     = false;
        entry.hasCurrentSaturationValue      = false;
        entry.hasColorLoopActiveValue        = false;
        entry.hasColorLoopDirectionValue     = false;
        entry.hasColorLoopTimeValue          = false;
        entry.hasColorTemperatureMiredsValue = false;
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
        entry.hasLockStateValue = false;
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
        entry.hasCurrentPositionLiftPercentageValue = false;
        entry.hasCurrentPositionTiltPercentageValue = false;
#endif
    }

    while (extensionFieldSetsIndex < extensionFieldSetsLen)
    {
        EmberAfClusterId clusterId;
        uint8_t length;

        // Each extension field set must contain a two-byte cluster id and a one-
        // byte length.  Otherwise, the command is malformed.
        if (extensionFieldSetsLen < extensionFieldSetsIndex + 3)
        {
            status = EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            goto kickout;
        }

        clusterId = emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        extensionFieldSetsIndex += 2;
        length = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        extensionFieldSetsIndex++;

        // If the length is off, the command is also malformed.
        if (length == 0)
        {
            continue;
        }
        else if (extensionFieldSetsLen < extensionFieldSetsIndex + length)
        {
            status = EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            goto kickout;
        }

        switch (clusterId)
        {
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
        case ZCL_ON_OFF_CLUSTER_ID:
            // We only know of one extension for the On/Off cluster and it is just one
            // byte, which means we can skip some logic for this cluster.  If other
            // extensions are added in this cluster, more logic will be needed here.
            entry.hasOnOffValue = true;
            entry.onOffValue    = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            break;
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
        case ZCL_LEVEL_CONTROL_CLUSTER_ID:
            // We only know of one extension for the Level Control cluster and it is
            // just one byte, which means we can skip some logic for this cluster.  If
            // other extensions are added in this cluster, more logic will be needed
            // here.
            entry.hasCurrentLevelValue = true;
            entry.currentLevelValue    = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            break;
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
        case ZCL_THERMOSTAT_CLUSTER_ID:
            if (length < 2)
            {
                break;
            }
            entry.hasOccupiedCoolingSetpointValue = true;
            entry.occupiedCoolingSetpointValue =
                (int16_t) emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex += 2;
            length -= 2;
            if (length < 2)
            {
                break;
            }
            entry.hasOccupiedHeatingSetpointValue = true;
            entry.occupiedHeatingSetpointValue =
                (int16_t) emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex += 2;
            length -= 2;
            if (length < 1)
            {
                break;
            }
            entry.hasSystemModeValue = true;
            entry.systemModeValue    = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            // If additional Thermostat extensions are added, adjust the index and
            // length variables here.
            break;
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
        case ZCL_COLOR_CONTROL_CLUSTER_ID:
            if (length < 2)
            {
                break;
            }
            entry.hasCurrentXValue = true;
            entry.currentXValue    = emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex += 2;
            length -= 2;
            if (length < 2)
            {
                break;
            }
            entry.hasCurrentYValue = true;
            entry.currentYValue    = emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            if (enhanced)
            {
                extensionFieldSetsIndex += 2;
                length -= 2;
                if (length < 2)
                {
                    break;
                }
                entry.hasEnhancedCurrentHueValue = true;
                entry.enhancedCurrentHueValue =
                    emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                extensionFieldSetsIndex += 2;
                length -= 2;
                if (length < 1)
                {
                    break;
                }
                entry.hasCurrentSaturationValue = true;
                entry.currentSaturationValue = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                extensionFieldSetsIndex++;
                length--;
                if (length < 1)
                {
                    break;
                }
                entry.hasColorLoopActiveValue = true;
                entry.colorLoopActiveValue    = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                extensionFieldSetsIndex++;
                length--;
                if (length < 1)
                {
                    break;
                }
                entry.hasColorLoopDirectionValue = true;
                entry.colorLoopDirectionValue = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                extensionFieldSetsIndex++;
                length--;
                if (length < 2)
                {
                    break;
                }
                entry.hasColorLoopTimeValue = true;
                entry.colorLoopTimeValue    = emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                extensionFieldSetsIndex += 2;
                length -= 2;
                if (length < 2)
                {
                    break;
                }
                entry.hasColorTemperatureMiredsValue = true;
                entry.colorTemperatureMiredsValue =
                    emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            }
            // If additional Color Control extensions are added, adjust the index and
            // length variables here.
            break;
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
        case ZCL_DOOR_LOCK_CLUSTER_ID:
            // We only know of one extension for the Door Lock cluster and it is just
            // one byte, which means we can skip some logic for this cluster.  If
            // other extensions are added in this cluster, more logic will be needed
            // here.
            entry.hasLockStateValue = true;
            entry.lockStateValue    = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            break;
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
        case ZCL_WINDOW_COVERING_CLUSTER_ID:
            // If we're here, we know we have at least one byte, so we can skip the
            // length check for the first field.
            entry.hasCurrentPositionLiftPercentageValue = true;
            entry.currentPositionLiftPercentageValue =
                emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex++;
            length--;
            if (length < 1)
            {
                break;
            }
            entry.hasCurrentPositionTiltPercentageValue = true;
            entry.currentPositionTiltPercentageValue =
                emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            // If additional Window Covering extensions are added, adjust the index
            // and length variables here.
            break;
#endif
        default:
            break;
        }

        extensionFieldSetsIndex += length;
    }

    // If we got this far, we either added a new entry or updated an existing one.
    // If we added, store the basic data and increment the scene count.  In either
    // case, save the entry.
    if (i != index)
    {
        entry.endpoint = endpoint;
        entry.groupId  = groupId;
        entry.sceneId  = sceneId;
        emberAfPluginScenesServerIncrNumSceneEntriesInUse();
        emberAfScenesSetSceneCountAttribute(endpoint, emberAfPluginScenesServerNumSceneEntriesInUse());
    }
    emberAfPluginScenesServerSaveSceneEntry(entry, index);
    status = EMBER_ZCL_STATUS_SUCCESS;

kickout:
    // Add Scene commands are only responded to when they are addressed to a
    // single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_SCENES_CLUSTER_ID,
                              (enhanced ? ZCL_ENHANCED_ADD_SCENE_RESPONSE_COMMAND_ID : ZCL_ADD_SCENE_RESPONSE_COMMAND_ID), "uvu",
                              status, groupId, sceneId);
    sendStatus = emberAfSendResponse();
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfScenesClusterPrintln("Scenes: failed to send %s response: 0x%x", "add_scene", sendStatus);
    }
    return true;
}

bool emberAfPluginScenesServerParseViewScene(const EmberAfClusterCommand * cmd, uint16_t groupId, uint8_t sceneId)
{
    EmberAfSceneTableEntry entry = {};
    EmberAfStatus status         = EMBER_ZCL_STATUS_NOT_FOUND;
    EmberStatus sendStatus;
    bool enhanced    = (cmd->commandId == ZCL_ENHANCED_VIEW_SCENE_COMMAND_ID);
    uint8_t endpoint = cmd->apsFrame->destinationEndpoint;

    emberAfScenesClusterPrintln("RX: %pViewScene 0x%2x, 0x%x", (enhanced ? "Enhanced" : ""), groupId, sceneId);

    // View Scene commands can only reference groups which we belong to.
    if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID && !emberAfGroupsClusterEndpointInGroupCallback(endpoint, groupId))
    {
        status = EMBER_ZCL_STATUS_INVALID_FIELD;
    }
    else
    {
        uint8_t i;
        for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
        {
            emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
            if (entry.endpoint == endpoint && entry.groupId == groupId && entry.sceneId == sceneId)
            {
                status = EMBER_ZCL_STATUS_SUCCESS;
                break;
            }
        }
    }

    // The status, group id, and scene id are always included in the response, but
    // the transition time, name, and extension fields are only included if the
    // scene was found.
    emberAfFillExternalBuffer(
        (ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT | EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES),
        ZCL_SCENES_CLUSTER_ID, (enhanced ? ZCL_ENHANCED_VIEW_SCENE_RESPONSE_COMMAND_ID : ZCL_VIEW_SCENE_RESPONSE_COMMAND_ID), "uvu",
        status, groupId, sceneId);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // The transition time is returned in seconds in the regular version of the
        // command and tenths of a second in the enhanced version.
        emberAfPutInt16uInResp(enhanced ? entry.transitionTime * 10 + entry.transitionTime100ms : entry.transitionTime);
#ifdef EMBER_AF_PLUGIN_SCENES_NAME_SUPPORT
        emberAfPutStringInResp(entry.name);
#else
        emberAfPutInt8uInResp(0); // name length
#endif
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
        if (entry.hasOnOffValue)
        {
            emberAfPutInt16uInResp(ZCL_ON_OFF_CLUSTER_ID);
            emberAfPutInt8uInResp(1); // length
            emberAfPutInt8uInResp(entry.onOffValue);
        }
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
        if (entry.hasCurrentLevelValue)
        {
            emberAfPutInt16uInResp(ZCL_LEVEL_CONTROL_CLUSTER_ID);
            emberAfPutInt8uInResp(1); // length
            emberAfPutInt8uInResp(entry.currentLevelValue);
        }
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
        if (entry.hasOccupiedCoolingSetpointValue)
        {
            uint8_t * length;
            emberAfPutInt16uInResp(ZCL_THERMOSTAT_CLUSTER_ID);
            length = &appResponseData[appResponseLength];
            emberAfPutInt8uInResp(0); // temporary length
            emberAfPutInt16uInResp(entry.occupiedCoolingSetpointValue);
            *length += 2;
            if (entry.hasOccupiedHeatingSetpointValue)
            {
                emberAfPutInt16uInResp(entry.occupiedHeatingSetpointValue);
                *length += 2;
                if (entry.hasSystemModeValue)
                {
                    emberAfPutInt8uInResp(entry.systemModeValue);
                    (*length)++;
                }
            }
        }
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
        if (entry.hasCurrentXValue)
        {
            uint8_t * length;
            emberAfPutInt16uInResp(ZCL_COLOR_CONTROL_CLUSTER_ID);
            length = &appResponseData[appResponseLength];
            emberAfPutInt8uInResp(0); // temporary length
            emberAfPutInt16uInResp(entry.currentXValue);
            *length += 2;
            if (entry.hasCurrentYValue)
            {
                emberAfPutInt16uInResp(entry.currentYValue);
                *length += 2;
                if (enhanced)
                {
                    if (entry.hasEnhancedCurrentHueValue)
                    {
                        emberAfPutInt16uInResp(entry.enhancedCurrentHueValue);
                        *length += 2;
                        if (entry.hasCurrentSaturationValue)
                        {
                            emberAfPutInt8uInResp(entry.currentSaturationValue);
                            (*length)++;
                            if (entry.hasColorLoopActiveValue)
                            {
                                emberAfPutInt8uInResp(entry.colorLoopActiveValue);
                                (*length)++;
                                if (entry.hasColorLoopDirectionValue)
                                {
                                    emberAfPutInt8uInResp(entry.colorLoopDirectionValue);
                                    (*length)++;
                                    if (entry.hasColorLoopTimeValue)
                                    {
                                        emberAfPutInt16uInResp(entry.colorLoopTimeValue);
                                        *length += 2;
                                        if (entry.hasColorTemperatureMiredsValue)
                                        {
                                            emberAfPutInt16uInResp(entry.colorTemperatureMiredsValue);
                                            *length += 2;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
        if (entry.hasLockStateValue)
        {
            emberAfPutInt16uInResp(ZCL_DOOR_LOCK_CLUSTER_ID);
            emberAfPutInt8uInResp(1); // length
            emberAfPutInt8uInResp(entry.lockStateValue);
        }
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
        if (entry.hasCurrentPositionLiftPercentageValue)
        {
            uint8_t * length;
            emberAfPutInt16uInResp(ZCL_WINDOW_COVERING_CLUSTER_ID);
            length = &appResponseData[appResponseLength];
            emberAfPutInt8uInResp(0); // temporary length
            emberAfPutInt8uInResp(entry.currentPositionLiftPercentageValue);
            (*length)++;
            if (entry.hasCurrentPositionTiltPercentageValue)
            {
                emberAfPutInt8uInResp(entry.currentPositionTiltPercentageValue);
                (*length)++;
            }
        }
#endif
    }

    // View Scene commands are only responded to when they are addressed to a
    // single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }
    sendStatus = emberAfSendResponse();
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfScenesClusterPrintln("Scenes: failed to send %s response: 0x%x", "view_scene", sendStatus);
    }
    return true;
}

void emberAfScenesClusterRemoveScenesInGroupCallback(uint8_t endpoint, uint16_t groupId)
{
    uint8_t i;
    for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
    {
        EmberAfSceneTableEntry entry;
        emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
        if (entry.endpoint == endpoint && entry.groupId == groupId)
        {
            entry.groupId  = ZCL_SCENES_GLOBAL_SCENE_GROUP_ID;
            entry.endpoint = EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
            emberAfPluginScenesServerSaveSceneEntry(entry, i);
            emberAfPluginScenesServerDecrNumSceneEntriesInUse();
            emberAfScenesSetSceneCountAttribute(emberAfCurrentEndpoint(), emberAfPluginScenesServerNumSceneEntriesInUse());
        }
    }
}
