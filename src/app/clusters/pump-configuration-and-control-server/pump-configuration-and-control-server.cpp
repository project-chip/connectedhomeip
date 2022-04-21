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
using namespace chip::app::Clusters::PumpConfigurationAndControl;
using namespace chip::app::Clusters::PumpConfigurationAndControl::Attributes;

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

static PumpControlMode determineEffectiveControlMode(PumpControlMode controlMode)
{
    PumpControlMode effectiveControlMode = controlMode;

    // If a remote sensor is detected and the OperationMode is kNormal, then the pump is operating in the
    // control mode indicated by the repective remote senor type
    RemoteSensorType sensorType = detectRemoteSensorConnected();
    switch (sensorType)
    {
    case RemoteSensorType::kNoSensor:
        // Set to current ControlMode;
        effectiveControlMode = controlMode;
        break;
    case RemoteSensorType::kFlowSensor:
        effectiveControlMode = PumpControlMode::kConstantFlow;
        break;
    case RemoteSensorType::kPressureSensor:
        effectiveControlMode = PumpControlMode::kConstantPressure;
        break;
    case RemoteSensorType::kTemperatureSensor:
        effectiveControlMode = PumpControlMode::kConstantTemperature;
        break;
    }

    return effectiveControlMode;
}

static PumpOperationMode determineEffectiveOperationMode(PumpOperationMode operationMode)
{
    PumpOperationMode effectiveOperationMode = operationMode;

    if (operationMode == PumpOperationMode::kNormal)
    {
        RemoteSensorType sensorType;
        sensorType = detectRemoteSensorConnected();
        if (sensorType != RemoteSensorType::kNoSensor)
        {
            // The pump runs in the control mode as per the type of the remote sensor
            // If the remote sensor is a Flow sensor the mode would be ConstantFlow
            // If the remote sensor is a Pressure sensor the mode would be ConstantPressure (not ProportionalPressure)
            // If the remote sensor is a Temperature sensor the mode would be ConstantTemperature
            // ARE NOT IMPLEMENTED!
        }
        else
        {
            // The pump is controlled by a setpoint, as defined by
            // the ControlMode attribute. (N.B. The setpoint is an internal variable which MAY be
            // controlled between 0% and 100%, e.g., by means of the Level Control cluster)
            // The ControlMode can be any of the following:
            // ConstantSpeed, ConstantPressure, ProportionalPressure,
            // ConstantFlow, ConstantTemperature or Automatic
        }
    }
    else
    {
        // The pump is controlled by the OperationMode attribute.
        // Maximum, Minimum or Local
    }

    return effectiveOperationMode;
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
    case ControlMode::Id: {
        PumpControlMode controlMode;
        ControlMode::Get(attributePath.mEndpointId, &controlMode);
        EffectiveControlMode::Set(attributePath.mEndpointId, determineEffectiveControlMode(controlMode));
    }
    break;
    case OperationMode::Id: {
        PumpOperationMode operationMode;
        OperationMode::Get(attributePath.mEndpointId, &operationMode);
        EffectiveOperationMode::Set(attributePath.mEndpointId, determineEffectiveOperationMode(operationMode));
    }
    break;
    default:
        emberAfDebugPrintln("PCC Server: unhandled attribute ID");
    }
}

void MatterPumpConfigurationAndControlPluginServerInitCallback() {}
