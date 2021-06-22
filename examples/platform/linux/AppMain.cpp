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

#include <iostream>
#include <thread>

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <core/CHIPError.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <support/CHIPMem.h>
#include <support/RandUtils.h>

#if defined(ENABLE_CHIP_SHELL)
#include <lib/shell/Engine.h>
#endif

#if defined(PW_RPC_ENABLED)
#include <CommonRpc.h>
#endif

#include "Options.h"

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

#if defined(ENABLE_CHIP_SHELL)
using chip::Shell::Engine;
#endif

namespace {
void EventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
    }
}
} // namespace

int ChipLinuxAppInit(int argc, char ** argv)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    err = ParseArguments(argc, argv);
    SuccessOrExit(err);

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    ConfigurationMgr().LogDeviceConfig();
#ifdef CONFIG_RENDEZVOUS_MODE
    PrintOnboardingCodes(static_cast<chip::RendezvousInformationFlags>(CONFIG_RENDEZVOUS_MODE));
#else
    PrintOnboardingCodes(chip::RendezvousInformationFlag::kBLE);
#endif

#if defined(PW_RPC_ENABLED)
    chip::rpc::Init();
    ChipLogProgress(NotSpecified, "PW_RPC initialized.");
#endif // defined(PW_RPC_ENABLED)

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(nullptr); // Use default device name (CHIP-XXXX)

#if CONFIG_NETWORK_LAYER_BLE
    chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(LinuxDeviceOptions::GetInstance().mBleDevice, false);
#endif

    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    if (LinuxDeviceOptions::GetInstance().mWiFi)
    {
        chip::DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

#if CHIP_ENABLE_OPENTHREAD
    if (LinuxDeviceOptions::GetInstance().mThread)
    {
        SuccessOrExit(err = chip::DeviceLayer::ThreadStackMgrImpl().InitThreadStack());
        ChipLogProgress(NotSpecified, "Thread initialized.");
    }
#endif // CHIP_ENABLE_OPENTHREAD

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "Failed to run Linux Lighting App: %s ", ErrorStr(err));
        // End the program with non zero error code to indicate a error.
        return 1;
    }
    return 0;
}

void ChipLinuxAppMainLoop()
{
#if defined(ENABLE_CHIP_SHELL)
    std::thread shellThread([]() { Engine::Root().RunMainLoop(); });
#endif

    // Init ZCL Data Model and CHIP App Server
    InitServer();

    chip::DeviceLayer::PlatformMgr().RunEventLoop();
#if defined(ENABLE_CHIP_SHELL)
    shellThread.join();
#endif
}
