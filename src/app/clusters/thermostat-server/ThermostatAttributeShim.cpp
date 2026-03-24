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

#include "ThermostatCluster.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;

using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

namespace LocalTemperature {

Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    value = cluster->GetLocalTemperature();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    if (cluster->SetLocalTemperature(value) != CHIP_NO_ERROR)
    {
        return Status::Failure;
    }
    return Status::Success;
}

} // namespace LocalTemperature

namespace SystemMode {

Status Get(EndpointId endpoint, chip::app::Clusters::Thermostat::SystemModeEnum * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    *value = cluster->GetSystemMode();
    return Status::Success;
}

Status Set(EndpointId endpoint, chip::app::Clusters::Thermostat::SystemModeEnum value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    if (cluster->SetSystemMode(value) != CHIP_NO_ERROR)
    {
        return Status::Failure;
    }
    return Status::Success;
}
} // namespace SystemMode

namespace ThermostatRunningMode {

Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::app::Clusters::Thermostat::ThermostatRunningModeEnum * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    *value = cluster->GetRunningMode();
    return Status::Success;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::Thermostat::ThermostatRunningModeEnum value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    if (cluster->SetRunningMode(value) != CHIP_NO_ERROR)
    {
        return Status::Failure;
    }
    return Status::Success;
}

} // namespace ThermostatRunningMode

namespace OccupiedCoolingSetpoint {

Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    *value = cluster->mSetpoints.occupiedCoolingSetpoint;
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    return cluster->ChangeSetpointAttribute(Id, value);
}

} // namespace OccupiedCoolingSetpoint

namespace OccupiedHeatingSetpoint {

Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    *value = cluster->mSetpoints.occupiedHeatingSetpoint;
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    return cluster->ChangeSetpointAttribute(Id, value);
}

} // namespace OccupiedHeatingSetpoint

namespace UnoccupiedCoolingSetpoint {

Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    if (!cluster->mSetpoints.occupancySupported)
    {
        ChipLogError(Zcl, "Thermostat cluster on endpoint %d does not support unoccupied setpoints", endpoint);
        return Status::UnsupportedAttribute;
    }
    *value = cluster->mSetpoints.unoccupiedCoolingSetpoint;
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    if (!cluster->mSetpoints.occupancySupported)
    {
        ChipLogError(Zcl, "Thermostat cluster on endpoint %d does not support unoccupied setpoints", endpoint);
        return Status::UnsupportedAttribute;
    }
    return cluster->ChangeSetpointAttribute(Id, value);
}

} // namespace UnoccupiedCoolingSetpoint

namespace UnoccupiedHeatingSetpoint {

Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    if (!cluster->mSetpoints.occupancySupported)
    {
        ChipLogError(Zcl, "Thermostat cluster on endpoint %d does not support unoccupied setpoints", endpoint);
        return Status::UnsupportedAttribute;
    }
    *value = cluster->mSetpoints.unoccupiedHeatingSetpoint;
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    if (!cluster->mSetpoints.occupancySupported)
    {
        ChipLogError(Zcl, "Thermostat cluster on endpoint %d does not support unoccupied setpoints", endpoint);
        return Status::UnsupportedAttribute;
    }
    return cluster->ChangeSetpointAttribute(Id, value);
}

} // namespace UnoccupiedHeatingSetpoint

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
