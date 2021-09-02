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

#include "relative-humidity-measurement-server.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

#ifndef emberAfRelativeHumidityMeasurementClusterPrintln
#define emberAfRelativeHumidityMeasurementClusterPrintln(...) ChipLogProgress(Zcl, __VA_ARGS__);
#endif

EmberAfStatus emberAfRelativeHumidityMeasurementClusterGetMeasuredValue(EndpointId endpoint, uint16_t * measuredValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                      ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) measuredValue,
                                      sizeof(*measuredValue));
}

EmberAfStatus emberAfRelativeHumidityMeasurementClusterGetMinMeasuredValue(EndpointId endpoint, uint16_t * minMeasuredValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                      ZCL_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) minMeasuredValue,
                                      sizeof(*minMeasuredValue));
}

EmberAfStatus emberAfRelativeHumidityMeasurementClusterGetMaxMeasuredValue(EndpointId endpoint, uint16_t * maxMeasuredValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                      ZCL_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) maxMeasuredValue,
                                      sizeof(*maxMeasuredValue));
}

EmberAfStatus emberAfRelativeHumidityMeasurementClusterSetMeasuredValueCallback(EndpointId endpoint, uint16_t measuredValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                       ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) &measuredValue,
                                       ZCL_INT16U_ATTRIBUTE_TYPE);
}

EmberAfStatus emberAfRelativeHumidityMeasurementClusterSetMinMeasuredValueCallback(EndpointId endpoint, uint16_t minMeasuredValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                       ZCL_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) &minMeasuredValue,
                                       ZCL_INT16U_ATTRIBUTE_TYPE);
}

EmberAfStatus emberAfRelativeHumidityMeasurementClusterSetMaxMeasuredValueCallback(EndpointId endpoint, uint16_t maxMeasuredValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                       ZCL_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE_ATTRIBUTE_ID, (uint8_t *) &maxMeasuredValue,
                                       ZCL_INT16U_ATTRIBUTE_TYPE);
}
