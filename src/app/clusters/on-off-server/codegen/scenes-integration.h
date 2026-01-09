/**
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <app/clusters/scenes-server/SceneTable.h> // nogncheck
#include <app/util/config.h>
#include <lib/core/CHIPConfig.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT

namespace chip::app::Clusters::OnOff::Internal::Scenes {

#if CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
chip::scenes::SceneHandler * GlobalHandler();
void RegisterGlobalHandler(EndpointId endpoint);
#else
inline chip::scenes::SceneHandler * GlobalHandler()
{
    return nullptr;
}
inline void RegisterGlobalHandler(EndpointId endpoint) {}
#endif

void Store(FabricIndex fabricIndex, EndpointId endpoint);
void Recall(FabricIndex fabricIndex, EndpointId endpoint);
void MarkInvalid(EndpointId endpoint);

} // namespace chip::app::Clusters::OnOff::Internal::Scenes

#else

namespace chip::app::Clusters::OnOff::Internal::Scenes {

inline chip::scenes::SceneHandler * GlobalHandler()
{
    return nullptr;
}
inline void RegisterGlobalHandler(EndpointId endpoint) {}
inline void Store(FabricIndex fabricIndex, EndpointId endpoint) {}
inline void Recall(FabricIndex fabricIndex, EndpointId endpoint) {}
inline void MarkInvalid(EndpointId endpoint) {}

} // namespace chip::app::Clusters::OnOff::Internal::Scenes

#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT
