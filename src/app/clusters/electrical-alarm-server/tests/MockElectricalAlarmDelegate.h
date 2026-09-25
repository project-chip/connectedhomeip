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

#include <app/clusters/electrical-alarm-server/electrical-alarm-delegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalAlarm {

class MockElectricalAlarmDelegate : public Delegate
{
public:
    // Configurable reject flags — set to false to simulate a delegate rejection.
    bool allowModifyEnabledAlarms = true;
    bool allowReset               = true;
    bool allowSetThresholds       = true;

    // Call counts for assertion in tests.
    int modifyEnabledAlarmsCallCount = 0;
    int resetAlarmsCallCount         = 0;
    int setThresholdsCallCount       = 0;

    bool ModifyEnabledAlarmsCallback(const BitMask<AlarmBitmap> mask) override
    {
        ++modifyEnabledAlarmsCallCount;
        return allowModifyEnabledAlarms;
    }

    bool ResetAlarmsCallback(const BitMask<AlarmBitmap> alarms) override
    {
        ++resetAlarmsCallCount;
        return allowReset;
    }

    bool SetElectricalAlarmThresholdsCallback(const Commands::SetElectricalAlarmThresholds::DecodableType & commandData) override
    {
        ++setThresholdsCallCount;
        return allowSetThresholds;
    }
};

} // namespace ElectricalAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
