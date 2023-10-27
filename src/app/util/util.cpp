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

#include "app/util/common.h"
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/print-cluster.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <app/util/generic-callbacks.h>
#include <lib/core/CHIPConfig.h>

// TODO: figure out a clear path for compile-time codegen
#include <app/PluginApplicationCallbacks.h>

#ifdef EMBER_AF_PLUGIN_GROUPS_SERVER
#include <app/clusters/groups-server/groups-server.h>
#endif // EMBER_AF_PLUGIN_GROUPS_SERVER

using namespace chip;

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Globals

const EmberAfClusterName zclClusterNames[] = {
    CLUSTER_IDS_TO_NAMES            // defined in print-cluster.h
    { kInvalidClusterId, nullptr }, // terminator
};

#ifdef EMBER_AF_GENERATED_PLUGIN_TICK_FUNCTION_DECLARATIONS
EMBER_AF_GENERATED_PLUGIN_TICK_FUNCTION_DECLARATIONS
#endif

//------------------------------------------------------------------------------

// Is the device identifying?
bool emberAfIsDeviceIdentifying(EndpointId endpoint)
{
#ifdef ZCL_USING_IDENTIFY_CLUSTER_SERVER
    uint16_t identifyTime;
    EmberAfStatus status = app::Clusters::Identify::Attributes::IdentifyTime::Get(endpoint, &identifyTime);
    return (status == EMBER_ZCL_STATUS_SUCCESS && 0 < identifyTime);
#else
    return false;
#endif
}

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
    emberAfInitializeAttributes(EMBER_BROADCAST_ENDPOINT);

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
void MatterDishwasherAlarmPluginServerInitCallback() {}
// ****************************************
// Print out information about each cluster
// ****************************************

uint16_t emberAfFindClusterNameIndex(ClusterId cluster)
{
    static_assert(sizeof(ClusterId) == 4, "May need to adjust our index type or somehow define it in terms of cluster id type");
    uint16_t index = 0;
    while (zclClusterNames[index].id != kInvalidClusterId)
    {
        if (zclClusterNames[index].id == cluster)
        {
            return index;
        }
        index++;
    }
    return 0xFFFF;
}

void emberAfCopyString(uint8_t * dest, const uint8_t * src, size_t size)
{
    if (src == nullptr)
    {
        dest[0] = 0; // Zero out the length of string
    }
    else if (src[0] == 0xFF)
    {
        dest[0] = src[0];
    }
    else
    {
        uint8_t length = emberAfStringLength(src);
        if (size < length)
        {
            // Since we have checked that size < length, size must be able to fit into the type of length.
            length = static_cast<decltype(length)>(size);
        }
        memmove(dest + 1, src + 1, length);
        dest[0] = length;
    }
}

void emberAfCopyLongString(uint8_t * dest, const uint8_t * src, size_t size)
{
    if (src == nullptr)
    {
        dest[0] = dest[1] = 0; // Zero out the length of string
    }
    else if ((src[0] == 0xFF) && (src[1] == 0xFF))
    {
        dest[0] = 0xFF;
        dest[1] = 0xFF;
    }
    else
    {
        uint16_t length = emberAfLongStringLength(src);
        if (size < length)
        {
            // Since we have checked that size < length, size must be able to fit into the type of length.
            length = static_cast<decltype(length)>(size);
        }
        memmove(dest + 2, src + 2, length);
        dest[0] = EMBER_LOW_BYTE(length);
        dest[1] = EMBER_HIGH_BYTE(length);
    }
}

#if (CHIP_CONFIG_BIG_ENDIAN_TARGET)
#define EM_BIG_ENDIAN true
#else
#define EM_BIG_ENDIAN false
#endif

// You can pass in val1 as NULL, which will assume that it is
// pointing to an array of all zeroes. This is used so that
// default value of NULL is treated as all zeroes.
int8_t emberAfCompareValues(const uint8_t * val1, const uint8_t * val2, uint16_t len, bool signedNumber)
{
    if (len == 0)
    {
        // no length means nothing to compare.  Shouldn't even happen, since len is sizeof(some-integer-type).
        return 0;
    }

    if (signedNumber)
    { // signed number comparison
        if (len <= 4)
        { // only number with 32-bits or less is supported
            int32_t accum1 = 0x0;
            int32_t accum2 = 0x0;
            int32_t all1s  = -1;

            for (uint16_t i = 0; i < len; i++)
            {
                uint8_t j = (val1 == nullptr ? 0 : (EM_BIG_ENDIAN ? val1[i] : val1[(len - 1) - i]));
                accum1 |= j << (8 * (len - 1 - i));

                uint8_t k = (EM_BIG_ENDIAN ? val2[i] : val2[(len - 1) - i]);
                accum2 |= k << (8 * (len - 1 - i));
            }

            // sign extending, no need for 32-bits numbers
            if (len < 4)
            {
                if ((accum1 & (1 << (8 * len - 1))) != 0)
                { // check sign
                    accum1 |= all1s - ((1 << (len * 8)) - 1);
                }
                if ((accum2 & (1 << (8 * len - 1))) != 0)
                { // check sign
                    accum2 |= all1s - ((1 << (len * 8)) - 1);
                }
            }

            if (accum1 > accum2)
            {
                return 1;
            }
            if (accum1 < accum2)
            {
                return -1;
            }

            return 0;
        }

        // not supported
        return 0;
    }

    // regular unsigned number comparison
    for (uint16_t i = 0; i < len; i++)
    {
        uint8_t j = (val1 == nullptr ? 0 : (EM_BIG_ENDIAN ? val1[i] : val1[(len - 1) - i]));
        uint8_t k = (EM_BIG_ENDIAN ? val2[i] : val2[(len - 1) - i]);

        if (j > k)
        {
            return 1;
        }
        if (k > j)
        {
            return -1;
        }
    }
    return 0;
}

// Zigbee spec says types between signed 8 bit and signed 64 bit
bool emberAfIsTypeSigned(EmberAfAttributeType dataType)
{
    return (dataType >= ZCL_INT8S_ATTRIBUTE_TYPE && dataType <= ZCL_INT64S_ATTRIBUTE_TYPE);
}

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
