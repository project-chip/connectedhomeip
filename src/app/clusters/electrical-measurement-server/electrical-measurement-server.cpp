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

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

using namespace chip;

EmberAfStatus emberAfElectricalMeasurementClusterGetMeasurementType(EndpointId endpoint, uint32_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_MEASUREMENT_TYPE_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetTotalActivePower(EndpointId endpoint, int32_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_TOTAL_ACTIVE_POWER_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsVoltage(EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsVoltageMin(EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_MIN_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsVoltageMax(EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_MAX_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsCurrent(EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsCurrentMin(EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_MIN_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsCurrentMax(EndpointId endpoint, uint16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_MAX_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetActivePower(EndpointId endpoint, int16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetActivePowerMin(EndpointId endpoint, int16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_MIN_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}
EmberAfStatus emberAfElectricalMeasurementClusterGetActivePowerMax(EndpointId endpoint, int16_t * value)
{
    return emberAfReadServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_MAX_ATTRIBUTE_ID,
                                      (uint8_t *) value, sizeof(*value));
}

EmberAfStatus emberAfElectricalMeasurementClusterSetMeasurementType(EndpointId endpoint, uint32_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_MEASUREMENT_TYPE_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetTotalActivePower(EndpointId endpoint, int32_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_TOTAL_ACTIVE_POWER_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT32S_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsVoltage(EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsVoltageMin(EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_MIN_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsVoltageMax(EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_VOLTAGE_MAX_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsCurrent(EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsCurrentMin(EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_MIN_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsCurrentMax(EndpointId endpoint, uint16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_RMS_CURRENT_MAX_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetActivePower(EndpointId endpoint, int16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16S_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetActivePowerMin(EndpointId endpoint, int16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_MIN_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16S_ATTRIBUTE_TYPE);
}
EmberAfStatus emberAfElectricalMeasurementClusterSetActivePowerMax(EndpointId endpoint, int16_t value)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, ZCL_ACTIVE_POWER_MAX_ATTRIBUTE_ID,
                                       (uint8_t *) &value, ZCL_INT16S_ATTRIBUTE_TYPE);
}
