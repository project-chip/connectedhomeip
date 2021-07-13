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

/**
 * @file
 * @brief Routines for the ZLL Scenes Server plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc.
 * www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 * Laboratories Inc. Your use of this software is
 * governed by the terms of Silicon Labs Master
 * Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * This software is distributed to you in Source Code
 * format and is governed by the sections of the MSLA
 * applicable to Source Code.
 *
 ******************************************************************************/

#include "../../include/af.h"
#include "../scenes/scenes.h"
#include <app/CommandHandler.h>

#define ZCL_SCENES_CLUSTER_MODE_COPY_ALL_SCENES_MASK EMBER_BIT(0)

bool emberAfScenesClusterEnhancedAddSceneCallback(chip::app::CommandHandler * commandObj, uint16_t groupId, uint8_t sceneId,
                                                  uint16_t transitionTime, uint8_t * sceneName, uint8_t * extensionFieldSets)
{
    return emberAfPluginScenesServerParseAddScene(emberAfCurrentCommand(), groupId, sceneId, transitionTime, sceneName,
                                                  extensionFieldSets);
}

bool emberAfScenesClusterEnhancedViewSceneCallback(chip::app::CommandHandler * commandObj, uint16_t groupId, uint8_t sceneId)
{
    return emberAfPluginScenesServerParseViewScene(emberAfCurrentCommand(), groupId, sceneId);
}

bool emberAfScenesClusterCopySceneCallback(chip::app::CommandHandler * commandObj, uint8_t mode, uint16_t groupIdFrom,
                                           uint8_t sceneIdFrom, uint16_t groupIdTo, uint8_t sceneIdTo)
{
    EmberStatus sendStatus = EMBER_SUCCESS;
    EmberAfStatus status   = EMBER_ZCL_STATUS_INVALID_FIELD;
    bool copyAllScenes     = (mode & ZCL_SCENES_CLUSTER_MODE_COPY_ALL_SCENES_MASK);
    uint8_t i;

    emberAfScenesClusterPrintln("RX: CopyScene 0x%x, 0x%2x, 0x%x, 0x%2x, 0x%x", mode, groupIdFrom, sceneIdFrom, groupIdTo,
                                sceneIdTo);

    // If a group id is specified but this endpoint isn't in it, take no action.
    if ((groupIdFrom != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID &&
         !emberAfGroupsClusterEndpointInGroupCallback(emberAfCurrentEndpoint(), groupIdFrom)) ||
        (groupIdTo != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID &&
         !emberAfGroupsClusterEndpointInGroupCallback(emberAfCurrentEndpoint(), groupIdTo)))
    {
        status = EMBER_ZCL_STATUS_INVALID_FIELD;
        goto kickout;
    }

    for (i = 0; i < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; i++)
    {
        EmberAfSceneTableEntry from;
        emberAfPluginScenesServerRetrieveSceneEntry(from, i);
        if (from.endpoint == emberAfCurrentEndpoint() && from.groupId == groupIdFrom &&
            (copyAllScenes || from.sceneId == sceneIdFrom))
        {
            uint8_t j, index = EMBER_AF_SCENE_TABLE_NULL_INDEX;
            for (j = 0; j < EMBER_AF_PLUGIN_SCENES_TABLE_SIZE; j++)
            {
                EmberAfSceneTableEntry to;
                if (i == j)
                {
                    continue;
                }
                emberAfPluginScenesServerRetrieveSceneEntry(to, j);
                if (to.endpoint == emberAfCurrentEndpoint() && to.groupId == groupIdTo &&
                    to.sceneId == (copyAllScenes ? from.sceneId : sceneIdTo))
                {
                    index = j;
                    break;
                }
                else if (index == EMBER_AF_SCENE_TABLE_NULL_INDEX && to.endpoint == EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID)
                {
                    index = j;
                }
            }

            // If the target index is still zero, the table is full.
            if (index == EMBER_AF_SCENE_TABLE_NULL_INDEX)
            {
                status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
                goto kickout;
            }

            // Save the "from" entry to the "to" index.  This makes a copy of "from"
            // with the correct group and scene ids and leaves the original in tact.
            from.groupId = groupIdTo;
            if (!copyAllScenes)
            {
                from.sceneId = sceneIdTo;
            }
            emberAfPluginScenesServerSaveSceneEntry(from, index);

            if (j != index)
            {
                emberAfPluginScenesServerIncrNumSceneEntriesInUse();
                emberAfScenesSetSceneCountAttribute(emberAfCurrentEndpoint(), emberAfPluginScenesServerNumSceneEntriesInUse());
            }

            // If we aren't copying all scenes, we can stop here.
            status = EMBER_ZCL_STATUS_SUCCESS;
            if (!copyAllScenes)
            {
                goto kickout;
            }
        }
    }

kickout:
    // Copy Scene commands are only responded to when they are addressed to a
    // single device.
    if (emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST || emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST_REPLY)
    {
        emberAfFillCommandScenesClusterCopySceneResponse(status, groupIdFrom, sceneIdFrom);
        sendStatus = emberAfSendResponse();
        if (EMBER_SUCCESS != sendStatus)
        {
            emberAfScenesClusterPrintln("Scenes: failed to send %s response: 0x%x", "copy_scene", sendStatus);
        }
    }
    return true;
}

EmberAfStatus emberAfPluginZllScenesServerRecallSceneZllExtensions(uint8_t endpoint)
{
    bool globalSceneControl = true;
    EmberAfStatus status    = emberAfWriteServerAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_GLOBAL_SCENE_CONTROL_ATTRIBUTE_ID,
                                                       (uint8_t *) &globalSceneControl, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfScenesClusterPrintln("ERR: writing global scene control %x", status);
    }
    return status;
}
