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

#include <device/types/on-off-light-switch/OnOffLightSwitch.h>
#include <oob-accessors/OOBAccessorRegistry.h>

namespace chip::app {

/**
 * Registers the OOB actions that make @p device send commands to its bound peers.
 *
 * Every action takes a TLV anonymous structure with context tag 1 = EndpointId (uint16). Actions
 * for other endpoints are not handled. Extra fields:
 *
 * | Action              | Tag 2                    | Tag 3                 |
 * |---------------------|--------------------------|-----------------------|
 * | SwitchOn            | -                        | -                     |
 * | SwitchOff           | -                        | -                     |
 * | SwitchToggle        | -                        | -                     |
 * | SwitchIdentify      | IdentifyTime (uint16, s) | -                     |
 * | SwitchTriggerEffect | EffectIdentifier (uint8) | EffectVariant (uint8) |
 */
void RegisterOOBAccessors(OnOffLightSwitch & device, OOBAccessorRegistry & registry);

} // namespace chip::app
