/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/server/Server.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissionerDiscoveryController.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <transport/TransportMgr.h>

#include "Options.h"

// Applications can optionally provide the endpoint id of a secondary network
// commissioning endpoint, if one is supported.
int ChipLinuxAppInit(int argc, char * const argv[], chip::ArgParser::OptionSet * customOptions = nullptr,
                     const chip::Optional<chip::EndpointId> secondaryNetworkCommissioningEndpoit = chip::NullOptional);

/**
 * A main loop implementation describes how an application main loop is to be
 * run:
 *    - how to execute the main loop
 *    - what to do to stop it (inside a signal handler - CTRL+C is captured
 *      by the main loop function)
 */
class AppMainLoopImplementation
{
public:
    virtual ~AppMainLoopImplementation() = default;
    /**
     * Execute main loop. Generally should have at least some
     * `DeviceLayer::PlatformMgr().RunEventLoop();` or equivalent setup
     *
     * This is expected to RUN and BLOCK until SignalSafeStopMainLoop is
     * called or some internal close logic is run (e.g. a UI may
     * stop when the window close button is clicked.)
     */
    virtual void RunMainLoop() = 0;

    /**
     * Stop the above `RunMainLoop` function.
     *
     * Generally should contain at least a
     *
     *    Server::GetInstance().GenerateShutDownEvent()
     *
     * and then call StopEventLoopTask() in whatever way is appropriate for the
     * way the event loop was started.
     */
    virtual void SignalSafeStopMainLoop() = 0;
};

class DefaultAppMainLoopImplementation : public AppMainLoopImplementation
{
public:
    void RunMainLoop() override { chip::DeviceLayer::PlatformMgr().RunEventLoop(); }
    void SignalSafeStopMainLoop() override
    {
        chip::Server::GetInstance().GenerateShutDownEvent();
        chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t) { chip::DeviceLayer::PlatformMgr().StopEventLoopTask(); });
    }
};

/**
 * Start up the Linux app and use the provided main loop for event processing.
 *
 * If no main loop implementation is provided, an equivalent of
 * DefaultAppMainLoopImplementation will be used.
 */
void ChipLinuxAppMainLoop(AppMainLoopImplementation * impl = nullptr);

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

using chip::Controller::DeviceCommissioner;
using chip::Transport::PeerAddress;

CHIP_ERROR CommissionerPairOnNetwork(uint32_t pincode, uint16_t disc, PeerAddress address);
CHIP_ERROR CommissionerPairUDC(uint32_t pincode, size_t index);

DeviceCommissioner * GetDeviceCommissioner();
CommissionerDiscoveryController * GetCommissionerDiscoveryController();

#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

// For extra init calls, the function will be called right before running Matter main loop.
void ApplicationInit();

// For extra shutdown calls, the function will be called before any of the core Matter objects are shut down.
void ApplicationShutdown();
