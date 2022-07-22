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

#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

// #include <support/CHIPMem.h>
// #include <support/ErrorStr.h>

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
// uint32_t sBluetoothAdapterId = 0;
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

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
/*
 * The device shall check every kWiFiStartCheckTimeUsec whether Wi-Fi management
 * has been fully initialized. If after kWiFiStartCheckAttempts Wi-Fi management
 * still hasn't been initialized, the device configuration is reset, and device
 * needs to be paired again.
 */
static constexpr useconds_t kWiFiStartCheckTimeUsec = 100 * 1000; // 100 ms
static constexpr uint8_t kWiFiStartCheckAttempts    = 5;
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
static bool EnsureWiFiIsStarted()
{
    for (int cnt = 0; cnt < kWiFiStartCheckAttempts; cnt++)
    {
        if (chip::DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted())
        {
            return true;
        }

        usleep(kWiFiStartCheckTimeUsec);
    }

    return chip::DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted();
}
#endif

using PostAttributeChangeCallback = void (*)(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId,
                                             uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value);

class PythonServerDelegate // : public ServerDelegate
{
public:
    void SetPostAttributeChangeCallback(PostAttributeChangeCallback cb)
    {
        // ChipLogProgress(NotSpecified, "callback %p", cb);
        mPostAttributeChangeCallback = cb;
    };
    PostAttributeChangeCallback mPostAttributeChangeCallback = nullptr;
};

PythonServerDelegate gPythonServerDelegate;

void pychip_server_set_callbacks(PostAttributeChangeCallback cb)
{
    // ChipLogProgress(NotSpecified, "setting cb");
    gPythonServerDelegate.SetPostAttributeChangeCallback(cb);
}

void pychip_server_native_init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int result;
    int tmpErrno;

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: memory init failed: %s", chip::ErrorStr(err));
    }

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: platform init failed: %s", chip::ErrorStr(err));
    }

    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));

#if defined(PW_RPC_ENABLED)
    chip::rpc::Init();
    ChipLogProgress(NotSpecified, "PW_RPC initialized.");
#endif // defined(PW_RPC_ENABLED)

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

#if CONFIG_NETWORK_LAYER_BLE
    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("RpiMatterDali"); // Use default device name (CHIP-XXXX)
    chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(LinuxDeviceOptions::GetInstance().mBleDevice, false);
    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    if (LinuxDeviceOptions::GetInstance().mWiFi)
    {
        chip::DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();
        if (!EnsureWiFiIsStarted())
        {
            ChipLogError(NotSpecified, "Wi-Fi Management taking too long to start - device configuration will be reset.");
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

    // parts from ChipLinuxAppMainLoop

    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.operationalServicePort        = CHIP_PORT;
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;

    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    // SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    result   = pthread_create(&sPlatformMainThread, nullptr, PlatformMainLoop, nullptr);
    tmpErrno = errno;

    if (result != 0)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: pthread_create failed: %s", strerror(tmpErrno));
    }

    return /*err*/;
}
}

void emberAfPostAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                        uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value)
{
    // ChipLogProgress(NotSpecified, "emberAfPostAttributeChangeCallback()");
    if (gPythonServerDelegate.mPostAttributeChangeCallback != nullptr)
    {
        // ChipLogProgress(NotSpecified, "callback %p", gPythonServerDelegate.mPostAttributeChangeCallback);
        gPythonServerDelegate.mPostAttributeChangeCallback(endpoint, clusterId, attributeId, manufacturerCode, type, size, value);
    }
    else
    {
        // ChipLogProgress(NotSpecified, "callback nullptr");
    }
};
