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
bool LevelControlHasFeature(chip::EndpointId endpoint, chip::app::Clusters::LevelControl::LevelControlFeature feature);

namespace LevelControlServer {

chip::Protocols::InteractionModel::Status
MoveToLevel(chip::EndpointId endpointId,
            const chip::app::Clusters::LevelControl::Commands::MoveToLevel::DecodableType & commandData);

} // namespace LevelControlServer
