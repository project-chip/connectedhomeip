/**
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/color-control-server/ColorControlSceneInvalidator.h>
#include <app/util/config.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/DataModelTypes.h>

// Ember-free Scene Management coupling for the Color Control cluster.
//
// All three entry points go through the code-driven Scene Management server
// (ScenesManagement::ScenesServer::Instance()) and the code-driven cluster registry, so this glue
// carries no Ember/ZAP dependency and lives outside codegen/. When Scene Management is not built (or
// default handlers are disabled) every entry point compiles to a no-op, so the core cluster can call
// them unconditionally.
namespace chip::app::Clusters::ColorControl {

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

/// Register the Color Control scene handler for the given endpoint with the Scene Management server.
void RegisterSceneHandler(EndpointId endpoint);
/// Remove the Color Control scene handler for the given endpoint.
void UnregisterSceneHandler(EndpointId endpoint);
/// Mark every fabric's stored scenes stale because the live color changed.
void MarkScenesInvalid(EndpointId endpoint);
/// The scene-invalidation hook injected into ColorControlCluster::Config so a color change can mark stored
/// scenes stale. Never null in this build; the core cluster only sees the abstract interface.
ColorControlSceneInvalidator * GetSceneInvalidator();

#else

inline void RegisterSceneHandler(EndpointId) {}
inline void UnregisterSceneHandler(EndpointId) {}
inline void MarkScenesInvalid(EndpointId) {}
/// No Scene Management coupling in this build: leaves ColorControlCluster::Config::sceneInvalidator null.
inline ColorControlSceneInvalidator * GetSceneInvalidator()
{
    return nullptr;
}

#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

} // namespace chip::app::Clusters::ColorControl
