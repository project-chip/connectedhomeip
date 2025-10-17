/*
 *
 *    Copyright (c) 20265 Project CHIP Authors
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

#pragma once

/**
 * @file EvseTargetsConfig.h
 *
 * This file contains constants needed by evse-common code that are not defined in the EVSE cluster.
 */

#include <cstdint>
namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

// Bitmask for all days of the week (union of TargetDayOfWeekBitmap values)
constexpr uint8_t kAllTargetDaysMask = 0x7f;
// A sensible minimum limit for mains voltage (100V) to avoid accidental use
// of 100mV instead of 100000mV
constexpr int64_t kMinimumMainsVoltage_mV = 100000;

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
