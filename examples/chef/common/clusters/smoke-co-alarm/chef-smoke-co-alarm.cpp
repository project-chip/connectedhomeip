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

#include "chef-smoke-co-alarm.h"

#include <app/clusters/smoke-co-alarm-server/CodegenIntegration.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <platform/CHIPDeviceLayer.h>

#include <array>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SmokeCoAlarm;
using namespace chip::DeviceLayer;

namespace {

constexpr uint16_t kSelfTestingTimeoutSec  = 10;
constexpr EndpointId kSmokeCoAlarmEndpoint = 1;

const std::array<ExpressedStateEnum, SmokeCoAlarmServer::kPriorityOrderLength> kPriorityOrder = {
    ExpressedStateEnum::kInoperative, ExpressedStateEnum::kSmokeAlarm,     ExpressedStateEnum::kInterconnectSmoke,
    ExpressedStateEnum::kCOAlarm,     ExpressedStateEnum::kInterconnectCO, ExpressedStateEnum::kHardwareFault,
    ExpressedStateEnum::kTesting,     ExpressedStateEnum::kEndOfService,   ExpressedStateEnum::kBatteryAlert
};

void EndSelfTestingEventHandler(System::Layer *, void *)
{
    SmokeCoAlarmServer::Instance().SetTestInProgress(kSmokeCoAlarmEndpoint, false);
    SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(kSmokeCoAlarmEndpoint, kPriorityOrder);
    ChipLogProgress(Zcl, "[Smoke-CO-Alarm] => Self test complete");
}

} // namespace

namespace chip::app::Clusters::SmokeCoAlarm {

void ChefSmokeCoAlarmDelegate::OnSelfTestRequested()
{
    ChipLogProgress(Zcl, "[Smoke-CO-Alarm] => Self test running");
    LogErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kSelfTestingTimeoutSec),
                                                            EndSelfTestingEventHandler, nullptr));
}

void ChefSmokeCoAlarmDelegate::OnSmokeSensitivityLevelChanged(SensitivityEnum newSmokeSensitivityLevel)
{
    ChipLogProgress(Zcl, "[Smoke-CO-Alarm] => SmokeSensitivityLevel changed to %u", to_underlying(newSmokeSensitivityLevel));
}

void ChefSmokeCoAlarmDelegate::OnExpressedStateChanged(ExpressedStateEnum newExpressedState)
{
    ChipLogProgress(Zcl, "[Smoke-CO-Alarm] => ExpressedState changed to %u", to_underlying(newExpressedState));
}

} // namespace chip::app::Clusters::SmokeCoAlarm

void SmokeCoAlarmShutdown()
{
    DeviceLayer::SystemLayer().CancelTimer(EndSelfTestingEventHandler, nullptr);
}
