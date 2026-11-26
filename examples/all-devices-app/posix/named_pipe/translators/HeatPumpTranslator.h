/*
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
 * Translates named-pipe JSON into the Heat Pump OOB action, which sends a Thermostat
 * SetpointRaiseLower command to the thermostats bound to the heat pump endpoint:
 *
 *   {"Name": "HeatPumpSetpointRaiseLower", "EndpointId": 1, "Mode": 0, "Amount": 10}
 *
 * Mode is a SetpointRaiseLowerModeEnum value (0 = Heat, 1 = Cool, 2 = Both). Amount is a signed
 * step in 0.1 °C.
 */
class HeatPumpTranslator : public CommandTranslator
{
public:
    static Span<const CharSpan> GetActionNames()
    {
        static constexpr CharSpan kNames[] = { "HeatPumpSetpointRaiseLower"_span };
        return Span<const CharSpan>(kNames);
    }

    CHIP_ERROR TranslateAndExecute(EndpointId endpointId, const Json::Value & json, OOBAccessorRegistry & registry) const override;
};

} // namespace chip::app::NamedPipe
