/*
 *   Copyright (c) 2021 Project CHIP Authors
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
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include "TraceHandlers.h"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

using DeviceControllerFactory = chip::Controller::DeviceControllerFactory;

const uint16_t kListenPort = 5541;

CHIP_ERROR CHIPCommandBridge::Run()
{
    StartTracing();
    NSLog(@"Running Command");

    mController = [CHIPDeviceController sharedController];
    [mController setListenPort:kListenPort];
    [mController startup:nil vendorId:0 nocSigner:nil];

    RunCommand();
    ReturnLogErrorOnFailure(StartWaiting(GetWaitDuration()));
    StopTracing();
    return CHIP_NO_ERROR;
}

void CHIPCommandBridge::StartTracing()
{
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::InitTrace();

    if (mTraceFile.HasValue()) {
        chip::trace::SetTraceStream(new chip::trace::TraceStreamFile(mTraceFile.Value()));
    } else if (mTraceLog.HasValue() && mTraceLog.Value() == true) {
        chip::trace::SetTraceStream(new chip::trace::TraceStreamLog());
    }
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
}

void CHIPCommandBridge::StopTracing()
{
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::DeInitTrace();
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
}

CHIPDeviceController * CHIPCommandBridge::CurrentCommissioner() { return mController; }

CHIP_ERROR CHIPCommandBridge::ShutdownCommissioner()
{
    NSLog(@"Shutting down controller");
    BOOL result = [CurrentCommissioner() shutdown];
    if (!result) {
        NSLog(@"Unable to shut down controller");
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
