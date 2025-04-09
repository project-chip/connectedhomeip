/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <ClosureDimensionManager.h>
#include <app/clusters/closure-dimension-server/closure-dimension-server.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureDimension;

using Protocols::InteractionModel::Status;

CHIP_ERROR ClosureDimensionDelegate::HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<TargetLatchEnum> & latch,
                                                     const Optional<Globals::ThreeLevelAutoEnum> & speed)
{

    if (pos.HasValue())
    {
        uint16_t position = pos.Value(); // 0 - 10000

        // TODO: Convert to hardware-specific range and move actuator using position

        // Optionally handle latch and speed
        if (latch.HasValue())
        {
            ChipLogProgress(NotSpecified, "Target latch mode: %d", static_cast<uint8_t>(latch.Value()));
        }

        if (speed.HasValue())
        {
            ChipLogProgress(NotSpecified, "Speed mode: %d", static_cast<uint8_t>(speed.Value()));
        }

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR ClosureDimensionDelegate::HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                                                const Optional<Globals::ThreeLevelAutoEnum> & speed)
{

    // Convert step to position delta
    int32_t stepSize = 100; // Each step = 1%
    int32_t delta    = numberOfSteps * stepSize;

    if (direction == StepDirectionEnum::kDecrease)
    {
        delta = -delta;
    }

    // Get current position from your actuator or state
    uint16_t currentPos = 0;
    //int32_t newPos      = std::clamp(static_cast<int32_t>(currentPos) + delta, 0, 10000);

    // TODO: MoveToPosition

    if (speed.HasValue())
    {
        ChipLogProgress(NotSpecified, "Speed mode: %d", static_cast<uint8_t>(speed.Value()));
    }

    return CHIP_NO_ERROR;
}

void ClosureDimensionManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}