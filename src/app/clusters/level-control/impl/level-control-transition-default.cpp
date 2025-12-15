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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <lib/support/logging/CHIPLogging.h>
#include <algorithm>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;
using chip::Protocols::InteractionModel::Status;

bool TryGetOnOffTransitionTimeDs(EndpointId endpoint, uint16_t & transitionTimeDs)
{
    bool containsAttribute = emberAfContainsAttribute(endpoint, LevelControl::Id, Attributes::OnOffTransitionTime::Id);
    if (!containsAttribute)
    {
        return false;
    }

    Status status = Attributes::OnOffTransitionTime::Get(endpoint, &transitionTimeDs);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "ERR: reading on/off transition time %x", to_underlying(status));
        return false;
    }

    return true;
}

Status ComputeTransitionTimeMsForMoveToLevel(EndpointId endpoint, DataModel::Nullable<uint16_t> transitionTimeDs,
                                             uint8_t actualStepSize, uint32_t fastestTransitionTimeMs,
                                             uint32_t & transitionTimeMs)
{
    uint16_t onOffTransitionTime = 0;

    if (transitionTimeDs.IsNull())
    {
        if (TryGetOnOffTransitionTimeDs(endpoint, onOffTransitionTime))
        {
            transitionTimeMs = (onOffTransitionTime * MILLISECOND_TICKS_PER_SECOND / 10);
        }
        else
        {
            transitionTimeMs = fastestTransitionTimeMs;
        }
    }
    else
    {
        transitionTimeMs = (transitionTimeDs.Value() * MILLISECOND_TICKS_PER_SECOND / 10);
    }

    // No additional status values beyond success in this implementation.
    return Status::Success;
}

Status ComputeEventDurationMsForMove(EndpointId endpoint, DataModel::Nullable<uint8_t> rate,
                                     uint32_t fastestTransitionTimeMs, uint8_t & eventDurationMs)
{
    Status status = Status::Success;

    if (rate.IsNull())
    {
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
    }
    else
    {
        // Already confirmed rate.Value() != 0 by the caller.
        eventDurationMs = static_cast<uint8_t>(MILLISECOND_TICKS_PER_SECOND / rate.Value());
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
        return transitionTimeMs;
    }
    transitionTimeMs = (transitionTimeDs.Value() * MILLISECOND_TICKS_PER_SECOND / 10);

    // If the new level was pegged at the minimum level, the transition time shall be proportionally reduced.
    if (stepSize != actualStepSize)
    {
        transitionTimeMs = (transitionTimeMs * actualStepSize / std::max(static_cast<uint8_t>(1u), stepSize));
    }

    return transitionTimeMs;
}
