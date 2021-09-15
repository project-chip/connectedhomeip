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
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>

#ifdef EMBER_AF_PLUGIN_GROUPS_SERVER
#include <app/clusters/groups-server/groups-server.h>
#endif

#ifdef EMBER_AF_PLUGIN_ZLL_SCENES_SERVER
#include "../zll-scenes-server/zll-scenes-server.h"
#endif

using namespace chip;

uint8_t emberAfPluginScenesServerEntriesInUse = 0;
#if !defined(EMBER_AF_PLUGIN_SCENES_USE_TOKENS) || defined(EZSP_HOST)
EmberAfSceneTableEntry emberAfPluginScenesServerSceneTable[EMBER_AF_PLUGIN_SCENES_TABLE_SIZE];
#endif

static bool readServerAttribute(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, const char * name,
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

static EmberAfStatus writeServerAttribute(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, const char * name,
                                          uint8_t * data, EmberAfAttributeType type)
{
    EmberAfStatus status = emberAfWriteServerAttribute(endpoint, clusterId, attributeId, data, type);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfScenesClusterPrintln("ERR: %ping %p 0x%x", "writ", name, status);
    }
    return status;
}

bool isEndpointInGroup(EndpointId endpoint, GroupId groupId)
{
#ifdef EMBER_AF_PLUGIN_GROUPS_SERVER
    return (groupId == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID || emberAfGroupsClusterEndpointInGroupCallback(endpoint, groupId));
#else
    return (groupId == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID);
#endif // EMBER_AF_PLUGIN_GROUPS_SERVER
}

void emberAfScenesClusterServerInitCallback(EndpointId endpoint)
{
#ifdef EMBER_AF_PLUGIN_SCENES_NAME_SUPPORT
    {
        // The high bit of Name Support indicates whether scene names are supported.
        uint8_t nameSupport = EMBER_BIT(7);
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

EmberAfStatus emberAfScenesSetSceneCountAttribute(EndpointId endpoint, uint8_t newCount)
{
    return writeServerAttribute(endpoint, ZCL_SCENES_CLUSTER_ID, ZCL_SCENE_COUNT_ATTRIBUTE_ID, "scene count", (uint8_t *) &newCount,
                                ZCL_INT8U_ATTRIBUTE_TYPE);
}

EmberAfStatus emberAfScenesMakeValid(EndpointId endpoint, uint8_t sceneId, GroupId groupId)
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

EmberAfStatus emberAfScenesClusterMakeInvalidCallback(EndpointId endpoint)
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
            emberAfCorePrint(" %2x %x %x %x %2x %2x", entry.enhancedCurrentHueValue, entry.currentSaturationValue,
                             entry.colorLoopActiveValue, entry.colorLoopDirectionValue, entry.colorLoopTimeValue,
                             entry.colorTemperatureMiredsValue);
            emberAfCoreFlush();
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
            emberAfCorePrint(" door %x", entry.lockStateValue);
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
            emberAfCorePrint(" Window percentage Lift %3u, Tilt %3u", entry.currentPositionLiftPercentageValue,
                             entry.currentPositionTiltPercentageValue);
            emberAfCorePrint(" Window percent100ths Lift %5u, Tilt %5u", entry.targetPositionLiftPercent100thsValue,
                             entry.targetPositionTiltPercent100thsValue);
#endif
        }
        emberAfCorePrintln("");
    }
}

bool emberAfScenesClusterAddSceneCallback(EndpointId endpoint, app::CommandHandler * commandObj, GroupId groupId, uint8_t sceneId,
                                          uint16_t transitionTime, uint8_t * sceneName, uint8_t * extensionFieldSets)
{
    return emberAfPluginScenesServerParseAddScene(commandObj, emberAfCurrentCommand(), groupId, sceneId, transitionTime, sceneName,
                                                  extensionFieldSets);
}

bool emberAfScenesClusterViewSceneCallback(EndpointId endpoint, app::CommandHandler * commandObj, GroupId groupId, uint8_t sceneId)
{
    return emberAfPluginScenesServerParseViewScene(commandObj, emberAfCurrentCommand(), groupId, sceneId);
}

bool emberAfScenesClusterRemoveSceneCallback(EndpointId endpoint, app::CommandHandler * commandObj, GroupId groupId,
                                             uint8_t sceneId)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_NOT_FOUND;
    CHIP_ERROR err       = CHIP_NO_ERROR;

    emberAfScenesClusterPrintln("RX: RemoveScene 0x%2x, 0x%x", groupId, sceneId);

    if (!isEndpointInGroup(emberAfCurrentEndpoint(), groupId))
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
        {
            app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_SCENES_CLUSTER_ID,
                                                 ZCL_REMOVE_SCENE_RESPONSE_COMMAND_ID, (app::CommandPathFlags::kEndpointIdValid) };
            TLV::TLVWriter * writer          = nullptr;
            SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
            VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(2), sceneId));
            SuccessOrExit(err = commandObj->FinishCommand());
        }
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfScenesClusterRemoveAllScenesCallback(EndpointId endpoint, app::CommandHandler * commandObj, GroupId groupId)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_INVALID_FIELD;
    CHIP_ERROR err       = CHIP_NO_ERROR;

    emberAfScenesClusterPrintln("RX: RemoveAllScenes 0x%2x", groupId);

    if (isEndpointInGroup(emberAfCurrentEndpoint(), groupId))
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
        {
            app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_SCENES_CLUSTER_ID,
                                                 ZCL_REMOVE_ALL_SCENES_RESPONSE_COMMAND_ID,
                                                 (app::CommandPathFlags::kEndpointIdValid) };
            TLV::TLVWriter * writer          = nullptr;
            SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
            VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
            SuccessOrExit(err = commandObj->FinishCommand());
        }
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfScenesClusterStoreSceneCallback(EndpointId endpoint, app::CommandHandler * commandObj, GroupId groupId, uint8_t sceneId)
{
    EmberAfStatus status;
    CHIP_ERROR err = CHIP_NO_ERROR;
    emberAfScenesClusterPrintln("RX: StoreScene 0x%2x, 0x%x", groupId, sceneId);
    status = emberAfScenesClusterStoreCurrentSceneCallback(emberAfCurrentEndpoint(), groupId, sceneId);

    // Store Scene commands are only responded to when they are addressed to a
    // single device.
    if (emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST || emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST_REPLY)
    {
        {
            app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_SCENES_CLUSTER_ID,
                                                 ZCL_STORE_SCENE_RESPONSE_COMMAND_ID, (app::CommandPathFlags::kEndpointIdValid) };
            TLV::TLVWriter * writer          = nullptr;
            SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
            VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(2), sceneId));
            SuccessOrExit(err = commandObj->FinishCommand());
        }
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfScenesClusterRecallSceneCallback(EndpointId endpoint, app::CommandHandler * commandObj, GroupId groupId,
                                             uint8_t sceneId, uint16_t transitionTime)
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
    EmberStatus sendStatus = EMBER_SUCCESS;
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

bool emberAfScenesClusterGetSceneMembershipCallback(EndpointId endpoint, app::CommandHandler * commandObj, GroupId groupId)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    uint8_t sceneCount   = 0;
    uint8_t sceneList[EMBER_AF_PLUGIN_SCENES_TABLE_SIZE];

    emberAfScenesClusterPrintln("RX: GetSceneMembership 0x%2x", groupId);

    if (!isEndpointInGroup(emberAfCurrentEndpoint(), groupId))
    {
        status = EMBER_ZCL_STATUS_INVALID_FIELD;
    }

    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        uint8_t i;
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

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_SCENES_CLUSTER_ID,
                                             ZCL_GET_SCENE_MEMBERSHIP_RESPONSE_COMMAND_ID,
                                             (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(1),
                                        static_cast<uint8_t>(EMBER_AF_PLUGIN_SCENES_TABLE_SIZE -
                                                             emberAfPluginScenesServerNumSceneEntriesInUse())));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(2), groupId));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(3), sceneCount));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(4), ByteSpan(sceneList, sceneCount)));
        SuccessOrExit(err = commandObj->FinishCommand());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

EmberAfStatus emberAfScenesClusterStoreCurrentSceneCallback(EndpointId endpoint, GroupId groupId, uint8_t sceneId)
{
    EmberAfSceneTableEntry entry;
    uint8_t i, index = EMBER_AF_SCENE_TABLE_NULL_INDEX;

    if (!isEndpointInGroup(endpoint, groupId))
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
    entry.hasCurrentPositionLiftPercentageValue =
        readServerAttribute(endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_WC_CURRENT_POSITION_LIFT_PERCENTAGE_ATTRIBUTE_ID,
                            "currentPositionLiftPercentage", (uint8_t *) &entry.currentPositionLiftPercentageValue,
                            sizeof(entry.currentPositionLiftPercentageValue));
    entry.hasCurrentPositionTiltPercentageValue =
        readServerAttribute(endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_WC_CURRENT_POSITION_TILT_PERCENTAGE_ATTRIBUTE_ID,
                            "currentPositionTiltPercentage", (uint8_t *) &entry.currentPositionTiltPercentageValue,
                            sizeof(entry.currentPositionTiltPercentageValue));
    entry.hasTargetPositionLiftPercent100thsValue =
        readServerAttribute(endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_WC_TARGET_POSITION_LIFT_PERCENT100_THS_ATTRIBUTE_ID,
                            "targetPositionLiftPercent100ths", (uint8_t *) &entry.targetPositionLiftPercent100thsValue,
                            sizeof(entry.targetPositionLiftPercent100thsValue));
    entry.hasTargetPositionTiltPercent100thsValue =
        readServerAttribute(endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_WC_TARGET_POSITION_TILT_PERCENT100_THS_ATTRIBUTE_ID,
                            "targetPositionTiltPercent100ths", (uint8_t *) &entry.targetPositionTiltPercent100thsValue,
                            sizeof(entry.targetPositionTiltPercent100thsValue));
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

EmberAfStatus emberAfScenesClusterRecallSavedSceneCallback(EndpointId endpoint, GroupId groupId, uint8_t sceneId)
{
    if (!isEndpointInGroup(endpoint, groupId))
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
                    writeServerAttribute(endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID,
                                         ZCL_WC_CURRENT_POSITION_LIFT_PERCENTAGE_ATTRIBUTE_ID, "CurrentPositionLiftPercentage",
                                         (uint8_t *) &entry.currentPositionLiftPercentageValue, ZCL_INT8U_ATTRIBUTE_TYPE);
                }
                if (entry.hasCurrentPositionTiltPercentageValue)
                {
                    writeServerAttribute(endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID,
                                         ZCL_WC_CURRENT_POSITION_TILT_PERCENTAGE_ATTRIBUTE_ID, "CurrentPositionTiltPercentage",
                                         (uint8_t *) &entry.currentPositionTiltPercentageValue, ZCL_INT8U_ATTRIBUTE_TYPE);
                }
                if (entry.hasTargetPositionLiftPercent100thsValue)
                {
                    writeServerAttribute(endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID,
                                         ZCL_WC_TARGET_POSITION_LIFT_PERCENT100_THS_ATTRIBUTE_ID, "TargetPositionLiftPercent100ths",
                                         (uint8_t *) &entry.targetPositionLiftPercent100thsValue, ZCL_INT16U_ATTRIBUTE_TYPE);
                }
                if (entry.hasTargetPositionTiltPercent100thsValue)
                {
                    writeServerAttribute(endpoint, ZCL_WINDOW_COVERING_CLUSTER_ID,
                                         ZCL_WC_TARGET_POSITION_TILT_PERCENT100_THS_ATTRIBUTE_ID, "TargetPositionTiltPercent100ths",
                                         (uint8_t *) &entry.targetPositionTiltPercent100thsValue, ZCL_INT16U_ATTRIBUTE_TYPE);
                }
#endif
                emberAfScenesMakeValid(endpoint, sceneId, groupId);
                return EMBER_ZCL_STATUS_SUCCESS;
            }
        }
    }

    return EMBER_ZCL_STATUS_NOT_FOUND;
}

bool emberAfPluginScenesServerParseAddScene(app::CommandHandler * commandObj, const EmberAfClusterCommand * cmd, GroupId groupId,
                                            uint8_t sceneId, uint16_t transitionTime, uint8_t * sceneName,
                                            uint8_t * extensionFieldSets)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EmberAfSceneTableEntry entry;
    EmberAfStatus status;
    bool enhanced                  = (cmd->commandId == ZCL_ENHANCED_ADD_SCENE_COMMAND_ID);
    uint16_t extensionFieldSetsLen = static_cast<uint16_t>(
        cmd->bufLen -
        (cmd->payloadStartIndex + sizeof(groupId) + sizeof(sceneId) + sizeof(transitionTime) + emberAfStringLength(sceneName) + 1));
    uint16_t extensionFieldSetsIndex = 0;
    EndpointId endpoint              = cmd->apsFrame->destinationEndpoint;
    uint8_t i, index = EMBER_AF_SCENE_TABLE_NULL_INDEX;

    emberAfScenesClusterPrint("RX: %pAddScene 0x%2x, 0x%x, 0x%2x, \"", (enhanced ? "Enhanced" : ""), groupId, sceneId,
                              transitionTime);
    emberAfScenesClusterPrintString(sceneName);
    emberAfScenesClusterPrint("\", ");
    emberAfScenesClusterPrintBuffer(extensionFieldSets, extensionFieldSetsLen, false);
    emberAfScenesClusterPrintln("");

    // Add Scene commands can only reference groups to which we belong.
    if (!isEndpointInGroup(endpoint, groupId))
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
        entry.hasCurrentPositionLiftPercentageValue   = false;
        entry.hasCurrentPositionTiltPercentageValue   = false;
        entry.hasTargetPositionLiftPercent100thsValue = false;
        entry.hasTargetPositionTiltPercent100thsValue = false;
#endif
    }

    while (extensionFieldSetsIndex < extensionFieldSetsLen)
    {
        ClusterId clusterId;
        uint8_t length;

        // Each extension field set must contain a two-byte cluster id and a one-
        // byte length.  Otherwise, the command is malformed.
        if (extensionFieldSetsLen < extensionFieldSetsIndex + 3)
        {
            status = EMBER_ZCL_STATUS_MALFORMED_COMMAND;
            goto kickout;
        }

        clusterId               = emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
        length                  = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
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
            extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
            length                  = static_cast<uint8_t>(length - 2);
            if (length < 2)
            {
                break;
            }
            entry.hasOccupiedHeatingSetpointValue = true;
            entry.occupiedHeatingSetpointValue =
                (int16_t) emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
            length                  = static_cast<uint8_t>(length - 2);
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
            entry.hasCurrentXValue  = true;
            entry.currentXValue     = emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
            length                  = static_cast<uint8_t>(length - 2);
            if (length < 2)
            {
                break;
            }
            entry.hasCurrentYValue = true;
            entry.currentYValue    = emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            if (enhanced)
            {
                extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
                length                  = static_cast<uint8_t>(length - 2);
                ;
                if (length < 2)
                {
                    break;
                }
                entry.hasEnhancedCurrentHueValue = true;
                entry.enhancedCurrentHueValue =
                    emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
                length                  = static_cast<uint8_t>(length - 2);
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
                extensionFieldSetsIndex     = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
                length                      = static_cast<uint8_t>(length - 2);
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
            extensionFieldSetsIndex++;
            length--;
            if (length < 2)
            {
                break;
            }
            entry.hasTargetPositionLiftPercent100thsValue = true;
            entry.targetPositionLiftPercent100thsValue =
                emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
            length                  = static_cast<uint8_t>(length - 2);
            if (length < 2)
            {
                break;
            }
            entry.hasTargetPositionTiltPercent100thsValue = true;
            entry.targetPositionTiltPercent100thsValue =
                emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            // If additional Window Covering extensions are added, adjust the index
            // and length variables here.
            break;
#endif
        default:
            break;
        }

        extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + length);
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
    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_SCENES_CLUSTER_ID,
                                             ZCL_ADD_SCENE_RESPONSE_COMMAND_ID, (app::CommandPathFlags::kEndpointIdValid) };
        if (enhanced)
        {
            cmdParams.mCommandId = ZCL_ENHANCED_ADD_SCENE_RESPONSE_COMMAND_ID;
        }
        TLV::TLVWriter * writer = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(2), sceneId));
        SuccessOrExit(err = commandObj->FinishCommand());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfPluginScenesServerParseViewScene(app::CommandHandler * commandObj, const EmberAfClusterCommand * cmd, GroupId groupId,
                                             uint8_t sceneId)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    EmberAfSceneTableEntry entry = {};
    EmberAfStatus status         = EMBER_ZCL_STATUS_NOT_FOUND;
    bool enhanced                = (cmd->commandId == ZCL_ENHANCED_VIEW_SCENE_COMMAND_ID);
    EndpointId endpoint          = cmd->apsFrame->destinationEndpoint;

    emberAfScenesClusterPrintln("RX: %pViewScene 0x%2x, 0x%x", (enhanced ? "Enhanced" : ""), groupId, sceneId);

    // View Scene commands can only reference groups which we belong to.
    if (!isEndpointInGroup(endpoint, groupId))
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
    app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_SCENES_CLUSTER_ID,
                                         ZCL_VIEW_SCENE_RESPONSE_COMMAND_ID, (app::CommandPathFlags::kEndpointIdValid) };
    if (enhanced)
    {
        cmdParams.mCommandId = ZCL_ENHANCED_VIEW_SCENE_RESPONSE_COMMAND_ID;
    }
    TLV::TLVWriter * writer = nullptr;
    SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
    VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(2), sceneId));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(3),
                                    static_cast<uint16_t>(enhanced ? entry.transitionTime * 10 + entry.transitionTime100ms
                                                                   : entry.transitionTime)));
#ifdef EMBER_AF_PLUGIN_SCENES_NAME_SUPPORT
    SuccessOrExit(err = writer->Put(TLV::ContextTag(4), entry.name));
#else
    SuccessOrExit(err = writer->PutString(TLV::ContextTag(4), ""));
#endif
    // #6620: Need the build the array for response.
    SuccessOrExit(err = writer->Put(TLV::ContextTag(5), ByteSpan(nullptr, 0)));
    SuccessOrExit(err = commandObj->FinishCommand());

    /*
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            // The transition time is returned in seconds in the regular version of the
            // command and tenths of a second in the enhanced version.
            emberAfPutInt16uInResp(
                static_cast<uint16_t>(enhanced ? entry.transitionTime * 10 + entry.transitionTime100ms : entry.transitionTime));
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
                emberAfPutInt16sInResp(entry.occupiedCoolingSetpointValue);
                *length += 2;
                if (entry.hasOccupiedHeatingSetpointValue)
                {
                    emberAfPutInt16sInResp(entry.occupiedHeatingSetpointValue);
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
                *length = static_cast<uint8_t>(*length + 2);
                if (entry.hasCurrentYValue)
                {
                    emberAfPutInt16uInResp(entry.currentYValue);
                    *length = static_cast<uint8_t>(*length + 2);
                    if (enhanced)
                    {
                        if (entry.hasEnhancedCurrentHueValue)
                        {
                            emberAfPutInt16uInResp(entry.enhancedCurrentHueValue);
                            *length = static_cast<uint8_t>(*length + 2);
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
                                            *length = static_cast<uint8_t>(*length + 2);
                                            if (entry.hasColorTemperatureMiredsValue)
                                            {
                                                emberAfPutInt16uInResp(entry.colorTemperatureMiredsValue);
                                                *length = static_cast<uint8_t>(*length + 2);
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
                    if (entry.hasTargetPositionLiftPercent100thsValue)
                    {
                        emberAfPutInt16uInResp(entry.targetPositionLiftPercent100thsValue);
                        *length = static_cast<uint8_t>(*length + 2);
                        if (entry.hasTargetPositionTiltPercent100thsValue)
                        {
                            emberAfPutInt16uInResp(entry.targetPositionTiltPercent100thsValue);
                            *length = static_cast<uint8_t>(*length + 2);
                        }
                    }
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
    */

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

void emberAfScenesClusterRemoveScenesInGroupCallback(EndpointId endpoint, GroupId groupId)
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
