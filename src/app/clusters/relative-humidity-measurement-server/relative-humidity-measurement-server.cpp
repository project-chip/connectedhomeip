/*
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

#include "relative-humidity-measurement-server.h"

#include <app/util/af.h>

#include "gen/att-storage.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

#include <support/logging/CHIPLogging.h>

#ifndef emberAfRelativeHumidityMeasurementClusterPrintln
#define emberAfRelativeHumidityMeasurementClusterPrintln(...) ChipLogProgress(Zcl, __VA_ARGS__);
#endif

EmberAfStatus emberAfRelativeHumidityMeasurementClusterGetMeasuredValue(chip::EndpointId endpoint, uint16_t * measuredValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                      ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) measuredValue,
                                      sizeof(*measuredValue));
}

EmberAfStatus emberAfRelativeHumidityMeasurementClusterGetMinMeasuredValue(chip::EndpointId endpoint, uint16_t * minMeasuredValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                      ZCL_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) minMeasuredValue,
                                      sizeof(*minMeasuredValue));
}

EmberAfStatus emberAfRelativeHumidityMeasurementClusterGetMaxMeasuredValue(chip::EndpointId endpoint, uint16_t * maxMeasuredValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                      ZCL_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) maxMeasuredValue,
                                      sizeof(*maxMeasuredValue));
}

void emberAfRelativeHumidityMeasurementClusterServerInitCallback(chip::EndpointId endpoint)
{
    uint16_t value       = 0;
    EmberAfStatus status = emberAfRelativeHumidityMeasurementClusterGetMeasuredValue(endpoint, &value);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        status = emberAfRelativeHumidityMeasurementClusterSetMeasuredValueCallback(endpoint, 0);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            emberAfRelativeHumidityMeasurementClusterPrintln("ERR: writing present value %x", status);
        }
    }

    status = emberAfRelativeHumidityMeasurementClusterGetMinMeasuredValue(endpoint, &value);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        status = emberAfRelativeHumidityMeasurementClusterSetMinMeasuredValueCallback(endpoint, UINT16_MAX);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            emberAfRelativeHumidityMeasurementClusterPrintln("ERR: writing present value %x", status);
        }
    }

    status = emberAfRelativeHumidityMeasurementClusterGetMaxMeasuredValue(endpoint, &value);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        status = emberAfRelativeHumidityMeasurementClusterSetMaxMeasuredValueCallback(endpoint, UINT16_MAX);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            emberAfRelativeHumidityMeasurementClusterPrintln("ERR: writing present value %x", status);
        }
    }
}

EmberAfStatus emberAfRelativeHumidityMeasurementClusterSetMeasuredValueCallback(chip::EndpointId endpoint, uint16_t measuredValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                       ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) &measuredValue,
                                       ZCL_INT16U_ATTRIBUTE_TYPE);
}

EmberAfStatus emberAfRelativeHumidityMeasurementClusterSetMinMeasuredValueCallback(chip::EndpointId endpoint,
                                                                                   uint16_t minMeasuredValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                       ZCL_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) &minMeasuredValue,
                                       ZCL_INT16U_ATTRIBUTE_TYPE);
}

EmberAfStatus emberAfRelativeHumidityMeasurementClusterSetMaxMeasuredValueCallback(chip::EndpointId endpoint,
                                                                                   uint16_t maxMeasuredValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                       ZCL_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) &maxMeasuredValue,
                                       ZCL_INT16U_ATTRIBUTE_TYPE);
}
