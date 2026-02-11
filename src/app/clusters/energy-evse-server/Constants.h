/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <cstddef>
#include <cstdint>

namespace chip::app::Clusters::EnergyEvse {

constexpr int64_t kMinimumChargeCurrentLimit       = 0;
constexpr int64_t kMinimumChargeCurrent            = 6000; // 6A in mA, spec default
constexpr uint32_t kMaxRandomizationDelayWindowSec = 86400;
constexpr uint8_t kEvseTargetsMaxNumberOfDays      = 7;
constexpr uint8_t kEvseTargetsMaxTargetsPerDay     = 10;
constexpr uint16_t kMaxMinutesPastMidnight         = 1439; // 24*60 - 1, spec range 0..1439
constexpr uint8_t kMaxTargetSoCPercent             = 100;
constexpr uint8_t kDayOfWeekBitmapMask             = 0x7F; // bits 0-6 for 7 days
constexpr size_t kMaxVehicleIDBufSize              = 32;

} // namespace chip::app::Clusters::EnergyEvse
