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

#include <posix/named_pipe/translators/BooleanStateTranslator.h>

namespace chip::app::NamedPipe {

CHIP_ERROR BooleanStateTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                                       OOBAccessorRegistry & registry) const
{
    auto newState = ExtractBool(json, "StateValue");
    if (!newState.has_value())
    {
        newState = ExtractBool(json, "NewState");
    }
    if (!newState.has_value())
    {
        newState = ExtractBool(json, "BooleanState");
    }
    VerifyOrReturnError(newState.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
    return DispatchAction(registry, "SetBooleanState"_span, endpointId, *newState);
}

} // namespace chip::app::NamedPipe
