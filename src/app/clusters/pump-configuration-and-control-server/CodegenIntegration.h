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

#include <app/clusters/pump-configuration-and-control-server/PumpConfigurationAndControlCluster.h>
#include <app/clusters/pump-configuration-and-control-server/PumpConfigurationAndControlDelegate.h>
#include <app/data-model/Nullable.h>
#include <clusters/PumpConfigurationAndControl/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BitMask.h>
#include <protocols/interaction_model/StatusCode.h>

#include <cstdint>

namespace chip::app::Clusters::PumpConfigurationAndControl {

/// Optional application hooks for a cluster created from the ZAP configuration.
///
/// The integration always gives the cluster its own Delegate. That delegate forwards the calls below
/// to the ApplicationDelegate installed with SetApplicationDelegate. While OperationMode is Minimum or
/// Maximum, it also sets the Level Control CurrentLevel on the same endpoint to MinLevel (at least 1)
/// or MaxLevel, if the application includes the Level Control cluster.
class ApplicationDelegate
{
public:
    virtual ~ApplicationDelegate() = default;

    /// See Delegate::GetRemoteSensorType. Without an ApplicationDelegate, no remote sensor is reported.
    virtual RemoteSensorType GetRemoteSensorType(EndpointId endpointId) { return RemoteSensorType::kNoSensor; }

    /// See Delegate::OnAttributeChanged.
    virtual void OnAttributeChanged(EndpointId endpointId, AttributeId attributeId) {}
};

/// Installs `delegate` for the cluster on `endpointId`, or removes it if `delegate` is nullptr.
///
/// Can be called before or after the cluster is created. If it is installed before, the cluster asks
/// it for the remote sensor during startup. If the cluster already exists, the remote sensor is
/// queried again (see PumpConfigurationAndControlCluster::OnRemoteSensorChanged). The delegate is
/// removed when the cluster shuts down, and must outlive its installation.
///
/// Returns CHIP_ERROR_NOT_FOUND if the endpoint has no Pump Configuration and Control server.
CHIP_ERROR SetApplicationDelegate(EndpointId endpointId, ApplicationDelegate * delegate);

/// Returns the cluster instance registered on the given endpoint, nullptr otherwise.
PumpConfigurationAndControlCluster * FindClusterOnEndpoint(EndpointId endpointId);

/// Ember-style accessors for the live attribute values of the cluster registered on `endpoint`.
///
/// They keep the signatures of the generated accessors that existed before the cluster became
/// code-driven, so that application code keeps compiling. Each call forwards to the cluster returned
/// by FindClusterOnEndpoint. They return UnsupportedEndpoint if there is no cluster,
/// UnsupportedAttribute if the attribute is not enabled on the endpoint, and Failure if a getter
/// receives a null pointer. Setters return the status of the cluster setter, e.g. ConstraintError
/// for a mode whose feature is not enabled.
///
/// Fixed attributes (MaxPressure, Min/Max limits) only have getters: their values come from the
/// ZAP defaults. For the configured ZAP default values, use the generated `GetDefault` functions in
/// `app-common/zap-generated/attributes/Accessors.h`.
namespace Attributes {

namespace MaxPressure {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
} // namespace MaxPressure

namespace MaxSpeed {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value);
} // namespace MaxSpeed

namespace MaxFlow {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value);
} // namespace MaxFlow

namespace MinConstPressure {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
} // namespace MinConstPressure

namespace MaxConstPressure {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
} // namespace MaxConstPressure

namespace MinCompPressure {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
} // namespace MinCompPressure

namespace MaxCompPressure {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
} // namespace MaxCompPressure

namespace MinConstSpeed {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value);
} // namespace MinConstSpeed

namespace MaxConstSpeed {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value);
} // namespace MaxConstSpeed

namespace MinConstFlow {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value);
} // namespace MinConstFlow

namespace MaxConstFlow {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value);
} // namespace MaxConstFlow

namespace MinConstTemp {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
} // namespace MinConstTemp

namespace MaxConstTemp {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
} // namespace MaxConstTemp

namespace Capacity {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);
Protocols::InteractionModel::Status SetNull(EndpointId endpoint);
Protocols::InteractionModel::Status Set(EndpointId endpoint, const DataModel::Nullable<int16_t> & value);
} // namespace Capacity

namespace Speed {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value);
Protocols::InteractionModel::Status SetNull(EndpointId endpoint);
Protocols::InteractionModel::Status Set(EndpointId endpoint, const DataModel::Nullable<uint16_t> & value);
} // namespace Speed

namespace LifetimeRunningHours {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint32_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value);
Protocols::InteractionModel::Status SetNull(EndpointId endpoint);
Protocols::InteractionModel::Status Set(EndpointId endpoint, const DataModel::Nullable<uint32_t> & value);
} // namespace LifetimeRunningHours

namespace Power {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint32_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value);
Protocols::InteractionModel::Status SetNull(EndpointId endpoint);
Protocols::InteractionModel::Status Set(EndpointId endpoint, const DataModel::Nullable<uint32_t> & value);
} // namespace Power

namespace LifetimeEnergyConsumed {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint32_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value);
Protocols::InteractionModel::Status SetNull(EndpointId endpoint);
Protocols::InteractionModel::Status Set(EndpointId endpoint, const DataModel::Nullable<uint32_t> & value);
} // namespace LifetimeEnergyConsumed

namespace PumpStatus {
Protocols::InteractionModel::Status Get(EndpointId endpoint, BitMask<PumpStatusBitmap> * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, BitMask<PumpStatusBitmap> value);
} // namespace PumpStatus

namespace EffectiveOperationMode {
Protocols::InteractionModel::Status Get(EndpointId endpoint, OperationModeEnum * value);
} // namespace EffectiveOperationMode

namespace EffectiveControlMode {
Protocols::InteractionModel::Status Get(EndpointId endpoint, ControlModeEnum * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, ControlModeEnum value);
} // namespace EffectiveControlMode

namespace OperationMode {
Protocols::InteractionModel::Status Get(EndpointId endpoint, OperationModeEnum * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, OperationModeEnum value);
} // namespace OperationMode

namespace ControlMode {
Protocols::InteractionModel::Status Get(EndpointId endpoint, ControlModeEnum * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, ControlModeEnum value);
} // namespace ControlMode

namespace FeatureMap {
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value);
} // namespace FeatureMap

} // namespace Attributes

} // namespace chip::app::Clusters::PumpConfigurationAndControl
