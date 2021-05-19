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

#include "temperature-measurement-server.h"

#include <app/util/af.h>

#include "gen/att-storage.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

#include <support/logging/CHIPLogging.h>

EmberAfStatus emberAfTemperatureMeasurementClusterGetMeasuredValue(chip::EndpointId endpoint, int16_t* measuredValue)
{
    return emberAfReadAttribute(endpoint,
                                ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                                ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                (uint8_t*)measuredValue,
                                sizeof(*measuredValue),
                                NULL);
}

EmberAfStatus emberAfTemperatureMeasurementClusterGetMinMeasuredValue(chip::EndpointId endpoint, int16_t* minMeasuredValue)
{
    return emberAfReadAttribute(endpoint,
                                ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                                ZCL_TEMP_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                (uint8_t*)minMeasuredValue,
                                sizeof(*minMeasuredValue),
                                NULL);
}

EmberAfStatus emberAfTemperatureMeasurementClusterGetMaxMeasuredValue(chip::EndpointId endpoint, int16_t* maxMeasuredValue)
{
    return emberAfReadAttribute(endpoint,
                                ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                                ZCL_TEMP_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                (uint8_t*)maxMeasuredValue,
                                sizeof(*maxMeasuredValue),
                                NULL);
}

void emberAfTemperatureMeasurementClusterServerInitCallback(chip::EndpointId endpoint)
{
    int16_t value = 0;
    EmberAfStatus status = emberAfTemperatureMeasurementClusterGetMeasuredValue(endpoint, &value);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        (void)emberAfTemperatureMeasurementClusterSetMeasuredValueCallback(endpoint, 0);
    }

    status = emberAfTemperatureMeasurementClusterGetMinMeasuredValue(endpoint, &value);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        (void)emberAfTemperatureMeasurementClusterSetMinMeasuredValueCallback(endpoint, UINT16_MAX);
    }

    status = emberAfTemperatureMeasurementClusterGetMaxMeasuredValue(endpoint, &value);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        (void)emberAfTemperatureMeasurementClusterSetMaxMeasuredValueCallback(endpoint, UINT16_MAX);
    }
}

EmberAfStatus emberAfTemperatureMeasurementClusterSetMeasuredValueCallback(chip::EndpointId endpoint, int16_t measuredValue)
{
    EmberAfStatus status = emberAfWriteAttribute(endpoint,
                                                 ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                                                 ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID,
                                                 CLUSTER_MASK_SERVER,
                                                 (uint8_t *)&measuredValue,
                                                 ZCL_INT16S_ATTRIBUTE_TYPE);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        // emberAfTemperatureMeasurementClusterPrintln("ERR: writing present value %x", status);
    }

    return status;
}

EmberAfStatus emberAfTemperatureMeasurementClusterSetMinMeasuredValueCallback(chip::EndpointId endpoint, int16_t minMeasuredValue)
{
    EmberAfStatus status = emberAfWriteAttribute(endpoint,
                                                 ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                                                 ZCL_TEMP_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
                                                 CLUSTER_MASK_SERVER,
                                                 (uint8_t *)&minMeasuredValue,
                                                 ZCL_INT16S_ATTRIBUTE_TYPE);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        // emberAfTemperatureMeasurementClusterPrintln("ERR: writing present value %x", status);
    }

    return status;
}

EmberAfStatus emberAfTemperatureMeasurementClusterSetMaxMeasuredValueCallback(chip::EndpointId endpoint, int16_t maxMeasuredValue)
{
    EmberAfStatus status = emberAfWriteAttribute(endpoint,
                                                 ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                                                 ZCL_TEMP_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
                                                 CLUSTER_MASK_SERVER,
                                                 (uint8_t *)&maxMeasuredValue,
                                                 ZCL_INT16S_ATTRIBUTE_TYPE);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        // emberAfTemperatureMeasurementClusterPrintln("ERR: writing present value %x", status);
    }

    return status;
}
