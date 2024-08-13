/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "thermostat-server.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

Status ThermostatAttrAccess::OnBeginWrite(EndpointId endpoint, AttributeId attributeId)
{
    switch (attributeId)
    {
    case Presets::Id:
        return BeginPresets(endpoint);
    case Schedules::Id:
        return Status::Success;
    default:
        return Status::InvalidInState;
    }
}

Status ThermostatAttrAccess::OnPreCommitWrite(EndpointId endpoint, AttributeId attributeId)
{
    switch (attributeId)
    {
    case Presets::Id:
        return PreCommitPresets(endpoint);
    case Schedules::Id:
        return Status::Success;
    default:
        return Status::InvalidInState;
    }
}

Status ThermostatAttrAccess::OnCommitWrite(EndpointId endpoint, AttributeId attributeId)
{
    switch (attributeId)
    {
    case Presets::Id:
        return CommitPresets(endpoint);
    case Schedules::Id:
        return Status::Success;
    default:
        return Status::InvalidInState;
    }
}

Status ThermostatAttrAccess::OnRollbackWrite(EndpointId endpoint, AttributeId attributeId)
{
    switch (attributeId)
    {
    case Presets::Id:
        return RollbackPresets(endpoint);
    case Schedules::Id:
        return Status::Success;
    default:
        return Status::InvalidInState;
    }
}

std::optional<System::Clock::Milliseconds16> ThermostatAttrAccess::GetWriteTimeout(EndpointId endpoint, AttributeId attributeId)
{
    auto delegate = GetDelegate(endpoint);
    if (delegate == nullptr)
    {
        return std::nullopt;
    }
    return delegate->GetWriteTimeout(attributeId);
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
