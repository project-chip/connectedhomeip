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

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

enum class SetpointAttributes : uint16_t
{
    kNone                 = 0,
    kOccupiedHeating      = 1 << 0,
    kOccupiedCooling      = 1 << 1,
    kUnoccupiedHeating    = 1 << 2,
    kUnoccupiedCooling    = 1 << 3,
    kMinSetpointDeadBand  = 1 << 4,
    kMinHeatSetpointLimit = 1 << 5,
    kMaxHeatSetpointLimit = 1 << 6,
    kMinCoolSetpointLimit = 1 << 7,
    kMaxCoolSetpointLimit = 1 << 8,
};

}
} // namespace Clusters
} // namespace app
} // namespace chip
