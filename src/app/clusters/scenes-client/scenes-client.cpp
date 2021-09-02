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
 * @brief Routines for the Scenes Client plugin, which
 *implements the client side of the Scenes cluster.
 *******************************************************************************
 ******************************************************************************/

#include "scenes-client.h"
#include <app/CommandHandler.h>
#include <app/util/af.h>

#include <zap-generated/command-id.h>

using namespace chip;

bool emberAfScenesClusterAddSceneResponseCallback(app::CommandHandler * commandObj, uint8_t status, GroupId groupId,
                                                  uint8_t sceneId)
{
    return emberAfPluginScenesClientParseAddSceneResponse(emberAfCurrentCommand(), status, groupId, sceneId);
}

bool emberAfScenesClusterViewSceneResponseCallback(app::CommandHandler * commandObj, uint8_t status, GroupId groupId,
                                                   uint8_t sceneId, uint16_t transitionTime, uint8_t * sceneName,
                                                   uint8_t * extensionFieldSets)
{
    return emberAfPluginScenesClientParseViewSceneResponse(emberAfCurrentCommand(), status, groupId, sceneId, transitionTime,
                                                           sceneName, extensionFieldSets);
}

bool emberAfScenesClusterRemoveSceneResponseCallback(app::CommandHandler * commandObj, uint8_t status, GroupId groupId,
                                                     uint8_t sceneId)
{
    emberAfScenesClusterPrintln("RX: RemoveSceneResponse 0x%x, 0x%2x, 0x%x", status, groupId, sceneId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfScenesClusterRemoveAllScenesResponseCallback(app::CommandHandler * commandObj, uint8_t status, GroupId groupId)
{
    emberAfScenesClusterPrintln("RX: RemoveAllScenesResponse 0x%x, 0x%2x", status, groupId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfScenesClusterStoreSceneResponseCallback(app::CommandHandler * commandObj, uint8_t status, GroupId groupId,
                                                    uint8_t sceneId)
{
    emberAfScenesClusterPrintln("RX: StoreSceneResponse 0x%x, 0x%2x, 0x%x", status, groupId, sceneId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfScenesClusterGetSceneMembershipResponseCallback(app::CommandHandler * commandObj, uint8_t status, uint8_t capacity,
                                                            GroupId groupId, uint8_t sceneCount, uint8_t * sceneList)
{
    emberAfScenesClusterPrint("RX: GetSceneMembershipResponse 0x%x, 0x%x, 0x%2x", status, capacity, groupId);

    // Scene count and the scene list only appear in the payload if the status is
    // SUCCESS.
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        uint8_t i;
        emberAfScenesClusterPrint(", 0x%x,", sceneCount);
        for (i = 0; i < sceneCount; i++)
        {
            emberAfScenesClusterPrint(" [0x%x]", sceneList[i]);
        }
    }

    emberAfScenesClusterPrintln("");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfPluginScenesClientParseAddSceneResponse(const EmberAfClusterCommand * cmd, uint8_t status, GroupId groupId,
                                                    uint8_t sceneId)
{
    bool enhanced = (cmd->commandId == ZCL_ENHANCED_ADD_SCENE_COMMAND_ID);
    emberAfScenesClusterPrintln("RX: %pAddSceneResponse 0x%x, 0x%2x, 0x%x", (enhanced ? "Enhanced" : ""), status, groupId, sceneId);
    emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfPluginScenesClientParseViewSceneResponse(const EmberAfClusterCommand * cmd, uint8_t status, GroupId groupId,
                                                     uint8_t sceneId, uint16_t transitionTime, const uint8_t * sceneName,
                                                     const uint8_t * extensionFieldSets)
{
    bool enhanced = (cmd->commandId == ZCL_ENHANCED_VIEW_SCENE_COMMAND_ID);

    emberAfScenesClusterPrint("RX: %pViewSceneResponse 0x%x, 0x%2x, 0x%x", (enhanced ? "Enhanced" : ""), status, groupId, sceneId);

    // Transition time, scene name, and the extension field sets only appear in
    // the payload if the status is SUCCESS.
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        uint16_t extensionFieldSetsLen =
            static_cast<uint16_t>(emberAfCurrentCommand()->bufLen -
                                  (emberAfCurrentCommand()->payloadStartIndex + sizeof(status) + sizeof(groupId) + sizeof(sceneId) +
                                   sizeof(transitionTime) + emberAfStringLength(sceneName) + 1));
        uint16_t extensionFieldSetsIndex = 0;

        emberAfScenesClusterPrint(", 0x%2x, \"", transitionTime);
        emberAfScenesClusterPrintString(sceneName);
        emberAfScenesClusterPrint("\",");

        // Each extension field set contains at least a two-byte cluster id and a
        // one-byte length.
        while (extensionFieldSetsIndex + 3 <= extensionFieldSetsLen)
        {
            ClusterId clusterId;
            uint8_t length;
            clusterId               = emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
            length                  = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex++;
            emberAfScenesClusterPrint(" [0x%2x 0x%x ", clusterId, length);
            if (extensionFieldSetsIndex + length <= extensionFieldSetsLen)
            {
                emberAfScenesClusterPrintBuffer(extensionFieldSets + extensionFieldSetsIndex, length, false);
            }
            emberAfScenesClusterPrint("]");
            emberAfScenesClusterFlush();
            extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + length);
        }
    }

    emberAfScenesClusterPrintln("");
    emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
