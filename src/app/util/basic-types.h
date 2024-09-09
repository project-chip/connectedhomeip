/**
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

/**
 * @file
 * @brief The include file for all the types for the data model that are not
 *        dependent on an individual application configuration.
 */

#pragma once

#include <cstdint>

// Pull in core types
#include <lib/core/DataModelTypes.h>

namespace chip {

typedef uint8_t Percent;
typedef uint16_t Percent100ths;
typedef int64_t Energy_mWh;
typedef int64_t Amperage_mA;
typedef int64_t Power_mW;
typedef int64_t Voltage_mV;

} // namespace chip
