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

#include <posix/named_pipe/translators/OccupancyTranslator.h>

namespace chip::app {

CHIP_ERROR OccupancyTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                                             OOBAccessorRegistry & registry)
{
    const std::string & actionName = json["Name"].asString();
    if (actionName == "SetOccupancy")
    {
        return TranslateSetOccupancy(endpointId, json, registry);
    }
    if (actionName == "SetHoldTime")
    {
        return TranslateSetHoldTime(endpointId, json, registry);
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR OccupancyTranslator::TranslateSetOccupancy(EndpointId endpointId, const Json::Value & json,
                                                              OOBAccessorRegistry & registry)
{
    auto occupancy = ExtractBool(json, "Occupancy");
    VerifyOrReturnError(occupancy.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
    return DispatchAction(registry, "SetOccupancy"_span, endpointId, *occupancy);
}

CHIP_ERROR OccupancyTranslator::TranslateSetHoldTime(EndpointId endpointId, const Json::Value & json,
                                                             OOBAccessorRegistry & registry)
{
    auto holdTime = ExtractUInt<uint16_t>(json, "HoldTime");
    VerifyOrReturnError(holdTime.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
    return DispatchAction(registry, "SetHoldTime"_span, endpointId, *holdTime);
}

} // namespace chip::app
