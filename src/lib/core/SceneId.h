/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <cstdint>
#include <lib/core/GroupId.h>

/**
 * @brief Indicates the absence of a Scene table entry.
 */
#define SCENE_TABLE_NULL_INDEX 0xFF
/**
 * @brief Value used when setting or getting the endpoint in a Scene table
 * entry.  It indicates that the entry is not in use.
 */
#define SCENE_TABLE_UNUSED_ENDPOINT_ID 0x00
/**
 * @brief Maximum length of Scene names, not including the length byte.
 */
#define ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH 16
/**
 * @brief The group identifier for the global scene.
 */
#define ZCL_SCENES_GLOBAL_SCENE_GROUP_ID 0x0000
/**
 * @brief The scene identifier for the global scene.
 */
#define ZCL_SCENES_GLOBAL_SCENE_SCENE_ID 0x00

/**
 * @brief The maximum number of scenes according to spec
 */
#define SCENE_MAX_NUMBER 16

/**
 * @brief The maximum number of scenes allowed on a single fabric
 */
#define SCENE_MAX_PER_FABRIC (SCENE_MAX_NUMBER / 2)

namespace chip {

typedef GroupId SceneGroupID;
typedef uint8_t SceneId;
typedef uint16_t SceneTransitionTime;
typedef uint8_t TransitionTime100ms;

constexpr SceneGroupID kGlobalGroupSceneId = ZCL_SCENES_GLOBAL_SCENE_GROUP_ID;
constexpr SceneId kUndefinedSceneId        = SCENE_TABLE_NULL_INDEX;
constexpr SceneId kUnusedEndpointId        = SCENE_TABLE_UNUSED_ENDPOINT_ID;

} // namespace chip
