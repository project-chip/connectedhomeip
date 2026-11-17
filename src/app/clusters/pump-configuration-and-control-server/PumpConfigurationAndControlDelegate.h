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

#include <clusters/PumpConfigurationAndControl/Enums.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

namespace chip::app::Clusters::PumpConfigurationAndControl {

/// Type of the remote sensor that controls the pump while OperationMode is Normal.
enum class RemoteSensorType : uint8_t
{
    kNoSensor,
    kPressureSensor,
    kFlowSensor,
    kTemperatureSensor,
};

/// Application hooks used by PumpConfigurationAndControlCluster when it computes
/// EffectiveOperationMode and EffectiveControlMode.
///
/// Methods are called from the Matter thread with the stack lock held.
class Delegate
{
public:
    virtual ~Delegate() = default;

    /// Returns the remote sensor currently controlling the pump on `endpointId`.
    ///
    /// While OperationMode is Normal, a remote sensor overrides ControlMode:
    /// a flow sensor selects ConstantFlow, a pressure sensor selects ConstantPressure
    /// and a temperature sensor selects ConstantTemperature.
    virtual RemoteSensorType GetRemoteSensorType(EndpointId endpointId) = 0;

    /// Called after the effective modes are updated while OperationMode is Minimum or Maximum.
    ///
    /// The application is expected to run the pump at its minimum or maximum speed
    /// (e.g. by updating the Level Control cluster on the same endpoint).
    virtual void OnOperationModeMinMax(EndpointId endpointId, OperationModeEnum operationMode) = 0;
};

} // namespace chip::app::Clusters::PumpConfigurationAndControl
