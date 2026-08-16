/*
 *
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
#include "PowerSourceCluster.h"

namespace chip::app::Clusters {

using FullWiredPowerSourceCluster =
    PowerSourceCluster<BitFlags<PowerSource::Feature>(PowerSource::Feature::kWired).Raw(), UINT32_MAX>;
using FullWiredPowerSourceConfig = FullWiredPowerSourceCluster::Config;

using FullBatteryPowerSourceCluster =
    PowerSourceCluster<BitFlags<PowerSource::Feature>(PowerSource::Feature::kBattery, PowerSource::Feature::kReplaceable,
                                                      PowerSource::Feature::kRechargeable)
                           .Raw(),
                       UINT32_MAX>;
using FullBatteryPowerSourceConfig = FullBatteryPowerSourceCluster::Config;

using MinimalWiredPowerSourceCluster = PowerSourceCluster<BitFlags<PowerSource::Feature>(PowerSource::Feature::kWired).Raw(), 0>;
using MinimalWiredPowerSourceConfig  = MinimalWiredPowerSourceCluster::Config;

using MinimalBatteryPowerSourceCluster =
    PowerSourceCluster<BitFlags<PowerSource::Feature>(PowerSource::Feature::kBattery).Raw(), 0>;
using MinimalBatteryPowerSourceConfig = MinimalBatteryPowerSourceCluster::Config;

} // namespace chip::app::Clusters
