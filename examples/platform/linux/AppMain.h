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

#include <AppMainLoop.h>
#include <app/server/Server.h>
#include <controller/CommissionerDiscoveryController.h>
#include <crypto/RawKeySessionKeystore.h>
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
 * Get default server initialization parameters for Linux example apps.
 *
 * Returns a reference to a static CommonCaseDeviceServerInitParams instance that has been
 * initialized with default resources, including the CodegenDataModelProvider, which will
 * have been initialized (but not started).
 *
 * This is suitable for most Linux example applications. Apps that need to customize
 * initialization can create their own ServerInitParams and pass it to ChipLinuxAppMainLoop().
 */
chip::CommonCaseDeviceServerInitParams & ChipLinuxDefaultServerInitParams();

/**
 * Start up the Linux app, optionally with custom server initialization parameters
 * and/or main loop implementation.
 *
 * This overload allows apps to provide their own ServerInitParams for cases where
 * custom configuration is needed before server initialization.
 *
 * @param initParams Server initialization parameters to use
 * @param impl Optional main loop implementation
 */
void ChipLinuxAppMainLoop(chip::ServerInitParams & initParams = ChipLinuxDefaultServerInitParams(),
                          AppMainLoopImplementation * impl    = nullptr);

inline void ChipLinuxAppMainLoop(AppMainLoopImplementation * impl)
{
    ChipLinuxAppMainLoop(ChipLinuxDefaultServerInitParams(), impl);
}

// For extra init calls, the function will be called right before running Matter main loop.
void ApplicationInit();

// For extra shutdown calls, the function will be called before any of the core Matter objects are shut down.
void ApplicationShutdown();
