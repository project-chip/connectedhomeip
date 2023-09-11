/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// Rate of level control tick execution.
// To increase tick frequency (for more granular updates of device state based
// on level), redefine EMBER_AF_PLUGIN_LEVEL_CONTROL_TICKS_PER_SECOND.
#ifndef EMBER_AF_PLUGIN_LEVEL_CONTROL_TICKS_PER_SECOND
#define EMBER_AF_PLUGIN_LEVEL_CONTROL_TICKS_PER_SECOND 32
#endif

#include <stdint.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/scenes-server/SceneTable.h>
#include <app/util/basic-types.h>

/** @brief Level Control Cluster Server Post Init
 *
 * Following resolution of the Level Control state at startup for this endpoint, perform any
 * additional initialization needed; e.g., synchronize hardware state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginLevelControlClusterServerPostInitCallback(chip::EndpointId endpoint);

/**
 * Check whether the instance of the Level Control cluster on the given endpoint
 * has the given feature.  The implementation is allowed to assume there is in
 * fact an instance of Level Control on the given endpoint.
 */
bool LevelControlHasFeature(chip::EndpointId endpoint, chip::app::Clusters::LevelControl::Feature feature);

namespace LevelControlServer {

chip::Protocols::InteractionModel::Status
MoveToLevel(chip::EndpointId endpointId,
            const chip::app::Clusters::LevelControl::Commands::MoveToLevel::DecodableType & commandData);

chip::scenes::SceneHandler * GetSceneHandler();

} // namespace LevelControlServer
