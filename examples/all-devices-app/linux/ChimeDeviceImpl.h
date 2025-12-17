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
#include <devices/chime/ChimeDevice.h>
#include <platform/DefaultTimerDelegate.h>
#include <vector>
#include <string>
#include <lib/support/Span.h>

class ChimeDeviceImpl : public chip::app::ChimeDevice,
                        public chip::app::Clusters::ChimeDelegate
{
public:
    ChimeDeviceImpl();
    ~ChimeDeviceImpl() override = default;

    // ChimeDelegate
    CHIP_ERROR GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, chip::MutableCharSpan & name) override;
    CHIP_ERROR GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID) override;
    chip::Protocols::InteractionModel::Status PlayChimeSound() override;

private:
    chip::app::DefaultTimerDelegate mTimerDelegate;
    
    struct ChimeSound {
        uint8_t id;
        std::string name;
    };
    std::vector<ChimeSound> mSounds;
};
