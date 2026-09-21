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

#include <posix/named_pipe/translators/SwitchTranslator.h>

namespace chip::app::NamedPipe {

CHIP_ERROR SwitchTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                                 OOBAccessorRegistry & registry) const
{
    auto currentPosition = ExtractUInt<uint8_t>(json, "CurrentPosition");
    VerifyOrReturnError(currentPosition.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
    return DispatchAction(registry, "SetCurrentPosition"_span, endpointId, *currentPosition);
}

} // namespace chip::app::NamedPipe
