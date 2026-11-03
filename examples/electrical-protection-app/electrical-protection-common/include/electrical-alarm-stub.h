/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/electrical-alarm-server/CodegenIntegration.h>

#include <clusters/ElectricalAlarm/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>

namespace chip::app::Clusters::ElectricalAlarm {

/// Every feature this cluster defines: the ten alarm classes, ADJUST, and RST. An example app that
/// offers all of them lets a test script gating on any single feature find a DUT.
inline constexpr BitMask<Feature> kAllFeatures(Feature::kReset, Feature::kAdjustableThresholds, Feature::kOverVoltage,
                                               Feature::kUnderVoltage, Feature::kOverFrequency, Feature::kUnderFrequency,
                                               Feature::kOverPower, Feature::kUnderPower, Feature::kOverCurrent,
                                               Feature::kUnderCurrent, Feature::kPowerImport, Feature::kPowerExport);

/// Every alarm this cluster defines. Supported is independent of the feature map: the alarm bits
/// carry no feature conformance, so the seven that have no matching feature (quality, polarity,
/// measurement and grid-power alarms) are declarable regardless.
inline constexpr BitMask<AlarmBitmap> kAllAlarms(AlarmBitmap::kOverVoltage, AlarmBitmap::kUnderVoltage, AlarmBitmap::kOverFrequency,
                                                 AlarmBitmap::kUnderFrequency, AlarmBitmap::kOverPower, AlarmBitmap::kUnderPower,
                                                 AlarmBitmap::kOverCurrent, AlarmBitmap::kUnderCurrent,
                                                 AlarmBitmap::kFrequencyQuality, AlarmBitmap::kVoltageQuality,
                                                 AlarmBitmap::kSwappedPolarity, AlarmBitmap::kLossOfMeasurement,
                                                 AlarmBitmap::kLossOfGridPower, AlarmBitmap::kPowerImported,
                                                 AlarmBitmap::kPowerExported);

/// Create and register the Electrical Alarm cluster on `endpointId` with `features`. The caller
/// chooses them, as the Power Topology stub next door does, so the app states what the endpoint
/// offers rather than burying it here. At least one alarm class is required; the Electrical Circuit
/// Breaker device type makes OverCurrent mandatory wherever this cluster appears.
/// Call from ApplicationInit().
CHIP_ERROR ElectricalAlarmInit(EndpointId endpointId, BitMask<Feature> features);

/// Unregister and destroy the cluster. Call from ApplicationShutdown().
void ElectricalAlarmShutdown();

} // namespace chip::app::Clusters::ElectricalAlarm
