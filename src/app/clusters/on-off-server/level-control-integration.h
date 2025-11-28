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

#include <app/util/config.h>
#include <lib/core/DataModelTypes.h>

#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL

/** @brief On/off Cluster Level Control Effect
 *
 * This is called by the framework when the on/off cluster initiates a command
 * that must effect a level control change. The implementation assumes that the
 * client will handle any effect on the On/Off Cluster.
 *
 * @param endpoint   Ver.: always
 * @param newValue   Ver.: always
 */
void emberAfOnOffClusterLevelControlEffectCallback(chip::EndpointId endpoint, bool newValue);

namespace chip::app::Clusters::OnOff::Internal {

/**
 * Called by the on-off cluster to conditionally trigger emberAfOnOffClusterLevelControlEffectCallback.
 *
 * @return true if the effect callback is called (i.e. level control was enabled on the endpoint)
 */
bool OnOffControlChangeForLevelControl(chip::EndpointId endpoint, bool newValue);

} // namespace chip::app::Clusters::OnOff::Internal

#else

namespace chip::app::Clusters::OnOff::Internal {

inline bool OnOffControlChangeForLevelControl(chip::EndpointId endpoint, bool newValue)
{
    return false;
}

} // namespace chip::app::Clusters::OnOff::Internal

#endif // MATTER_DM_PLUGIN_LEVEL_CONTROL
