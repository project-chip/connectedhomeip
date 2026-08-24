/**
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

#include "ThermostatClusterOccupancy.h"
#include "ThermostatClusterCore.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <clusters/Thermostat/Metadata.h>

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

std::optional<temperature> ThermostatOccupancy::Delegate::GetUnoccupiedHeatingSetpoint() const
{
    return std::nullopt;
}

Protocols::InteractionModel::Status
ThermostatOccupancy::Delegate::SetUnoccupiedHeatingSetpoint(std::optional<temperature> unoccupiedHeatingSetpoint, bool & changed)
{
    changed = false;
    return Status::Success;
}

std::optional<temperature> ThermostatOccupancy::Delegate::GetUnoccupiedCoolingSetpoint() const
{
    return std::nullopt;
}

Protocols::InteractionModel::Status
ThermostatOccupancy::Delegate::SetUnoccupiedCoolingSetpoint(std::optional<temperature> unoccupiedCoolingSetpoint, bool & changed)
{
    changed = false;
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> ThermostatOccupancy::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Occupancy::Id:
        return encoder.Encode(mDelegate.GetOccupancy());
    }

    return std::nullopt;
}

Status ThermostatOccupancy::SetOccupancy(BitMask<OccupancyBitmap> occupied)
{
    bool changed = false;
    if (auto err = mDelegate.SetOccupancy(occupied, changed); err != Status::Success)
    {
        return err;
    }
    if (changed)
    {
        mCluster.NotifyAttributeChanged(Occupancy::Id);
    }
    return Status::Success;
}

CHIP_ERROR ThermostatOccupancy::Attributes(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    return builder.AppendElements({ Occupancy::kMetadataEntry });
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
