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

#include "temperature-measurement-server.h"

#include <app/util/af.h>

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

#include <support/logging/CHIPLogging.h>

using namespace chip;

#ifndef emberAfTemperatureMeasurementClusterPrintln
#define emberAfTemperatureMeasurementClusterPrintln(...) ChipLogProgress(Zcl, __VA_ARGS__);
#endif

EmberAfStatus emberAfTemperatureMeasurementClusterGetMeasuredValue(chip::EndpointId endpoint, int16_t * measuredValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_TEMP_MEASUREMENT_CLUSTER_ID, ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID,
                                      (uint8_t *) measuredValue, sizeof(*measuredValue));
}

EmberAfStatus emberAfTemperatureMeasurementClusterGetMinMeasuredValue(chip::EndpointId endpoint, int16_t * minMeasuredValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_TEMP_MEASUREMENT_CLUSTER_ID, ZCL_TEMP_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
                                      (uint8_t *) minMeasuredValue, sizeof(*minMeasuredValue));
}

EmberAfStatus emberAfTemperatureMeasurementClusterGetMaxMeasuredValue(chip::EndpointId endpoint, int16_t * maxMeasuredValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_TEMP_MEASUREMENT_CLUSTER_ID, ZCL_TEMP_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
                                      (uint8_t *) maxMeasuredValue, sizeof(*maxMeasuredValue));
}

EmberAfStatus emberAfTemperatureMeasurementClusterSetMeasuredValueCallback(chip::EndpointId endpoint, int16_t measuredValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_TEMP_MEASUREMENT_CLUSTER_ID, ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID,
                                       (uint8_t *) &measuredValue, ZCL_INT16S_ATTRIBUTE_TYPE);
}

EmberAfStatus emberAfTemperatureMeasurementClusterSetMinMeasuredValueCallback(chip::EndpointId endpoint, int16_t minMeasuredValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_TEMP_MEASUREMENT_CLUSTER_ID, ZCL_TEMP_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
                                       (uint8_t *) &minMeasuredValue, ZCL_INT16S_ATTRIBUTE_TYPE);
}

EmberAfStatus emberAfTemperatureMeasurementClusterSetMaxMeasuredValueCallback(chip::EndpointId endpoint, int16_t maxMeasuredValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_TEMP_MEASUREMENT_CLUSTER_ID, ZCL_TEMP_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
                                       (uint8_t *) &maxMeasuredValue, ZCL_INT16S_ATTRIBUTE_TYPE);
}
