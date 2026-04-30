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

#include <app/util/attribute-table.h>

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

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    using Traits = NumericAttributeTraits<chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    auto status = cluster->SetControlSequenceOfOperation(value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to change control sequence of operation attribute for endpoint %d", endpoint);
        return status;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_ENUM8_ATTRIBUTE_TYPE));
}
} // namespace ControlSequenceOfOperation

namespace LocalTemperature {


Status Set(EndpointId endpoint, int16_t value, MarkAttributeDirty markDirty)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    using Traits = NumericAttributeTraits<int16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    auto status = cluster->SetLocalTemperature(value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to change local temperature attribute for endpoint %d", endpoint);
        return status;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_TEMPERATURE_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Status Set(EndpointId endpoint, int16_t value)
{
    return Set(endpoint, value, MarkAttributeDirty::kNo);
}


} // namespace LocalTemperature

namespace SystemMode {

Status Set(EndpointId endpoint, chip::app::Clusters::Thermostat::SystemModeEnum value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    using Traits = NumericAttributeTraits<chip::app::Clusters::Thermostat::SystemModeEnum>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    auto status = cluster->SetSystemMode(value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to change system mode attribute for endpoint %d", endpoint);
        return status;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_ENUM8_ATTRIBUTE_TYPE));
}
} // namespace SystemMode

namespace ThermostatRunningMode {

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::Thermostat::ThermostatRunningModeEnum value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    using Traits = NumericAttributeTraits<chip::app::Clusters::Thermostat::ThermostatRunningModeEnum>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    auto status = cluster->SetRunningMode(value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to change thermostat running mode attribute for endpoint %d", endpoint);
        return status;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_ENUM8_ATTRIBUTE_TYPE));
}

} // namespace ThermostatRunningMode

namespace ThermostatRunningState {

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::BitMask<chip::app::Clusters::Thermostat::RelayStateBitmap> value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::Thermostat::RelayStateBitmap>>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    auto status = cluster->SetRunningState(value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to change running state attribute for endpoint %d", endpoint);
        return status;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP16_ATTRIBUTE_TYPE));
}
} // namespace ThermostatRunningState

namespace AbsMinHeatSetpointLimit {

Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    using Traits = NumericAttributeTraits<int16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    auto status = cluster->ChangeSetpointAttribute(Id, value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to change absolute min heat setpoint limit attribute for endpoint %d", endpoint);
        return status;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_TEMPERATURE_ATTRIBUTE_TYPE));
}
} // namespace AbsMinHeatSetpointLimit

namespace AbsMaxHeatSetpointLimit {

Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    using Traits = NumericAttributeTraits<int16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    auto status = cluster->ChangeSetpointAttribute(Id, value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to change absolute max heat setpoint limit attribute for endpoint %d", endpoint);
        return status;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_TEMPERATURE_ATTRIBUTE_TYPE));
}
} // namespace AbsMaxHeatSetpointLimit

namespace OccupiedCoolingSetpoint {

Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    using Traits = NumericAttributeTraits<int16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    auto status = cluster->ChangeSetpointAttribute(Id, value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to change occupied cooling setpoint attribute for endpoint %d", endpoint);
        return status;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_TEMPERATURE_ATTRIBUTE_TYPE));
}

} // namespace OccupiedCoolingSetpoint

namespace OccupiedHeatingSetpoint {

Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    using Traits = NumericAttributeTraits<int16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    auto status = cluster->ChangeSetpointAttribute(Id, value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to change occupied heating setpoint attribute for endpoint %d", endpoint);
        return status;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_TEMPERATURE_ATTRIBUTE_TYPE));
}

} // namespace OccupiedHeatingSetpoint

namespace UnoccupiedCoolingSetpoint {

Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    using Traits = NumericAttributeTraits<int16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    auto status = cluster->ChangeSetpointAttribute(Id, value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to change unoccupied cooling setpoint attribute for endpoint %d", endpoint);
        return status;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_TEMPERATURE_ATTRIBUTE_TYPE));
}

} // namespace UnoccupiedCoolingSetpoint

namespace UnoccupiedHeatingSetpoint {

Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = chip::app::Clusters::Thermostat::ClusterForEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Status::Failure;
    }
    using Traits = NumericAttributeTraits<int16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    auto status = cluster->ChangeSetpointAttribute(Id, value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to change unoccupied heating setpoint attribute for endpoint %d", endpoint);
        return status;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_TEMPERATURE_ATTRIBUTE_TYPE));
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
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Thermostat::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE));
}
} // namespace FeatureMap

// These attributes are deprecated, and only in "use" by ThermostaticRadiatorValveManager, and will be removed in the future
namespace PICoolingDemand {

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::Thermostat::Id, Id, writable, ZCL_INT8U_ATTRIBUTE_TYPE);
}

} // namespace PICoolingDemand

namespace PIHeatingDemand {

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value)
{
     using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::Thermostat::Id, Id, writable, ZCL_INT8U_ATTRIBUTE_TYPE);
}

} // namespace PIHeatingDemand

} // namespace Attributes
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
