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

#include "electrical-measurement-server.h"

#include <app/util/af.h>

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

EmberAfStatus emberAfElectricalMeasurementClusterGetMeasurementType(chip::EndpointId endpoint, uint32_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_MEASUREMENT_TYPE_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetTotalActivePower(chip::EndpointId endpoint, int32_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_TOTAL_ACTIVE_POWER_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsVoltage(chip::EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsVoltageMin(chip::EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_MIN_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsVoltageMax(chip::EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_MAX_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsCurrent(chip::EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsCurrentMin(chip::EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_MIN_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsCurrentMax(chip::EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_MAX_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetActivePower(chip::EndpointId endpoint, int16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetActivePowerMin(chip::EndpointId endpoint, int16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_MIN_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetActivePowerMax(chip::EndpointId endpoint, int16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_MAX_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}

EmberAfStatus emberAfElectricalMeasurementClusterSetMeasurementType(chip::EndpointId endpoint, uint32_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_MEASUREMENT_TYPE_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetTotalActivePower(chip::EndpointId endpoint, int32_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_TOTAL_ACTIVE_POWER_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT32S_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsVoltage(chip::EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsVoltageMin(chip::EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_MIN_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsVoltageMax(chip::EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_MAX_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsCurrent(chip::EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsCurrentMin(chip::EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_MIN_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsCurrentMax(chip::EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_MAX_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetActivePower(chip::EndpointId endpoint, int16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16S_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetActivePowerMin(chip::EndpointId endpoint, int16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_MIN_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16S_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetActivePowerMax(chip::EndpointId endpoint, int16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_MAX_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16S_ATTRIBUTE_TYPE);
}
