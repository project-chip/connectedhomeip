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
#include <app/util/util.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <app/util/ember-compatibility-functions.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PumpConfigurationAndControl;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace PumpConfigurationAndControl {

// Enum for RemoteSensorType
enum class RemoteSensorType : uint8_t
{
    kNoSensor          = 0x00,
    kPressureSensor    = 0x01,
    kFlowSensor        = 0x02,
    kTemperatureSensor = 0x03,
};

static RemoteSensorType detectRemoteSensorConnected()
{
    // TODO: Detect the sensor types attached to the pump control cluster
    // this could be pressure, flow or temperature sensors.

    return RemoteSensorType::kNoSensor;
}

static void setEffectiveModes(EndpointId endpoint)
{
    ControlModeEnum controlMode;
    OperationModeEnum operationMode;
    BitMask<PumpStatusBitmap> pumpStatus;
    bool isControlModeAvailable = true;
    bool isPumpStatusAvailable  = true;

    isControlModeAvailable = emberAfContainsAttribute(endpoint, PumpConfigurationAndControl::Id, Attributes::ControlMode::Id);
    isPumpStatusAvailable  = emberAfContainsAttribute(endpoint, PumpConfigurationAndControl::Id, Attributes::PumpStatus::Id);

    // Get the current control- and operation modes
    Attributes::OperationMode::Get(endpoint, &operationMode);

    if (isControlModeAvailable)
    {
        Attributes::ControlMode::Get(endpoint, &controlMode);
    }
    else
    {
        // If controlMode attribute is not available, then use the default value
        // of the effectiveControlMode attribute as the effectiveControlMode
        // if this is not suitable, the application should override this value in
        // the post attribute change callback for the operation mode attribute
        const EmberAfAttributeMetadata * effectiveControlModeMetaData;
        effectiveControlModeMetaData = GetAttributeMetadata(
            app::ConcreteAttributePath(endpoint, PumpConfigurationAndControl::Id, Attributes::EffectiveControlMode::Id));
        controlMode = static_cast<ControlModeEnum>(effectiveControlModeMetaData->defaultValue.defaultValue);
    }

    if (isPumpStatusAvailable)
    {
        Attributes::PumpStatus::Get(endpoint, &pumpStatus);
    }

    switch (operationMode)
    {
    case OperationModeEnum::kNormal: {
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
            Attributes::EffectiveControlMode::Set(endpoint, ControlModeEnum::kConstantFlow);
            if (isPumpStatusAvailable)
            {
                pumpStatus.Set(PumpStatusBitmap::kRemoteFlow);
                pumpStatus.Clear(PumpStatusBitmap::kRemotePressure);
                pumpStatus.Clear(PumpStatusBitmap::kRemoteTemperature);
            }
            break;
        case RemoteSensorType::kPressureSensor:
            Attributes::EffectiveControlMode::Set(endpoint, ControlModeEnum::kConstantPressure);
            if (isPumpStatusAvailable)
            {
                pumpStatus.Clear(PumpStatusBitmap::kRemoteFlow);
                pumpStatus.Set(PumpStatusBitmap::kRemotePressure);
                pumpStatus.Clear(PumpStatusBitmap::kRemoteTemperature);
            }
            break;
        case RemoteSensorType::kTemperatureSensor:
            Attributes::EffectiveControlMode::Set(endpoint, ControlModeEnum::kConstantTemperature);
            if (isPumpStatusAvailable)
            {
                pumpStatus.Clear(PumpStatusBitmap::kRemoteFlow);
                pumpStatus.Clear(PumpStatusBitmap::kRemotePressure);
                pumpStatus.Set(PumpStatusBitmap::kRemoteTemperature);
            }
            break;
        case RemoteSensorType::kNoSensor:
            // The pump is controlled by a setpoint, as defined by
            // the ControlMode attribute. (N.B. The setpoint is an internal variable which MAY be
            // controlled between 0% and 100%, e.g., by means of the Level Control cluster)
            // The ControlMode can be any of the following:
            // ConstantSpeed, ConstantPressure, ProportionalPressure,
            // ConstantFlow, ConstantTemperature or Automatic. The actual ControlMode
            // which would be the EffectiveControlMode is dependant on the actual
            // physical pump application running "on-top" of this cluster server.
            Attributes::EffectiveControlMode::Set(endpoint, controlMode);
            if (isPumpStatusAvailable)
            {
                pumpStatus.Clear(PumpStatusBitmap::kRemoteFlow);
                pumpStatus.Clear(PumpStatusBitmap::kRemotePressure);
                pumpStatus.Clear(PumpStatusBitmap::kRemoteTemperature);
            }
            break;
        }
        // Set the overall effective operation mode to Normal
        Attributes::EffectiveOperationMode::Set(endpoint, OperationModeEnum::kNormal);
    }
    break;

        // The pump is controlled by the OperationMode attribute.
        // Maximum, Minimum or Local

    case OperationModeEnum::kMaximum: {
#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL
        uint8_t maxLevel;
#endif
        Attributes::EffectiveOperationMode::Set(endpoint, OperationModeEnum::kMaximum);
        Attributes::EffectiveControlMode::Set(endpoint, ControlModeEnum::kConstantSpeed);
#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL
        LevelControl::Attributes::MaxLevel::Get(endpoint, &maxLevel);
        LevelControl::Attributes::CurrentLevel::Set(endpoint, maxLevel);
#endif
        if (isPumpStatusAvailable)
        {
            pumpStatus.Clear(PumpStatusBitmap::kRemoteFlow);
            pumpStatus.Clear(PumpStatusBitmap::kRemotePressure);
            pumpStatus.Clear(PumpStatusBitmap::kRemoteTemperature);
        }
    }
    break;

    case OperationModeEnum::kMinimum: {
#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL
        uint8_t minLevel;
#endif
        Attributes::EffectiveOperationMode::Set(endpoint, OperationModeEnum::kMinimum);
        Attributes::EffectiveControlMode::Set(endpoint, ControlModeEnum::kConstantSpeed);
#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL
        LevelControl::Attributes::MinLevel::Get(endpoint, &minLevel);
        if (minLevel == 0)
        {
            // Bump the minimum level to 1, since the value of 0 means stop
            minLevel = 1;
        }
        LevelControl::Attributes::CurrentLevel::Set(endpoint, minLevel);
#endif
        if (isPumpStatusAvailable)
        {
            pumpStatus.Clear(PumpStatusBitmap::kRemoteFlow);
            pumpStatus.Clear(PumpStatusBitmap::kRemotePressure);
            pumpStatus.Clear(PumpStatusBitmap::kRemoteTemperature);
        }
    }
    break;

    case OperationModeEnum::kLocal: {
        // If the Application sets the OperatioMode to kLocal the application "owns" the EffectiveControlMode, which
        // it also does if the external entity sets the OperationMode to kLocal. So in any case the application
        // must set the EffectiveControlMode to something which applies to the current ControlMode in the application.
        // So to keeps things short: the application layer owns the EffetiveControlMode when OperationMode is kLocal.
        Attributes::EffectiveOperationMode::Set(endpoint, OperationModeEnum::kLocal);
        // Set the current ControlMode for now. Perhaps the application will set the EffectiveControlMode to something else.
        Attributes::EffectiveControlMode::Set(endpoint, controlMode);
        // Clear out the remote sensors from the PumpStatus flags.
        if (isPumpStatusAvailable)
        {
            pumpStatus.Clear(PumpStatusBitmap::kRemoteFlow);
            pumpStatus.Clear(PumpStatusBitmap::kRemotePressure);
            pumpStatus.Clear(PumpStatusBitmap::kRemoteTemperature);
        }
    }
    break;

    case OperationModeEnum::kUnknownEnumValue: {
        // Not expected; see check in MatterPumpConfigurationAndControlClusterServerPreAttributeChangedCallback.
        break;
    }
    }

    if (isPumpStatusAvailable)
    {
        Attributes::PumpStatus::Set(endpoint, pumpStatus);
    }
}

bool HasFeature(EndpointId endpoint, Feature feature)
{
    bool hasFeature;
    uint32_t featureMap;
    hasFeature = (Attributes::FeatureMap::Get(endpoint, &featureMap) == Status::Success);

    return hasFeature ? ((featureMap & to_underlying(feature)) != 0) : false;
}

} // namespace PumpConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip

// SDK Callbacks

void emberAfPumpConfigurationAndControlClusterServerInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Initialize PCC Server Cluster [EP:%d]", endpoint);
}

chip::Protocols::InteractionModel::Status MatterPumpConfigurationAndControlClusterServerPreAttributeChangedCallback(
    const chip::app::ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    ChipLogProgress(Zcl, "PCC Server Cluster Attribute Pre-changed [EP:%d, ID:0x%x]", attributePath.mEndpointId,
                    (unsigned int) attributePath.mAttributeId);

    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::Success;

    switch (attributePath.mAttributeId)
    {
    case Attributes::ControlMode::Id: {
        ControlModeEnum controlMode;
        NumericAttributeTraits<ControlModeEnum>::StorageType tmp;
        memcpy(&tmp, value, size);
        controlMode = NumericAttributeTraits<ControlModeEnum>::StorageToWorking(tmp);
        switch (controlMode)
        {
        case ControlModeEnum::kConstantFlow:
            if (!HasFeature(attributePath.mEndpointId, Feature::kConstantFlow))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case ControlModeEnum::kConstantPressure:
            if (!HasFeature(attributePath.mEndpointId, Feature::kConstantPressure))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case ControlModeEnum::kConstantSpeed:
            if (!HasFeature(attributePath.mEndpointId, Feature::kConstantSpeed))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case ControlModeEnum::kConstantTemperature:
            if (!HasFeature(attributePath.mEndpointId, Feature::kConstantTemperature))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case ControlModeEnum::kProportionalPressure:
            if (!HasFeature(attributePath.mEndpointId, Feature::kCompensatedPressure))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case ControlModeEnum::kAutomatic:
            if (!HasFeature(attributePath.mEndpointId, Feature::kAutomatic))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case ControlModeEnum::kUnknownEnumValue:
            status = Protocols::InteractionModel::Status::ConstraintError;
            break;
        }
    }
    break;

    case Attributes::OperationMode::Id:
        OperationModeEnum operationMode;
        NumericAttributeTraits<OperationModeEnum>::StorageType tmp;
        memcpy(&tmp, value, size);
        operationMode = NumericAttributeTraits<OperationModeEnum>::StorageToWorking(tmp);

        switch (operationMode)
        {
        case OperationModeEnum::kMinimum:
            if (!HasFeature(attributePath.mEndpointId, Feature::kConstantSpeed))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case OperationModeEnum::kMaximum:
            if (!HasFeature(attributePath.mEndpointId, Feature::kConstantSpeed))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case OperationModeEnum::kLocal:
            if (!HasFeature(attributePath.mEndpointId, Feature::kLocalOperation))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case OperationModeEnum::kNormal:
            status = Protocols::InteractionModel::Status::Success;
            break;
        case OperationModeEnum::kUnknownEnumValue:
            status = Protocols::InteractionModel::Status::ConstraintError;
            break;
        }

        break;
    default:
        status = Protocols::InteractionModel::Status::Success;
    }

    return status;
}

void MatterPumpConfigurationAndControlClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    ChipLogProgress(Zcl, "PCC Server Cluster Attribute changed [EP:%d, ID:0x%x]", attributePath.mEndpointId,
                    (unsigned int) attributePath.mAttributeId);

    switch (attributePath.mAttributeId)
    {
    case Attributes::ControlMode::Id:
    case Attributes::OperationMode::Id:
        setEffectiveModes(attributePath.mEndpointId);
        break;
    default:
        ChipLogProgress(Zcl, "PCC Server: unhandled attribute ID");
    }
}

void MatterPumpConfigurationAndControlPluginServerInitCallback() {}
