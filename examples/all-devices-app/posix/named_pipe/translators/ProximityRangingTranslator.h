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
 * Named-pipe translator for the Proximity Ranging cluster out of band OOB settings.
 *
 * "SetRangingConstraints" updates the RangingConstraints (attribute 0x0007) published  on the target endpoint:
 *
 *   {"Name": "SetRangingConstraints", "EndpointId": 1, "Constraints": [
 *      {"Technology": 3, "Role": 3, "Enabled": false},
 *      {"Technology": 3, "Role": 2, "MinRangingInterval": 10, "MaxSessionDuration": 60, "MaxRangingInstances": 5}
 *   ]}
 *
 * Technology / Role are the RangingTechEnum / RangingRoleEnum enumeration values.
 * An empty Constraints array clears the list.
 */
class ProximityRangingTranslator : public CommandTranslator
{
public:
    static Span<const CharSpan> GetActionNames()
    {
        static constexpr CharSpan kNames[] = { "SetRangingConstraints"_span };
        return Span<const CharSpan>(kNames);
    }

    CHIP_ERROR TranslateAndExecute(EndpointId endpointId, const Json::Value & json, OOBAccessorRegistry & registry) const override;

private:
    CHIP_ERROR TranslateSetRangingConstraints(EndpointId endpointId, const Json::Value & json,
                                              OOBAccessorRegistry & registry) const;
};

} // namespace chip::app::NamedPipe
