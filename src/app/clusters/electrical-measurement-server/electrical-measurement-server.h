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

#pragma once

#include <app/util/af-types.h>

EmberAfStatus emberAfElectricalMeasurementClusterGetMeasurementType(chip::EndpointId endpoint, uint32_t * value);
EmberAfStatus emberAfElectricalMeasurementClusterGetTotalActivePower(chip::EndpointId endpoint, int32_t * value);
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsVoltage(chip::EndpointId endpoint, uint16_t * value);
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsVoltageMin(chip::EndpointId endpoint, uint16_t * value);
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsVoltageMax(chip::EndpointId endpoint, uint16_t * value);
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsCurrent(chip::EndpointId endpoint, uint16_t * value);
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsCurrentMin(chip::EndpointId endpoint, uint16_t * value);
EmberAfStatus emberAfElectricalMeasurementClusterGetRmsCurrentMax(chip::EndpointId endpoint, uint16_t * value);
EmberAfStatus emberAfElectricalMeasurementClusterGetActivePower(chip::EndpointId endpoint, int16_t * value);
EmberAfStatus emberAfElectricalMeasurementClusterGetActivePowerMin(chip::EndpointId endpoint, int16_t * value);
EmberAfStatus emberAfElectricalMeasurementClusterGetActivePowerMax(chip::EndpointId endpoint, int16_t * value);

EmberAfStatus emberAfElectricalMeasurementClusterSetMeasurementType(chip::EndpointId endpoint, uint32_t value);
EmberAfStatus emberAfElectricalMeasurementClusterSetTotalActivePower(chip::EndpointId endpoint, int32_t value);
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsVoltage(chip::EndpointId endpoint, uint16_t value);
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsVoltageMin(chip::EndpointId endpoint, uint16_t value);
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsVoltageMax(chip::EndpointId endpoint, uint16_t value);
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsCurrent(chip::EndpointId endpoint, uint16_t value);
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsCurrentMin(chip::EndpointId endpoint, uint16_t value);
EmberAfStatus emberAfElectricalMeasurementClusterSetRmsCurrentMax(chip::EndpointId endpoint, uint16_t value);
EmberAfStatus emberAfElectricalMeasurementClusterSetActivePower(chip::EndpointId endpoint, int16_t value);
EmberAfStatus emberAfElectricalMeasurementClusterSetActivePowerMin(chip::EndpointId endpoint, int16_t value);
EmberAfStatus emberAfElectricalMeasurementClusterSetActivePowerMax(chip::EndpointId endpoint, int16_t value);
