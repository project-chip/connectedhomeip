/**
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

#include "level-control-internals.h"

#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

bool TryGetOnOffTransitionTimeDs(EndpointId, uint16_t &)
{
    return false;
}

Status ComputeTransitionTimeMsForMoveToLevel(EndpointId endpoint, DataModel::Nullable<uint16_t>, uint8_t,
                                             uint32_t fastestTransitionTimeMs, uint32_t & transitionTimeMs)
{
    ChipLogProgress(Zcl, "Device does not support transition, ignoring transition time");
    transitionTimeMs = fastestTransitionTimeMs;
    return Status::Success;
}

Status ComputeEventDurationMsForMove(EndpointId endpoint, DataModel::Nullable<uint8_t>, uint32_t fastestTransitionTimeMs,
                                     uint8_t & eventDurationMs)
{
    ChipLogProgress(Zcl, "Device does not support transition, ignoring rate");
    eventDurationMs = static_cast<uint8_t>(fastestTransitionTimeMs);
    return Status::Success;
}

uint32_t ComputeTransitionTimeMsForStep(EndpointId endpoint, DataModel::Nullable<uint16_t>, uint8_t, uint8_t,
                                        uint32_t fastestTransitionTimeMs)
{
    ChipLogProgress(Zcl, "Device does not support transition, ignoring transition time");
    return fastestTransitionTimeMs;
}
