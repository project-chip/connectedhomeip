/**
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

#include "ThermostatDelegate.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

using Protocols::InteractionModel::Status;

std::optional<temperature> CoolingDelegate::GetAbsMinCoolSetpointLimit() const
{
    return {};
};
std::optional<temperature> CoolingDelegate::GetAbsMaxCoolSetpointLimit() const
{
    return {};
};

std::optional<temperature> CoolingDelegate::GetMinCoolSetpointLimit() const
{
    return {};
};
Status CoolingDelegate::SetMinCoolSetpointLimit(std::optional<temperature> minCoolSetpointLimit, bool & changed)
{
    changed = false;
    return Status::Success;
};

std::optional<temperature> CoolingDelegate::GetMaxCoolSetpointLimit() const
{
    return {};
};
Status CoolingDelegate::SetMaxCoolSetpointLimit(std::optional<temperature> maxCoolSetpointLimit, bool & changed)
{
    changed = false;
    return Status::Success;
};

std::optional<temperature> HeatingDelegate::GetAbsMinHeatSetpointLimit() const
{
    return {};
};
std::optional<temperature> HeatingDelegate::GetAbsMaxHeatSetpointLimit() const
{
    return {};
};

std::optional<temperature> HeatingDelegate::GetMinHeatSetpointLimit() const
{
    return {};
};
Status HeatingDelegate::SetMinHeatSetpointLimit(std::optional<temperature> minHeatSetpointLimit, bool & changed)
{
    changed = false;
    return Status::Success;
};

std::optional<temperature> HeatingDelegate::GetMaxHeatSetpointLimit() const
{
    return {};
};
Status HeatingDelegate::SetMaxHeatSetpointLimit(std::optional<temperature> maxHeatSetpointLimit, bool & changed)
{
    changed = false;
    return Status::Success;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
