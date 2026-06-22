/*
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

#include <app/clusters/smoke-co-alarm-server/SmokeCoAlarmCluster.h>
#include <devices/smoke-co-alarm/SmokeCoAlarmDevice.h>
#include <lib/support/TimerDelegate.h>

#include <array>

namespace chip {
namespace app {

/**
 * @brief An example smoke + CO alarm device with no real hardware behind it.
 *
 * It implements SmokeCoAlarmDelegate: a self-test request is simulated with a timer, and every
 * other callback simply logs. This is the device the factory builds for the all-devices-app.
 */
class LoggingOnlySmokeCoAlarmDevice : public Clusters::SmokeCoAlarmDelegate, public SmokeCoAlarmDevice, public TimerContext
{
public:
    static constexpr uint16_t kSelfTestTimeoutSec = 10;

    explicit LoggingOnlySmokeCoAlarmDevice(TimerDelegate & timerDelegate);
    ~LoggingOnlySmokeCoAlarmDevice() override;

    // SmokeCoAlarmDelegate
    void OnSelfTestRequested() override;
    void OnSmokeSensitivityLevelChanged(Clusters::SmokeCoAlarm::SensitivityEnum newLevel) override;
    void OnExpressedStateChanged(Clusters::SmokeCoAlarm::ExpressedStateEnum newState) override;

    // TimerContext
    void TimerFired() override;

private:
    static const std::array<Clusters::SmokeCoAlarm::ExpressedStateEnum, Clusters::SmokeCoAlarmCluster::kPriorityOrderLength>
        sPriorityOrder;
};

} // namespace app
} // namespace chip
