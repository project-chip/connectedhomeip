/*
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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/level-control/LevelControlCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class SpeakerDevice : public SingleEndpointDevice
{
public:
    // Takes delegates for the clusters.
    SpeakerDevice(Clusters::LevelControlDelegate & levelDelegate, Clusters::OnOffDelegate & onOffDelegate,
                  TimerDelegate & timerDelegate);
    ~SpeakerDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Accessors for subclasses/implementations to interact with clusters
    Clusters::OnOffCluster & OnOffCluster();
    Clusters::LevelControlCluster & LevelControlCluster();

protected:
    Clusters::LevelControlDelegate & mLevelDelegate;
    Clusters::OnOffDelegate & mOnOffDelegate;
    TimerDelegate & mTimerDelegate;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OnOffCluster> mOnOffCluster;
    LazyRegisteredServerCluster<Clusters::LevelControlCluster> mLevelControlCluster;
};

} // namespace app
} // namespace chip
