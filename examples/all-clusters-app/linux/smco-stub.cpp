/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/smoke-co-alarm-server/SmokeCOTestEventTriggerDelegate.h>
#include <app/clusters/smoke-co-alarm-server/smoke-co-alarm-server.h>

#include <platform/CHIPDeviceLayer.h>

#include <thread>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SmokeCoAlarm;
using namespace chip::DeviceLayer;

void TestModeTimeoutThread(int timeout)
{
    ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm] => Self test running\033[0;37m");

    std::this_thread::sleep_for(std::chrono::seconds(timeout));

    PlatformMgr().LockChipStack();
    SmokeCoAlarmServer::Instance().SetTestInProgress(1, false);
    SmokeCoAlarmServer::Instance().AutoSetExpressedState(1);
    PlatformMgr().UnlockChipStack();

    ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm] => Self Test Timer Complete\033[0;37m");
}

bool emberAfPluginSmokeCoAlarmSelfTestRequestCommand(chip::EndpointId endpointId)
{
    ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm] => Starting Self Test Timer\033[0;37m");

    SmokeCoAlarmServer::Instance().SetTestInProgress(1, true);
    std::thread(TestModeTimeoutThread, 20).detach();
    return true;
}
