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
 * @brief Definitions for the Scenes Client plugin.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <app/util/af-types.h>

bool emberAfPluginScenesClientParseAddSceneResponse(const EmberAfClusterCommand * cmd, uint8_t status, chip::GroupId groupId,
                                                    uint8_t sceneId);

bool emberAfPluginScenesClientParseViewSceneResponse(const EmberAfClusterCommand * cmd, uint8_t status, chip::GroupId groupId,
                                                     uint8_t sceneId, uint16_t transitionTime, const uint8_t * sceneName,
                                                     const uint8_t * extensionFieldSets);
