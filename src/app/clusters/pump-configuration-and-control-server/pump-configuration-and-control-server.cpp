/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/util/af.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PumpConfigurationAndControl;

namespace {

// Enum for RemoteSensorType
enum class RemoteSensorType : uint8_t
{
    kNoSensor          = 0x00,
    kPressureSensor    = 0x01,
    kFlowSensor        = 0x02,
    kTemperatureSensor = 0x03,
};

static RemoteSensorType detectRemoteSensorConnected(void)
{
    // TODO: Detect the sensor types attached to the pump control cluster
    // this could be pressure, flow or temperature sensors.

    return RemoteSensorType::kNoSensor;
}

static void updateAttributeLinks(EndpointId endpoint)
{
    PumpControlMode controlMode;
    PumpOperationMode operationMode;
    
    // Get the current control- and operation modes
    Attributes::ControlMode::Get(endpoint, &controlMode);
    Attributes::OperationMode::Get(endpoint, &operationMode);

    switch (operationMode)
    {
        case PumpOperationMode::kNormal:
        {
            // The pump runs in the control mode as per the type of the remote sensor
            // If the remote sensor is a Flow sensor the mode would be ConstantFlow
            // If the remote sensor is a Pressure sensor the mode would be ConstantPressure (not ProportionalPressure)
            // If the remote sensor is a Temperature sensor the mode would be ConstantTemperature

            // If a remote sensor is detected and the OperationMode is kNormal, then the pump is operating in the
            // control mode indicated by the repective remote senor type
            RemoteSensorType sensorType = detectRemoteSensorConnected();
            switch (sensorType)
            {
            case RemoteSensorType::kFlowSensor:
                Attributes::EffectiveControlMode::Set(endpoint, PumpControlMode::kConstantFlow);
                Attributes::PumpStatus::Set(endpoint, PumpStatus::kRemoteFlow);
                break;
            case RemoteSensorType::kPressureSensor:
                Attributes::EffectiveControlMode::Set(endpoint, PumpControlMode::kConstantPressure);
                Attributes::PumpStatus::Set(endpoint, PumpStatus::kRemotePressure);
                break;
            case RemoteSensorType::kTemperatureSensor:
                Attributes::EffectiveControlMode::Set(endpoint, PumpControlMode::kConstantTemperature);
                Attributes::PumpStatus::Set(endpoint, PumpStatus::kRemoteTemperature);
                break;
            case RemoteSensorType::kNoSensor:
                // The pump is controlled by a setpoint, as defined by
                // the ControlMode attribute. (N.B. The setpoint is an internal variable which MAY be
                // controlled between 0% and 100%, e.g., by means of the Level Control cluster)
                // The ControlMode can be any of the following:
                // ConstantSpeed, ConstantPressure, ProportionalPressure,
                // ConstantFlow, ConstantTemperature or Automatic
                Attributes::EffectiveControlMode::Set(endpoint, controlMode);
                break;
            }
            // Set the overall effective operation mode to Normal
            Attributes::EffectiveOperationMode::Set(endpoint, PumpOperationMode::kNormal);
        }
        break;

        // The pump is controlled by the OperationMode attribute.
        // Maximum, Minimum or Local

        case PumpOperationMode::kMaximum:
            Attributes::EffectiveOperationMode::Set(endpoint, PumpOperationMode::kMaximum);
            Attributes::EffectiveControlMode::Set(endpoint, PumpControlMode::kConstantSpeed);
            LevelControl::Attributes::CurrentLevel::Set(endpoint, 200);
            break;

        case PumpOperationMode::kMinimum:
            Attributes::EffectiveOperationMode::Set(endpoint, PumpOperationMode::kMinimum);
            Attributes::EffectiveControlMode::Set(endpoint, PumpControlMode::kConstantSpeed);
            LevelControl::Attributes::CurrentLevel::Set(endpoint, 1);
            break;

        case PumpOperationMode::kLocal:
            Attributes::EffectiveOperationMode::Set(endpoint, PumpOperationMode::kLocal);
            Attributes::EffectiveControlMode::Set(endpoint, controlMode);
            Attributes::PumpStatus::Set(endpoint, PumpStatus::kLocalOverride);
            break;
    }
}
} // namespace

void emberAfPumpConfigurationAndControlClusterServerInitCallback(EndpointId endpoint)
{
    emberAfDebugPrintln("Initialize PCC Server Cluster [EP:%d]", endpoint);
}

void MatterPumpConfigurationAndControlClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    emberAfDebugPrintln("PCC Server Cluster Attribute changed [EP:%d, ID:0x%x]", attributePath.mEndpointId,
                        (unsigned int) attributePath.mAttributeId);

    switch (attributePath.mAttributeId)
    {
    case Attributes::ControlMode::Id: {
        updateAttributeLinks(attributePath.mEndpointId);
    }
    break;
    case Attributes::OperationMode::Id: {
        updateAttributeLinks(attributePath.mEndpointId);
    }
    break;
    default:
        emberAfDebugPrintln("PCC Server: unhandled attribute ID");
    }
}

void MatterPumpConfigurationAndControlPluginServerInitCallback() {}
