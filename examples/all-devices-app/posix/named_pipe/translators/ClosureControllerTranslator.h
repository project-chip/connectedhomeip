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

#include <posix/named_pipe/CommandTranslator.h>

namespace chip::app::NamedPipe {

/**
 * Translates named-pipe JSON into the Closure Controller OOB actions, which send commands to the
 * closures bound to the controller endpoint:
 *
 *   {"Name": "ClosureMoveTo" | "ClosureGroupedMoveTo", "EndpointId": 1, "Position": 1, "Latch": true, "Speed": 1}
 *   {"Name": "ClosureSetTarget" | "ClosureGroupedSetTarget", "EndpointId": 1, "Position": 5000, "Latch": true, "Speed": 1}
 *   {"Name": "ClosureStep" | "ClosureGroupedStep", "EndpointId": 1, "Direction": 0, "NumberOfSteps": 2, "Speed": 1}
 *   {"Name": "ClosureStop" | "ClosureCalibrate", "EndpointId": 1}
 *   {"Name": "ClosureIdentify", "EndpointId": 1, "IdentifyTime": 10}
 *   {"Name": "ClosureTriggerEffect", "EndpointId": 1, "EffectIdentifier": 0, "EffectVariant": 0}
 *
 * Position, Latch and Speed are optional; an absent key leaves the command field unset.
 * EffectVariant is optional and defaults to 0 (Default).
 */
class ClosureControllerTranslator : public CommandTranslator
{
public:
    static Span<const CharSpan> GetActionNames()
    {
        static constexpr CharSpan kNames[] = {
            "ClosureMoveTo"_span,   "ClosureGroupedMoveTo"_span, "ClosureSetTarget"_span, "ClosureGroupedSetTarget"_span,
            "ClosureStep"_span,     "ClosureGroupedStep"_span,   "ClosureStop"_span,      "ClosureCalibrate"_span,
            "ClosureIdentify"_span, "ClosureTriggerEffect"_span,
        };
        return Span<const CharSpan>(kNames);
    }

    CHIP_ERROR TranslateAndExecute(EndpointId endpointId, const Json::Value & json, OOBAccessorRegistry & registry) const override;
};

} // namespace chip::app::NamedPipe
