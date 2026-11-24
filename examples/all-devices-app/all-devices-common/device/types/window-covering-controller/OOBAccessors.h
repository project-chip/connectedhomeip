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

#include "WindowCoveringController.h"
#include <oob-accessors/OOBAccessorRegistry.h>

namespace chip::app {

/**
 * Registers the OOB actions that make @p device send commands to its bound window coverings.
 *
 * Every action takes a TLV anonymous structure with context tag 1 = EndpointId (uint16). Actions
 * for other endpoints are not handled.
 *
 * | Action                           | Command sent                       | Tag 2 (uint16)         |
 * |----------------------------------|------------------------------------|------------------------|
 * | WindowCoveringUpOrOpen           | WindowCovering::UpOrOpen           | -                      |
 * | WindowCoveringDownOrClose        | WindowCovering::DownOrClose        | -                      |
 * | WindowCoveringStopMotion         | WindowCovering::StopMotion         | -                      |
 * | WindowCoveringGoToLiftPercentage | WindowCovering::GoToLiftPercentage | LiftPercent100thsValue |
 * | WindowCoveringGoToTiltPercentage | WindowCovering::GoToTiltPercentage | TiltPercent100thsValue |
 *
 * Tag 2 is mandatory for the two GoTo actions and must be at most 10000 (100.00%).
 */
void RegisterOOBAccessors(WindowCoveringController & device, OOBAccessorRegistry & registry);

} // namespace chip::app
