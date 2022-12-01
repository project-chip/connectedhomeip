/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
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

#include "TemperatureSensor.h"

#ifdef __cplusplus
extern "C" {
#endif
// This is a C implementation. Need the ifdef __cplusplus else we get linking issues
#include "sl_sensor_rht.h"

#ifdef __cplusplus
}
#endif

namespace TemperatureSensor {
constexpr uint16_t kSensorTemperatureOffset = 800;

sl_status_t Init()
{
    return sl_sensor_rht_init();
}

sl_status_t GetTemp(uint32_t * relativeHumidity, int16_t * temperature)
{
    // Sensor resolution 0.001 C
    // DataModel resolution 0.01 C
    int32_t temp;
    sl_status_t status = sl_sensor_rht_get(relativeHumidity, &temp);
    *temperature       = static_cast<int16_t>(temp / 10) - kSensorTemperatureOffset;
    return status;
}
}; // namespace TemperatureSensor
