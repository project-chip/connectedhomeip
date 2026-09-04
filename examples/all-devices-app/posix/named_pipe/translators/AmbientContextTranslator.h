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

#include <string>
#include <vector>

#include <posix/named_pipe/CommandTranslator.h>

namespace chip::app::NamedPipe {

class AmbientContextTranslator : public CommandTranslator
{
public:
    static Span<const CharSpan> GetActionNames()
    {
        static constexpr CharSpan kNames[] = {
            "SetAmbientContextSupport"_span, "AddAmbientContextDetect"_span, "SetPredictedActivity"_span,
            "SetSensorFusionSupported"_span, "SetObjectCount"_span,
        };
        return Span<const CharSpan>(kNames);
    }

    CHIP_ERROR TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                   OOBAccessorRegistry & registry) const override;

private:
    CHIP_ERROR TranslateSetAmbientContextSupport(EndpointId endpointId, const Json::Value & json,
                                                 OOBAccessorRegistry & registry) const;
    CHIP_ERROR TranslateAddAmbientContextDetect(EndpointId endpointId, const Json::Value & json,
                                                OOBAccessorRegistry & registry) const;
    CHIP_ERROR TranslateSetPredictedActivity(EndpointId endpointId, const Json::Value & json, OOBAccessorRegistry & registry) const;
    CHIP_ERROR TranslateSetSensorFusionSupported(EndpointId endpointId, const Json::Value & json,
                                                 OOBAccessorRegistry & registry) const;
    CHIP_ERROR TranslateSetObjectCount(EndpointId endpointId, const Json::Value & json, OOBAccessorRegistry & registry) const;
};

} // namespace chip::app::NamedPipe
