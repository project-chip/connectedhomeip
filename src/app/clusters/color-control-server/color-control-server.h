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

#pragma once

#include <app/util/basic-types.h>

/** @brief Compute Pwm from HSV
 *
 * This function is called from the color server when it is time for the PWMs to
 * be driven with a new value from the HSV values.
 *
 * @param endpoint The identifying endpoint Ver.: always
 */
void emberAfPluginColorControlServerComputePwmFromHsvCallback(chip::EndpointId endpoint);

/** @brief Compute Pwm from HSV
 *
 * This function is called from the color server when it is time for the PWMs to
 * be driven with a new value from the color temperature.
 *
 * @param endpoint The identifying endpoint Ver.: always
 */
void emberAfPluginColorControlServerComputePwmFromTempCallback(chip::EndpointId endpoint);

/** @brief Compute Pwm from HSV
 *
 * This function is called from the color server when it is time for the PWMs to
 * be driven with a new value from the color X and color Y values.
 *
 * @param endpoint The identifying endpoint Ver.: always
 */
void emberAfPluginColorControlServerComputePwmFromXyCallback(chip::EndpointId endpoint);
