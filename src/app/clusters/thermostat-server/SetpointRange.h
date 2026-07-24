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

#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-storage.h>
#include <protocols/interaction_model/Constants.h>

#include "Setpoint.h"
#include "SetpointAttributes.h"
#include "Temperature.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class Setpoints;

struct SetpointRange
{
    AbsoluteSetpoint heating;
    AbsoluteSetpoint cooling;

    SetpointRange(AbsoluteSetpoint heat, AbsoluteSetpoint cool) : heating(heat), cooling(cool) {}

    SetpointRange(const SetpointRange & spr) : heating(spr.heating), cooling(spr.cooling) {}

    SetpointRange & operator=(const SetpointRange & other) = default;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
