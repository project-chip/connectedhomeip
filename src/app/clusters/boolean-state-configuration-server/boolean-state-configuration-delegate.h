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

    // These methods are called when specific attributes are updated via WriteAttribute or InvokeCommand.
    // Default implementations are empty - override only the callbacks you need.

    virtual void OnCurrentSensitivityLevelChanged(uint8_t newValue) {}
    virtual void OnAlarmsActiveChanged(chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> newValue) {}
    virtual void OnAlarmsSuppressedChanged(chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> newValue) {}
    virtual void OnAlarmsEnabledChanged(chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> newValue) {}
    virtual void OnSensorFaultChanged(chip::BitMask<BooleanStateConfiguration::SensorFaultBitmap> newValue) {}
};

} // namespace BooleanStateConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
