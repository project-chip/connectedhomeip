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

EmberAfStatus emberAfRelativeHumidityMeasurementClusterSetMeasuredValueCallback(chip::EndpointId endpoint, uint16_t measuredValue);
EmberAfStatus emberAfRelativeHumidityMeasurementClusterSetMinMeasuredValueCallback(chip::EndpointId endpoint,
                                                                                   uint16_t minMeasuredValue);
EmberAfStatus emberAfRelativeHumidityMeasurementClusterSetMaxMeasuredValueCallback(chip::EndpointId endpoint,
                                                                                   uint16_t maxMeasuredValue);

EmberAfStatus emberAfRelativeHumidityMeasurementClusterGetMeasuredValue(chip::EndpointId endpoint, uint16_t * measuredValue);
EmberAfStatus emberAfRelativeHumidityMeasurementClusterGetMinMeasuredValue(chip::EndpointId endpoint, uint16_t * minMeasuredValue);
EmberAfStatus emberAfRelativeHumidityMeasurementClusterGetMaxMeasuredValue(chip::EndpointId endpoint, uint16_t * maxMeasuredValue);
