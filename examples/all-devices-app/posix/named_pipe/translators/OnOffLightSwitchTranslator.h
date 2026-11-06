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
 * Translates named-pipe JSON into the On/Off Light Switch OOB actions, which send commands to the
 * peers bound to the switch endpoint:
 *
 *   {"Name": "SwitchOn" | "SwitchOff" | "SwitchToggle", "EndpointId": 1}
 *   {"Name": "SwitchIdentify", "EndpointId": 1, "IdentifyTime": 10}
 *   {"Name": "SwitchTriggerEffect", "EndpointId": 1, "EffectIdentifier": 0, "EffectVariant": 0}
 *
 * EffectVariant is optional and defaults to 0 (Default).
 */
class OnOffLightSwitchTranslator : public CommandTranslator
{
public:
    static Span<const CharSpan> GetActionNames()
    {
        static constexpr CharSpan kNames[] = {
            "SwitchOn"_span, "SwitchOff"_span, "SwitchToggle"_span, "SwitchIdentify"_span, "SwitchTriggerEffect"_span,
        };
        return Span<const CharSpan>(kNames);
    }

    CHIP_ERROR TranslateAndExecute(EndpointId endpointId, const Json::Value & json, OOBAccessorRegistry & registry) const override;
};

} // namespace chip::app::NamedPipe
