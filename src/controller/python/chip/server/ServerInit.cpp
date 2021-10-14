/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <errno.h>
#include <pthread.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <core/CHIPError.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

// #include <support/CHIPMem.h>
// #include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

// #include <examples/platform/linux/AppMain.h>

#include "Options.h"

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

namespace {

void EventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
    }
}

pthread_t sPlatformMainThread;

#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
//uint32_t sBluetoothAdapterId = 0;
#endif

void * PlatformMainLoop(void *)
{
    ChipLogProgress(DeviceLayer, "Platform main loop started.");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(DeviceLayer, "Platform main loop completed.");
    return nullptr;
}

} // namespace

extern "C" {

// CHIP_ERROR pychip_BLEMgrImpl_ConfigureBle(uint32_t bluetoothAdapterId)
// {
// #if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
//     // By default, Linux device is configured as a BLE peripheral while the controller needs a BLE central.
//     sBluetoothAdapterId = bluetoothAdapterId;
//     ReturnErrorOnFailure(
//         chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(/* BLE adapter ID */ bluetoothAdapterId, /* BLE central */ true));
// #endif
//     return CHIP_NO_ERROR;
// }

using PostAttributeChangeCallback = void (*)(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value);

class PythonServerDelegate// : public ServerDelegate
{
public:
    void SetPostAttributeChangeCallback(PostAttributeChangeCallback cb) {
        //ChipLogProgress(NotSpecified, "callback %p", cb);
        mPostAttributeChangeCallback = cb;
    };
    PostAttributeChangeCallback mPostAttributeChangeCallback = nullptr;
};

PythonServerDelegate gPythonServerDelegate;

void pychip_server_set_callbacks(PostAttributeChangeCallback cb)
{
    //ChipLogProgress(NotSpecified, "setting cb");
    gPythonServerDelegate.SetPostAttributeChangeCallback(cb);
}

void pychip_server_native_init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int result;
    int tmpErrno;

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
//    PrintOnboardingCodes(chip::RendezvousInformationFlag::kOnNetwork);

#if defined(PW_RPC_ENABLED)
    chip::rpc::Init();
    ChipLogProgress(NotSpecified, "PW_RPC initialized.");
#endif // defined(PW_RPC_ENABLED)

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("RpiMatterDali"); // Use default device name (CHIP-XXXX)

#if CONFIG_NETWORK_LAYER_BLE
    chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(LinuxDeviceOptions::GetInstance().mBleDevice, false);
#endif

//    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false);
    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    ChipLogProgress(NotSpecified, "CHIP_DEVICE_CONFIG_ENABLE_WPA");
    if (LinuxDeviceOptions::GetInstance().mWiFi)
    {
        ChipLogProgress(NotSpecified, "mWiFi");
        chip::DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

// #if CHIP_ENABLE_OPENTHREAD
//     if (LinuxDeviceOptions::GetInstance().mThread)
//     {
//         SuccessOrExit(err = chip::DeviceLayer::ThreadStackMgrImpl().InitThreadStack());
//         ChipLogProgress(NotSpecified, "Thread initialized.");
//     }
// #endif // CHIP_ENABLE_OPENTHREAD

    InitServer();

    result   = pthread_create(&sPlatformMainThread, nullptr, PlatformMainLoop, nullptr);
    tmpErrno = errno;

    if (result != 0)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: pthread_create failed: %s", strerror(tmpErrno));
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack");
    }
    return /*err*/;
}

}

void emberAfPostAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                        uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value)
{
    //ChipLogProgress(NotSpecified, "emberAfPostAttributeChangeCallback()");
    if (gPythonServerDelegate.mPostAttributeChangeCallback != nullptr ) {
        //ChipLogProgress(NotSpecified, "callback %p", gPythonServerDelegate.mPostAttributeChangeCallback);
        gPythonServerDelegate.mPostAttributeChangeCallback(endpoint, clusterId, attributeId, mask, manufacturerCode, type, size, value);
    } else {
        //ChipLogProgress(NotSpecified, "callback nullptr");
    }
};
