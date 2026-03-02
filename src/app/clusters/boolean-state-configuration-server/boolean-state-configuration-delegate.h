/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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

#include <clusters/BooleanStateConfiguration/Enums.h>
#include <clusters/BooleanStateConfiguration/Events.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BitMask.h>

namespace chip {
namespace app {
namespace Clusters {

namespace BooleanStateConfiguration {

/** @brief
 *    Defines methods for implementing application-specific logic for the Boolean State Configuration Cluster.
 */
class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    virtual CHIP_ERROR HandleSuppressAlarm(BooleanStateConfiguration::AlarmModeBitmap alarmToSuppress)             = 0;
    virtual CHIP_ERROR HandleEnableDisableAlarms(chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarms) = 0;

    // These methods are called when the corresponding attributes are updated by the server, whether
    //  via WriteAttribute, InvokeCommand, or direct server APIs. Default implementations are empty
    //  override only the callbacks you need.

    virtual bool OnCurrentSensitivityLevelChanged(uint8_t newValue) { return true; }
    virtual bool OnAlarmsActiveChanged(chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> newValue) { return true; }
    virtual bool OnAlarmsSuppressedChanged(chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> newValue) { return true; }
    virtual bool OnAlarmsEnabledChanged(chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> newValue) { return true; }
    virtual bool OnSensorFaultChanged(chip::BitMask<BooleanStateConfiguration::SensorFaultBitmap> newValue) { return true; }
};

} // namespace BooleanStateConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
