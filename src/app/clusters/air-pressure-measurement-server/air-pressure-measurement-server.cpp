/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <support/logging/CHIPLogging.h>

#ifndef emberAfAirPressureMeasurementClusterPrintln
#define emberAfAirPressureMeasurementClusterPrintln(...) ChipLogProgress(Zcl, __VA_ARGS__);
#endif

EmberAfStatus emberAfAirPressureMeasurementClusterGetMeasuredValue(chip::EndpointId endpoint, uint16_t * measuredValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_AIR_PRESSURE_MEASUREMENT_CLUSTER_ID,
                                      ZCL_AIR_PRESSURE_MEASUREMENT_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) measuredValue,
                                      sizeof(*measuredValue));
}

static EmberAfStatus emberAfAirPressureMeasurementClusterSetAltitudeCallback(chip::EndpointId endpoint, int16_t altitude)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_AIR_PRESSURE_MEASUREMENT_CLUSTER_ID,
                                       ZCL_AIR_PRESSURE_MEASUREMENT_ALTITUDE_ATTRIBUTE_ID, (uint8_t *) &altitude,
                                       ZCL_INT16S_ATTRIBUTE_TYPE);
}

void emberAfAirPressureMeasurementClusterServerInitCallback(chip::EndpointId endpoint)
{
    /**
     * nothing to do here - default values set by attribute storage
     */
    (void) endpoint;
}

EmberAfStatus emberAfAirPressureMeasurementClusterSetMeasuredValueCallback(chip::EndpointId endpoint, uint16_t measuredValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_AIR_PRESSURE_MEASUREMENT_CLUSTER_ID,
                                       ZCL_AIR_PRESSURE_MEASUREMENT_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) &measuredValue,
                                       ZCL_INT16U_ATTRIBUTE_TYPE);
}

EmberAfStatus emberAfAirPressureMeasurementClusterGetAltitudeValue(chip::EndpointId endpoint, int16_t * altitude)
{
    return emberAfReadServerAttribute(endpoint, ZCL_AIR_PRESSURE_MEASUREMENT_CLUSTER_ID,
                                      ZCL_AIR_PRESSURE_MEASUREMENT_ALTITUDE_ATTRIBUTE_ID, (uint8_t *) altitude, sizeof(*altitude));
}
