/*
 *    Copyright (c) 2024-2025 Project CHIP Authors
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

#include "UloopHandler.h"

#include <lib/core/CHIPConfig.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <chrono>

extern "C" {
#include <libubox/uloop.h>
}

#ifndef CHIP_ULOOP_DEBUGGING
#define CHIP_ULOOP_DEBUGGING 0
#endif

namespace chip {
namespace ubus {

namespace {
System::LayerSocketsLoop & SystemLayer()
{
    return static_cast<System::LayerSocketsLoop &>(DeviceLayer::SystemLayer());
}
} // namespace

///// UloopHandler

UloopHandler UloopHandler::gInstance;
int UloopHandler::gRegistered = 0;

CHIP_ERROR UloopHandler::Register()
{
    VerifyOrReturnError(gRegistered++ == 0, CHIP_NO_ERROR);

    if (uloop_fd_set_cb != nullptr)
    {
        gRegistered = 0;
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogDetail(DeviceLayer, "Registering uloop handler");
    uloop_fd_set_cb      = &UloopFdSet;
    uloop_handle_sigchld = false;
    {
        UloopSignalGuard guard;
        uloop_init();
    }
    SystemLayer().AddLoopHandler(gInstance);

    return CHIP_NO_ERROR;
}

void UloopHandler::Unregister()
{
    auto count = gRegistered--;
    VerifyOrDie(count > 0);
    VerifyOrReturn(count == 1);

    ChipLogDetail(DeviceLayer, "Unregistering uloop handler");
    SystemLayer().RemoveLoopHandler(gInstance);
    uloop_done();
    uloop_fd_set_cb = nullptr;
}

System::Clock::Timestamp UloopHandler::PrepareEvents(System::Clock::Timestamp now)
{
    std::chrono::duration<int, std::milli> timeout(uloop_get_next_timeout());
    VerifyOrReturnValue(timeout.count() >= 0, System::Clock::Timestamp::max());
#if CHIP_ULOOP_DEBUGGING
    ChipLogDetail(DeviceLayer, "Next uloop timeout: %d", timeout.count());
#endif
    return now + std::chrono::duration_cast<System::Clock::Timestamp>(timeout);
}

void UloopHandler::HandleEvents()
{
    uloop_run_timeout(0);
}

void UloopHandler::UloopFdSet(uloop_fd * fd, unsigned int events)
{
#if CHIP_ULOOP_DEBUGGING
    ChipLogDetail(DeviceLayer, "Uloop fd_set(%d, 0x%x)", fd->fd, events);
#endif
    VerifyOrDieWithMsg(!(events & ~(ULOOP_READ | ULOOP_WRITE | ULOOP_BLOCKING)), DeviceLayer,
                       "Unsupported uloop fd_set events: 0x%x", events);

    auto & system = SystemLayer();
    System::SocketWatchToken watch;
    CHIP_ERROR err = system.StartWatchingSocket(fd->fd, &watch); // or find existing watch
    if (events != 0)
    {
        int changed = fd->flags ^ events;
        if (changed & ULOOP_READ)
        {
            if (events & ULOOP_READ)
            {
                system.RequestCallbackOnPendingRead(watch);
            }
            else
            {
                system.ClearCallbackOnPendingRead(watch);
            }
        }
        if (changed & ULOOP_WRITE)
        {
            if (events & ULOOP_WRITE)
            {
                system.RequestCallbackOnPendingWrite(watch);
            }
            else
            {
                system.ClearCallbackOnPendingWrite(watch);
            }
        }
    }
    else
    {
        VerifyOrReturn(err == CHIP_NO_ERROR); // shouldn't happen, but ignore
        SuccessOrExit(err = system.StopWatchingSocket(&watch));
    }

    return;
exit:
    ChipLogError(DeviceLayer, "Failed to handle uloop fd_set(%d, %d): %" CHIP_ERROR_FORMAT, fd->fd, events, err.Format());
}

///// UloopSignalGuard

UloopSignalGuard::UloopSignalGuard()
{
    Guard(SIGINT, mSigInt);
    Guard(SIGTERM, mSigTerm);
}

UloopSignalGuard::~UloopSignalGuard()
{
    Restore(SIGINT, mSigInt);
    Restore(SIGTERM, mSigTerm);
}

void UloopSignalGuard::Guard(int sig, struct sigaction & action)
{
    sigaction(sig, nullptr, &action);
    VerifyOrReturn(action.sa_handler == SIG_DFL);

    // Uloop will only modify signal handlers if they are currently set to SIG_DFL.
    // In that case, install a handler (which won't equal SIG_DFL) while the guard
    // is active. In the unlikely case that the guard handler receives a signal,
    // the handler is reset to SIG_DFL via SA_RESETHAND and re-raised.
    struct sigaction guard = {};
    sigemptyset(&guard.sa_mask);
    guard.sa_flags   = SA_RESETHAND;
    guard.sa_handler = [](int received) { raise(received); };
    sigaction(sig, &guard, nullptr);
}

void UloopSignalGuard::Restore(int sig, struct sigaction & action)
{
    // Restore unconditionally (belt and suspenders ...)
    sigaction(sig, &action, nullptr);
}

} // namespace ubus
} // namespace chip
