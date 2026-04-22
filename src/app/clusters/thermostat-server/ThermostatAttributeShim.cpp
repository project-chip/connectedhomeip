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

namespace ControlSequenceOfOperation {

Protocols::InteractionModel::Status
Get(EndpointId endpoint, chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum * value) // ControlSequenceOfOperationEnum
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    *value = cluster->GetControlSequenceOfOperation();
    return Status::Success;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    return cluster->SetControlSequenceOfOperation(value);
}
} // namespace ControlSequenceOfOperation

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
    return cluster->SetLocalTemperature(value);
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
    return cluster->SetSystemMode(value);
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
    return cluster->SetRunningMode(value);
}

} // namespace ThermostatRunningMode

namespace ThermostatRunningState {
Protocols::InteractionModel::Status Get(EndpointId endpoint,
                                        chip::BitMask<chip::app::Clusters::Thermostat::RelayStateBitmap> * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    *value = cluster->GetRunningState();
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::BitMask<chip::app::Clusters::Thermostat::RelayStateBitmap> value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    return cluster->SetRunningState(value);
}
} // namespace ThermostatRunningState

namespace AbsMinHeatSetpointLimit {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    *value = cluster->mSetpoints.absoluteHeatLimits.minimum;
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
} // namespace AbsMinHeatSetpointLimit

namespace AbsMaxHeatSetpointLimit {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    *value = cluster->mSetpoints.absoluteHeatLimits.maximum;
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
} // namespace AbsMaxHeatSetpointLimit

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
    return cluster->ChangeSetpointAttribute(Id, value);
}

} // namespace UnoccupiedHeatingSetpoint

namespace FeatureMap {
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    cluster->mFeatures = BitFlags<Thermostat::Feature>(value);
    return Status::Success;
}
} // namespace FeatureMap
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
