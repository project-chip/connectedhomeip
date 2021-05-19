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

#include "air-pressure-measurement-server.h"

#include <app/util/af.h>

#include "gen/att-storage.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

EmberAfStatus emberAfAirPressureMeasurementClusterGetMeasuredValue(chip::EndpointId endpoint, uint16_t* measuredValue)
{
    return emberAfReadAttribute(endpoint,
                                ZCL_AIR_PRESSURE_MEASUREMENT_CLUSTER_ID,
                                ZCL_AIR_PRESSURE_MEASUREMENT_MEASURED_VALUE_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                (uint8_t*)measuredValue,
                                sizeof(*measuredValue),
                                NULL);
}

static EmberAfStatus emberAfAirPressureMeasurementClusterSetAltitudeCallback(chip::EndpointId endpoint, int16_t altitude)
{
    EmberAfStatus status = emberAfWriteAttribute(endpoint,
                                                 ZCL_AIR_PRESSURE_MEASUREMENT_CLUSTER_ID,
                                                 ZCL_AIR_PRESSURE_MEASUREMENT_ALTITUDE_ATTRIBUTE_ID,
                                                 CLUSTER_MASK_SERVER,
                                                 (uint8_t *)&altitude,
                                                 ZCL_INT16S_ATTRIBUTE_TYPE);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        // emberAfAirPressureMeasurementClusterPrintln("ERR: writing present value %x", status);
    }

    return status;
}

void emberAfAirPressureMeasurementClusterServerInitCallback(chip::EndpointId endpoint)
{
    uint16_t measuredValue = 0;
    EmberAfStatus status = emberAfAirPressureMeasurementClusterGetMeasuredValue(endpoint, &measuredValue);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        (void)emberAfAirPressureMeasurementClusterSetMeasuredValueCallback(endpoint, 10132);
    }

    int16_t altitude = 0;
    status = emberAfAirPressureMeasurementClusterGetAltitudeValue(endpoint, &altitude);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        (void)emberAfAirPressureMeasurementClusterSetAltitudeCallback(endpoint, 0);
    }
}

EmberAfStatus emberAfAirPressureMeasurementClusterSetMeasuredValueCallback(chip::EndpointId endpoint, uint16_t measuredValue)
{
    EmberAfStatus status = emberAfWriteAttribute(endpoint,
                                                 ZCL_AIR_PRESSURE_MEASUREMENT_CLUSTER_ID,
                                                 ZCL_AIR_PRESSURE_MEASUREMENT_MEASURED_VALUE_ATTRIBUTE_ID,
                                                 CLUSTER_MASK_SERVER,
                                                 (uint8_t *)&measuredValue,
                                                 ZCL_INT16U_ATTRIBUTE_TYPE);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        // emberAfAirPressureMeasurementClusterPrintln("ERR: writing present value %x", status);
    }

    return status;
}

EmberAfStatus emberAfAirPressureMeasurementClusterGetAltitudeValue(chip::EndpointId endpoint, int16_t* altitude)
{
    return emberAfReadAttribute(endpoint,
                                ZCL_AIR_PRESSURE_MEASUREMENT_CLUSTER_ID,
                                ZCL_AIR_PRESSURE_MEASUREMENT_ALTITUDE_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                (uint8_t*)altitude,
                                sizeof(*altitude),
                                NULL);
}
