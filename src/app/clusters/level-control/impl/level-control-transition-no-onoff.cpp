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

#include <app/util/config.h>
#include <lib/support/logging/CHIPLogging.h>
#include <algorithm>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;
using chip::Protocols::InteractionModel::Status;

bool TryGetOnOffTransitionTimeDs(EndpointId, uint16_t &)
{
    return false;
}

Status ComputeTransitionTimeMsForMoveToLevel(EndpointId endpoint, DataModel::Nullable<uint16_t> transitionTimeDs,
                                             uint8_t actualStepSize, uint32_t fastestTransitionTimeMs,
                                             uint32_t & transitionTimeMs)
{
    if (transitionTimeDs.IsNull())
    {
        transitionTimeMs = fastestTransitionTimeMs;
    }
    else
    {
        transitionTimeMs = (transitionTimeDs.Value() * MILLISECOND_TICKS_PER_SECOND / 10);
    }

    return Status::Success;
}

Status ComputeEventDurationMsForMove(EndpointId endpoint, DataModel::Nullable<uint8_t> rate,
                                     uint32_t fastestTransitionTimeMs, uint8_t & eventDurationMs)
{
    Status status = Status::Success;

    if (!rate.IsNull())
    {
        eventDurationMs = static_cast<uint8_t>(MILLISECOND_TICKS_PER_SECOND / rate.Value());
        return status;
    }

    DataModel::Nullable<uint8_t> defaultMoveRate;
    status = Attributes::DefaultMoveRate::Get(endpoint, defaultMoveRate);
    if (status != Status::Success || defaultMoveRate.IsNull())
    {
        ChipLogProgress(Zcl, "ERR: reading default move rate %x", to_underlying(status));
        eventDurationMs = static_cast<uint8_t>(fastestTransitionTimeMs);
        status          = Status::Success;
    }
    else if (defaultMoveRate.Value() == 0)
    {
        status = Status::InvalidCommand;
    }
    else
    {
        eventDurationMs = static_cast<uint8_t>(MILLISECOND_TICKS_PER_SECOND / defaultMoveRate.Value());
    }
    return status;
}

uint32_t ComputeTransitionTimeMsForStep(EndpointId endpoint, DataModel::Nullable<uint16_t> transitionTimeDs, uint8_t stepSize,
                                        uint8_t actualStepSize, uint32_t fastestTransitionTimeMs)
{
    uint32_t transitionTimeMs;

    if (transitionTimeDs.IsNull())
    {
        transitionTimeMs = fastestTransitionTimeMs;
    }
    else
    {
        transitionTimeMs = (transitionTimeDs.Value() * MILLISECOND_TICKS_PER_SECOND / 10);
        if (stepSize != actualStepSize)
        {
            transitionTimeMs = (transitionTimeMs * actualStepSize / std::max(static_cast<uint8_t>(1u), stepSize));
        }
    }

    return transitionTimeMs;
}
