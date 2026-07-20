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

#include <app/clusters/color-control-server/SceneIntegration.h>

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <clusters/ColorControl/ClusterId.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h> // nogncheck

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
#include <app/clusters/color-control-server/DefaultColorControlSceneHandler.h>
#include <app/clusters/scenes-server/CodegenIntegration.h> // ScenesManagement::ScenesServer  // nogncheck
#endif

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
namespace chip::app::Clusters {

// Ember-free lookup used by DefaultColorControlSceneHandler: resolve the ColorControlCluster on an
// endpoint through the code-driven registry. A null return means the cluster is not on that endpoint,
// which doubles as the handler's presence check.
ColorControlCluster * FindClusterOnEndpoint(EndpointId endpoint)
{
    return static_cast<ColorControlCluster *>(
        CodegenDataModelProvider::Instance().Registry().Get({ endpoint, ColorControl::Id }));
}

} // namespace chip::app::Clusters
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

namespace chip::app::Clusters::ColorControl {

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

void RegisterSceneHandler(EndpointId endpoint)
{
    ScenesManagement::ScenesServer::Instance().RegisterSceneHandler(endpoint, &sColorControlSceneHandler);
}

void UnregisterSceneHandler(EndpointId endpoint)
{
    ScenesManagement::ScenesServer::Instance().UnregisterSceneHandler(endpoint, &sColorControlSceneHandler);
}

void MarkScenesInvalid(EndpointId endpoint)
{
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(endpoint);
}

#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

} // namespace chip::app::Clusters::ColorControl