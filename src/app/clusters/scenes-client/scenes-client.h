/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <app/util/af-types.h>

bool emberAfPluginScenesClientParseAddSceneResponse(const EmberAfClusterCommand * cmd, uint8_t status, chip::GroupId groupId,
                                                    uint8_t sceneId);

bool emberAfPluginScenesClientParseViewSceneResponse(const EmberAfClusterCommand * cmd, uint8_t status, chip::GroupId groupId,
                                                     uint8_t sceneId, uint16_t transitionTime, const uint8_t * sceneName,
                                                     const uint8_t * extensionFieldSets);
