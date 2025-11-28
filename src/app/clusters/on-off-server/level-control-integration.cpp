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
#include <app/clusters/on-off-server/level-control-integration.h>

#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL

#include <app/clusters/level-control/level-control.h> //nogncheck
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app::Clusters;

static bool LevelControlWithOnOffFeaturePresent(EndpointId endpoint)
{
    if (!emberAfContainsServer(endpoint, LevelControl::Id))
    {
        return false;
    }

    return LevelControlHasFeature(endpoint, LevelControl::Feature::kOnOff);
}

namespace chip::app::Clusters::OnOff::Internal {

bool OnOffControlChangeForLevelControl(EndpointId endpoint, bool newValue)
{
    VerifyOrReturnValue(LevelControlWithOnOffFeaturePresent(endpoint), false);

    emberAfOnOffClusterLevelControlEffectCallback(endpoint, newValue);
    return true;
}

} // namespace chip::app::Clusters::OnOff::Internal

#endif // MATTER_DM_PLUGIN_LEVEL_CONTROL
