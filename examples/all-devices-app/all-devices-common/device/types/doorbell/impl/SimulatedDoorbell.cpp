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

#include <device/types/doorbell/impl/SimulatedDoorbell.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

SimulatedDoorbell::SimulatedDoorbell(const Config & config) : Doorbell(config) {}

CHIP_ERROR SimulatedDoorbell::HandleShortPress()
{
    // A basic doorbell short press simulates pressing the momentary switch (position 1),
    // triggering the chime, and then returning it to its idle released state (position 0).
    ReturnErrorOnFailure(SetSwitchPosition(1));
    RETURN_SAFELY_IGNORED mSwitchCluster.Cluster().OnInitialPress(1);
    // Chime trigger should happen here.
    ReturnErrorOnFailure(SetSwitchPosition(0));
    RETURN_SAFELY_IGNORED mSwitchCluster.Cluster().OnShortRelease(1);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedDoorbell::HandleSetCurrentPosition(uint8_t currentPosition)
{
    ReturnErrorOnFailure(SetSwitchPosition(currentPosition));
    // Chime should be triggered according to the position.
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
