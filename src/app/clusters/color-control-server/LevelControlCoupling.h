/**
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

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ColorControl {

/**
 * @brief Coupling entry point invoked by the Level Control cluster when its CurrentLevel changes.
 *
 * The Level Control cluster calls this whenever its CurrentLevel changes AND its
 * CoupleColorTempToLevel option bit is set (that option gate is enforced on the Level Control side,
 * so it is not re-checked here). The implementation resolves both clusters through the code-driven
 * ServerClusterInterface registry and reads Level Control's live CurrentLevel via its typed getter,
 * then pushes the value into Color Control's CoupleColorTempToLevel() API.
 *
 * This is intentionally NOT placed under codegen/: because the lookup and the cross-cluster read go
 * through the code-driven registry and typed getters, this glue carries no Ember/ZAP dependency.
 * It replaces the legacy emberAfPluginLevelControlCoupledColorTempChangeCallback hook.
 */
void NotifyLevelControlCurrentLevelChanged(EndpointId endpoint);

} // namespace ColorControl
} // namespace Clusters
} // namespace app
} // namespace chip
