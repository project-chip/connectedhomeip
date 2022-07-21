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
#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PumpConfigurationAndControl;

namespace {

class PumpConfigurationAndControlAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Pump Configuration And Control cluster on all endpoints.
    PumpConfigurationAndControlAttrAccess() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), PumpConfigurationAndControl::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
};

PumpConfigurationAndControlAttrAccess gAttrAccess;

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
    BitMask<PumpStatus> pumpStatus;
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
        controlMode = static_cast<PumpControlMode>(effectiveControlModeMetaData->defaultValue.defaultValue);
    }

    if (isPumpStatusAvailable)
    {
        Attributes::PumpStatus::Get(endpoint, &pumpStatus);
    }

    switch (operationMode)
    {
    case PumpOperationMode::kNormal: {
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
            if (isPumpStatusAvailable)
            {
                pumpStatus.Set(PumpStatus::kRemoteFlow);
                pumpStatus.Clear(PumpStatus::kRemotePressure);
                pumpStatus.Clear(PumpStatus::kRemoteTemperature);
            }
            break;
        case RemoteSensorType::kPressureSensor:
            Attributes::EffectiveControlMode::Set(endpoint, PumpControlMode::kConstantPressure);
            if (isPumpStatusAvailable)
            {
                pumpStatus.Clear(PumpStatus::kRemoteFlow);
                pumpStatus.Set(PumpStatus::kRemotePressure);
                pumpStatus.Clear(PumpStatus::kRemoteTemperature);
            }
            break;
        case RemoteSensorType::kTemperatureSensor:
            Attributes::EffectiveControlMode::Set(endpoint, PumpControlMode::kConstantTemperature);
            if (isPumpStatusAvailable)
            {
                pumpStatus.Clear(PumpStatus::kRemoteFlow);
                pumpStatus.Clear(PumpStatus::kRemotePressure);
                pumpStatus.Set(PumpStatus::kRemoteTemperature);
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
                pumpStatus.Clear(PumpStatus::kRemoteFlow);
                pumpStatus.Clear(PumpStatus::kRemotePressure);
                pumpStatus.Clear(PumpStatus::kRemoteTemperature);
            }
            break;
        }
        // Set the overall effective operation mode to Normal
        Attributes::EffectiveOperationMode::Set(endpoint, PumpOperationMode::kNormal);
    }
    break;

        // The pump is controlled by the OperationMode attribute.
        // Maximum, Minimum or Local

    case PumpOperationMode::kMaximum: {
#ifdef EMBER_AF_PLUGIN_LEVEL_CONTROL
        uint8_t maxLevel;
#endif
        Attributes::EffectiveOperationMode::Set(endpoint, PumpOperationMode::kMaximum);
        Attributes::EffectiveControlMode::Set(endpoint, PumpControlMode::kConstantSpeed);
#ifdef EMBER_AF_PLUGIN_LEVEL_CONTROL
        LevelControl::Attributes::MaxLevel::Get(endpoint, &maxLevel);
        LevelControl::Attributes::CurrentLevel::Set(endpoint, maxLevel);
#endif
        if (isPumpStatusAvailable)
        {
            pumpStatus.Clear(PumpStatus::kRemoteFlow);
            pumpStatus.Clear(PumpStatus::kRemotePressure);
            pumpStatus.Clear(PumpStatus::kRemoteTemperature);
        }
    }
    break;

    case PumpOperationMode::kMinimum: {
#ifdef EMBER_AF_PLUGIN_LEVEL_CONTROL
        uint8_t minLevel;
#endif
        Attributes::EffectiveOperationMode::Set(endpoint, PumpOperationMode::kMinimum);
        Attributes::EffectiveControlMode::Set(endpoint, PumpControlMode::kConstantSpeed);
#ifdef EMBER_AF_PLUGIN_LEVEL_CONTROL
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
            pumpStatus.Clear(PumpStatus::kRemoteFlow);
            pumpStatus.Clear(PumpStatus::kRemotePressure);
            pumpStatus.Clear(PumpStatus::kRemoteTemperature);
        }
    }
    break;

    case PumpOperationMode::kLocal: {
        // If the Application sets the OperatioMode to kLocal the application "owns" the EffectiveControlMode, which
        // it also does if the external entity sets the OperationMode to kLocal. So in any case the application
        // must set the EffectiveControlMode to something which applies to the current ControlMode in the application.
        // So to keeps things short: the application layer owns the EffetiveControlMode when OperationMode is kLocal.
        Attributes::EffectiveOperationMode::Set(endpoint, PumpOperationMode::kLocal);
        // Set the current ControlMode for now. Perhaps the application will set the EffectiveControlMode to something else.
        Attributes::EffectiveControlMode::Set(endpoint, controlMode);
        // Clear out the remote sensors from the PumpStatus flags.
        if (isPumpStatusAvailable)
        {
            pumpStatus.Clear(PumpStatus::kRemoteFlow);
            pumpStatus.Clear(PumpStatus::kRemotePressure);
            pumpStatus.Clear(PumpStatus::kRemoteTemperature);
        }
    }
    break;

    case PumpOperationMode::kUnknownEnumValue: {
        // Not expected; see check in MatterPumpConfigurationAndControlClusterServerPreAttributeChangedCallback.
        break;
    }
    }

    if (isPumpStatusAvailable)
    {
        Attributes::PumpStatus::Set(endpoint, pumpStatus);
    }
}

CHIP_ERROR PumpConfigurationAndControlAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    emberAfDebugPrintln("Reading from PCC");

    VerifyOrDie(aPath.mClusterId == PumpConfigurationAndControl::Id);

    switch (aPath.mAttributeId)
    {
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    emberAfDebugPrintln("Writing to PCC");

    VerifyOrDie(aPath.mClusterId == PumpConfigurationAndControl::Id);

    switch (aPath.mAttributeId)
    {
    default:
        break;
    }
    return CHIP_NO_ERROR;
}
} // namespace

// SDK Callbacks

template <typename T1, typename T2>
bool IsFeatureSupported(EndpointId endpoint, EmberAfStatus (*getFn1)(chip::EndpointId endpointId, T1 & value),
                        EmberAfStatus (*getFn2)(chip::EndpointId endpointId, T2 & value))
{
    EmberAfStatus status;

    T1 value1;
    T2 value2;

    status = getFn1(endpoint, value1);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        if (!value1.IsNull())
        {
            status = getFn2(endpoint, value2);
            if (status == EMBER_ZCL_STATUS_SUCCESS)
            {
                if (!value2.IsNull())
                {
                    return true;
                }
            }
        }
    }

    return false;
}

template <typename T1, typename T2>
const char * FeatureSupportedDebugString(EndpointId endpoint, EmberAfStatus (*getFn1)(chip::EndpointId endpointId, T1 & value),
                                         EmberAfStatus (*getFn2)(chip::EndpointId endpointId, T2 & value))
{
    return IsFeatureSupported(endpoint, getFn1, getFn2) ? "Supported" : "Not Supported";
}

void emberAfPumpConfigurationAndControlClusterServerInitCallback(EndpointId endpoint)
{
    emberAfDebugPrintln("Initialize PCC Server Cluster [EP:%d]", endpoint);

    // Determine the internal feature set of the pump, depending on the pump
    // specific attributes available, and their values. This is a temporary
    // implementation to get a kind of "pseudo-FeatureMap" until we get a real
    // FeatureMap implementation in the PCC cluster. If an attribute is
    // present/available, then there is a possibility for the associated
    // feature being present as well. But we will have to distinguis between
    // the attributes being available and null also. At this point (init)
    // we can only examine the list of attributes available, and then detect
    // if they each have a NonNull value. Later on, when the pump application
    // has finished its init process, it might setup these attributevalues
    // to something NonNull, and then we must re-calcualte the feature set.

    emberAfDebugPrintln(
        "Constant Pressure %s",
        FeatureSupportedDebugString(endpoint, Attributes::MinConstPressure::Get, Attributes::MaxConstPressure::Get));
    emberAfDebugPrintln("PCC Server: Constant Proportional Pressure %s",
                        FeatureSupportedDebugString(endpoint, Attributes::MinCompPressure::Get, Attributes::MaxCompPressure::Get));
    emberAfDebugPrintln("PCC Server: Constant Flow %s",
                        FeatureSupportedDebugString(endpoint, Attributes::MinConstFlow::Get, Attributes::MaxConstFlow::Get));
    emberAfDebugPrintln("PCC Server: Constant Temperature %s",
                        FeatureSupportedDebugString(endpoint, Attributes::MinConstTemp::Get, Attributes::MaxConstTemp::Get));
    emberAfDebugPrintln("PCC Server: Constant Speed %s",
                        FeatureSupportedDebugString(endpoint, Attributes::MinConstSpeed::Get, Attributes::MaxConstSpeed::Get));

    if (!emberAfContainsAttribute(endpoint, PumpConfigurationAndControl::Id, Attributes::ControlMode::Id))
    {
        emberAfDebugPrintln("PCC Server: ControlMode attribute not available");
    }

    if (!emberAfContainsAttribute(endpoint, PumpConfigurationAndControl::Id, Attributes::PumpStatus::Id))
    {
        emberAfDebugPrintln("PCC Server: PumpStatus attribute not available");
    }
}

chip::Protocols::InteractionModel::Status MatterPumpConfigurationAndControlClusterServerPreAttributeChangedCallback(
    const chip::app::ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    emberAfDebugPrintln("PCC Server Cluster Attribute Pre-changed [EP:%d, ID:0x%x]", attributePath.mEndpointId,
                        (unsigned int) attributePath.mAttributeId);

    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::Success;

    switch (attributePath.mAttributeId)
    {
    case Attributes::ControlMode::Id: {
        PumpControlMode controlMode;
        NumericAttributeTraits<PumpControlMode>::StorageType tmp;
        memcpy(&tmp, value, size);
        controlMode = NumericAttributeTraits<PumpControlMode>::StorageToWorking(tmp);
        switch (controlMode)
        {
        case PumpControlMode::kConstantFlow:
            if (!IsFeatureSupported(attributePath.mEndpointId, Attributes::MinConstFlow::Get, Attributes::MaxConstFlow::Get))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case PumpControlMode::kConstantPressure:
            if (!IsFeatureSupported(attributePath.mEndpointId, Attributes::MinConstPressure::Get,
                                    Attributes::MaxConstPressure::Get))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case PumpControlMode::kConstantSpeed:
            if (!IsFeatureSupported(attributePath.mEndpointId, Attributes::MinConstSpeed::Get, Attributes::MaxConstSpeed::Get))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case PumpControlMode::kConstantTemperature:
            if (!IsFeatureSupported(attributePath.mEndpointId, Attributes::MinConstTemp::Get, Attributes::MaxConstTemp::Get))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case PumpControlMode::kProportionalPressure:
            if (!IsFeatureSupported(attributePath.mEndpointId, Attributes::MinCompPressure::Get, Attributes::MaxCompPressure::Get))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case PumpControlMode::kAutomatic:
            status = Protocols::InteractionModel::Status::Success;
            break;
        case PumpControlMode::kUnknownEnumValue:
            status = Protocols::InteractionModel::Status::ConstraintError;
            break;
        }
    }
    break;

    case Attributes::OperationMode::Id:
        PumpOperationMode operationMode;
        NumericAttributeTraits<PumpOperationMode>::StorageType tmp;
        memcpy(&tmp, value, size);
        operationMode = NumericAttributeTraits<PumpOperationMode>::StorageToWorking(tmp);

        switch (operationMode)
        {
        case PumpOperationMode::kMinimum:
            if (!IsFeatureSupported(attributePath.mEndpointId, Attributes::MinConstSpeed::Get, Attributes::MaxConstSpeed::Get))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case PumpOperationMode::kMaximum:
            if (!IsFeatureSupported(attributePath.mEndpointId, Attributes::MinConstSpeed::Get, Attributes::MaxConstSpeed::Get))
            {
                status = Protocols::InteractionModel::Status::ConstraintError;
            }
            break;
        case PumpOperationMode::kLocal:
        case PumpOperationMode::kNormal:
            status = Protocols::InteractionModel::Status::Success;
            break;
        case PumpOperationMode::kUnknownEnumValue:
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

void MatterPumpConfigurationAndControlPluginServerInitCallback(void)
{
    emberAfDebugPrintln("Initialize PCC Plugin Server Cluster.");

    registerAttributeAccessOverride(&gAttrAccess);
}
