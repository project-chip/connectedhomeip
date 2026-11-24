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
 * Translates named-pipe JSON into the Window Covering Controller OOB actions, which send commands
 * to the window coverings bound to the controller endpoint:
 *
 *   {"Name": "WindowCoveringUpOrOpen" | "WindowCoveringDownOrClose" | "WindowCoveringStopMotion", "EndpointId": 1}
 *   {"Name": "WindowCoveringGoToLiftPercentage", "EndpointId": 1, "LiftPercent100thsValue": 5000}
 *   {"Name": "WindowCoveringGoToTiltPercentage", "EndpointId": 1, "TiltPercent100thsValue": 5000}
 *
 * The percentage keys are mandatory. The accessor rejects values above 10000 (100.00%).
 */
class WindowCoveringControllerTranslator : public CommandTranslator
{
public:
    static Span<const CharSpan> GetActionNames()
    {
        static constexpr CharSpan kNames[] = {
            "WindowCoveringUpOrOpen"_span,
            "WindowCoveringDownOrClose"_span,
            "WindowCoveringStopMotion"_span,
            "WindowCoveringGoToLiftPercentage"_span,
            "WindowCoveringGoToTiltPercentage"_span,
        };
        return Span<const CharSpan>(kNames);
    }

    CHIP_ERROR TranslateAndExecute(EndpointId endpointId, const Json::Value & json, OOBAccessorRegistry & registry) const override;
};

} // namespace chip::app::NamedPipe
