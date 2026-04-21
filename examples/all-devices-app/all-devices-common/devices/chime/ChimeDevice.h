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
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/Span.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class ChimeDevice : public SingleEndpointDevice, public Clusters::ChimeDelegate
{
public:
    struct Sound
    {
        uint8_t id;
        CharSpan name;
    };

    // Note: sounds array must outlive the ChimeDevice lifetime (i.e. often static or similar)
    ChimeDevice(TimerDelegate & timerDelegate, Span<const Sound> sounds);
    ~ChimeDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::ChimeCluster & ChimeCluster();

    // ChimeDelegate
    CHIP_ERROR GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name) override;
    CHIP_ERROR GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID) override;
    virtual Protocols::InteractionModel::Status PlayChimeSound(uint8_t chimeID) override;

protected:
    TimerDelegate & mTimerDelegate;
    Span<const Sound> mSounds;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::ChimeCluster> mChimeCluster;
};

} // namespace app
} // namespace chip
