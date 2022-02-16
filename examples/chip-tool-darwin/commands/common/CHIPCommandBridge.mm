/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "CHIPCommandBridge.h"

#import <CHIP/CHIPDeviceController.h>
#include <core/CHIPBuildConfig.h>
#include <lib/support/CodeUtils.h>

const uint16_t kListenPort = 5541;

CHIP_ERROR CHIPCommandBridge::Run()
{
    ChipLogProgress(chipTool, "Running Command");

    mController = [CHIPDeviceController sharedController];
    if (mController == nil) {
        ChipLogError(chipTool, "Controller is nil");
        return CHIP_ERROR_INTERNAL;
    }

    [mController setListenPort:kListenPort];

    if (![mController startup:nil vendorId:0 nocSigner:nil]) {
        ChipLogError(chipTool, "Controller startup failure.");
        return CHIP_ERROR_INTERNAL;
    }

    ReturnLogErrorOnFailure(RunCommand());
    ReturnLogErrorOnFailure(StartWaiting(GetWaitDuration()));

    return CHIP_NO_ERROR;
}

CHIPDeviceController * CHIPCommandBridge::CurrentCommissioner() { return mController; }

CHIP_ERROR CHIPCommandBridge::ShutdownCommissioner()
{
    ChipLogProgress(chipTool, "Shutting down controller");
    BOOL result = [CurrentCommissioner() shutdown];
    if (!result) {
        ChipLogError(chipTool, "Unable to shut down controller");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

#if !CONFIG_USE_SEPARATE_EVENTLOOP
static void OnResponseTimeout(chip::System::Layer *, void * appState)
{
    (reinterpret_cast<CHIPCommandBridge *>(appState))->SetCommandExitStatus(CHIP_ERROR_TIMEOUT);
}
#endif // !CONFIG_USE_SEPARATE_EVENTLOOP

CHIP_ERROR CHIPCommandBridge::StartWaiting(chip::System::Clock::Timeout duration)
{
#if CONFIG_USE_SEPARATE_EVENTLOOP
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();
    auto waitingUntil = std::chrono::system_clock::now() + std::chrono::duration_cast<std::chrono::seconds>(duration);
    {
        std::unique_lock<std::mutex> lk(cvWaitingForResponseMutex);
        if (!cvWaitingForResponse.wait_until(lk, waitingUntil, [this]() { return !this->mWaitingForResponse; })) {
            mCommandExitStatus = CHIP_ERROR_TIMEOUT;
        }
    }
    LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().StopEventLoopTask());
#else
    ReturnLogErrorOnFailure(chip::DeviceLayer::SystemLayer().StartTimer(duration, OnResponseTimeout, this));
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
#endif // CONFIG_USE_SEPARATE_EVENTLOOP

    return mCommandExitStatus;
}

void CHIPCommandBridge::StopWaiting()
{
#if CONFIG_USE_SEPARATE_EVENTLOOP
    {
        std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
        mWaitingForResponse = false;
    }
    cvWaitingForResponse.notify_all();
#else // CONFIG_USE_SEPARATE_EVENTLOOP
    LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().StopEventLoopTask());
#endif // CONFIG_USE_SEPARATE_EVENTLOOP
}
