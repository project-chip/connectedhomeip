/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/chime-server/ChimeCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class ChimeDevice : public SingleEndpointDevice
{
public:
    ChimeDevice(Clusters::ChimeDelegate & delegate, TimerDelegate * timerDelegate = nullptr);
    ~ChimeDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void UnRegister(CodeDrivenDataModelProvider & provider) override;

    Clusters::ChimeCluster & ChimeCluster() { return mChimeCluster.Cluster(); }

protected:
    Clusters::ChimeDelegate & mDelegate;
    TimerDelegate * mTimerDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::ChimeCluster> mChimeCluster;
};

} // namespace app
} // namespace chip
