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

#include <app/clusters/mode-select-server/ModeSelectCluster.h>
#include <device/api/SingleEndpoint.h>

namespace chip {
namespace app {

class ModeSelect : public SingleEndpoint
{
public:
    struct Config
    {
        Clusters::ModeSelectCluster::Delegate & delegate;
        Clusters::ModeSelectCluster::Config clusterConfig;
    };

    ModeSelect(const Config & config);
    ~ModeSelect() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::ModeSelectCluster & ModeSelectCluster() { return mModeSelectCluster.Cluster(); }

private:
    const Config mConfig;
    LazyRegisteredServerCluster<Clusters::ModeSelectCluster> mModeSelectCluster;
};

} // namespace app
} // namespace chip
