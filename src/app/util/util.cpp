/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <app/util/util.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <app/util/ember-strings.h>
#include <app/util/generic-callbacks.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPEncoding.h>
#include <protocols/interaction_model/StatusCode.h>

// TODO: figure out a clear path for compile-time codegen
#include <app/PluginApplicationCallbacks.h>

using namespace chip;

using chip::Protocols::InteractionModel::Status;

// Calculates difference. See EmberAfDifferenceType for the maximum data size
// that this function will support.
EmberAfDifferenceType emberAfGetDifference(uint8_t * pData, EmberAfDifferenceType value, uint8_t dataSize)
{
    EmberAfDifferenceType value2 = 0, diff;
    uint8_t i;

    // only support data types up to 8 bytes
    if (dataSize > sizeof(EmberAfDifferenceType))
    {
        return 0;
    }

    // get the value
    for (i = 0; i < dataSize; i++)
    {
        value2 = value2 << 8;
#if (CHIP_CONFIG_BIG_ENDIAN_TARGET)
        value2 += pData[i];
#else  // BIGENDIAN
        value2 += pData[dataSize - i - 1];
#endif // BIGENDIAN
    }

    if (value > value2)
    {
        diff = value - value2;
    }
    else
    {
        diff = value2 - value;
    }

    return diff;
}

// ****************************************
// Initialize Clusters
// ****************************************
void emberAfInit()
{
    emberAfInitializeAttributes(kInvalidEndpointId);

    MATTER_PLUGINS_INIT

    emAfCallInits();
}

// Cluster init functions that don't have a cluster implementation to define
// them in.
void MatterBallastConfigurationPluginServerInitCallback() {}
void MatterBooleanStatePluginServerInitCallback() {}
void MatterElectricalMeasurementPluginServerInitCallback() {}
void MatterRelativeHumidityMeasurementPluginServerInitCallback() {}
void MatterIlluminanceMeasurementPluginServerInitCallback() {}
void MatterBinaryInputBasicPluginServerInitCallback() {}
void MatterPressureMeasurementPluginServerInitCallback() {}
void MatterTemperatureMeasurementPluginServerInitCallback() {}
void MatterFlowMeasurementPluginServerInitCallback() {}
void MatterOnOffSwitchConfigurationPluginServerInitCallback() {}
void MatterThermostatUserInterfaceConfigurationPluginServerInitCallback() {}
void MatterBridgedDeviceBasicInformationPluginServerInitCallback() {}
void MatterPowerConfigurationPluginServerInitCallback() {}
void MatterPowerProfilePluginServerInitCallback() {}
void MatterPulseWidthModulationPluginServerInitCallback() {}
void MatterAlarmsPluginServerInitCallback() {}
void MatterTimePluginServerInitCallback() {}
void MatterAclPluginServerInitCallback() {}
void MatterPollControlPluginServerInitCallback() {}
void MatterUnitLocalizationPluginServerInitCallback() {}
void MatterProxyValidPluginServerInitCallback() {}
void MatterProxyDiscoveryPluginServerInitCallback() {}
void MatterProxyConfigurationPluginServerInitCallback() {}
void MatterFanControlPluginServerInitCallback() {}
void MatterActivatedCarbonFilterMonitoringPluginServerInitCallback() {}
void MatterHepaFilterMonitoringPluginServerInitCallback() {}
void MatterAirQualityPluginServerInitCallback() {}
void MatterCarbonMonoxideConcentrationMeasurementPluginServerInitCallback() {}
void MatterCarbonDioxideConcentrationMeasurementPluginServerInitCallback() {}
void MatterFormaldehydeConcentrationMeasurementPluginServerInitCallback() {}
void MatterNitrogenDioxideConcentrationMeasurementPluginServerInitCallback() {}
void MatterOzoneConcentrationMeasurementPluginServerInitCallback() {}
void MatterPm10ConcentrationMeasurementPluginServerInitCallback() {}
void MatterPm1ConcentrationMeasurementPluginServerInitCallback() {}
void MatterPm25ConcentrationMeasurementPluginServerInitCallback() {}
void MatterRadonConcentrationMeasurementPluginServerInitCallback() {}
void MatterTotalVolatileOrganicCompoundsConcentrationMeasurementPluginServerInitCallback() {}
void MatterRvcRunModePluginServerInitCallback() {}
void MatterRvcCleanModePluginServerInitCallback() {}
void MatterDishwasherModePluginServerInitCallback() {}
void MatterLaundryWasherModePluginServerInitCallback() {}
void MatterRefrigeratorAndTemperatureControlledCabinetModePluginServerInitCallback() {}
void MatterOperationalStatePluginServerInitCallback() {}
void MatterRvcOperationalStatePluginServerInitCallback() {}
void MatterOvenModePluginServerInitCallback() {}
void MatterOvenCavityOperationalStatePluginServerInitCallback() {}
void MatterDishwasherAlarmPluginServerInitCallback() {}
void MatterMicrowaveOvenModePluginServerInitCallback() {}
void MatterDeviceEnergyManagementModePluginServerInitCallback() {}
void MatterEnergyEvseModePluginServerInitCallback() {}
void MatterPowerTopologyPluginServerInitCallback() {}
void MatterElectricalEnergyMeasurementPluginServerInitCallback() {}
void MatterElectricalPowerMeasurementPluginServerInitCallback() {}

bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    return (emberAfGetServerAttributeIndexByAttributeId(endpoint, clusterId, attributeId) != UINT16_MAX);
}

bool emberAfIsKnownVolatileAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    const EmberAfAttributeMetadata * metadata = emberAfLocateAttributeMetadata(endpoint, clusterId, attributeId);

    if (metadata == nullptr)
    {
        return false;
    }

    return !metadata->IsAutomaticallyPersisted() && !metadata->IsExternal();
}
