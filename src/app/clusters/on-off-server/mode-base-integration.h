/**
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <app/util/config.h>
#include <lib/core/DataModelTypes.h>

#ifdef MATTER_DM_PLUGIN_MODE_BASE

/**
 * For all ModeBase alias clusters on the given endpoint, if the OnOff feature is supported and
 * the OnMode attribute is set, update the CurrentMode attribute value to the OnMode value.
 * @param endpoint
 */
void UpdateModeBaseCurrentModeToOnMode(chip::EndpointId endpoint);

#else

inline void UpdateModeBaseCurrentModeToOnMode(chip::EndpointId endpoint) {}

#endif // MATTER_DM_PLUGIN_MODE_BASE
