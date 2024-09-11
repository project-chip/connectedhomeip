/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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

#include "Endpoint.h"

#include "clusters/Clusters.h"

namespace matter {
namespace casting {
namespace core {

void Endpoint::RegisterClusters(std::vector<chip::ClusterId> clusters)
{
    for (chip::ClusterId clusterId : clusters)
    {
        ChipLogProgress(AppServer, "Endpoint::RegisterClusters() Registering clusterId %d for endpointId %d", clusterId, GetId());
        switch (clusterId)
        {
        case chip::app::Clusters::ApplicationBasic::Id:
            RegisterCluster<clusters::application_basic::ApplicationBasicCluster>(clusterId);
            break;

        case chip::app::Clusters::ApplicationLauncher::Id:
            RegisterCluster<clusters::application_launcher::ApplicationLauncherCluster>(clusterId);
            break;

        case chip::app::Clusters::ContentLauncher::Id:
            RegisterCluster<clusters::content_launcher::ContentLauncherCluster>(clusterId);
            break;

        case chip::app::Clusters::KeypadInput::Id:
            RegisterCluster<clusters::keypad_input::KeypadInputCluster>(clusterId);
            break;

        case chip::app::Clusters::LevelControl::Id:
            RegisterCluster<clusters::level_control::LevelControlCluster>(clusterId);
            break;

        case chip::app::Clusters::OnOff::Id:
            RegisterCluster<clusters::on_off::OnOffCluster>(clusterId);
            break;

        case chip::app::Clusters::MediaPlayback::Id:
            RegisterCluster<clusters::media_playback::MediaPlaybackCluster>(clusterId);
            break;

        case chip::app::Clusters::TargetNavigator::Id:
            RegisterCluster<clusters::target_navigator::TargetNavigatorCluster>(clusterId);
            break;

        case chip::app::Clusters::WakeOnLan::Id:
            RegisterCluster<clusters::wake_on_lan::WakeOnLanCluster>(clusterId);
            break;

        default:
            ChipLogProgress(AppServer, "Endpoint::RegisterClusters() Skipping registration of clusterId %d for endpointId %d",
                            clusterId, GetId());
            break;
        }
    }
}

}; // namespace core
}; // namespace casting
}; // namespace matter
