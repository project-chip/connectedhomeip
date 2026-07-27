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

#include <app/clusters/smoke-co-alarm-server/SmokeCoAlarmCluster.h>

namespace chip::app::Clusters::SmokeCoAlarm {
/**
 * @brief Chef delegate for the (code-driven) Smoke CO Alarm cluster.
 *
 * Handles SelfTestRequest by simulating a self-test that completes after a
 * short timeout, mirroring the behaviour of all-clusters-app's smco-stub.cpp.
 */
class ChefSmokeCoAlarmDelegate : public SmokeCoAlarmDelegate
{
public:
    void OnSelfTestRequested() override;
    void OnSmokeSensitivityLevelChanged(SmokeCoAlarm::SensitivityEnum newSmokeSensitivityLevel) override;
    void OnExpressedStateChanged(SmokeCoAlarm::ExpressedStateEnum newExpressedState) override;
};

} // namespace chip::app::Clusters::SmokeCoAlarm

/// Cancels the pending self-test timer (if any). Called from ApplicationShutdown().
void SmokeCoAlarmShutdown();
