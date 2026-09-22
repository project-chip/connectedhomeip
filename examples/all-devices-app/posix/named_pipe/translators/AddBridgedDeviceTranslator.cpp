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

#include <posix/named_pipe/translators/AddBridgedDeviceTranslator.h>

namespace chip::app::NamedPipe {

CHIP_ERROR AddBridgedDeviceTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                                            OOBAccessorRegistry & registry) const
{
    if (!json.isMember("Device") || !json["Device"].isString())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    endpointId = ExtractUInt<EndpointId>(json, "ParentEndpointId").value_or(endpointId);

    const std::string device = json["Device"].asString();
    return DispatchStringAction(registry, "AddBridgedDevice"_span, endpointId,
                                CharSpan(device.data(), device.size()));
}

} // namespace chip::app::NamedPipe
