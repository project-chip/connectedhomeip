/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/thermostat-server/ThermostatDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/**
 * @brief Removes every entry in the ThermostatSuggestions attribute list whose PresetHandle no longer matches a
 *        preset in the Presets attribute list. Per spec, this cascade must run after a Presets atomic write commits
 *        a preset removal. If the entry being removed is the CurrentThermostatSuggestion, RemoveFromThermostatSuggestionsList
 *        is responsible for setting CurrentThermostatSuggestion to null, per its API contract.
 *
 * @param[in]  delegate The delegate to use.
 * @param[out] didRemoveAnEntry Set to true if at least one entry was removed from the ThermostatSuggestions list,
 *             false otherwise. Callers can use this to decide whether the ThermostatSuggestions attribute needs to
 *             be reported as changed.
 *
 * @return CHIP_NO_ERROR if successful, an error code if not.
 */
CHIP_ERROR RemoveThermostatSuggestionsForRemovedPresets(Delegate * delegate, bool & didRemoveAnEntry);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
