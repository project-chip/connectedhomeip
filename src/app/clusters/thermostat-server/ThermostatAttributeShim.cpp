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
namespace Attributes {

namespace ControlSequenceOfOperation {

Protocols::InteractionModel::Status
Get(EndpointId endpoint, chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum * value) // ControlSequenceOfOperationEnum
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
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
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
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
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
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
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    return cluster->SetLocalTemperature(value);
}

Status Set(EndpointId endpoint, int16_t value, MarkAttributeDirty markDirty)
{
    return Set(endpoint, value);
}

} // namespace LocalTemperature

namespace SystemMode {

Status Get(EndpointId endpoint, chip::app::Clusters::Thermostat::SystemModeEnum * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
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
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    return cluster->SetSystemMode(value);
}
} // namespace SystemMode

namespace ThermostatRunningMode {

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::Thermostat::ThermostatRunningModeEnum value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    return cluster->SetRunningMode(value);
}

} // namespace ThermostatRunningMode

namespace ThermostatRunningState {

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::BitMask<chip::app::Clusters::Thermostat::RelayStateBitmap> value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
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
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    *value = cluster->mSetpoints.absoluteHeatLimits.minimum.Temperature();
    return Status::Success;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    return cluster->ChangeSetpointAttribute(Id, value).GetStatusCode().GetStatus();
}
} // namespace AbsMinHeatSetpointLimit

namespace AbsMaxHeatSetpointLimit {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    *value = cluster->mSetpoints.absoluteHeatLimits.maximum.Temperature();
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    return cluster->ChangeSetpointAttribute(Id, value).GetStatusCode().GetStatus();
}
} // namespace AbsMaxHeatSetpointLimit

namespace OccupiedCoolingSetpoint {

Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    *value = cluster->mSetpoints.occupied.cooling.Temperature();
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    return cluster->ChangeSetpointAttribute(Id, value).GetStatusCode().GetStatus();
}

} // namespace OccupiedCoolingSetpoint

namespace OccupiedHeatingSetpoint {

Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    *value = cluster->mSetpoints.occupied.heating.Temperature();
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    return cluster->ChangeSetpointAttribute(Id, value).GetStatusCode().GetStatus();
}

} // namespace OccupiedHeatingSetpoint

namespace UnoccupiedCoolingSetpoint {

Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    *value = cluster->mSetpoints.unoccupied.cooling.Temperature();
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    return cluster->ChangeSetpointAttribute(Id, value).GetStatusCode().GetStatus();
}

} // namespace UnoccupiedCoolingSetpoint

namespace UnoccupiedHeatingSetpoint {

Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    *value = cluster->mSetpoints.unoccupied.heating.Temperature();
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    return cluster->ChangeSetpointAttribute(Id, value).GetStatusCode().GetStatus();
}

} // namespace UnoccupiedHeatingSetpoint

namespace FeatureMap {
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    cluster->mFeatures = BitFlags<Thermostat::Feature>(value);
    return Status::Success;
}
} // namespace FeatureMap

// These attributes are deprecated, and only in "use" by ThermostaticRadiatorValveManager, and will be removed in the future
namespace PICoolingDemand {
uint8_t piCoolingDemand = 0;
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint8_t * value)
{
    *value = piCoolingDemand;
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value)
{
    piCoolingDemand = value;
    return Status::Success;
}
} // namespace PICoolingDemand

namespace PIHeatingDemand {
uint8_t piHeatingDemand = 0;
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint8_t * value)
{
    *value = piHeatingDemand;
    return Status::Success;
}
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value)
{
    piHeatingDemand = value;
    return Status::Success;
}
} // namespace PIHeatingDemand

} // namespace Attributes
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
