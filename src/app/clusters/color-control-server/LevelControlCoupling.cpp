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

#include <app/clusters/color-control-server/LevelControlCoupling.h>

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <app/clusters/level-control/LevelControlCluster.h>          // nogncheck
#include <clusters/ColorControl/ClusterId.h>
#include <clusters/LevelControl/ClusterId.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>    // nogncheck
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

/// Fully Ember-free (endpoint, cluster) -> live cluster resolution via the code-driven registry.
/// A null return simply means the requested cluster is not present on that endpoint, which doubles
/// as the presence check.
template <typename ClusterType>
ClusterType * FindClusterOnEndpoint(EndpointId endpoint, ClusterId clusterId)
{
    return static_cast<ClusterType *>(CodegenDataModelProvider::Instance().Registry().Get({ endpoint, clusterId }));
}

/// Spec fallback for CurrentLevel when it is unavailable or null (07-5123-07 §5.2.2.2.1: mid-scale).
constexpr uint8_t kDefaultCurrentLevel = 0x7F;

} // namespace

void chip::app::Clusters::ColorControl::NotifyLevelControlCurrentLevelChanged(EndpointId endpoint)
{
    auto * colorControl = FindClusterOnEndpoint<ColorControlCluster>(endpoint, ColorControl::Id);
    VerifyOrReturn(colorControl != nullptr);

    // Read Level Control's live CurrentLevel through its typed getter (no Ember/ZAP). If Level Control
    // is somehow not present, fall back to the spec's mid-scale default.
    uint8_t currentLevel = kDefaultCurrentLevel;
    if (auto * levelControl = FindClusterOnEndpoint<LevelControlCluster>(endpoint, LevelControl::Id))
    {
        currentLevel = levelControl->GetCurrentLevel().ValueOr(kDefaultCurrentLevel);
    }

    colorControl->CoupleColorTempToLevel(currentLevel);
}